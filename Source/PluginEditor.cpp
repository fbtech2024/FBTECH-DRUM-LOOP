#include "PluginEditor.h"
#include <limits>

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
    fillButton.onClick = [this] { processor.requestFill(); };
    fillButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffb45309));
    addAndMakeVisible(fillButton);

    for (int bank = 0; bank < 4; ++bank)
    {
        patternButtons[bank].setButtonText("PATTERN " + juce::String::charToString(static_cast<juce_wchar>('A' + bank)));
        patternButtons[bank].onClick = [this, bank]
        {
            processor.selectPattern(bank);
            refreshPatternButtons();
        };
        addAndMakeVisible(patternButtons[bank]);
    }
    songToggle.setToggleState(processor.isSongMode(), juce::dontSendNotification);
    songToggle.onClick = [this] { processor.setSongMode(songToggle.getToggleState()); };
    addAndMakeVisible(songToggle);
    stopAtEndToggle.setToggleState(processor.isStopAtEnd(), juce::dontSendNotification);
    stopAtEndToggle.onClick = [this] { processor.setStopAtEnd(stopAtEndToggle.getToggleState()); };
    addAndMakeVisible(stopAtEndToggle);
    autoFillToggle.setToggleState(processor.isAutoFill(), juce::dontSendNotification);
    autoFillToggle.onClick = [this] { processor.setAutoFill(autoFillToggle.getToggleState()); };
    addAndMakeVisible(autoFillToggle);
    songLengthLabel.setText("SECTIONS", juce::dontSendNotification);
    songLengthLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(songLengthLabel);
    songLengthSlider.setSliderStyle(juce::Slider::IncDecButtons);
    songLengthSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 35, 22);
    songLengthSlider.setRange(1, 8, 1);
    songLengthSlider.setValue(processor.getSongLength(), juce::dontSendNotification);
    songLengthSlider.onValueChange = [this] { processor.setSongLength(static_cast<int>(songLengthSlider.getValue())); };
    addAndMakeVisible(songLengthSlider);
    for (int i = 0; i < 8; ++i)
    {
        sectionLabels[i].setText("SLOT " + juce::String(i + 1), juce::dontSendNotification);
        sectionLabels[i].setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(sectionLabels[i]);
        sectionPatterns[i].addItemList(juce::StringArray { "PATTERN A", "PATTERN B", "PATTERN C", "PATTERN D" }, 1);
        sectionPatterns[i].setSelectedId(processor.getSongPattern(i) + 1, juce::dontSendNotification);
        sectionPatterns[i].onChange = [this, i] { processor.setSongPattern(i, sectionPatterns[i].getSelectedId() - 1); };
        addAndMakeVisible(sectionPatterns[i]);
        sectionBars[i].setSliderStyle(juce::Slider::IncDecButtons);
        sectionBars[i].setTextBoxStyle(juce::Slider::TextBoxLeft, false, 35, 22);
        sectionBars[i].setRange(1, 32, 1);
        sectionBars[i].setValue(processor.getSongBars(i), juce::dontSendNotification);
        sectionBars[i].onValueChange = [this, i] { processor.setSongBars(i, static_cast<int>(sectionBars[i].getValue())); };
        addAndMakeVisible(sectionBars[i]);
    }
    for (int i = 0; i < 8; ++i)
        moveSlotSelector.addItem("SLOT " + juce::String(i + 1), i + 1);
    moveSlotSelector.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(moveSlotSelector);
    auto moveSlot = [this](int direction)
    {
        // Only reorder while stopped, so the audio thread never sees a half-updated song.
        if (processor.isManualPlaying() || processor.isTransportRunning())
        {
            songStatus.setText("STOP PLAYBACK BEFORE MOVING", juce::dontSendNotification);
            return;
        }
        const int source = moveSlotSelector.getSelectedId() - 1;
        const int destination = source + direction;
        if (source < 0 || destination < 0 || destination >= processor.getSongLength()) return;
        const int sourcePattern = processor.getSongPattern(source);
        const int sourceBars = processor.getSongBars(source);
        processor.setSongPattern(source, processor.getSongPattern(destination));
        processor.setSongBars(source, processor.getSongBars(destination));
        processor.setSongPattern(destination, sourcePattern);
        processor.setSongBars(destination, sourceBars);
        moveSlotSelector.setSelectedId(destination + 1, juce::dontSendNotification);
        refreshSongControls();
        songStatus.setText("SECTIONS REORDERED - SAVE SONG TO KEEP", juce::dontSendNotification);
    };
    moveEarlierButton.onClick = [moveSlot] { moveSlot(-1); };
    moveLaterButton.onClick = [moveSlot] { moveSlot(1); };
    addAndMakeVisible(moveEarlierButton);
    addAndMakeVisible(moveLaterButton);
    insertSlotButton.onClick = [this]
    {
        if (processor.isManualPlaying() || processor.isTransportRunning())
        {
            songStatus.setText("STOP PLAYBACK BEFORE EDITING", juce::dontSendNotification);
            return;
        }
        const int count = processor.getSongLength();
        const int selected = moveSlotSelector.getSelectedId() - 1;
        if (count >= 8 || selected < 0 || selected >= count) return;
        const int insertion = selected + 1;
        for (int i = count; i > insertion; --i)
        {
            processor.setSongPattern(i, processor.getSongPattern(i - 1));
            processor.setSongBars(i, processor.getSongBars(i - 1));
        }
        processor.setSongPattern(insertion, processor.getSongPattern(selected));
        processor.setSongBars(insertion, processor.getSongBars(selected));
        processor.setSongLength(count + 1);
        moveSlotSelector.setSelectedId(insertion + 1, juce::dontSendNotification);
        refreshSongControls();
        songStatus.setText("SLOT ADDED - SAVE SONG TO KEEP", juce::dontSendNotification);
    };
    removeSlotButton.onClick = [this]
    {
        if (processor.isManualPlaying() || processor.isTransportRunning())
        {
            songStatus.setText("STOP PLAYBACK BEFORE EDITING", juce::dontSendNotification);
            return;
        }
        const int count = processor.getSongLength();
        const int selected = moveSlotSelector.getSelectedId() - 1;
        if (count <= 1 || selected < 0 || selected >= count) return;
        for (int i = selected; i < count - 1; ++i)
        {
            processor.setSongPattern(i, processor.getSongPattern(i + 1));
            processor.setSongBars(i, processor.getSongBars(i + 1));
        }
        processor.setSongLength(count - 1);
        moveSlotSelector.setSelectedId(juce::jmin(selected, count - 2) + 1, juce::dontSendNotification);
        refreshSongControls();
        songStatus.setText("SLOT REMOVED - SAVE SONG TO KEEP", juce::dontSendNotification);
    };
    addAndMakeVisible(insertSlotButton);
    addAndMakeVisible(removeSlotButton);

    saveSongButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser>("Save FBTECH song preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("FBTECH Song.fbdl"), "*.fbdl");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file == juce::File{}) return;
                if (!file.hasFileExtension(".fbdl")) file = file.withFileExtension(".fbdl");
                juce::MemoryBlock data;
                processor.getStateInformation(data);
                songStatus.setText(file.replaceWithData(data.getData(), data.getSize()) ? "SONG PRESET SAVED" : "SAVE FAILED", juce::dontSendNotification);
            });
    };
    loadSongButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser>("Load FBTECH song preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.fbdl");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                const auto file = fc.getResult();
                if (file == juce::File{}) return;
                juce::MemoryBlock data;
                if (!file.loadFileAsData(data) || data.getSize() == 0)
                {
                    songStatus.setText("LOAD FAILED", juce::dontSendNotification);
                    return;
                }
                if (data.getSize() > static_cast<size_t>(std::numeric_limits<int>::max()))
                {
                    songStatus.setText("PRESET TOO LARGE", juce::dontSendNotification);
                    return;
                }
                auto presetXml = processor.getXmlFromBinary(data.getData(), static_cast<int>(data.getSize()));
                if (!presetXml || !presetXml->hasTagName("FBTECH_DRUM_LOOP_STATE"))
                {
                    songStatus.setText("INVALID SONG PRESET", juce::dontSendNotification);
                    return;
                }
                processor.setStateInformation(data.getData(), static_cast<int>(data.getSize()));
                refreshSongControls();
                songStatus.setText("SONG PRESET LOADED", juce::dontSendNotification);
            });
    };
    addAndMakeVisible(saveSongButton);
    addAndMakeVisible(loadSongButton);
    songStatus.setColour(juce::Label::textColourId, juce::Colour(0xff5eead4));
    addAndMakeVisible(songStatus);
    refreshSongControls();
    startTimerHz(12);

    // resized() accesses all 64 step buttons; create them before sizing the editor.
    setSize(980, 740);
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
    for (int bank = 0; bank < 4; ++bank)
        patternButtons[bank].setBounds(284 + bank * 120, 353, 112, 30);
    songToggle.setBounds(30, 405, 130, 30);
    songLengthLabel.setBounds(175, 405, 85, 26);
    songLengthSlider.setBounds(260, 405, 95, 26);
    for (int i = 0; i < 8; ++i)
    {
        const int x = 30 + (i % 4) * 235;
        const int y = 478 + (i / 4) * 55;
        sectionLabels[i].setBounds(x, y, 65, 26);
        sectionPatterns[i].setBounds(x + 65, y, 105, 26);
        sectionBars[i].setBounds(x + 170, y, 65, 26);
    }
    stopAtEndToggle.setBounds(30, 440, 150, 28);
    autoFillToggle.setBounds(190, 440, 130, 28);
    songStatus.setBounds(330, 440, 610, 28);
    moveSlotSelector.setBounds(30, 610, 125, 32);
    moveEarlierButton.setBounds(165, 610, 135, 32);
    moveLaterButton.setBounds(310, 610, 135, 32);
    insertSlotButton.setBounds(30, 565, 135, 32);
    removeSlotButton.setBounds(175, 565, 150, 32);
    saveSongButton.setBounds(560, 610, 175, 32);
    loadSongButton.setBounds(755, 610, 175, 32);
    kitButton.setBounds(26, 665, 170, 32);
    playButton.setBounds(344, 665, 105, 32);
    stopButton.setBounds(459, 665, 105, 32);
    fillButton.setBounds(580, 665, 175, 32);
    clearButton.setBounds(808, 665, 140, 32);
}

