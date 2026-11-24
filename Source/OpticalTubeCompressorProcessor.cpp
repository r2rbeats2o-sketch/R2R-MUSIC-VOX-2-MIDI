#include "OpticalTubeCompressorProcessor.h"
#include "OpticalTubeCompressorEditor.h"

R2ROpticalTubeCompressorAudioProcessor::R2ROpticalTubeCompressorAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "R2R_OPTICAL_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
R2ROpticalTubeCompressorAudioProcessor::createParameterLayout()
{
    using Float = juce::AudioParameterFloat;
    using Bool = juce::AudioParameterBool;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<Float>("peakReduction", "Peak Reduction",
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.01f), 18.0f));
    layout.add(std::make_unique<Float>("outputGain", "Output Gain",
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.01f), 25.0f));
    layout.add(std::make_unique<Float>("tubeWarmth", "Tube Warmth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 22.0f));
    layout.add(std::make_unique<Bool>("limitMode", "Limit Mode", false));
    layout.add(std::make_unique<Bool>("power", "Power", true));
    return layout;
}

void R2ROpticalTubeCompressorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    processSpec.sampleRate = sampleRate;
    processSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    processSpec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    compressor.prepare(processSpec);
    compressor.reset();
    meterDb.store(-60.0f, std::memory_order_relaxed);
}

void R2ROpticalTubeCompressorAudioProcessor::releaseResources()
{
    compressor.reset();
}

bool R2ROpticalTubeCompressorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    return (input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo())
        && output == input;
}

void R2ROpticalTubeCompressorAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto channels = buffer.getNumChannels();
    const auto samples = buffer.getNumSamples();
    if (channels == 0 || samples == 0)
        return;

    const auto enabled = parameters.getRawParameterValue("power")->load() > 0.5f;
    if (!enabled)
    {
        meterDb.store(-60.0f, std::memory_order_relaxed);
        return;
    }

    const auto reduction = parameters.getRawParameterValue("peakReduction")->load();
    const auto output = parameters.getRawParameterValue("outputGain")->load();
    const auto warmth = parameters.getRawParameterValue("tubeWarmth")->load() / 100.0f;
    const auto limit = parameters.getRawParameterValue("limitMode")->load() > 0.5f;

    const auto threshold = limit ? -3.0f : (-6.0f - reduction * 0.62f);
    const auto ratio = limit ? 20.0f : (2.0f + reduction * 0.16f);
    compressor.setThreshold(threshold);
    compressor.setRatio(ratio);
    compressor.setAttack(12.0f);
    compressor.setRelease(180.0f);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);

    const auto gainDb = (output - 25.0f) * 0.48f;
    const auto gain = juce::Decibels::decibelsToGain(gainDb);
    float peak = 0.0f;
    for (int channel = 0; channel < channels; ++channel)
    {
        auto* data = buffer.getWritePointer(channel);
        for (int i = 0; i < samples; ++i)
        {
            float value = data[i] * gain;
            if (warmth > 0.001f)
            {
                const float driven = std::tanh(value * (1.0f + warmth * 3.5f));
                value = value * (1.0f - warmth * 0.28f) + driven * (warmth * 0.28f);
            }
            data[i] = value;
            peak = juce::jmax(peak, std::abs(value));
        }
    }
    const auto db = peak > 1.0e-6f ? juce::Decibels::gainToDecibels(peak) : -60.0f;
    meterDb.store(juce::jlimit(-60.0f, 3.0f, db), std::memory_order_relaxed);
}

void R2ROpticalTubeCompressorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void R2ROpticalTubeCompressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* R2ROpticalTubeCompressorAudioProcessor::createEditor()
{
    return new R2ROpticalTubeCompressorAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new R2ROpticalTubeCompressorAudioProcessor();
}