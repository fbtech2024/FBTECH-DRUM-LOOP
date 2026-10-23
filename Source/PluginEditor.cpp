#include "PluginEditor.h"
#include <limits>

FBTECHDRUMLOOPAudioProcessorEditor::FBTECHDRUMLOOPAudioProcessorEditor(FBTECHDRUMLOOPAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    const juce::StringArray views { "SUONA", "EDITOR RITMI", "ARRANGIAMENTO" };
    for (int i = 0; i < 3; ++i)
    {
        viewButtons[i].setButtonText(views[i]);
        viewButtons[i].onClick = [this, i] { activeView = i; updateView(); };
        addAndMakeVisible(viewButtons[i]);
    }
    title.setText("FBTECH DRUM LOOP", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centredLeft);
    title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xfff1f5f9));
    addAndMakeVisible(title);

    tempoLabel.setJustificationType(juce::Justification::centredRight);
    tempoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff5eead4));
    addAndMakeVisible(tempoLabel);

    hint.setText("16 pad WAV SHIFT+clic | MIDI LEARN clic=nota, ALT+clic=CC volume (pag. 1), SHIFT=cancella", juce::dontSendNotification);
    hint.setColour(juce::Label::textColourId, juce::Colour(0xffa8b5c5));
    addAndMakeVisible(hint);

    stepPageSelector.addItem("STEPS 1-4", 1);
    stepPageSelector.addItem("STEPS 5-8", 2);
    stepPageSelector.addItem("STEPS 9-12", 3);
    stepPageSelector.addItem("STEPS 13-16", 4);
    stepPageSelector.setSelectedId(1, juce::dontSendNotification);
    stepPageSelector.onChange = [this]
    {
        processor.armMidiLearn(-1); // Changing pages must not assign a note to a hidden pad.
        processor.armVolumeCcLearn(-1);
        stepPage = juce::jlimit(0, 3, stepPageSelector.getSelectedId() - 1);
    };
    addAndMakeVisible(stepPageSelector);

    const juce::StringArray names { "KICK", "SNARE", "HI-HAT", "TOM" };
    const juce::StringArray padNames { "CASSA", "RULLANTE", "CHARLESTON", "TOM" };
    // Sixteen physical-style pads, four per instrument. The numbered pads
    // deliberately share voices until additional kit slots are implemented.
    for (int i = 0; i < 16; ++i)
    {
        const int instrument = i % 4;
        const auto loadedName = processor.getSampleName(i);
        drumPads[i].setButtonText(loadedName != "Empty pad" && loadedName != "Synth (default)"
            ? loadedName.substring(0, 18)
            : padNames[instrument] + "  " + juce::String(40 + 20 * (i / 4)) + "%");
        drumPads[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff164e63));
        drumPads[i].setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff0891b2));
        // Keep the four velocity rows as an expressive default for the 16 pad slots.
        const float padVelocity = 0.4f + 0.2f * static_cast<float>(i / 4);
        drumPads[i].onClick = [this, i, padVelocity]
        {
            if (juce::ModifierKeys::getCurrentModifiers().isShiftDown())
            {
                chooser = std::make_unique<juce::FileChooser>("Select WAV for pad " + juce::String(i + 1),
                                                               juce::File{}, "*.wav");
                chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                    [this, i](const juce::FileChooser& fc)
                    {
                        const auto file = fc.getResult();
                        if (file == juce::File{}) return; // User cancelled the dialog.
                        if (processor.loadWav(i, file))
                        {
                            drumPads[i].setButtonText(processor.getSampleName(i).substring(0, 18));
                            songStatus.setText("WAV LOADED - PAD " + juce::String(i + 1), juce::dontSendNotification);
                        }
                        else
                        {
                            songStatus.setText("INVALID WAV - PAD " + juce::String(i + 1), juce::dontSendNotification);
                        }
                    });
            }
            else
                processor.playPad(i, padVelocity);
        };
        addAndMakeVisible(drumPads[i]);
    }
    for (int inst = 0; inst < 4; ++inst)
    {
        midiLearnButtons[inst].setButtonText("MIDI LEARN");
        midiLearnButtons[inst].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff334155));
        midiLearnButtons[inst].onClick = [this, inst]
        {
            const int pad = inst + 4 * stepPage;
            const auto mods = juce::ModifierKeys::getCurrentModifiers();
            if (mods.isAltDown() && stepPage == 0)
            {
                processor.armMidiLearn(-1);
                processor.armVolumeCcLearn(processor.getVolumeCcLearnInstrument() == inst ? -1 : inst);
            }
            else if (mods.isShiftDown())
            {
                processor.clearMidiLearn(pad);
                processor.armMidiLearn(-1);
                if (stepPage == 0) processor.clearVolumeCcLearn(inst);
                processor.armVolumeCcLearn(-1);
            }
            else
            {
                processor.armVolumeCcLearn(-1);
                processor.armMidiLearn(processor.getMidiLearnInstrument() == pad ? -1 : pad);
            }
        };
        addAndMakeVisible(midiLearnButtons[inst]);
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
            b->onClick = [this, inst, step, b] { processor.setStep(inst + 4 * stepPage, step, b->getToggleState()); };
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
                // Reject oversized presets before reading them into memory.
                constexpr juce::int64 maxPresetBytes = 96LL * 1024LL * 1024LL;
                if (file.getSize() <= 0 || file.getSize() > maxPresetBytes)
                {
                    songStatus.setText("INVALID OR OVERSIZED PRESET", juce::dontSendNotification);
                    return;
                }
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

    recordToggle.setToggleState(processor.isRecording(), juce::dontSendNotification);
    recordToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xfffb7185));
    recordToggle.onClick = [this] { processor.setRecording(recordToggle.getToggleState()); };
    addAndMakeVisible(recordToggle);

    // Keep the existing arrangement controls and add a dedicated playable pad row.
    setSize(980, 660);
    updateView();
}

