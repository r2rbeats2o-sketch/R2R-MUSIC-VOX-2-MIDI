#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class R2RMusicVox2MidiAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit R2RMusicVox2MidiAudioProcessorEditor(R2RMusicVox2MidiAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    R2RMusicVox2MidiAudioProcessor& processor;
    juce::TextButton loadButton { "LOAD AUDIO  +" };
    juce::TextButton exportButton { "EXPORT MIDI  ↓" };
    juce::Slider sensitivitySlider;
    juce::Label titleLabel, statusLabel, sensitivityLabel;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::File loadedFile;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2RMusicVox2MidiAudioProcessorEditor)
};
