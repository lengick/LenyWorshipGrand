#pragma once
#include <JuceHeader.h>

class LenyWorshipGrandAudioProcessor final : public juce::AudioProcessor
{
public:
    LenyWorshipGrandAudioProcessor();
    ~LenyWorshipGrandAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "LenyWorship Grand"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    void loadPianoSamples(const juce::File& folder);
    juce::String getSampleStatus() const;

private:
    struct PadVoice
    {
        bool active = false;
        int note = 60;
        float velocity = 0.0f;
        double phaseA = 0.0, phaseB = 0.0;
        float envelope = 0.0f;
        bool releasing = false;
        bool keyDown = false;
    };
    bool padSustainDown = false;

    juce::Synthesiser piano;
    juce::AudioFormatManager formatManager;
    juce::CriticalSection sampleLock;
    std::array<PadVoice, 128> padVoices;
    double currentSampleRate = 44100.0;
    float padAttack = 2.2f;
    float padRelease = 4.5f;
    float padLevel = 0.0f;
    float reverbWet = 0.22f;
    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay { 96000 };
    juce::AudioBuffer<float> pianoBuffer;
    juce::AudioBuffer<float> padBuffer;
    juce::String sampleStatus { "NO PIANO SAMPLES LOADED" };

    void renderPad(juce::AudioBuffer<float>& output, const juce::MidiBuffer& midi);
    void handlePadMidi(const juce::MidiBuffer& midi);
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LenyWorshipGrandAudioProcessor)
};
