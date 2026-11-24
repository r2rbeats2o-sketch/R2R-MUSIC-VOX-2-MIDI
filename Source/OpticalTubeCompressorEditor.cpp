#include "OpticalTubeCompressorEditor.h"

namespace
{
    const juce::Colour metal(0xffc4c2be), dark(0xff171717), red(0xffd8201c),
        ink(0xff101010), meterGold(0xffffbd52);
}

void R2ROpticalTubeCompressorAudioProcessorEditor::MetalLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    const auto side = static_cast<float>(juce::jmin(width, height));
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                                static_cast<float>(width), static_cast<float>(height));
    const auto centre = bounds.getCentre();
    const auto radius = side * 0.37f;
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    for (int i = 0; i < 50; ++i)
    {
        const auto a = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * i / 49.0f;
        const auto inner = radius + side * 0.105f;
        const auto outer = inner + (i % 5 == 0 ? side * 0.055f : side * 0.027f);
        g.setColour(juce::Colour(0xff242424));
        g.drawLine(centre.x + std::cos(a) * inner, centre.y + std::sin(a) * inner,
                   centre.x + std::cos(a) * outer, centre.y + std::sin(a) * outer,
                   i % 5 == 0 ? 2.0f : 1.0f);
    }

    g.setColour(juce::Colour(0xff777777));
    g.fillEllipse(centre.x - radius - 5, centre.y - radius - 5,
                  (radius + 5) * 2, (radius + 5) * 2);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff333333),
        centre.x, centre.y - radius, juce::Colour(0xff050505),
        centre.x, centre.y + radius, false));
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2, radius * 2);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xfff0efeb),
        centre.x - radius * 0.35f, centre.y - radius * 0.55f,
        juce::Colour(0xff777775), centre.x + radius * 0.5f,
        centre.y + radius * 0.65f, true));
    g.fillEllipse(centre.x - radius * 0.73f, centre.y - radius * 0.73f,
                  radius * 1.46f, radius * 1.46f);
    g.setColour(juce::Colour(0xff4a4a4a));
    g.drawEllipse(centre.x - radius * 0.73f, centre.y - radius * 0.73f,
                  radius * 1.46f, radius * 1.46f, 1.0f);

    juce::Path pointer;
    pointer.addRoundedRectangle(-2.0f, -radius * 0.80f, 4.0f, radius * 0.42f, 1.5f);
    g.setColour(red);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
    g.setColour(juce::Colour(0xffd8d8d8));
    g.fillEllipse(centre.x - 4, centre.y - 4, 8, 8);
}

R2ROpticalTubeCompressorAudioProcessorEditor::R2ROpticalTubeCompressorAudioProcessorEditor(
    R2ROpticalTubeCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&metalLook);
    setSize(1180, 590);

    auto setupKnob = [this](juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        slider.setColour(juce::Slider::textBoxTextColourId, ink);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(slider);
    };
    setupKnob(peakReduction);
    setupKnob(outputGain);
    setupKnob(tubeWarmth);

    peakReduction.setRange(0.0, 50.0, 0.01);
    outputGain.setRange(0.0, 50.0, 0.01);
    tubeWarmth.setRange(0.0, 100.0, 0.01);
    peakReduction.setValue(18.0);
    outputGain.setValue(25.0);
    tubeWarmth.setValue(22.0);

    for (auto* button : { &limitMode, &power })
    {
        button->setColour(juce::ToggleButton::textColourId, ink);
        addAndMakeVisible(*button);
    }
    limitMode.setButtonText("LIMIT");
    power.setButtonText("ON");
    limitMode.setClickingTogglesState(true);
    power.setClickingTogglesState(true);

    auto setupLabel = [this](juce::Label& label, const juce::String& text, int size)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(juce::FontOptions(static_cast<float>(size), juce::Font::bold)));
        label.setColour(juce::Label::textColourId, ink);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };
    setupLabel(peakLabel, "PEAK REDUCTION", 17);
    setupLabel(outputLabel, "OUTPUT GAIN", 17);
    setupLabel(warmthLabel, "TUBE WARMTH", 16);
    setupLabel(modeLabel, "COMPRESS", 15);
    setupLabel(titleLabel, "DJ R2R  OPTICAL TUBE", 27);
    titleLabel.setColour(juce::Label::textColourId, red);
    setupLabel(subtitleLabel, "COMPRESSOR / LIMITER", 21);

    peakAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "peakReduction", peakReduction);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "outputGain", outputGain);
    warmthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "tubeWarmth", tubeWarmth);
    limitAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, "limitMode", limitMode);
    powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, "power", power);

    startTimerHz(30);
}

R2ROpticalTubeCompressorAudioProcessorEditor::~R2ROpticalTubeCompressorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void R2ROpticalTubeCompressorAudioProcessorEditor::timerCallback()
{
    displayedMeterDb = processor.getMeterDb();
    repaint(360, 75, 460, 245);
}

void R2ROpticalTubeCompressorAudioProcessorEditor::drawScrew(
    juce::Graphics& g, float x, float y) const
{
    g.setColour(juce::Colour(0xff555555));
    g.fillEllipse(x - 11, y - 11, 22, 22);
    g.setColour(juce::Colour(0xffe5e3df));
    g.drawEllipse(x - 9, y - 9, 18, 18, 1.5f);
    g.setColour(juce::Colour(0xff242424));
    g.drawLine(x - 5, y - 5, x + 5, y + 5, 2.2f);
    g.drawLine(x - 5, y + 5, x + 5, y - 5, 2.2f);
}

