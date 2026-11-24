#include "PluginProcessor.h"
#include "PluginEditor.h"

R2RMusicVox2MidiAudioProcessor::R2RMusicVox2MidiAudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}

void R2RMusicVox2MidiAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    monoScratch.setSize(1, juce::jmax(512, samplesPerBlock));
    activeNote = -1;
}

bool R2RMusicVox2MidiAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

float R2RMusicVox2MidiAudioProcessor::detectPitch(const float* x, int n, double rate, float& confidence) const
{
    confidence = 0.0f;
    if (n < 256 || rate <= 0.0) return 0.0f;
    double energy = 0.0;
    for (int i = 0; i < n; ++i) energy += double(x[i]) * x[i];
    if (energy / n < 1.0e-6) return 0.0f;

    const int minLag = juce::jmax(2, int(rate / 1000.0));
    const int maxLag = juce::jmin(n / 2, int(rate / 65.0));
    double best = 0.0;
    int bestLag = 0;
    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double corr = 0.0, a = 0.0, b = 0.0;
        for (int i = 0; i < n - lag; ++i)
        {
            corr += double(x[i]) * x[i + lag];
            a += double(x[i]) * x[i];
            b += double(x[i + lag]) * x[i + lag];
        }
        const double score = corr / std::sqrt(a * b + 1.0e-12);
        if (score > best) { best = score; bestLag = lag; }
    }
    confidence = float(best);
    if (bestLag == 0 || best < sensitivity) return 0.0f;
    return float(rate / bestLag);
}

void R2RMusicVox2MidiAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();
    if (channels == 0) return;
    if (monoScratch.getNumSamples() < samples) monoScratch.setSize(1, samples, false, false, true);
    auto* mono = monoScratch.getWritePointer(0);
    for (int i = 0; i < samples; ++i)
    {
        float v = 0.0f;
        for (int c = 0; c < channels; ++c) v += buffer.getReadPointer(c)[i];
        mono[i] = v / float(channels);
    }
    float confidence = 0.0f;
    const float hz = detectPitch(mono, samples, currentSampleRate, confidence);
    int note = hz > 0.0f ? juce::jlimit(0, 127, int(std::round(69.0 + 12.0 * std::log2(hz / 440.0)))) : -1;
    if (note != activeNote)
    {
        if (activeNote >= 0) midi.addEvent(juce::MidiMessage::noteOff(midiChannel, activeNote), 0);
        if (note >= 0) midi.addEvent(juce::MidiMessage::noteOn(midiChannel, note, 0.85f), 0);
        activeNote = note;
    }
    buffer.clear();
}

bool R2RMusicVox2MidiAudioProcessor::convertAudioFileToMidi(const juce::File& input, const juce::File& output)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(input));
    if (!reader || reader->lengthInSamples <= 0) return false;

    constexpr int blockSize = 2048;
    juce::AudioBuffer<float> block(juce::jmax(1, int(reader->numChannels)), blockSize);
    juce::MidiMessageSequence sequence;
    double time = 0.0;
    int previousNote = -1;
    const double rate = reader->sampleRate;
    for (juce::int64 pos = 0; pos < reader->lengthInSamples; pos += blockSize)
    {
        const int count = int(juce::jmin<juce::int64>(blockSize, reader->lengthInSamples - pos));
        block.clear();
        reader->read(&block, 0, count, pos, true, true);
        for (int i = 0; i < count; ++i)
        {
            float sum = 0.0f;
            for (int c = 0; c < block.getNumChannels(); ++c) sum += block.getReadPointer(c)[i];
            block.setSample(0, i, sum / float(block.getNumChannels()));
        }
        float confidence = 0.0f;
        const float hz = detectPitch(block.getReadPointer(0), count, rate, confidence);
        const int note = hz > 0.0f ? juce::jlimit(0, 127, int(std::round(69.0 + 12.0 * std::log2(hz / 440.0)))) : -1;
        if (note != previousNote)
        {
            if (previousNote >= 0) sequence.addEvent(juce::MidiMessage::noteOff(1, previousNote, 0.0f), time);
            if (note >= 0) sequence.addEvent(juce::MidiMessage::noteOn(1, note, 0.8f), time);
            previousNote = note;
        }
        time += double(count) / rate;
    }
    if (previousNote >= 0) sequence.addEvent(juce::MidiMessage::noteOff(1, previousNote), time);

    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(960);
    midiFile.addTrack(sequence);
    output.getParentDirectory().createDirectory();
    auto stream = output.createOutputStream();
    if (!stream) return false;
    juce::FileOutputStream* raw = stream.get();
    juce::MidiFile::StandardMidiFileWriter writer(midiFile, 960);
    return writer.writeTo(*raw);
}

juce::AudioProcessorEditor* R2RMusicVox2MidiAudioProcessor::createEditor()
{
    return new R2RMusicVox2MidiAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new R2RMusicVox2MidiAudioProcessor();
}
