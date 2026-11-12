#include "PluginEditor.h"

LenyWorshipGrandAudioProcessorEditor::LenyWorshipGrandAudioProcessorEditor(LenyWorshipGrandAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    background = juce::ImageCache::getFromMemory(BinaryData::LenyWorshipinterface, BinaryData::LenyWorshipinterfaceSize);
    const std::array<juce::String, 6> ids { "volume", "pad", "delay", "cutoff", "reverb", "padSustain" };
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& s = knobs[i];
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff35e5f4));
        s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff16333c));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffc5fbff));
        addAndMakeVisible(s);
        attachments[i] = std::make_unique<SliderAttachment>(processor.parameters, ids[i], s);
    }
    filterButton.setClickingTogglesState(true); filterButton.setToggleState(true, juce::dontSendNotification);
    filterButton.onClick = [this] { processor.parameters.getParameter("filterOn")->setValueNotifyingHost(filterButton.getToggleState() ? 1.0f : 0.0f); };
    addAndMakeVisible(filterButton);
    bypassButton.onClick = [this] { processor.parameters.getParameter("pad")->setValueNotifyingHost(0.0f); };
    addAndMakeVisible(bypassButton);
    loadButton.onClick = [this]
    {
        chooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
            [this](const juce::FileChooser& fc) { auto f = fc.getResult(); if (f.isDirectory()) { processor.loadPianoSamples(f); status.setText(processor.getSampleStatus(), juce::dontSendNotification); } });
    };
    addAndMakeVisible(loadButton);
    status.setText(processor.getSampleStatus(), juce::dontSendNotification);
    status.setColour(juce::Label::textColourId, juce::Colour(0xff85efff));
    status.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(status);
    setSize(1200, 760);
}

void LenyWorshipGrandAudioProcessorEditor::paint(juce::Graphics& g)
{
    if (background.isValid()) g.drawImage(background, getLocalBounds().toFloat());
    else { g.fillAll(juce::Colour(0xff050b10)); g.setColour(juce::Colour(0xff35e5f4)); g.drawFittedText("LENYWORSHIP GRAND", getLocalBounds(), juce::Justification::centred, 1); }
}

void LenyWorshipGrandAudioProcessorEditor::resized()
{
    const float sx = getWidth() / 1536.0f, sy = getHeight() / 1024.0f;
    auto place = [this, sx, sy](juce::Component& c, float x, float y, float w, float h)
    { c.setBounds((int)(x*sx), (int)(y*sy), (int)(w*sx), (int)(h*sy)); };
    // Coordinates align the six rotary controls with the supplied concept artwork.
    place(knobs[0], 250, 245, 170, 170); place(knobs[1], 535, 245, 170, 170); place(knobs[2], 825, 245, 170, 170);
    place(knobs[3], 345, 465, 170, 170); place(knobs[4], 690, 465, 170, 170); place(knobs[5], 1035, 465, 170, 170);
    place(filterButton, 1090, 825, 120, 42); place(bypassButton, 1225, 825, 105, 42);
    place(loadButton, 170, 915, 190, 36); place(status, 370, 915, 390, 36);
}
