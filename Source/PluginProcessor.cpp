#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float pi = juce::MathConstants<float>::pi;
    float getParam(juce::AudioProcessorValueTreeState& state, const char* id)
    {
        return state.getRawParameterValue(id)->load();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout LenyWorshipGrandAudioProcessor::createParameterLayout()
{
    using P = juce::AudioProcessorValueTreeState;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto add = [&p](const juce::String& id, const juce::String& name, float lo, float hi, float def)
    { p.push_back(std::make_unique<juce::AudioParameterFloat>(id, name, juce::NormalisableRange<float>(lo, hi), def)); };
    add("volume", "Volume", 0.0f, 1.0f, 0.82f);
    add("pad", "Pad", 0.0f, 1.0f, 0.12f);
    add("delay", "Delay", 0.0f, 1.0f, 0.08f);
    add("cutoff", "Cutoff", 300.0f, 12000.0f, 6500.0f);
    add("reverb", "Reverb", 0.0f, 1.0f, 0.24f);
    add("padSustain", "Pad Sustain", 0.5f, 12.0f, 5.5f);
    add("attack", "Pad Attack", 0.05f, 8.0f, 2.2f);
    p.push_back(std::make_unique<juce::AudioParameterBool>("filterOn", "Filter On", true));
    return { p.begin(), p.end() };
}

LenyWorshipGrandAudioProcessor::LenyWorshipGrandAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "LENY_STATE", createParameterLayout())
{
    formatManager.registerBasicFormats();
    piano.clearVoices();
    for (int i = 0; i < 48; ++i) piano.addVoice(new juce::SamplerVoice());
}

void LenyWorshipGrandAudioProcessor::prepareToPlay(double sr, int samplesPerBlock)
{
    currentSampleRate = sr;
    piano.setCurrentPlaybackSampleRate(sr);
    pianoBuffer.setSize(2, samplesPerBlock);
    padBuffer.setSize(2, samplesPerBlock);
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.72f; rp.damping = 0.48f; rp.wetLevel = reverbWet; rp.dryLevel = 1.0f; rp.width = 1.0f;
    reverb.setParameters(rp);
    delay.prepare({ sr, (juce::uint32)samplesPerBlock, 2 });
    delay.setDelay(0.32f * (float)sr);
    delay.reset();
}

void LenyWorshipGrandAudioProcessor::releaseResources() {}

void LenyWorshipGrandAudioProcessor::handlePadMidi(const juce::MidiBuffer& midi)
{
    for (const auto metadata : midi)
    {
        const auto m = metadata.getMessage();
        if (m.isNoteOn())
        {
            for (auto& v : padVoices) if (!v.active) { v.active = true; v.note = m.getNoteNumber(); v.velocity = m.getFloatVelocity(); v.phaseA = v.phaseB = 0.0; v.envelope = 0.0f; v.releasing = false; v.keyDown = true; break; }
        }
        else if (m.isNoteOff())
        {
            for (auto& v : padVoices) if (v.active && v.note == m.getNoteNumber()) { v.keyDown = false; if (!padSustainDown) v.releasing = true; }
        }
        else if (m.isAllNotesOff() || m.isAllSoundOff())
        {
            for (auto& v : padVoices) if (m.isAllSoundOff()) v.active = false; else v.releasing = true;
        }
        else if (m.isController() && m.getControllerNumber() == 64)
        {
            const bool down = m.getControllerValue() >= 64;
            if (padSustainDown && !down) for (auto& v : padVoices) if (v.active && !v.keyDown) v.releasing = true;
            padSustainDown = down;
        }
    }
}

