#pragma once
#include <JuceHeader.h>

class FBTECHDRUMLOOPAudioProcessor final : public juce::AudioProcessor
{
public:
    FBTECHDRUMLOOPAudioProcessor();
    ~FBTECHDRUMLOOPAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "FBTECH DRUM LOOP"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    bool getStep(int instrument, int step) const;
    void setStep(int instrument, int step, bool enabled);
    void clearPattern();
    void selectPattern(int index) noexcept;
    void requestFill() noexcept { fillRequested.store(true); }
    int getSelectedPattern() const noexcept { return selectedPattern.load(); }
    int getPlayingPattern() const noexcept { return playingPattern.load(); }
    int getQueuedPattern() const noexcept { return queuedPattern.load(); }
    bool loadWav(int instrument, const juce::File& file);
    bool loadKitFolder(const juce::File& folder);
    juce::String getSampleName(int instrument) const;
    double getTempo() const noexcept { return tempoBpm; }
    void setInstrumentVolume(int instrument, float value) noexcept;
    float getInstrumentVolume(int instrument) const noexcept;
    void setManualPlaying(bool playing) noexcept { if (playing) songFinished.store(false); manualPlaying.store(playing); }
    bool isManualPlaying() const noexcept { return manualPlaying.load(); }
    void setSongMode(bool enabled) noexcept { songMode.store(enabled); }
    bool isSongMode() const noexcept { return songMode.load(); }
    void setSongBars(int section, int bars) noexcept;
    int getSongBars(int section) const noexcept;
    int getSongSection() const noexcept { return songSection.load(); }
    void setStopAtEnd(bool enabled) noexcept { stopAtEnd.store(enabled); }
    bool isStopAtEnd() const noexcept { return stopAtEnd.load(); }
    bool isSongFinished() const noexcept { return songFinished.load(); }
    void setAutoFill(bool enabled) noexcept { autoFill.store(enabled); }
    bool isAutoFill() const noexcept { return autoFill.load(); }
    void setSongLength(int count) noexcept { songLength.store(juce::jlimit(1, 8, count)); }
    int getSongLength() const noexcept { return songLength.load(); }
    void setSongPattern(int section, int bank) noexcept;
    int getSongPattern(int section) const noexcept;

private:
    static constexpr int instrumentCount = 4;
    static constexpr int stepCount = 16;
    bool pattern[4][instrumentCount][stepCount]{};
    std::atomic<int> selectedPattern { 0 };
    std::atomic<int> playingPattern { 0 };
    std::atomic<int> queuedPattern { -1 };
    std::atomic<bool> transportRunning { false };
    std::atomic<bool> songMode { false };
    std::atomic<bool> stopAtEnd { false };
    std::atomic<bool> autoFill { false };
    std::atomic<bool> songFinished { false };
    std::atomic<int> songLength { 4 };
    std::atomic<int> songPattern[8] { 0, 1, 2, 3, 1, 2, 1, 3 };
    std::atomic<int> songBars[8] { 4, 8, 8, 4, 8, 8, 8, 4 };
    std::atomic<int> songSection { 0 };
    int songBarCount = 0;
    std::atomic<float> instrumentVolumes[4] { 1.0f, 1.0f, 1.0f, 1.0f };
    double currentSampleRate = 44100.0;
    double tempoBpm = 120.0;
    double freeRunPpq = 0.0;
    int lastStep = -1;
    std::atomic<bool> manualPlaying { false };
    bool wasPlaying = false;
    std::atomic<bool> fillRequested { false };
    bool fillActive = false;
    double kickPhase = 0.0, kickEnvelope = 0.0;
    double snarePhase = 0.0, snareEnvelope = 0.0;
    double hatEnvelope = 0.0, tomPhase = 0.0, tomEnvelope = 0.0;
    juce::Random random;
    struct SampleData { juce::AudioBuffer<float> audio; juce::String name; double sampleRate = 44100.0; };
    std::shared_ptr<SampleData> samples[4];
    std::shared_ptr<SampleData> activeSamples[4];
    double samplePositions[4] { -1, -1, -1, -1 };
    juce::AudioFormatManager formatManager;

    void triggerInstrument(int instrument);
    float renderInstrument(int instrument);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FBTECHDRUMLOOPAudioProcessor)
};
