#include "OpticalTubeEditor.h"
using namespace juce;

namespace
{
    const Colour bg(0xff080d16), panel(0xff101a29), cyan(0xff00eaff),
                 red(0xffff315b), muted(0xff91a8bd), amber(0xffffc45e);
}

OpticalTubeEditor::OpticalTubeEditor(OpticalTubeProcessor& p)
    : AudioProcessorEditor(&p), processor(p), meter(p)
{
    setLookAndFeel(&neonLook);
    setSize(1100, 570);
    styleKnob(peakReduction); styleKnob(outputGain); styleKnob(tubeWarmth);
    for (auto* s : { &peakReduction, &outputGain, &tubeWarmth }) addAndMakeVisible(*s);
    for (auto* b : { &limitMode, &power })
    {
        b->setClickingTogglesState(true);
        b->setColour(ToggleButton::textColourId, Colours::white);
        b->setColour(ToggleButton::tickColourId, cyan);
        b->setColour(ToggleButton::tickDisabledColourId, Colour(0xff34475b));
        addAndMakeVisible(*b);
    }
    addAndMakeVisible(meter);
    peakAttachment = std::make_unique<SliderAttachment>(processor.parameters, "peakReduction", peakReduction);
    outputAttachment = std::make_unique<SliderAttachment>(processor.parameters, "outputGain", outputGain);
    warmthAttachment = std::make_unique<SliderAttachment>(processor.parameters, "tubeWarmth", tubeWarmth);
    modeAttachment = std::make_unique<ButtonAttachment>(processor.parameters, "limitMode", limitMode);
    powerAttachment = std::make_unique<ButtonAttachment>(processor.parameters, "power", power);

    auto label = [this](Label& l, const String& text, float size, Colour colour)
    {
        l.setText(text, dontSendNotification);
        l.setFont(Font(size, Font::bold));
        l.setColour(Label::textColourId, colour);
        l.setJustificationType(Justification::centred);
        addAndMakeVisible(l);
    };
    label(logo, "DJ R2R MUSIC", 27.0f, cyan);
    label(title, "OPTICAL TUBE  /  COMPRESSOR", 23.0f, Colours::white);
    label(peakLabel, "PEAK REDUCTION", 16.0f, cyan);
    label(outputLabel, "OUTPUT GAIN", 16.0f, cyan);
    label(warmthLabel, "TUBE WARMTH", 15.0f, amber);
    label(modeLabel, "DYNAMICS MODE", 13.0f, muted);
}

OpticalTubeEditor::~OpticalTubeEditor()
{
    setLookAndFeel(nullptr);
}

void OpticalTubeEditor::styleKnob(Slider& s)
{
    s.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(Slider::TextBoxBelow, false, 78, 22);
    s.setColour(Slider::textBoxTextColourId, cyan);
    s.setColour(Slider::textBoxBackgroundColourId, Colour(0xff101a29));
    s.setColour(Slider::textBoxOutlineColourId, Colour(0xff176078));
    s.setColour(Slider::rotarySliderFillColourId, cyan);
    s.setColour(Slider::rotarySliderOutlineColourId, Colour(0xff263b50));
}

void OpticalTubeEditor::NeonLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int w, int h,
    float pos, float start, float end, Slider&)
{
    const float d = jmin(w, h) * 0.67f;
    const float cx = x + w * 0.5f, cy = y + h * 0.42f, r = d * 0.5f;
    const float angle = start + pos * (end - start);
    const Rectangle<float> dial(cx-r, cy-r, d, d);

    g.setColour(Colour(0x9900dfff));
    g.drawEllipse(dial.expanded(5.0f), 2.0f);
    g.setColour(Colour(0x3300eaff));
    g.drawEllipse(dial.expanded(9.0f), 1.0f);

    g.setColour(Colour(0xff02050a));
    g.fillEllipse(dial.expanded(1.0f));
    g.setGradientFill(ColourGradient(Colour(0xff34485d), cx-r, cy-r,
                                     Colour(0xff070b12), cx+r, cy+r, false));
    g.fillEllipse(dial);
    g.setColour(Colour(0xff526b80));
    g.drawEllipse(dial, 2.0f);

    auto inner = dial.reduced(d*0.19f);
    g.setGradientFill(ColourGradient(Colour(0xff24374a), inner.getX(), inner.getY(),
                                     Colour(0xff070d16), inner.getRight(), inner.getBottom(), false));
    g.fillEllipse(inner);
    g.setColour(Colour(0xff46647c));
    g.drawEllipse(inner, 1.0f);

    Path arc;
    arc.addCentredArc(cx, cy, r+5.0f, r+5.0f, 0.0f, start, angle, true);
    g.setColour(cyan);
    g.strokePath(arc, PathStrokeType(3.2f, PathStrokeType::curved, PathStrokeType::rounded));

    Path pointer;
    pointer.addRoundedRectangle(-2.0f, -r*0.83f, 4.0f, r*0.30f, 2.0f);
    g.setColour(red);
    g.fillPath(pointer, AffineTransform::rotation(angle).translated(cx, cy));
    g.setColour(Colour(0xffd6faff));
    g.fillEllipse(cx-3.0f, cy-3.0f, 6.0f, 6.0f);
}

