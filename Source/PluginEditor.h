#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class FBTECHDRUMLOOPAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit FBTECHDRUMLOOPAudioProcessorEditor(FBTECHDRUMLOOPAudioProcessor&);
    ~FBTECHDRUMLOOPAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    FBTECHDRUMLOOPAudioProcessor& processor;
    juce::Label title, tempoLabel, hint;
    juce::Label instrumentLabels[4];
    juce::TextButton clearButton { "CLEAR PATTERN" };
    juce::TextButton loadButtons[4];
    std::unique_ptr<juce::FileChooser> chooser;
    juce::OwnedArray<juce::TextButton> stepButtons;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FBTECHDRUMLOOPAudioProcessorEditor)
};