void FBTECHDRUMLOOPAudioProcessorEditor::updateView()
{
    const bool pads = activeView == 0;
    const bool editor = activeView == 1;
    const bool song = activeView == 2;
    for (int i = 0; i < 3; ++i)
        viewButtons[i].setColour(juce::TextButton::buttonColourId,
            i == activeView ? juce::Colour(0xff008e9d) : juce::Colour(0xff263545));
    for (auto& b : drumPads) b.setVisible(pads);
    kitButton.setVisible(pads);
    for (auto& b : patternButtons) b.setVisible(pads || editor);
    playButton.setVisible(true);
    stopButton.setVisible(true);
    fillButton.setVisible(pads || editor);
    recordToggle.setVisible(pads || editor);
    clearButton.setVisible(editor);
    stepPageSelector.setVisible(editor);
    for (auto* b : stepButtons) b->setVisible(editor);
    for (auto& b : instrumentLabels) b.setVisible(editor);
    for (auto& b : loadButtons) b.setVisible(editor && stepPage == 0);
    for (auto& b : volumeSliders) b.setVisible(editor && stepPage == 0);
    for (auto& b : midiLearnButtons) b.setVisible(editor);
    songToggle.setVisible(song);
    stopAtEndToggle.setVisible(song);
    autoFillToggle.setVisible(song);
    songLengthLabel.setVisible(song);
    songLengthSlider.setVisible(song);
    for (auto& b : sectionLabels) b.setVisible(song);
    for (auto& b : sectionPatterns) b.setVisible(song);
    for (auto& b : sectionBars) b.setVisible(song);
    moveSlotSelector.setVisible(song);
    moveEarlierButton.setVisible(song);
    moveLaterButton.setVisible(song);
    insertSlotButton.setVisible(song);
    removeSlotButton.setVisible(song);
    saveSongButton.setVisible(song);
    loadSongButton.setVisible(song);
    songStatus.setVisible(song);
    hint.setText(pads ? "Suona i pad. SHIFT + clic su un pad per caricare un WAV." :
                 editor ? "Modifica il ritmo e assegna i controlli MIDI." :
                          "Costruisci il brano scegliendo pattern e battute.",
                 juce::dontSendNotification);
    resized();
}

void FBTECHDRUMLOOPAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0c131c));
    g.setColour(juce::Colour(0xff172534));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(9.0f), 14.0f);
    g.setColour(juce::Colour(0xff008e9d));
    g.fillRoundedRectangle(24.0f, 84.0f, 932.0f, 3.0f, 1.5f);
}