void LenyWorshipGrandAudioProcessor::renderPad(juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi)
{
    handlePadMidi(midi);
    out.clear();
    const float padMix = getParam(parameters, "pad");
    const float cutoff = getParam(parameters, "cutoff");
    const float attack = getParam(parameters, "attack");
    const float release = getParam(parameters, "padSustain");
    const bool filterEnabled = parameters.getRawParameterValue("filterOn")->load() > 0.5f;
    const float alpha = filterEnabled ? juce::jlimit(0.002f, 0.9f, 2.0f * pi * cutoff / (float)currentSampleRate) : 1.0f;
    static float lpL = 0.0f, lpR = 0.0f;
    for (int s = 0; s < out.getNumSamples(); ++s)
    {
        float l = 0.0f, r = 0.0f;
        for (auto& v : padVoices)
        {
            if (!v.active) continue;
            const double freq = juce::MidiMessage::getMidiNoteInHertz(v.note);
            const double incA = freq / currentSampleRate;
            const double incB = (freq * 1.003) / currentSampleRate;
            v.phaseA += incA; v.phaseB += incB;
            if (v.phaseA >= 1.0) v.phaseA -= 1.0;
            if (v.phaseB >= 1.0) v.phaseB -= 1.0;
            if (!v.releasing) v.envelope = juce::jmin(1.0f, v.envelope + 1.0f / (attack * (float)currentSampleRate));
            else { v.envelope -= 1.0f / (release * (float)currentSampleRate); if (v.envelope <= 0.0001f) { v.active = false; continue; } }
            const float a = std::sin((float)v.phaseA * 2.0f * pi);
            const float b = std::sin((float)v.phaseB * 2.0f * pi);
            const float sample = (a + b) * 0.25f * v.envelope * v.velocity * padMix * 0.22f;
            l += sample; r += sample * 0.985f;
        }
        lpL += alpha * (l - lpL); lpR += alpha * (r - lpR);
        out.setSample(0, s, lpL); out.setSample(1, s, lpR);
    }
}

void LenyWorshipGrandAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (pianoBuffer.getNumSamples() < n) pianoBuffer.setSize(2, n, false, false, true);
    if (padBuffer.getNumSamples() < n) padBuffer.setSize(2, n, false, false, true);
    pianoBuffer.clear(); padBuffer.clear();
    {
        const juce::ScopedLock lock(sampleLock);
        piano.renderNextBlock(pianoBuffer, midi, 0, n);
    }
    renderPad(padBuffer, midi);
    const float master = getParam(parameters, "volume");
    const float wet = getParam(parameters, "reverb");
    const float delayAmount = getParam(parameters, "delay");
    juce::dsp::Reverb::Parameters rp = reverb.getParameters(); rp.wetLevel = wet * 0.48f; reverb.setParameters(rp);
    for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
    {
        auto* dst = buffer.getWritePointer(ch);
        auto* p = pianoBuffer.getReadPointer(ch);
        auto* d = padBuffer.getReadPointer(ch);
        for (int i = 0; i < n; ++i) dst[i] = (p[i] + d[i]) * master;
    }
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
    if (delayAmount > 0.001f)
    {
        for (int i = 0; i < n; ++i)
            for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
            {
                const float in = buffer.getSample(ch, i);
                const float delayed = delay.popSample(ch);
                delay.pushSample(ch, in + delayed * 0.24f);
                buffer.setSample(ch, i, in + delayed * delayAmount * 0.35f);
            }
    }
}

void LenyWorshipGrandAudioProcessor::loadPianoSamples(const juce::File& folder)
{
    const juce::ScopedLock lock(sampleLock);
    piano.clearSounds();
    int loaded = 0;
    for (const auto& file : folder.findChildFiles(juce::File::findFiles, false, "*.wav"))
    {
        auto name = file.getFileNameWithoutExtension().toUpperCase();
        // Naming: C2.wav, C#2.wav, D2.wav ... one root-note sample per file.
        int note = -1;
        static const juce::StringArray names { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        for (int octave = 0; octave <= 8 && note < 0; ++octave)
            for (int key = 0; key < 12; ++key)
                if (name == names[key] + juce::String(octave)) note = (octave + 1) * 12 + key;
        if (note < 0) continue;
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader == nullptr) continue;
        juce::BigInteger range; range.setRange(note, note, true);
        piano.addSound(new juce::SamplerSound(file.getFileName(), *reader, range, note, 0.015, 2.5, 30.0));
        ++loaded;
    }
    sampleStatus = loaded > 0 ? juce::String(loaded) + " PIANO SAMPLES LOADED" : "NO PIANO SAMPLES LOADED";
}

juce::String LenyWorshipGrandAudioProcessor::getSampleStatus() const { return sampleStatus; }

void LenyWorshipGrandAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{ auto state = parameters.copyState(); std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml, dest); }
void LenyWorshipGrandAudioProcessor::setStateInformation(const void* data, int size)
{ std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size)); if (xml && xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml)); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new LenyWorshipGrandAudioProcessor(); }
