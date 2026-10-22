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
    bool loadWav(int instrument, const juce::File& file);
    bool loadKitFolder(const juce::File& folder);
    juce::String getSampleName(int instrument) const;
    double getTempo() const noexcept { return tempoBpm; }
    void setInstrumentVolume(int instrument, float value) noexcept;
    float getInstrumentVolume(int instrument) const noexcept;
    void setManualPlaying(bool playing) noexcept { manualPlaying.store(playing); }
    bool isManualPlaying() const noexcept { return manualPlaying.load(); }

private:
    static constexpr int instrumentCount = 4;
    static constexpr int stepCount = 16;
    bool pattern[instrumentCount][stepCount]{};
    std::atomic<float> instrumentVolumes[4] { 1.0f, 1.0f, 1.0f, 1.0f };
    double currentSampleRate = 44100.0;
    double tempoBpm = 120.0;
    double freeRunPpq = 0.0;
    int lastStep = -1;
    std::atomic<bool> manualPlaying { false };
    bool wasPlaying = false;
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
