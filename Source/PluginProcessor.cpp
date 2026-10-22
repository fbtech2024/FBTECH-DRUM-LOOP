#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace { constexpr double twoPi = juce::MathConstants<double>::twoPi; }

FBTECHDRUMLOOPAudioProcessor::FBTECHDRUMLOOPAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    formatManager.registerBasicFormats();
    // Default 4/4 groove: kick, snare, eighth-note hats and one tom accent.
    for (int s : { 0, 8 }) pattern[0][s] = true;
    for (int s : { 4, 12 }) pattern[1][s] = true;
    for (int s = 0; s < stepCount; s += 2) pattern[2][s] = true;
    pattern[3][10] = true;
}

void FBTECHDRUMLOOPAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    freeRunPpq = 0.0;
    lastStep = -1;
    wasPlaying = false;
}

void FBTECHDRUMLOOPAudioProcessor::releaseResources() {}

bool FBTECHDRUMLOOPAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

bool FBTECHDRUMLOOPAudioProcessor::getStep(int instrument, int step) const
{
    return instrument >= 0 && instrument < instrumentCount && step >= 0 && step < stepCount
        ? pattern[instrument][step] : false;
}

void FBTECHDRUMLOOPAudioProcessor::setStep(int instrument, int step, bool enabled)
{
    if (instrument >= 0 && instrument < instrumentCount && step >= 0 && step < stepCount)
        pattern[instrument][step] = enabled;
}

void FBTECHDRUMLOOPAudioProcessor::clearPattern()
{
    for (auto& row : pattern)
        for (bool& step : row) step = false;
}

bool FBTECHDRUMLOOPAudioProcessor::loadWav(int instrument, const juce::File& file)
{
    if (instrument < 0 || instrument >= instrumentCount || !file.existsAsFile()) return false;
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (!reader || reader->lengthInSamples <= 0 || reader->lengthInSamples > 44100 * 60 || reader->numChannels == 0) return false;
    auto data = std::make_shared<SampleData>();
    data->audio.setSize(1, static_cast<int>(reader->lengthInSamples));
    if (!reader->read(&data->audio, 0, data->audio.getNumSamples(), 0, true, false)) return false;
    data->sampleRate = reader->sampleRate;
    data->name = file.getFileName();
    std::atomic_store(&samples[instrument], data);
    return true;
}

bool FBTECHDRUMLOOPAudioProcessor::loadKitFolder(const juce::File& folder)
{
    if (!folder.isDirectory()) return false;
    const char* names[] = { "kick.wav", "snare.wav", "hihat.wav", "tom.wav" };
    // Validate every sample first, avoiding partially loaded kits.
    std::shared_ptr<SampleData> newKit[4];
    for (int i = 0; i < instrumentCount; ++i)
    {
        const auto file = folder.getChildFile(names[i]);
        if (!file.existsAsFile()) return false;
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (!reader || reader->lengthInSamples <= 0 || reader->lengthInSamples > 44100 * 60
            || reader->numChannels == 0 || reader->sampleRate <= 0.0) return false;
        auto data = std::make_shared<SampleData>();
        data->audio.setSize(1, static_cast<int>(reader->lengthInSamples));
        if (!reader->read(&data->audio, 0, data->audio.getNumSamples(), 0, true, false)) return false;
        data->sampleRate = reader->sampleRate;
        data->name = folder.getFileName() + " / " + file.getFileName();
        newKit[i] = std::move(data);
    }
    for (int i = 0; i < instrumentCount; ++i)
        std::atomic_store(&samples[i], newKit[i]);
    return true;
}

juce::String FBTECHDRUMLOOPAudioProcessor::getSampleName(int instrument) const
{
    if (instrument < 0 || instrument >= instrumentCount) return {};
    auto data = std::atomic_load(&samples[instrument]);
    return data ? data->name : juce::String("Synth (default)");
}

void FBTECHDRUMLOOPAudioProcessor::triggerInstrument(int instrument)
{
    auto sample = std::atomic_load(&samples[instrument]);
    if (sample) { activeSamples[instrument] = sample; samplePositions[instrument] = 0; return; }
    samplePositions[instrument] = -1;
    switch (instrument)
    {
        case 0: kickPhase = 0.0; kickEnvelope = 1.0; break;
        case 1: snarePhase = 0.0; snareEnvelope = 1.0; break;
        case 2: hatEnvelope = 1.0; break;
        case 3: tomPhase = 0.0; tomEnvelope = 1.0; break;
        default: break;
    }
}

