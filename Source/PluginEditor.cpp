#include "PluginEditor.h"

namespace {
const juce::Colour bg(0xff090b13), panel(0xff141827), violet(0xffa855f7),
                   cyan(0xff22d3ee), lime(0xffc6f43a), muted(0xff8992a8);
}
R2RMusicVox2MidiAudioProcessorEditor::R2RMusicVox2MidiAudioProcessorEditor(R2RMusicVox2MidiAudioProcessor& p)
 : AudioProcessorEditor(&p), processor(p)
{
    setSize(760, 500);
    titleLabel.setText("R2R MUSIC  /  VOX 2 MIDI", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(25.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    statusLabel.setText("READY  •  MONOPHONIC PITCH TRACKING", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, lime);
    addAndMakeVisible(statusLabel);
    sensitivityLabel.setText("PITCH CONFIDENCE", juce::dontSendNotification);
    sensitivityLabel.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(sensitivityLabel);
    sensitivitySlider.setRange(0.05, 0.95, 0.01);
    sensitivitySlider.setValue(0.35);
    sensitivitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 26);
    sensitivitySlider.setColour(juce::Slider::trackColourId, violet);
    sensitivitySlider.setColour(juce::Slider::thumbColourId, cyan);
    sensitivitySlider.onValueChange = [this] { processor.setSensitivity(float(sensitivitySlider.getValue())); };
    addAndMakeVisible(sensitivitySlider);

    for (auto* b : { &loadButton, &exportButton }) {
        b->setColour(juce::TextButton::buttonColourId, panel);
        b->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        b->setColour(juce::TextButton::buttonOnColourId, violet);
        addAndMakeVisible(b);
    }
    loadButton.onClick = [this] {
        chooser = std::make_unique<juce::FileChooser>("Choose a vocal or audio file", juce::File{}, "*.wav;*.aiff;*.aif;*.flac;*.mp3");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                loadedFile = fc.getResult();
                if (loadedFile.existsAsFile())
                    statusLabel.setText("LOADED  •  " + loadedFile.getFileName(), juce::dontSendNotification);
            });
    };
    exportButton.onClick = [this] {
        if (!loadedFile.existsAsFile()) {
            statusLabel.setText("LOAD AN AUDIO FILE FIRST", juce::dontSendNotification);
            return;
        }
        chooser = std::make_unique<juce::FileChooser>("Save MIDI file", loadedFile.withFileExtension(".mid"), "*.mid");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                auto destination = fc.getResult().withFileExtension(".mid");
                if (destination == juce::File{}) return;
                const bool ok = processor.convertAudioFileToMidi(loadedFile, destination);
                statusLabel.setText(ok ? "MIDI EXPORTED  •  " + destination.getFileName()
                                        : "CONVERSION FAILED  •  CHECK AUDIO FORMAT",
                                    juce::dontSendNotification);
            });
    };
}
void R2RMusicVox2MidiAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(panel); g.fillRoundedRectangle(24, 90, 712, 300, 18.0f);
    g.setColour(violet); g.fillRoundedRectangle(24, 90, 6, 300, 3.0f);
    g.setColour(cyan); g.drawRoundedRectangle(24, 90, 712, 300, 18.0f, 1.0f);
    g.setColour(lime); g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText("DJ R2R  •  VOCAL INTELLIGENCE", 48, 112, 400, 22, juce::Justification::left);
    g.setColour(muted); g.setFont(juce::Font(juce::FontOptions(13.0f)));
    g.drawText("REAL-TIME MIDI  /  AUDIO FILE CONVERSION", 48, 150, 620, 24, juce::Justification::left);
    for (int i = 0; i < 54; ++i) {
        const float x = 50.0f + i * 12.0f;
        const float h = 18.0f + float((i * 37) % 53);
        g.setColour(i % 3 == 0 ? cyan : (i % 3 == 1 ? lime : violet));
        g.drawRoundedRectangle(x, 215.0f - h * 0.5f, 4.0f, h, 2.0f, 0.0f);
    }
    g.setColour(muted);
    g.drawText("Monophonic pitch extraction • MIDI note events • WAV / AIFF / FLAC / MP3 input",
               48, 330, 650, 24, juce::Justification::left);
    g.setColour(violet); g.fillRoundedRectangle(24, 420, 712, 1, 1.0f);
}
void R2RMusicVox2MidiAudioProcessorEditor::resized()
{
    titleLabel.setBounds(28, 20, 600, 42);
    statusLabel.setBounds(30, 58, 690, 24);
    loadButton.setBounds(48, 270, 190, 48);
    exportButton.setBounds(252, 270, 190, 48);
    sensitivityLabel.setBounds(48, 445, 160, 24);
    sensitivitySlider.setBounds(210, 440, 330, 34);
}
