#pragma once
#include <JuceHeader.h>
#include "OpticalTubeProcessor.h"

class OpticalTubeEditor final : public juce::AudioProcessorEditor
{
public:
    explicit OpticalTubeEditor(OpticalTubeProcessor&);
    ~OpticalTubeEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    class NeonLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    };
    class VUMeter final : public juce::Component, private juce::Timer
    {
    public:
        explicit VUMeter(OpticalTubeProcessor& p) : processor(p) { startTimerHz(30); }
        void paint(juce::Graphics&) override;
    private:
        void timerCallback() override { repaint(); }
        OpticalTubeProcessor& processor;
    };
    OpticalTubeProcessor& processor;
    NeonLookAndFeel neonLook;
    VUMeter meter;
    juce::Slider peakReduction, outputGain, tubeWarmth;
    juce::ToggleButton limitMode { "LIMIT" }, power { "ON" };
    juce::Label logo, title, peakLabel, outputLabel, warmthLabel, modeLabel;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> peakAttachment, outputAttachment, warmthAttachment;
    std::unique_ptr<ButtonAttachment> modeAttachment, powerAttachment;
    void styleKnob(juce::Slider&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpticalTubeEditor)
};
