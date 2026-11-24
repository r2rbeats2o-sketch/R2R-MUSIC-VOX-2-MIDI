#pragma once
#include <JuceHeader.h>

class R2RMusicVox2MidiAudioProcessor final : public juce::AudioProcessor
{
public:
    R2RMusicVox2MidiAudioProcessor();
    ~R2RMusicVox2MidiAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "R2R MUSIC VOX 2 MIDI"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    bool convertAudioFileToMidi(const juce::File& input, const juce::File& output);
    void setMidiChannel(int channel) { midiChannel = juce::jlimit(1, 16, channel); }
    void setSensitivity(float value) { sensitivity = juce::jlimit(0.05f, 0.95f, value); }

private:
    float detectPitch(const float* data, int count, double rate, float& confidence) const;
    double currentSampleRate = 44100.0;
    int midiChannel = 1;
    int activeNote = -1;
    float sensitivity = 0.35f;
    juce::AudioBuffer<float> monoScratch;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2RMusicVox2MidiAudioProcessor)
};