void OpticalTubeEditor::VUMeter::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(Colour(0xff02060c));
    g.fillRoundedRectangle(b, 10.0f);
    g.setColour(Colour(0xff00cfe8));
    g.drawRoundedRectangle(b.reduced(1.5f), 10.0f, 2.0f);
    g.setColour(Colour(0x3300dfff));
    g.drawRoundedRectangle(b.expanded(3.0f), 12.0f, 2.0f);

    auto face = b.reduced(12.0f);
    g.setGradientFill(ColourGradient(Colour(0xffffd987), face.getX(), face.getY(),
                                     Colour(0xff9c4c16), face.getX(), face.getBottom(), false));
    g.fillRoundedRectangle(face, 5.0f);
    g.setColour(Colour(0xff5b2d0c));
    g.drawRoundedRectangle(face, 5.0f, 1.5f);

    const float cx = face.getCentreX(), cy = face.getBottom()-23.0f;
    const float radius = jmin(face.getWidth()*0.43f, face.getHeight()*0.87f);
    for (int i=0; i<=23; ++i)
    {
        const float a = MathConstants<float>::pi*(1.12f+0.76f*i/23.0f);
        const float r1 = radius-(i%2==0 ? 14.0f : 8.0f);
        g.setColour(i > 17 ? Colour(0xffbd1f20) : Colour(0xff25180e));
        g.drawLine(cx+std::cos(a)*r1, cy+std::sin(a)*r1,
                   cx+std::cos(a)*radius, cy+std::sin(a)*radius, i%2==0 ? 2.0f : 1.0f);
        if (i%3==0)
        {
            g.setFont(Font(12.0f, Font::bold));
            g.drawText(String(-20+i), Rectangle<int>((int)(cx+std::cos(a)*(radius-29)-15),
                (int)(cy+std::sin(a)*(radius-29)-8), 30, 16), Justification::centred);
        }
    }
    const float gr = jlimit(0.0f, 20.0f, processor.getGainReductionDb());
    const float needle = MathConstants<float>::pi*(1.88f-0.76f*gr/20.0f);
    g.setColour(Colour(0xff24120a));
    g.drawLine(cx, cy, cx+std::cos(needle)*(radius-11.0f),
               cy+std::sin(needle)*(radius-11.0f), 2.3f);
    g.fillEllipse(cx-4.5f, cy-4.5f, 9.0f, 9.0f);
    g.setFont(Font(23.0f, Font::bold));
    g.drawText("VU", face.withY(face.getY()+face.getHeight()*0.54f).withHeight(31), Justification::centred);
    g.setFont(Font(9.0f, Font::bold));
    g.drawText("GAIN REDUCTION  /  dB", face.removeFromBottom(16), Justification::centred);
}

void OpticalTubeEditor::paint(Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(bg);
    g.setGradientFill(ColourGradient(Colour(0xff14263a), 0.0f, 0.0f,
                                     Colour(0xff070b13), 0.0f, (float)getHeight(), false));
    g.fillRoundedRectangle(bounds.reduced(5.0f), 13.0f);

    g.setColour(Colour(0xff00dff5));
    g.drawRoundedRectangle(bounds.reduced(5.0f), 13.0f, 2.0f);
    g.setColour(Colour(0x2200eaff));
    g.drawRoundedRectangle(bounds.reduced(10.0f), 10.0f, 1.0f);

    // Dark inset control deck and subtle neon divider.
    g.setColour(panel);
    g.fillRoundedRectangle(20.0f, 105.0f, 1060.0f, 365.0f, 12.0f);
    g.setColour(Colour(0xff1c5268));
    g.drawRoundedRectangle(20.0f, 105.0f, 1060.0f, 365.0f, 12.0f, 1.0f);

    g.setColour(cyan.withAlpha(0.55f));
    g.drawLine(42.0f, 92.0f, 1058.0f, 92.0f, 1.5f);
    g.setColour(red);
    g.drawLine(42.0f, 515.0f, 1058.0f, 515.0f, 2.0f);

    // Screw heads and lower black equipment rail.
    g.setColour(Colour(0xff03070d));
    g.fillRoundedRectangle(10.0f, 520.0f, 1080.0f, 38.0f, 6.0f);
    g.setColour(Colour(0xff39546a));
    for (auto p : { Point<float>(28,28), Point<float>(1072,28),
                    Point<float>(28,492), Point<float>(1072,492) })
    {
        g.fillEllipse(p.x-8.0f,p.y-8.0f,16.0f,16.0f);
        g.setColour(Colour(0xff8aa2b5));
        g.drawLine(p.x-4,p.y-4,p.x+4,p.y+4,1.2f);
        g.drawLine(p.x-4,p.y+4,p.x+4,p.y-4,1.2f);
        g.setColour(Colour(0xff39546a));
    }

    g.setColour(Colour(0xff718da4));
    g.setFont(Font(10.0f, Font::bold));
    g.drawText("R2R  /  ANALOG-INSPIRED DYNAMICS", 34, 535, 350, 14, Justification::centredLeft);
    g.setColour(cyan);
    g.drawText("DARK NEON SERIES  //  VST3", 760, 535, 300, 14, Justification::centredRight);
}

void OpticalTubeEditor::resized()
{
    logo.setBounds(35, 34, 245, 48);
    peakReduction.setBounds(105, 145, 225, 245);
    peakLabel.setBounds(100, 390, 235, 30);
    meter.setBounds(390, 135, 320, 215);
    title.setBounds(350, 365, 400, 45);
    outputGain.setBounds(770, 145, 225, 245);
    outputLabel.setBounds(765, 390, 235, 30);
    warmthLabel.setBounds(895, 32, 175, 25);
    tubeWarmth.setBounds(920, 55, 125, 115);
    modeLabel.setBounds(38, 430, 175, 22);
    limitMode.setBounds(45, 452, 150, 38);
    power.setBounds(930, 445, 130, 45);
}
