#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <cstring>
#include <array>

namespace { constexpr double twoPi = juce::MathConstants<double>::twoPi; }

FBTECHDRUMLOOPAudioProcessor::FBTECHDRUMLOOPAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    formatManager.registerBasicFormats();
    // Default 4/4 groove: kick, snare, eighth-note hats and one tom accent.
    for (int s : { 0, 8 }) pattern[0][0][s] = true;
    for (int s : { 4, 12 }) pattern[0][1][s] = true;
    for (int s = 0; s < stepCount; s += 2) pattern[0][2][s] = true;
    pattern[0][3][10] = true;
}

void FBTECHDRUMLOOPAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    freeRunPpq = 0.0;
    lastStep = -1;
    currentStep.store(-1);
    wasPlaying = false;
    fillActive = false;
    fillRequested.store(false);
    transportRunning.store(false);
    queuedPattern.store(-1);
    playingPattern.store(selectedPattern.load());
    songSection.store(0);
    songBarCount = 0;
    songFinished.store(false);
}

void FBTECHDRUMLOOPAudioProcessor::releaseResources() {}

bool FBTECHDRUMLOOPAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void FBTECHDRUMLOOPAudioProcessor::setInstrumentVolume(int instrument, float value) noexcept
{
    if (instrument >= 0 && instrument < instrumentCount)
        instrumentVolumes[instrument].store(juce::jlimit(0.0f, 1.5f, value));
}

float FBTECHDRUMLOOPAudioProcessor::getInstrumentVolume(int instrument) const noexcept
{
    return instrument >= 0 && instrument < instrumentCount ? instrumentVolumes[instrument].load() : 1.0f;
}

void FBTECHDRUMLOOPAudioProcessor::setSongBars(int section, int bars) noexcept
{
    if (section >= 0 && section < 8) songBars[section].store(juce::jlimit(1, 32, bars));
}

int FBTECHDRUMLOOPAudioProcessor::getSongBars(int section) const noexcept
{
    return section >= 0 && section < 8 ? songBars[section].load() : 1;
}

void FBTECHDRUMLOOPAudioProcessor::setSongPattern(int section, int bank) noexcept
{
    if (section >= 0 && section < 8 && bank >= 0 && bank < 4) songPattern[section].store(bank);
}

int FBTECHDRUMLOOPAudioProcessor::getSongPattern(int section) const noexcept
{
    return section >= 0 && section < 8 ? songPattern[section].load() : 0;
}

void FBTECHDRUMLOOPAudioProcessor::selectPattern(int index) noexcept
{
    if (index < 0 || index >= 4) return;
    selectedPattern.store(index);
    if (songMode.load()) return;
    if (transportRunning.load())
        queuedPattern.store(index == playingPattern.load() ? -1 : index);
    else
    {
        queuedPattern.store(-1);
        playingPattern.store(index);
    }
}

bool FBTECHDRUMLOOPAudioProcessor::getStep(int instrument, int step) const
{
    return instrument >= 0 && instrument < instrumentCount && step >= 0 && step < stepCount
        ? pattern[selectedPattern.load()][instrument][step].load() : false;
}

void FBTECHDRUMLOOPAudioProcessor::setStep(int instrument, int step, bool enabled)
{
    if (instrument >= 0 && instrument < instrumentCount && step >= 0 && step < stepCount)
        pattern[selectedPattern.load()][instrument][step] = enabled;
}

void FBTECHDRUMLOOPAudioProcessor::playPad(int instrument, float velocity) noexcept
{
    if (instrument < 0 || instrument >= instrumentCount) return;
    const int scaledVelocity = juce::jlimit(1, 127, juce::roundToInt(velocity * 127.0f));
    pendingPadHits[instrument].store(scaledVelocity);
}

void FBTECHDRUMLOOPAudioProcessor::clearPattern()
{
    for (auto& row : pattern[selectedPattern.load()])
        for (auto& step : row) step.store(false);
}

