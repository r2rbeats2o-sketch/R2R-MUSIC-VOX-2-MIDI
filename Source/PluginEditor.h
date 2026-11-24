#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class R2RMusicVox2MidiAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
 explicit R2RMusicVox2MidiAudioProcessorEditor(R2RMusicVox2MidiAudioProcessor&);
 void paint(juce::Graphics&) override; void resized() override;
private:
 void importAudio(); void analyse();
 R2RMusicVox2MidiAudioProcessor& processor;
 juce::TextButton load{"＋ IMPORT VOCAL"}, analyseButton{"ANALYSE NOTES"}, exportButton{"↓ EXPORT MIDI"};
 juce::ComboBox chord; juce::Slider octave, sensitivity;
 juce::Label title,status,chordLabel,octaveLabel,sensitivityLabel;
 std::unique_ptr<juce::FileChooser> chooser; juce::File audioFile;
 juce::AudioBuffer<float> waveform; juce::Array<int> notes;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2RMusicVox2MidiAudioProcessorEditor)
};