#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class FBTECHDRUMLOOPAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
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
    juce::ComboBox stepPageSelector;
    int stepPage = 0;
    juce::TextButton drumPads[16];
    juce::TextButton midiLearnButtons[4];
    juce::TextButton clearButton { "CLEAR PATTERN" };
    juce::TextButton kitButton { "LOAD KIT FOLDER" };
    juce::TextButton playButton { "PLAY" }, stopButton { "STOP" };
    juce::ToggleButton recordToggle { "REGISTRA PAD" };
    juce::TextButton fillButton { "FILL NEXT BAR" };
    juce::TextButton saveSongButton { "SAVE SONG" }, loadSongButton { "LOAD SONG" };
    void refreshSongControls();
    juce::ToggleButton songToggle { "SONG MODE" };
    juce::ToggleButton stopAtEndToggle { "STOP AT END" };
    juce::ToggleButton autoFillToggle { "AUTO FILL" };
    juce::Slider sectionBars[8];
    juce::ComboBox sectionPatterns[8];
    juce::Label sectionLabels[8];
    juce::Slider songLengthSlider;
    juce::ComboBox moveSlotSelector;
    juce::TextButton moveEarlierButton { "MOVE LEFT" }, moveLaterButton { "MOVE RIGHT" };
    juce::TextButton insertSlotButton { "ADD SLOT" }, removeSlotButton { "REMOVE SLOT" };
    juce::Label songLengthLabel;
    juce::Label songStatus;
    juce::TextButton loadButtons[4];
    juce::TextButton patternButtons[4];
    void refreshPatternButtons();
    void timerCallback() override;
    juce::Slider volumeSliders[4];
    std::unique_ptr<juce::FileChooser> chooser;
    juce::OwnedArray<juce::TextButton> stepButtons;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FBTECHDRUMLOOPAudioProcessorEditor)
};
