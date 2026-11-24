#include "OpticalTubeEditor.h"
using namespace juce;

OpticalTubeEditor::OpticalTubeEditor(OpticalTubeProcessor& p)
    : AudioProcessorEditor(&p), processor(p), meter(p)
{
    setLookAndFeel(&metalLook);
    setSize(1100, 570);
    styleKnob(peakReduction); styleKnob(outputGain); styleKnob(tubeWarmth);
    for (auto* s : { &peakReduction, &outputGain, &tubeWarmth }) addAndMakeVisible(*s);
    for (auto* b : { &limitMode, &power }) { b->setClickingTogglesState(true); addAndMakeVisible(*b); }
    addAndMakeVisible(meter);
    peakAttachment = std::make_unique<SliderAttachment>(processor.parameters, "peakReduction", peakReduction);
    outputAttachment = std::make_unique<SliderAttachment>(processor.parameters, "outputGain", outputGain);
    warmthAttachment = std::make_unique<SliderAttachment>(processor.parameters, "tubeWarmth", tubeWarmth);
    modeAttachment = std::make_unique<ButtonAttachment>(processor.parameters, "limitMode", limitMode);
    powerAttachment = std::make_unique<ButtonAttachment>(processor.parameters, "power", power);

    auto label = [this](Label& l, const String& text, float size, bool red = false)
    {
        l.setText(text, dontSendNotification);
        l.setFont(Font(size, Font::bold));
        l.setColour(Label::textColourId, red ? Colour(0xffd62920) : Colours::black);
        l.setJustificationType(Justification::centred);
        addAndMakeVisible(l);
    };
    label(logo, "DJ R2R MUSIC", 29.0f);
    label(title, "OPTICAL TUBE  COMPRESSOR / LIMITER", 24.0f, true);
    label(peakLabel, "PEAK REDUCTION", 17.0f);
    label(outputLabel, "OUTPUT GAIN", 17.0f);
    label(warmthLabel, "TUBE WARMTH", 16.0f);
    label(modeLabel, "COMPRESS / LIMIT", 14.0f);
}

void OpticalTubeEditor::styleKnob(Slider& s)
{
    s.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(Slider::TextBoxBelow, false, 76, 22);
    s.setColour(Slider::textBoxTextColourId, Colours::black);
    s.setColour(Slider::textBoxBackgroundColourId, Colour(0xffd6d2ca));
    s.setColour(Slider::textBoxOutlineColourId, Colour(0xff77736e));
}

void OpticalTubeEditor::MetalLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int w, int h,
    float pos, float start, float end, Slider&)
{
    const float d = jmin(w, h) * 0.68f, cx = x + w * 0.5f, cy = y + h * 0.43f, r = d * 0.5f;
    const float angle = start + pos * (end - start);
    g.setColour(Colour(0x44000000)); g.fillEllipse(cx-r+3, cy-r+5, d, d);
    g.setGradientFill(ColourGradient(Colour(0xff55585b), cx-r, cy-r, Colour(0xff090a0b), cx+r, cy+r, false));
    g.fillEllipse(cx-r, cy-r, d, d);
    g.setColour(Colour(0xff17191b)); g.drawEllipse(cx-r, cy-r, d, d, 3.0f);
    g.setGradientFill(ColourGradient(Colour(0xfff1f1ef), cx-r*0.55f, cy-r*0.55f,
                                     Colour(0xff777a7c), cx+r*0.55f, cy+r*0.55f, false));
    g.fillEllipse(cx-r*0.70f, cy-r*0.70f, d*0.70f, d*0.70f);
    Path pointer; pointer.addRoundedRectangle(-2.2f, -r*0.88f, 4.4f, r*0.28f, 1.5f);
    g.setColour(Colour(0xff171717)); g.fillPath(pointer, AffineTransform::rotation(angle).translated(cx, cy));
}