void FBTECHDRUMLOOPAudioProcessorEditor::resized()
{
    title.setBounds(26, 15, 500, 42);
    tempoLabel.setBounds(680, 20, 265, 30);
    hint.setBounds(27, 93, 900, 25);
    for (int i = 0; i < 3; ++i)
        viewButtons[i].setBounds(26 + i * 185, 132, 176, 40);
    if (activeView == 0)
    {
        for (int i = 0; i < 16; ++i)
            drumPads[i].setBounds(27 + (i % 4) * 235, 198 + (i / 4) * 85, 220, 73);
        kitButton.setBounds(28, 562, 188, 38);
        fillButton.setBounds(555, 562, 180, 38);
        recordToggle.setBounds(754, 565, 185, 34);
    }
    else if (activeView == 1)
    {
        const int left = 276, top = 202, cellW = 40, gap = 2, rowH = 66;
        for (int inst = 0; inst < 4; ++inst)
        {
            instrumentLabels[inst].setBounds(27, top + inst * rowH, 100, 22);
            loadButtons[inst].setBounds(27, top + inst * rowH + 26, 94, 28);
            volumeSliders[inst].setBounds(126, top + inst * rowH, 136, 55);
            for (int step = 0; step < 16; ++step)
                stepButtons[inst * 16 + step]->setBounds(left + step * (cellW + gap), top + inst * rowH, cellW, 38);
            midiLearnButtons[inst].setBounds(27 + inst * 235, 488, 219, 34);
        }
        stepPageSelector.setBounds(777, 139, 168, 34);
        clearButton.setBounds(782, 562, 164, 36);
        fillButton.setBounds(580, 562, 184, 36);
        recordToggle.setBounds(27, 559, 180, 36);
    }
    else
    {
        songToggle.setBounds(28, 198, 140, 30);
        songLengthLabel.setBounds(195, 198, 90, 30);
        songLengthSlider.setBounds(288, 198, 90, 30);
        stopAtEndToggle.setBounds(410, 198, 150, 30);
        autoFillToggle.setBounds(585, 198, 130, 30);
        songStatus.setBounds(28, 237, 910, 28);
        for (int i = 0; i < 8; ++i)
        {
            const int x = 28 + (i % 4) * 235;
            const int y = 280 + (i / 4) * 58;
            sectionLabels[i].setBounds(x, y, 65, 26);
            sectionPatterns[i].setBounds(x + 64, y, 108, 26);
            sectionBars[i].setBounds(x + 174, y, 58, 26);
        }
        insertSlotButton.setBounds(28, 410, 140, 34);
        removeSlotButton.setBounds(179, 410, 145, 34);
        moveSlotSelector.setBounds(28, 458, 128, 32);
        moveEarlierButton.setBounds(168, 458, 130, 32);
        moveLaterButton.setBounds(310, 458, 130, 32);
        saveSongButton.setBounds(553, 458, 180, 34);
        loadSongButton.setBounds(746, 458, 190, 34);
    }
    for (int i = 0; i < 4; ++i)
        patternButtons[i].setBounds(28 + i * 177, 527, 168, 30);
    playButton.setBounds(735, 606, 100, 36);
    stopButton.setBounds(845, 606, 100, 36);
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
        loadButtons[i].setButtonText(processor.getSampleName(i) == "Synth (default)" ? "LOAD WAV" : "WAV OK");
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
    // Highlight the active column without changing the stored step toggles.
    for (int inst = 0; inst < 4; ++inst)
    {
        const int pad = inst + 4 * stepPage;
        const int note = processor.getLearnedNote(pad);
        const bool armed = processor.getMidiLearnInstrument() == pad;
        const int cc = stepPage == 0 ? processor.getLearnedVolumeCc(inst) : -1;
        const bool ccArmed = stepPage == 0 && processor.getVolumeCcLearnInstrument() == inst;
        midiLearnButtons[inst].setButtonText(ccArmed ? "MUOVI MANOPOLA MIDI..." :
            armed ? "PREMI UN PAD MIDI..." :
            note >= 0 ? "NOTA " + juce::String(note) + (cc >= 0 ? " / CC " + juce::String(cc) : "") :
            cc >= 0 ? "MIDI LEARN / CC " + juce::String(cc) : "MIDI LEARN");
    }
    for (int row = 0; row < 4; ++row)
    {
        const int pad = row + 4 * stepPage;
        instrumentLabels[row].setText(stepPage == 0
            ? juce::StringArray { "KICK", "SNARE", "HI-HAT", "TOM" }[row]
            : "PAD " + juce::String(pad + 1), juce::dontSendNotification);
        loadButtons[row].setVisible(activeView == 1 && stepPage == 0);
        volumeSliders[row].setVisible(activeView == 1 && stepPage == 0);
    }
    if (stepPage == 0)
        for (int inst = 0; inst < 4; ++inst)
            volumeSliders[inst].setValue(processor.getInstrumentVolume(inst), juce::dontSendNotification);
    // Live recording changes the atomic pattern from the audio thread.
    // Reflect those hits in the step buttons without firing their onClick callbacks.
    recordToggle.setToggleState(processor.isRecording(), juce::dontSendNotification);
    const int activeStep = processor.isTransportRunning() ? processor.getCurrentStep() : -1;
    for (int instrument = 0; instrument < 4; ++instrument)
        for (int step = 0; step < 16; ++step)
        {
            auto* button = stepButtons[instrument * 16 + step];
            button->setToggleState(processor.getStep(instrument + 4 * stepPage, step), juce::dontSendNotification);
            const bool active = step == activeStep;
            button->setColour(juce::TextButton::buttonColourId,
                              active ? juce::Colour(0xff2563eb) : juce::Colour(0xff202d3b));
            button->setColour(juce::TextButton::buttonOnColourId,
                              active ? juce::Colour(0xff38bdf8) : juce::Colour(0xff0d9488));
        }
    // Preserve user-facing preset errors and confirmations until the next action.
    const auto status = songStatus.getText();
    const bool isPresetFeedback = status == "LOAD FAILED" || status == "PRESET TOO LARGE"
        || status == "INVALID OR OVERSIZED PRESET" || status == "INVALID SONG PRESET"
        || status == "SONG PRESET LOADED" || status == "SONG PRESET SAVED"
        || status.startsWith("INVALID WAV") || status.startsWith("WAV LOADED");
    if (!isPresetFeedback)
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
