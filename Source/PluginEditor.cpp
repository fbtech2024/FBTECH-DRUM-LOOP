#include "PluginEditor.h"

FBTECHDRUMLOOPAudioProcessorEditor::FBTECHDRUMLOOPAudioProcessorEditor(FBTECHDRUMLOOPAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    title.setText("FBTECH DRUM LOOP", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centredLeft);
    title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xfff1f5f9));
    addAndMakeVisible(title);

    tempoLabel.setJustificationType(juce::Justification::centredRight);
    tempoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff5eead4));
    addAndMakeVisible(tempoLabel);

    hint.setText("16-step sequencer  |  Kick / Snare / Hi-Hat / Tom  |  DAW tempo sync", juce::dontSendNotification);
    hint.setColour(juce::Label::textColourId, juce::Colour(0xffa8b5c5));
    addAndMakeVisible(hint);

    const juce::StringArray names { "KICK", "SNARE", "HI-HAT", "TOM" };
    for (int inst = 0; inst < 4; ++inst)
    {
        instrumentLabels[inst].setText(names[inst], juce::dontSendNotification);
        instrumentLabels[inst].setColour(juce::Label::textColourId, juce::Colour(0xffcbd5e1));
        instrumentLabels[inst].setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        instrumentLabels[inst].setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(instrumentLabels[inst]);
        loadButtons[inst].onClick = [this, inst]
        {
            chooser = std::make_unique<juce::FileChooser>("Select drum WAV", juce::File{}, "*.wav");
            chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this, inst](const juce::FileChooser& fc)
                {
                    if (processor.loadWav(inst, fc.getResult()))
                        loadButtons[inst].setButtonText("WAV OK");
                });
        };
        addAndMakeVisible(loadButtons[inst]);
        volumeSliders[inst].setSliderStyle(juce::Slider::LinearHorizontal);
        volumeSliders[inst].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        volumeSliders[inst].setRange(0.0, 1.5, 0.01);
        volumeSliders[inst].setValue(processor.getInstrumentVolume(inst), juce::dontSendNotification);
        volumeSliders[inst].setColour(juce::Slider::thumbColourId, juce::Colour(0xff2dd4bf));
        volumeSliders[inst].onValueChange = [this, inst] { processor.setInstrumentVolume(inst, static_cast<float>(volumeSliders[inst].getValue())); };
        addAndMakeVisible(volumeSliders[inst]);

        for (int step = 0; step < 16; ++step)
        {
            auto* b = stepButtons.add(new juce::TextButton());
            b->setButtonText(juce::String(step + 1));
            b->setClickingTogglesState(true);
            b->setToggleState(processor.getStep(inst, step), juce::dontSendNotification);
            b->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff202d3b));
            b->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff0d9488));
            b->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffdbe5ef));
            b->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            b->onClick = [this, inst, step, b] { processor.setStep(inst, step, b->getToggleState()); };
            addAndMakeVisible(b);
        }
    }

    clearButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff334155));
    clearButton.onClick = [this]
    {
        processor.clearPattern();
        for (int i = 0; i < stepButtons.size(); ++i)
            stepButtons[i]->setToggleState(false, juce::dontSendNotification);
    };
    addAndMakeVisible(clearButton);

    kitButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser>("Select a drum kit folder containing kick.wav, snare.wav, hihat.wav and tom.wav");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
            [this](const juce::FileChooser& fc)
            {
                if (processor.loadKitFolder(fc.getResult()))
                {
                    kitButton.setButtonText("KIT LOADED");
                    for (auto& button : loadButtons) button.setButtonText("WAV OK");
                }
                else if (fc.getResult() != juce::File{})
                    kitButton.setButtonText("INVALID KIT");
            });
    };
    addAndMakeVisible(kitButton);

    playButton.onClick = [this] { processor.setManualPlaying(true); };
    stopButton.onClick = [this] { processor.setManualPlaying(false); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0d9488));
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff475569));
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);

    // resized() accesses all 64 step buttons; create them before sizing the editor.
    setSize(980, 430);
}

void FBTECHDRUMLOOPAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff101820));
    g.setColour(juce::Colour(0xff1c2a38));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(8.0f), 12.0f);
    g.setColour(juce::Colour(0xff0d9488));
    g.fillRoundedRectangle(18.0f, 74.0f, 5.0f, 210.0f, 2.0f);
    g.setColour(juce::Colour(0xff344556));
    for (int i = 0; i < 4; ++i)
        g.drawLine(32.0f, 137.0f + i * 48.0f, 948.0f, 137.0f + i * 48.0f, 1.0f);
}

void FBTECHDRUMLOOPAudioProcessorEditor::resized()
{
    title.setBounds(24, 16, 430, 42);
    tempoLabel.setBounds(700, 20, 250, 30);
    tempoLabel.setText("HOST TEMPO: " + juce::String(processor.getTempo(), 1) + " BPM", juce::dontSendNotification);
    hint.setBounds(26, 54, 900, 24);

    const int left = 284, top = 108, cellW = 40, gap = 3, rowH = 58;
    for (int inst = 0; inst < 4; ++inst)
    {
        instrumentLabels[inst].setBounds(26, top + inst * rowH, 82, 22);
        loadButtons[inst].setBounds(26, top + inst * rowH + 24, 82, 25);
        volumeSliders[inst].setBounds(122, top + inst * rowH, 150, 51);
        for (int step = 0; step < 16; ++step)
            stepButtons[inst * 16 + step]->setBounds(left + step * (cellW + gap), top + inst * rowH, cellW, 34);
    }
    kitButton.setBounds(26, 365, 170, 32);
    playButton.setBounds(250, 365, 105, 32);
    stopButton.setBounds(365, 365, 105, 32);
    clearButton.setBounds(808, 365, 140, 32);
}