void FBTECHDRUMLOOPAudioProcessorEditor::refreshSongControls()
{
    songToggle.setToggleState(processor.isSongMode(), juce::dontSendNotification);
    stopAtEndToggle.setToggleState(processor.isStopAtEnd(), juce::dontSendNotification);
    autoFillToggle.setToggleState(processor.isAutoFill(), juce::dontSendNotification);
    songLengthSlider.setValue(processor.getSongLength(), juce::dontSendNotification);
    for (int i = 0; i < 8; ++i)
    {
        sectionPatterns[i].setSelectedId(processor.getSongPattern(i) + 1, juce::dontSendNotification);
        sectionBars[i].setValue(processor.getSongBars(i), juce::dontSendNotification);
    }
    for (int i = 0; i < 4; ++i)
    {
        volumeSliders[i].setValue(processor.getInstrumentVolume(i), juce::dontSendNotification);
        loadButtons[i].setButtonText(processor.getSampleName(i).isNotEmpty() ? "WAV OK" : "LOAD WAV");
    }
    refreshPatternButtons();
}

void FBTECHDRUMLOOPAudioProcessorEditor::refreshPatternButtons()
{
    const int selected = processor.getSelectedPattern();
    const int playing = processor.getPlayingPattern();
    const int queued = processor.getQueuedPattern();
    for (int bank = 0; bank < 4; ++bank)
    {
        const auto letter = juce::String::charToString(static_cast<juce_wchar>('A' + bank));
        const auto label = queued == bank ? "NEXT " : playing == bank ? "PLAYING " : "PATTERN ";
        patternButtons[bank].setButtonText(juce::String(label) + letter);
        patternButtons[bank].setColour(juce::TextButton::buttonColourId,
            queued == bank ? juce::Colour(0xffb45309) :
            playing == bank ? juce::Colour(0xff0d9488) :
            bank == selected ? juce::Colour(0xff2563eb) : juce::Colour(0xff334155));
    }
    for (int inst = 0; inst < 4; ++inst)
        for (int step = 0; step < 16; ++step)
            stepButtons[inst * 16 + step]->setToggleState(processor.getStep(inst, step), juce::dontSendNotification);
}

void FBTECHDRUMLOOPAudioProcessorEditor::timerCallback()
{
    songStatus.setText(processor.isSongFinished() ? juce::String("SONG FINISHED - press PLAY to restart") : processor.isSongMode() ? juce::String("SONG ACTIVE - SECTION ") + juce::String(processor.getSongSection() + 1) + " / " + juce::String(processor.getSongLength()) : juce::String("SONG OFF - manual pattern selection"), juce::dontSendNotification);
    const int playing = processor.getPlayingPattern();
    const int queued = processor.getQueuedPattern();
    for (int bank = 0; bank < 4; ++bank)
    {
        const auto letter = juce::String::charToString(static_cast<juce_wchar>('A' + bank));
        const auto label = queued == bank ? "NEXT " : playing == bank ? "PLAYING " : "PATTERN ";
        patternButtons[bank].setButtonText(juce::String(label) + letter);
        patternButtons[bank].setColour(juce::TextButton::buttonColourId,
            queued == bank ? juce::Colour(0xffb45309) :
            playing == bank ? juce::Colour(0xff0d9488) : juce::Colour(0xff334155));
    }
}