void OpticalTubeEditor::VUMeter::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(Colour(0xff181614)); g.fillRoundedRectangle(b, 9.0f);
    auto face = b.reduced(13.0f);
    g.setGradientFill(ColourGradient(Colour(0xffffd477), face.getX(), face.getY(),
                                     Colour(0xffc77a20), face.getX(), face.getBottom(), false));
    g.fillRoundedRectangle(face, 5.0f);
    g.setColour(Colour(0xff2d1b08)); g.drawRoundedRectangle(face, 5.0f, 2.0f);
    const float cx = face.getCentreX(), cy = face.getBottom()-24.0f;
    const float radius = jmin(face.getWidth()*0.43f, face.getHeight()*0.88f);
    g.setColour(Colour(0xff17120c));
    for (int i=0; i<=23; ++i)
    {
        const float a = MathConstants<float>::pi*(1.12f+0.76f*i/23.0f);
        const float r1 = radius-(i%2==0 ? 15.0f : 9.0f);
        g.drawLine(cx+std::cos(a)*r1, cy+std::sin(a)*r1, cx+std::cos(a)*radius,
                   cy+std::sin(a)*radius, i%2==0 ? 2.2f : 1.2f);
        if (i%3==0)
            g.drawText(String(-20+i), Rectangle<int>((int)(cx+std::cos(a)*(radius-31)-15),
                (int)(cy+std::sin(a)*(radius-31)-9), 30, 18), Justification::centred);
    }
    const float gr = jlimit(0.0f, 20.0f, processor.getGainReductionDb());
    const float a = MathConstants<float>::pi*(1.88f-0.76f*gr/20.0f);
    g.setColour(Colour(0xff17120c));
    g.drawLine(cx, cy, cx+std::cos(a)*(radius-12), cy+std::sin(a)*(radius-12), 2.4f);
    g.fillEllipse(cx-5, cy-5, 10, 10);
    g.setFont(Font(24.0f, Font::bold));
    g.drawText("VU", face.withY(face.getY()+face.getHeight()*0.54f).withHeight(34), Justification::centred);
    g.setFont(Font(10.0f, Font::bold));
    g.drawText("GAIN REDUCTION (dB)", face.removeFromBottom(17), Justification::centred);
}

void OpticalTubeEditor::paint(Graphics& g)
{
    auto area = getLocalBounds();
    g.fillAll(Colour(0xffb9b8b5));
    g.setGradientFill(ColourGradient(Colour(0xfff1f0ec), 0, 0, Colour(0xffa7a6a3), 0, (float)getHeight(), false));
    g.fillRoundedRectangle(area.toFloat().reduced(5), 8.0f);
    g.setColour(Colour(0xff777674)); g.drawRoundedRectangle(area.toFloat().reduced(5), 8.0f, 2.0f);
    g.setColour(Colour(0x18ffffff));
    for (int yy=12; yy<getHeight()-18; yy+=4) g.drawHorizontalLine(yy, 12.0f, (float)getWidth()-12.0f);
    g.setColour(Colour(0xff171717)); g.fillRoundedRectangle(10.0f, 515.0f, 1080.0f, 43.0f, 5.0f);
    g.setColour(Colour(0xffd52b20)); g.drawLine(42.0f, 515.0f, 1058.0f, 515.0f, 2.0f);
    g.setColour(Colour(0xff252525));
    for (auto p : { Point<float>(28,28), Point<float>(1072,28), Point<float>(28,492), Point<float>(1072,492) })
    {
        g.fillEllipse(p.x-9,p.y-9,18,18);
        g.setColour(Colour(0xffaaa9a5));
        g.drawLine(p.x-5,p.y-5,p.x+5,p.y+5,1.4f); g.drawLine(p.x-5,p.y+5,p.x+5,p.y-5,1.4f);
        g.setColour(Colour(0xff252525));
    }
}

void OpticalTubeEditor::resized()
{
    logo.setBounds(34,48,190,55);
    peakReduction.setBounds(170,120,210,260); peakLabel.setBounds(155,385,240,32);
    meter.setBounds(410,88,320,210); title.setBounds(390,320,390,62);
    outputGain.setBounds(760,120,210,260); outputLabel.setBounds(755,385,220,32);
    warmthLabel.setBounds(930,55,145,26); tubeWarmth.setBounds(930,76,145,145);
    modeLabel.setBounds(30,425,180,24); limitMode.setBounds(45,450,140,42);
    power.setBounds(950,440,115,48);
}