void R2ROpticalTubeCompressorAudioProcessorEditor::drawVuMeter(
    juce::Graphics& g, juce::Rectangle<float> area) const
{
    g.setColour(juce::Colour(0xff111111));
    g.fillRoundedRectangle(area, 10.0f);
    auto face = area.reduced(15.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffd078),
        face.getCentreX(), face.getY(), meterGold, face.getCentreX(),
        face.getBottom(), false));
    g.fillRoundedRectangle(face, 7.0f);

    const auto centre = juce::Point<float>(face.getCentreX(), face.getBottom() - 4.0f);
    const auto radius = face.getWidth() * 0.39f;
    const auto start = juce::MathConstants<float>::pi * 1.15f;
    const auto end = juce::MathConstants<float>::pi * 1.85f;
    g.setColour(ink);
    g.drawArc(centre.x - radius, centre.y - radius, radius * 2, radius * 2,
              start, end, true, 4.0f);

    for (int i = 0; i <= 20; ++i)
    {
        const auto t = i / 20.0f;
        const auto a = start + t * (end - start);
        const auto inner = radius - (i % 2 == 0 ? 17.0f : 10.0f);
        g.drawLine(centre.x + std::cos(a) * inner, centre.y + std::sin(a) * inner,
                   centre.x + std::cos(a) * radius, centre.y + std::sin(a) * radius,
                   i % 2 == 0 ? 2.0f : 1.0f);
        if (i % 2 == 0)
        {
            const auto db = -20 + i * 1.15f;
            g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
            g.drawText(juce::String(static_cast<int>(std::round(db))),
                juce::Rectangle<float>(centre.x + std::cos(a) * (radius - 42.0f) - 15.0f,
                    centre.y + std::sin(a) * (radius - 42.0f) - 9.0f, 30.0f, 18.0f),
                juce::Justification::centred);
        }
    }

    const auto normalised = juce::jlimit(0.0f, 1.0f, (displayedMeterDb + 20.0f) / 23.0f);
    const auto needleAngle = start + normalised * (end - start);
    g.setColour(juce::Colour(0xff151515));
    g.drawLine(centre.x, centre.y,
        centre.x + std::cos(needleAngle) * (radius - 12.0f),
        centre.y + std::sin(needleAngle) * (radius - 12.0f), 2.6f);
    g.fillEllipse(centre.x - 6.0f, centre.y - 6.0f, 12.0f, 12.0f);
    g.setFont(juce::Font(juce::FontOptions(25.0f, juce::Font::bold)));
    g.drawText("VU", face.withY(face.getCentreY() + 12.0f).withHeight(35.0f),
               juce::Justification::centred);
}

void R2ROpticalTubeCompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xff101010));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffeeeeeb),
        0.0f, 0.0f, metal, 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds.reduced(5.0f), 12.0f);

    for (int y = 10; y < getHeight() - 10; y += 3)
    {
        g.setColour(juce::Colours::white.withAlpha(0.055f));
        g.drawHorizontalLine(y, 8.0f, static_cast<float>(getWidth() - 8));
    }

    g.setColour(juce::Colour(0xff171717));
    g.fillRoundedRectangle(8.0f, 8.0f, bounds.getWidth() - 16.0f, 38.0f, 5.0f);
    g.setColour(red);
    g.fillRect(45.0f, bounds.getBottom() - 8.0f, bounds.getWidth() - 90.0f, 2.0f);

    drawScrew(g, 28, 28); drawScrew(g, getWidth() - 28, 28);
    drawScrew(g, 28, getHeight() - 28); drawScrew(g, getWidth() - 28, getHeight() - 28);

    g.setColour(ink);
    g.setFont(juce::Font(juce::FontOptions(31.0f, juce::Font::bold | juce::Font::italic)));
    g.drawText("DJ R2R", 35, 88, 190, 40, juce::Justification::centredLeft);
    g.setColour(red);
    g.setFont(juce::Font(juce::FontOptions(29.0f, juce::Font::bold | juce::Font::italic)));
    g.drawText("Music", 73, 120, 140, 36, juce::Justification::centredLeft);

    drawVuMeter(g, { 390.0f, 65.0f, 405.0f, 225.0f });

    g.setColour(ink);
    g.drawLine(390.0f, 500.0f, 455.0f, 500.0f, 3.0f);
    g.drawLine(730.0f, 500.0f, 795.0f, 500.0f, 3.0f);
}

void R2ROpticalTubeCompressorAudioProcessorEditor::resized()
{
    peakReduction.setBounds(185, 160, 190, 210);
    peakLabel.setBounds(155, 365, 250, 32);
    outputGain.setBounds(825, 160, 190, 210);
    outputLabel.setBounds(805, 365, 230, 32);
    tubeWarmth.setBounds(1000, 70, 145, 150);
    warmthLabel.setBounds(995, 40, 150, 25);
    limitMode.setBounds(48, 340, 120, 42);
    modeLabel.setBounds(42, 390, 130, 25);
    power.setBounds(1030, 400, 110, 42);
    titleLabel.setBounds(430, 440, 340, 38);
    subtitleLabel.setBounds(430, 478, 340, 32);
}