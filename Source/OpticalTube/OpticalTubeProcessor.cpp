#include "OpticalTubeProcessor.h"
#include "OpticalTubeEditor.h"

OpticalTubeProcessor::OpticalTubeProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "R2R_OPTICAL_TUBE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout OpticalTubeProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<AudioParameterFloat>("peakReduction", "Peak Reduction",
        NormalisableRange<float>(0.0f, 50.0f, 0.1f), 22.0f));
    layout.add(std::make_unique<AudioParameterFloat>("outputGain", "Output Gain",
        NormalisableRange<float>(0.0f, 50.0f, 0.1f), 25.0f));
    layout.add(std::make_unique<AudioParameterFloat>("tubeWarmth", "Tube Warmth",
        NormalisableRange<float>(0.0f, 100.0f, 0.1f), 18.0f));
    layout.add(std::make_unique<AudioParameterBool>("limitMode", "Limit", false));
    layout.add(std::make_unique<AudioParameterBool>("power", "Power", true));
    return layout;
}

void OpticalTubeProcessor::prepareToPlay(double sampleRate, int)
{
    const auto sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    envelope = 0.0f;
    attackCoeff = std::exp(-1.0f / (0.010f * static_cast<float>(sr)));
    releaseCoeff = std::exp(-1.0f / (0.180f * static_cast<float>(sr)));
}

void OpticalTubeProcessor::releaseResources() {}

bool OpticalTubeProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void OpticalTubeProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = getTotalNumInputChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, samples);

    if (parameters.getRawParameterValue("power")->load() < 0.5f)
    {
        gainReductionDb.store(0.0f);
        return;
    }

    const float reduction = parameters.getRawParameterValue("peakReduction")->load();
    const float gainControl = parameters.getRawParameterValue("outputGain")->load();
    const float warmth = parameters.getRawParameterValue("tubeWarmth")->load() * 0.01f;
    const bool limiter = parameters.getRawParameterValue("limitMode")->load() > 0.5f;
    const float thresholdDb = -reduction * 0.60f;
    const float ratio = limiter ? 20.0f : 4.0f;
    const float makeup = juce::Decibels::decibelsToGain((gainControl - 25.0f) * 0.24f);
    float maxGr = 0.0f;

    for (int i = 0; i < samples; ++i)
    {
        float detector = 0.0f;
        for (int ch = 0; ch < channels; ++ch)
            detector = juce::jmax(detector, std::abs(buffer.getSample(ch, i)));
        const float coeff = detector > envelope ? attackCoeff : releaseCoeff;
        envelope = coeff * envelope + (1.0f - coeff) * detector;
        const float levelDb = juce::Decibels::gainToDecibels(juce::jmax(envelope, 1.0e-8f));
        const float overDb = juce::jmax(0.0f, levelDb - thresholdDb);
        const float grDb = overDb * (1.0f - 1.0f / ratio);
        maxGr = juce::jmax(maxGr, grDb);
        const float compGain = juce::Decibels::decibelsToGain(-grDb);

        for (int ch = 0; ch < channels; ++ch)
        {
            float x = buffer.getSample(ch, i) * compGain;
            if (warmth > 0.001f)
            {
                const float saturated = std::tanh(x * (1.0f + warmth * 3.5f));
                x += (saturated - x) * warmth * 0.65f;
            }
            buffer.setSample(ch, i, x * makeup);
        }
    }
    gainReductionDb.store(maxGr);
}

juce::AudioProcessorEditor* OpticalTubeProcessor::createEditor()
{
    return new OpticalTubeEditor(*this);
}

void OpticalTubeProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void OpticalTubeProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

// JUCE plugin entry point for this independent VST3 target.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OpticalTubeProcessor();
}