float FBTECHDRUMLOOPAudioProcessor::renderInstrument(int instrument)
{
    if (samplePositions[instrument] >= 0 && activeSamples[instrument])
    {
        const auto& data = *activeSamples[instrument];
        const auto& audio = data.audio;
        const double pos = samplePositions[instrument];
        const int index = static_cast<int>(pos);
        if (index + 1 < audio.getNumSamples())
        {
            const float fraction = static_cast<float>(pos - index);
            const float result = audio.getSample(0, index) * (1.0f - fraction) + audio.getSample(0, index + 1) * fraction;
            samplePositions[instrument] += data.sampleRate / currentSampleRate;
            return result;
        }
        samplePositions[instrument] = -1;
        activeSamples[instrument].reset();
        return 0.0f;
    }
    const double sr = currentSampleRate;
    switch (instrument)
    {
        case 0: {
            const double freq = 45.0 + 115.0 * kickEnvelope;
            const float y = static_cast<float>(std::sin(kickPhase) * kickEnvelope * 0.92);
            kickPhase += twoPi * freq / sr;
            kickEnvelope *= std::exp(-1.0 / (0.23 * sr));
            return y;
        }
        case 1: {
            const float noise = random.nextFloat() * 2.0f - 1.0f;
            const float tone = static_cast<float>(std::sin(snarePhase));
            snarePhase += twoPi * 185.0 / sr;
            const float y = (noise * 0.72f + tone * 0.28f) * static_cast<float>(snareEnvelope) * 0.62f;
            snareEnvelope *= std::exp(-1.0 / (0.16 * sr));
            return y;
        }
        case 2: {
            const float noise = random.nextFloat() * 2.0f - 1.0f;
            const float y = noise * static_cast<float>(hatEnvelope) * 0.20f;
            hatEnvelope *= std::exp(-1.0 / (0.055 * sr));
            return y;
        }
        case 3: {
            const float y = static_cast<float>(std::sin(tomPhase) * tomEnvelope * 0.58);
            tomPhase += twoPi * (105.0 + 35.0 * tomEnvelope) / sr;
            tomEnvelope *= std::exp(-1.0 / (0.31 * sr));
            return y;
        }
        default: return 0.0f;
    }
}

void FBTECHDRUMLOOPAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    midi.clear();

    bool hostPlaying = false;
    if (auto* playHead = getPlayHead())
    {
        if (auto info = playHead->getPosition())
        {
            hostPlaying = info->getIsPlaying();
            if (auto bpm = info->getBpm(); bpm.hasValue() && *bpm > 0.0)
                tempoBpm = *bpm;
            if (auto ppq = info->getPpqPosition(); ppq.hasValue())
            {
                if (hostPlaying) freeRunPpq = *ppq;
            }
        }
    }

    const bool running = hostPlaying || manualPlaying.load();
    if (!running)
    {
        lastStep = -1;
        wasPlaying = false;
        for (int i = 0; i < instrumentCount; ++i)
        {
            samplePositions[i] = -1;
            activeSamples[i].reset();
        }
        kickEnvelope = snareEnvelope = hatEnvelope = tomEnvelope = 0.0;
        return;
    }
    if (!wasPlaying)
    {
        lastStep = -1;
        if (!hostPlaying) freeRunPpq = 0.0;
    }
    wasPlaying = true;
    const double beatsPerSample = tempoBpm / (60.0 * currentSampleRate);
    const double startPpq = freeRunPpq;
    const int channels = buffer.getNumChannels();
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const double ppq = startPpq + beatsPerSample * sample;
        const int step = ((static_cast<int>(std::floor(ppq * 4.0 + 1.0e-7)) % stepCount) + stepCount) % stepCount;
        if (step != lastStep)
        {
            for (int inst = 0; inst < instrumentCount; ++inst)
                if (pattern[inst][step]) triggerInstrument(inst);
            lastStep = step;
        }

        float mixed = 0.0f;
        for (int inst = 0; inst < instrumentCount; ++inst)
            mixed += renderInstrument(inst);
        mixed = std::tanh(mixed * 1.3f) * 0.72f;
        for (int ch = 0; ch < channels; ++ch)
            buffer.setSample(ch, sample, mixed);
    }

    freeRunPpq = startPpq + beatsPerSample * buffer.getNumSamples();
}

void FBTECHDRUMLOOPAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::String state;
    for (int inst = 0; inst < instrumentCount; ++inst)
        for (int step = 0; step < stepCount; ++step)
            state += pattern[inst][step] ? '1' : '0';
    auto xml = std::make_unique<juce::XmlElement>("FBTECH_DRUM_LOOP_STATE");
    xml->setAttribute("pattern", state);
    xml->setAttribute("version", 1);
    copyXmlToBinary(*xml, destData);
}

void FBTECHDRUMLOOPAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml == nullptr || !xml->hasTagName("FBTECH_DRUM_LOOP_STATE")) return;
    const auto state = xml->getStringAttribute("pattern");
    if (state.length() != instrumentCount * stepCount) return;
    for (int inst = 0; inst < instrumentCount; ++inst)
        for (int step = 0; step < stepCount; ++step)
            pattern[inst][step] = state[inst * stepCount + step] == '1';
}

juce::AudioProcessorEditor* FBTECHDRUMLOOPAudioProcessor::createEditor()
{
    return new FBTECHDRUMLOOPAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FBTECHDRUMLOOPAudioProcessor();
}
