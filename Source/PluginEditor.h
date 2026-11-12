#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class LenyWorshipGrandAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit LenyWorshipGrandAudioProcessorEditor(LenyWorshipGrandAudioProcessor&);
    ~LenyWorshipGrandAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    LenyWorshipGrandAudioProcessor& processor;
    juce::Image background;
    std::array<juce::Slider, 6> knobs;
    std::array<std::unique_ptr<SliderAttachment>, 6> attachments;
    juce::TextButton filterButton { "FILTER ON" };
    juce::TextButton bypassButton { "BYPASS" };
    juce::TextButton loadButton { "LOAD PIANO SAMPLES" };
    juce::Label status;
    juce::FileChooser chooser { "Select your Grand Piano WAV sample folder", juce::File(), "" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LenyWorshipGrandAudioProcessorEditor)
};
