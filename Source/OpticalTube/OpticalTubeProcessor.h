#pragma once
#include <JuceHeader.h>
#include <atomic>

class OpticalTubeProcessor final : public juce::AudioProcessor
{
public:
    OpticalTubeProcessor();
    ~OpticalTubeProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "DJ R2R Optical Tube Compressor"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState parameters;
    float getGainReductionDb() const noexcept { return gainReductionDb.load(); }
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float envelope = 0.0f, attackCoeff = 0.0f, releaseCoeff = 0.0f;
    std::atomic<float> gainReductionDb { 0.0f };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpticalTubeProcessor)
};