bool FBTECHDRUMLOOPAudioProcessor::loadWav(int instrument, const juce::File& file)
{
    if (instrument < 0 || instrument >= instrumentCount || !file.existsAsFile()) return false;
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (!reader || reader->lengthInSamples <= 0 || reader->sampleRate < 8000.0 || reader->sampleRate > 384000.0
        || !std::isfinite(reader->sampleRate)
        || static_cast<double>(reader->lengthInSamples) / reader->sampleRate > 60.0
        || reader->lengthInSamples > 16 * 1024 * 1024
        || reader->numChannels == 0) return false;
    auto data = std::make_shared<SampleData>();
    const int frames = static_cast<int>(reader->lengthInSamples);
    juce::AudioBuffer<float> decoded(juce::jmin(2, static_cast<int>(reader->numChannels)), frames);
    if (!reader->read(&decoded, 0, frames, 0, true, decoded.getNumChannels() > 1)) return false;
    data->audio.setSize(1, frames);
    for (int frame = 0; frame < frames; ++frame)
    {
        const float left = decoded.getSample(0, frame);
        const float right = decoded.getNumChannels() > 1 ? decoded.getSample(1, frame) : left;
        const float mixed = 0.5f * (left + right);
        if (!std::isfinite(mixed)) return false;
        data->audio.setSample(0, frame, mixed);
    }
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
        if (!reader || reader->lengthInSamples <= 0 || reader->sampleRate < 8000.0 || reader->sampleRate > 384000.0
            || !std::isfinite(reader->sampleRate)
            || static_cast<double>(reader->lengthInSamples) / reader->sampleRate > 60.0
            || reader->lengthInSamples > 16 * 1024 * 1024
            || reader->numChannels == 0) return false;
        auto data = std::make_shared<SampleData>();
        const int frames = static_cast<int>(reader->lengthInSamples);
        juce::AudioBuffer<float> decoded(juce::jmin(2, static_cast<int>(reader->numChannels)), frames);
        if (!reader->read(&decoded, 0, frames, 0, true, decoded.getNumChannels() > 1)) return false;
        data->audio.setSize(1, frames);
        for (int frame = 0; frame < frames; ++frame)
        {
            const float left = decoded.getSample(0, frame);
            const float right = decoded.getNumChannels() > 1 ? decoded.getSample(1, frame) : left;
            const float mixed = 0.5f * (left + right);
            if (!std::isfinite(mixed)) return false;
            data->audio.setSample(0, frame, mixed);
        }
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
    sampleVelocities[instrument] = 1.0f;
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
        if (index >= 0 && index < audio.getNumSamples())
        {
            const float fraction = static_cast<float>(pos - index);
            const int nextIndex = juce::jmin(index + 1, audio.getNumSamples() - 1);
            const float result = audio.getSample(0, index) * (1.0f - fraction) + audio.getSample(0, nextIndex) * fraction;
            samplePositions[instrument] += data.sampleRate / currentSampleRate;
            return result * sampleVelocities[instrument];
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

    // Read MIDI note-on events before clearing the input buffer. Standard GM
    // drum notes work with ATOM and other controllers configured to send MIDI.
    // Events are dispatched at their sample offset for accurate timing.
    struct DrumHit { int offset, instrument; float velocity; };
    // Fixed capacity avoids heap allocation on the realtime audio thread.
    std::array<DrumHit, 256> incomingHits {};
    size_t hitCount = 0;
    for (const auto metadata : midi)
    {
        const auto message = metadata.getMessage();
        if (!message.isNoteOn()) continue;
        const int note = message.getNoteNumber();
        const int instrument = note == 36 || note == 35 ? 0
                             : note == 38 || note == 40 ? 1
                             : note == 42 || note == 44 || note == 46 ? 2
                             : note == 41 || note == 43 || note == 45 || note == 47 || note == 48 || note == 50 ? 3 : -1;
        if (instrument >= 0 && metadata.samplePosition >= 0
            && metadata.samplePosition < buffer.getNumSamples())
            if (hitCount < incomingHits.size())
                incomingHits[hitCount++] = { metadata.samplePosition, instrument, message.getFloatVelocity() };
    }
    midi.clear();
    // Transfer UI pad requests to the audio thread; never mutate voices from GUI.
    for (int inst = 0; inst < instrumentCount; ++inst)
    {
        const int velocity = pendingPadHits[inst].exchange(0);
        if (velocity > 0 && hitCount < incomingHits.size())
            incomingHits[hitCount++] = { 0, inst, static_cast<float>(velocity) / 127.0f };
    }

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
    transportRunning.store(running);
    if (!running)
    {
        queuedPattern.store(-1);
        playingPattern.store(selectedPattern.load());
        songSection.store(0);
        songBarCount = 0;
        lastStep = -1;
        currentStep.store(-1);
        fillActive = false;
        fillRequested.store(false);
        // Clear sequencer voices only on the running-to-stopped transition.
        // Clearing samples on every stopped block would truncate live MIDI WAV hits.
        if (wasPlaying)
        {
            for (int i = 0; i < instrumentCount; ++i)
            {
                samplePositions[i] = -1;
                activeSamples[i].reset();
            }
            kickEnvelope = snareEnvelope = hatEnvelope = tomEnvelope = 0.0;
        }
    }
    if (running && !wasPlaying)
    {
        lastStep = -1;
        if (!hostPlaying) freeRunPpq = 0.0;
        songSection.store(0);
        songBarCount = 0;
        songFinished.store(false);
        if (songMode.load()) playingPattern.store(getSongPattern(0));
    }
    wasPlaying = running;
    // Reject invalid host tempo values before they reach sequencer timing math.
    if (!std::isfinite(tempoBpm) || tempoBpm < 1.0 || tempoBpm > 1000.0)
        tempoBpm = 120.0;
    // Defensive guard: hosts may briefly supply invalid sample rates during setup.
    if (!std::isfinite(currentSampleRate) || currentSampleRate <= 0.0)
        return;
    const double beatsPerSample = tempoBpm / (60.0 * currentSampleRate);
    // Re-anchor the free-running clock before its floating-point precision degrades
    // during multi-day sessions without a DAW transport position.
    if (!hostPlaying && std::abs(freeRunPpq) > 4096.0)
    {
        freeRunPpq = std::fmod(freeRunPpq, 4.0);
        lastStep = -1;
    }
    const double startPpq = freeRunPpq;
    const int channels = buffer.getNumChannels();
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const double ppq = startPpq + beatsPerSample * sample;
        // Keep the step counter bounded even after very long DAW sessions.
        const double stepPosition = std::floor(ppq * 4.0 + 1.0e-7);
        const int step = static_cast<int>(std::fmod(std::fmod(stepPosition, static_cast<double>(stepCount))
                                                     + stepCount, static_cast<double>(stepCount)));
        // MIDI hits are audible independently of the sequencer transport.
        for (size_t hitIndex = 0; hitIndex < hitCount; ++hitIndex)
            if (const auto& hit = incomingHits[hitIndex]; hit.offset == sample)
            {
                triggerInstrument(hit.instrument);
                // Velocity shapes the built-in synthesised drum envelopes.
                const double velocity = juce::jlimit(0.0, 1.0, static_cast<double>(hit.velocity));
                sampleVelocities[hit.instrument] = static_cast<float>(velocity);
                // Capture only played MIDI/GUI hits, not sequencer-generated notes.
                // Quantize to the nearest sixteenth on the running host/manual clock.
                if (running && recording.load() && !songMode.load())
                {
                    const int recordedStep = static_cast<int>(std::floor(ppq * 4.0 + 0.5)) & 15;
                    pattern[selectedPattern.load()][hit.instrument][recordedStep].store(true);
                }
                switch (hit.instrument)
                {
                    case 0: kickEnvelope *= velocity; break;
                    case 1: snareEnvelope *= velocity; break;
                    case 2: hatEnvelope *= velocity; break;
                    case 3: tomEnvelope *= velocity; break;
                }
            }
        if (running && step != lastStep)
        {
            if (step == 0)
            {
                if (songMode.load())
                {
                    if (lastStep >= 0)
                    {
                        ++songBarCount;
                        const int section = songSection.load();
                        if (songBarCount >= getSongBars(section))
                        {
                            songBarCount = 0;
                            if (section == getSongLength() - 1 && stopAtEnd.load())
                            {
                                songFinished.store(true);
                                manualPlaying.store(false);
                                break;
                            }
                            songSection.store((section + 1) % getSongLength());
                        }
                    }
                    playingPattern.store(getSongPattern(songSection.load()));
                    queuedPattern.store(-1);
                }
                else
                {
                    const int next = queuedPattern.exchange(-1);
                    if (next >= 0 && next < 4) playingPattern.store(next);
                }
                fillActive = false;
                if (fillRequested.exchange(false)) fillActive = true;
                if (songMode.load() && autoFill.load() && songSection.load() < getSongLength() - 1
                    && songBarCount == getSongBars(songSection.load()) - 1)
                    fillActive = true;
            }
            const int bank = playingPattern.load();
            for (int inst = 0; inst < instrumentCount; ++inst)
            {
                bool hit = pattern[bank][inst][step];
                if (fillActive)
                {
                    // One bar of extra snare and tom accents, without changing the saved groove.
                    if (inst == 1 && (step == 6 || step == 10 || step == 12 || step == 14)) hit = true;
                    if (inst == 3 && (step == 8 || step == 11 || step == 13 || step == 15)) hit = true;
                }
                if (hit) triggerInstrument(inst);
            }
            lastStep = step;
            currentStep.store(step);
        }

        if (running && songFinished.load()) break;
        float mixed = 0.0f;
        for (int inst = 0; inst < instrumentCount; ++inst)
            mixed += renderInstrument(inst) * instrumentVolumes[inst].load();
        mixed = std::tanh(mixed * 1.3f) * 0.72f;
        for (int ch = 0; ch < channels; ++ch)
            buffer.setSample(ch, sample, mixed);
    }

    // Advance only while the sequencer is running. Keep the autonomous clock
    // bounded, but never rewrite the host transport position.
    if (running && !hostPlaying)
        freeRunPpq = std::fmod(startPpq + beatsPerSample * buffer.getNumSamples(), 4096.0);
    else if (running)
        freeRunPpq = startPpq + beatsPerSample * buffer.getNumSamples();
}

void FBTECHDRUMLOOPAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto xml = std::make_unique<juce::XmlElement>("FBTECH_DRUM_LOOP_STATE");
    for (int bank = 0; bank < 4; ++bank)
    {
        juce::String bits;
        for (int inst = 0; inst < instrumentCount; ++inst)
            for (int step = 0; step < stepCount; ++step)
                bits += pattern[bank][inst][step] ? '1' : '0';
        xml->setAttribute("pattern" + juce::String(bank), bits);
    }
    xml->setAttribute("pattern", xml->getStringAttribute("pattern0"));
    xml->setAttribute("selectedPattern", selectedPattern.load());
    xml->setAttribute("songMode", songMode.load());
    xml->setAttribute("stopAtEnd", stopAtEnd.load());
    xml->setAttribute("autoFill", autoFill.load());
    xml->setAttribute("songLength", getSongLength());
    for (int section = 0; section < 8; ++section)
    {
        xml->setAttribute("songBars" + juce::String(section), getSongBars(section));
        xml->setAttribute("songPattern" + juce::String(section), getSongPattern(section));
    }
    // Embed the actual mono PCM samples: presets remain portable across machines.
    for (int i = 0; i < instrumentCount; ++i)
    {
        const auto sample = std::atomic_load(&samples[i]);
        if (!sample) continue;
        auto* item = xml->createNewChildElement("SAMPLE");
        item->setAttribute("instrument", i);
        item->setAttribute("name", sample->name);
        item->setAttribute("rate", sample->sampleRate);
        const int count = sample->audio.getNumSamples();
        item->setAttribute("frames", count);
        juce::MemoryBlock pcm(sample->audio.getReadPointer(0),
                              static_cast<size_t>(count) * sizeof(float));
        item->setAttribute("pcm", pcm.toBase64Encoding());
    }
    for (int i = 0; i < instrumentCount; ++i)
        xml->setAttribute("volume" + juce::String(i), static_cast<double>(getInstrumentVolume(i)));
    xml->setAttribute("version", 7);
    copyXmlToBinary(*xml, destData);
}

void FBTECHDRUMLOOPAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml == nullptr || !xml->hasTagName("FBTECH_DRUM_LOOP_STATE")) return;
    for (int bank = 0; bank < 4; ++bank)
    {
        const auto key = "pattern" + juce::String(bank);
        const auto bits = xml->getStringAttribute(key, bank == 0 ? xml->getStringAttribute("pattern") : juce::String());
        if (bits.length() != instrumentCount * stepCount) continue;
        for (int inst = 0; inst < instrumentCount; ++inst)
            for (int step = 0; step < stepCount; ++step)
                pattern[bank][inst][step] = bits[inst * stepCount + step] == '1';
    }
    for (int i = 0; i < instrumentCount; ++i)
        setInstrumentVolume(i, static_cast<float>(xml->getDoubleAttribute("volume" + juce::String(i), 1.0)));
    songMode.store(xml->getBoolAttribute("songMode", false));
    stopAtEnd.store(xml->getBoolAttribute("stopAtEnd", false));
    autoFill.store(xml->getBoolAttribute("autoFill", false));
    setSongLength(xml->getIntAttribute("songLength", 4));
    for (int section = 0; section < 8; ++section)
    {
        setSongBars(section, xml->getIntAttribute("songBars" + juce::String(section), section == 0 || section == 3 ? 4 : 8));
        setSongPattern(section, xml->getIntAttribute("songPattern" + juce::String(section), section % 4));
    }
    // Restore only validated embedded samples. Legacy presets without SAMPLE
    // elements keep their previous behaviour.
    if (xml->getIntAttribute("version", 0) >= 7)
    {
        std::shared_ptr<SampleData> restored[4];
        for (auto* item = xml->getFirstChildElement(); item != nullptr; item = item->getNextElement())
        {
            if (!item->hasTagName("SAMPLE")) continue;
            const int instrument = item->getIntAttribute("instrument", -1);
            const int frames = item->getIntAttribute("frames", 0);
            const double rate = item->getDoubleAttribute("rate", 0.0);
            if (instrument < 0 || instrument >= instrumentCount || frames <= 0
                || !std::isfinite(rate) || rate < 8000.0 || rate > 384000.0
                || static_cast<double>(frames) / rate > 60.0
                || static_cast<size_t>(frames) * sizeof(float) > 64u * 1024u * 1024u) continue;
            juce::MemoryBlock pcm;
            if (!pcm.fromBase64Encoding(item->getStringAttribute("pcm"))
                || pcm.getSize() != static_cast<size_t>(frames) * sizeof(float)) continue;
            auto data = std::make_shared<SampleData>();
            data->audio.setSize(1, frames);
            std::memcpy(data->audio.getWritePointer(0), pcm.getData(), pcm.getSize());
            // Untrusted preset audio must not inject NaN/Inf into the audio output.
            bool validAudio = true;
            for (int frame = 0; frame < frames; ++frame)
            {
                const float value = data->audio.getSample(0, frame);
                if (!std::isfinite(value)) { validAudio = false; break; }
            }
            if (!validAudio) continue;
            data->sampleRate = rate;
            data->name = item->getStringAttribute("name").substring(0, 256);
            restored[instrument] = std::move(data);
        }
        for (int i = 0; i < instrumentCount; ++i)
            std::atomic_store(&samples[i], restored[i]);
    }
    transportRunning.store(false);
    selectPattern(xml->getIntAttribute("selectedPattern", 0));
}

juce::AudioProcessorEditor* FBTECHDRUMLOOPAudioProcessor::createEditor()
{
    return new FBTECHDRUMLOOPAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FBTECHDRUMLOOPAudioProcessor();
}
