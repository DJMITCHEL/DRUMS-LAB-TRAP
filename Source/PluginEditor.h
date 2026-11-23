#pragma once
#include "PluginProcessor.h"

/** Estilo visual: negro + amarillo. */
class MdlLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MdlLookAndFeel();
};

/** Pad visual (ilumina al sonar). */
class PadComponent : public juce::Component
{
public:
    std::function<void (int)> onPress;
    int index = 0;
    juce::String title, noteText;
    bool selected = false, dimmed = false;
    float glow = 0.f;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override { if (onPress) onPress (index); }
};

class MichelDrumLabEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MichelDrumLabEditor (MichelDrumLabProcessor&);
    ~MichelDrumLabEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void timerCallback() override;
    void selectPad (int pad);
    void refreshPadTexts();
    void refreshSampleBox();
    void refreshKitLists();
    void setupKnob (Knob& k, const juce::String& text, const char* paramName, const juce::String& suffix);

    MichelDrumLabProcessor& processor;
    MdlLookAndFeel laf;

    std::array<PadComponent, mdl::kNumPads> pads;
    int selectedPad = 0;

    static constexpr int kNumKnobs = 8;
    std::array<Knob, kNumKnobs> knobs;

    juce::ComboBox sampleBox;
    juce::ToggleButton muteButton { "MUTE" }, soloButton { "SOLO" };
    juce::Slider noteSlider;
    juce::Label  noteLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAtt, soloAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noteAtt;

    juce::ComboBox kitBox, presetBox;
    juce::TextEditor nameEditor;
    juce::TextButton saveKitButton { "SAVE KIT" }, savePresetButton { "SAVE PRESET" };
    juce::Label kitLabel, presetLabel;

    juce::Slider masterSlider;
    juce::Label  masterLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterAtt;
    float meterLevel = 0.f;
    bool updatingUi = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MichelDrumLabEditor)
};
