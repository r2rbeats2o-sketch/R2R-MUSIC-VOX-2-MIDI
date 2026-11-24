#pragma once
#include <JuceHeader.h>
#include "OpticalTubeCompressorProcessor.h"

class R2ROpticalTubeCompressorAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                           private juce::Timer
{
public:
    explicit R2ROpticalTubeCompressorAudioProcessorEditor(R2ROpticalTubeCompressorAudioProcessor&);
    ~R2ROpticalTubeCompressorAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class MetalLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float,
                              float, float, juce::Slider&) override;
    };

    void timerCallback() override;
    void drawScrew(juce::Graphics&, float, float) const;
    void drawVuMeter(juce::Graphics&, juce::Rectangle<float>) const;

    R2ROpticalTubeCompressorAudioProcessor& processor;
    MetalLookAndFeel metalLook;
    juce::Slider peakReduction, outputGain, tubeWarmth;
    juce::ToggleButton limitMode, power;
    juce::Label peakLabel, outputLabel, warmthLabel, modeLabel, titleLabel, subtitleLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakAttachment,
        outputAttachment, warmthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limitAttachment,
        powerAttachment;
    float displayedMeterDb = -60.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2ROpticalTubeCompressorAudioProcessorEditor)
};