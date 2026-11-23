#include "PluginEditor.h"

using namespace mdl;

// ---------------------------------------------------------------- LookAndFeel
MdlLookAndFeel::MdlLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, kBlack);
    setColour (juce::Slider::rotarySliderFillColourId, kYellow);
    setColour (juce::Slider::rotarySliderOutlineColourId, kYellowDim);
    setColour (juce::Slider::thumbColourId, kYellow);
    setColour (juce::Slider::trackColourId, kYellow);
    setColour (juce::Slider::backgroundColourId, kYellowDim);
    setColour (juce::Slider::textBoxTextColourId, kYellow);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, kYellow);
    setColour (juce::ComboBox::backgroundColourId, kPanel);
    setColour (juce::ComboBox::textColourId, kYellow);
    setColour (juce::ComboBox::outlineColourId, kYellowDim);
    setColour (juce::ComboBox::arrowColourId, kYellow);
    setColour (juce::PopupMenu::backgroundColourId, kPanel);
    setColour (juce::PopupMenu::textColourId, kYellow);
    setColour (juce::PopupMenu::headerTextColourId, juce::Colours::white);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, kYellow);
    setColour (juce::PopupMenu::highlightedTextColourId, kBlack);
    setColour (juce::TextButton::buttonColourId, kPanel);
    setColour (juce::TextButton::textColourOffId, kYellow);
    setColour (juce::TextButton::buttonOnColourId, kYellow);
    setColour (juce::TextButton::textColourOnId, kBlack);
    setColour (juce::ToggleButton::textColourId, kYellow);
    setColour (juce::ToggleButton::tickColourId, kYellow);
    setColour (juce::ToggleButton::tickDisabledColourId, kYellowDim);
    setColour (juce::TextEditor::backgroundColourId, kPanel);
    setColour (juce::TextEditor::textColourId, kYellow);
    setColour (juce::TextEditor::outlineColourId, kYellowDim);
    setColour (juce::TextEditor::focusedOutlineColourId, kYellow);
}

// ---------------------------------------------------------------- Pad
void PadComponent::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (3.f);
    g.setColour (juce::Colour (0xff121212));
    g.fillRoundedRectangle (b, 10.f);

    if (glow > 0.01f)
    {
        g.setColour (kYellow.withAlpha (juce::jmin (1.f, glow) * 0.9f));
        g.fillRoundedRectangle (b, 10.f);
    }

    g.setColour (selected ? kYellow : kYellowDim);
    g.drawRoundedRectangle (b, 10.f, selected ? 3.f : 1.5f);

    const auto txt = (glow > 0.55f) ? kBlack : (dimmed ? kYellowDim : kYellow);
    g.setColour (txt);
    g.setFont (juce::FontOptions (12.f, juce::Font::bold));
    g.drawText (juce::String (index + 1), b.reduced (8.f).removeFromTop (16.f), juce::Justification::topLeft);
    g.setFont (juce::FontOptions (15.f, juce::Font::bold));
    g.drawFittedText (title, b.reduced (8.f).toNearestInt(), juce::Justification::centred, 2);
    g.setFont (juce::FontOptions (11.f));
    g.drawText (noteText, b.reduced (8.f), juce::Justification::bottomRight);
}

// ---------------------------------------------------------------- Editor
void MichelDrumLabEditor::setupKnob (Knob& k, const juce::String& text, const char* /*paramName*/, const juce::String& suffix)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    k.slider.setTextValueSuffix (suffix);
    k.label.setText (text, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold)));
    addAndMakeVisible (k.slider);
    addAndMakeVisible (k.label);
}

MichelDrumLabEditor::MichelDrumLabEditor (MichelDrumLabProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&laf);
    setSize (1100, 720);

    for (int i = 0; i < kNumPads; ++i)
    {
        pads[(size_t) i].index = i;
        pads[(size_t) i].onPress = [this] (int idx)
        {
            selectPad (idx);
            processor.uiTrigger[(size_t) idx].store (110);   // tocar el pad con el raton
        };
        addAndMakeVisible (pads[(size_t) i]);
    }

    static const char* names[kNumKnobs]    = { "VOLUME", "PITCH", "PAN", "ATTACK", "DECAY", "FILTER", "REVERB", "SATURATION" };
    static const char* suffixes[kNumKnobs] = { "", " st", "", " ms", " ms", " Hz", "", "" };
    for (int i = 0; i < kNumKnobs; ++i) setupKnob (knobs[(size_t) i], names[i], names[i], suffixes[i]);

    addAndMakeVisible (sampleBox);
    sampleBox.onChange = [this]
    {
        if (updatingUi) return;
        const int idx = sampleBox.getSelectedId() - 1;
        if (idx >= 0)
        {
            processor.setPadSample (selectedPad, idx);
            processor.uiTrigger[(size_t) selectedPad].store (110);   // previsualizar
            refreshPadTexts();
        }
    };

    addAndMakeVisible (muteButton);
    addAndMakeVisible (soloButton);
    noteSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    noteSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 20);
    noteLabel.setText ("MIDI NOTE", juce::dontSendNotification);
    addAndMakeVisible (noteSlider);
    addAndMakeVisible (noteLabel);

    // Kits / presets
    kitLabel.setText ("KIT", juce::dontSendNotification);
    presetLabel.setText ("PRESET", juce::dontSendNotification);
    addAndMakeVisible (kitLabel);   addAndMakeVisible (presetLabel);
    addAndMakeVisible (kitBox);     addAndMakeVisible (presetBox);
    addAndMakeVisible (nameEditor); addAndMakeVisible (saveKitButton); addAndMakeVisible (savePresetButton);
    nameEditor.setTextToShowWhenEmpty ("nombre...", kYellowDim);

    kitBox.onChange = [this]
    {
        if (updatingUi) return;
        if (auto xml = processor.getKits().loadKit (kitBox.getText()))
        {
            processor.applyStateXml (*xml);
            refreshPadTexts(); refreshSampleBox();
        }
    };
    presetBox.onChange = [this]
    {
        if (updatingUi) return;
        if (auto xml = processor.getPresets().load (presetBox.getText()))
        {
            processor.applyStateXml (*xml);
            refreshPadTexts(); refreshSampleBox();
        }
    };
    saveKitButton.onClick = [this]
    {
        auto name = nameEditor.getText().trim();
        if (name.isEmpty()) name = kitBox.getText();
        if (name.isEmpty()) return;
        if (auto xml = processor.createStateXml (true)) processor.getKits().saveKit (name, *xml);
        refreshKitLists();
        updatingUi = true; kitBox.setText (name, juce::dontSendNotification); updatingUi = false;
    };
    savePresetButton.onClick = [this]
    {
        auto name = nameEditor.getText().trim();
        if (name.isEmpty()) name = presetBox.getText();
        if (name.isEmpty()) return;
        if (auto xml = processor.createStateXml (false)) processor.getPresets().save (name, *xml);
        refreshKitLists();
        updatingUi = true; presetBox.setText (name, juce::dontSendNotification); updatingUi = false;
    };

    masterSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    masterSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 20);
    masterLabel.setText ("MASTER", juce::dontSendNotification);
    addAndMakeVisible (masterSlider);
    addAndMakeVisible (masterLabel);
    masterAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "master", masterSlider);

    refreshKitLists();
    selectPad (0);
    refreshPadTexts();
    startTimerHz (30);
}

MichelDrumLabEditor::~MichelDrumLabEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void MichelDrumLabEditor::selectPad (int pad)
{
    selectedPad = juce::jlimit (0, kNumPads - 1, pad);

    static const char* ids[kNumKnobs] = { "vol", "pitch", "pan", "attack", "decay", "filter", "reverb", "sat" };
    for (int i = 0; i < kNumKnobs; ++i)
    {
        knobs[(size_t) i].attachment.reset();   // desconecta antes de reconectar
        knobs[(size_t) i].attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, padParamId (selectedPad, ids[i]), knobs[(size_t) i].slider);
    }
    muteAtt.reset(); soloAtt.reset(); noteAtt.reset();
    muteAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.apvts, padParamId (selectedPad, "mute"), muteButton);
    soloAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.apvts, padParamId (selectedPad, "solo"), soloButton);
    noteAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, padParamId (selectedPad, "note"), noteSlider);

    for (int i = 0; i < kNumPads; ++i) pads[(size_t) i].selected = (i == selectedPad);
    refreshSampleBox();
    repaint();
}

void MichelDrumLabEditor::refreshSampleBox()
{
    updatingUi = true;
    sampleBox.clear (juce::dontSendNotification);

    const auto& lib = processor.getLibrary();
    juce::String lastCat;
    for (int i = 0; i < lib.size(); ++i)
    {
        const auto& s = lib.all()[(size_t) i];
        if (s.category != lastCat)
        {
            sampleBox.addSectionHeading (mdl::SampleLibrary::categoryLabel (s.category));
            lastCat = s.category;
        }
        sampleBox.addItem (s.name, i + 1);
    }
    const int cur = processor.getPadSample (selectedPad);
    if (cur >= 0) sampleBox.setSelectedId (cur + 1, juce::dontSendNotification);
    updatingUi = false;
}

void MichelDrumLabEditor::refreshKitLists()
{
    updatingUi = true;
    kitBox.clear (juce::dontSendNotification);
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (auto& n : processor.getKits().listKits())    kitBox.addItem (n, id++);
    id = 1;
    for (auto& n : processor.getPresets().listNames()) presetBox.addItem (n, id++);
    updatingUi = false;
}

void MichelDrumLabEditor::refreshPadTexts()
{
    const auto& lib = processor.getLibrary();
    for (int i = 0; i < kNumPads; ++i)
    {
        auto& pc = pads[(size_t) i];
        const int idx = processor.getPadSample (i);
        const juce::String title = (idx >= 0 && idx < lib.size())
                     ? lib.all()[(size_t) idx].name + "\n" + mdl::SampleLibrary::categoryLabel (lib.all()[(size_t) idx].category)
                     : juce::String ("-");
        const int note = (int) std::lround (processor.apvts.getRawParameterValue (padParamId (i, "note"))->load());
        const juce::String noteText = juce::MidiMessage::getMidiNoteName (note, true, true, 5) + "  (" + juce::String (note) + ")";
        const bool dim = processor.apvts.getRawParameterValue (padParamId (i, "mute"))->load() > 0.5f;
        if (title != pc.title || noteText != pc.noteText || dim != pc.dimmed)
        {
            pc.title = title; pc.noteText = noteText; pc.dimmed = dim;
            pc.repaint();
        }
    }
}

void MichelDrumLabEditor::timerCallback()
{
    for (int i = 0; i < kNumPads; ++i)
    {
        auto& pc = pads[(size_t) i];
        const float f = processor.padFlash[(size_t) i].exchange (0.f);
        const float newGlow = juce::jmax (f, pc.glow * 0.85f);
        if (std::abs (newGlow - pc.glow) > 0.002f) { pc.glow = newGlow < 0.01f ? 0.f : newGlow; pc.repaint(); }
    }
    refreshPadTexts();

    const float pk = processor.takeOutputPeak();
    meterLevel = juce::jmax (pk, meterLevel * 0.88f);
    repaint (getWidth() - 60, 90, 40, 520);
}

void MichelDrumLabEditor::paint (juce::Graphics& g)
{
    g.fillAll (kBlack);

    g.setColour (kYellow);
    g.setFont (juce::FontOptions (34.f, juce::Font::bold));
    g.drawText ("MICHEL DRUM LAB", 24, 14, 500, 44, juce::Justification::centredLeft);
    g.setFont (juce::FontOptions (12.f));
    g.setColour (kYellow.withAlpha (0.6f));
    g.drawText ("16-PAD SAMPLER  //  v0.1", 28, 56, 300, 16, juce::Justification::centredLeft);

    g.setColour (kYellowDim);
    g.drawLine (20.f, 78.f, (float) getWidth() - 20.f, 78.f, 2.f);

    // panel del pad seleccionado
    auto panel = juce::Rectangle<float> (565.f, 92.f, 460.f, 515.f);
    g.setColour (kPanel);
    g.fillRoundedRectangle (panel, 12.f);
    g.setColour (kYellowDim);
    g.drawRoundedRectangle (panel, 12.f, 1.5f);
    g.setColour (kYellow);
    g.setFont (juce::FontOptions (18.f, juce::Font::bold));
    g.drawText ("PAD " + juce::String (selectedPad + 1), 585, 100, 300, 28, juce::Justification::centredLeft);

    // medidor de salida
    auto meter = juce::Rectangle<float> ((float) getWidth() - 50.f, 100.f, 24.f, 500.f);
    g.setColour (kPanel);
    g.fillRoundedRectangle (meter, 4.f);
    const float h = meter.getHeight() * juce::jlimit (0.f, 1.f, meterLevel);
    g.setColour (meterLevel > 0.97f ? juce::Colours::red : kYellow);
    g.fillRoundedRectangle (meter.withTop (meter.getBottom() - h), 4.f);
    g.setColour (kYellowDim);
    g.drawRoundedRectangle (meter, 4.f, 1.f);

    // barra inferior
    g.setColour (kYellowDim);
    g.drawLine (20.f, 628.f, (float) getWidth() - 20.f, 628.f, 1.f);
}

void MichelDrumLabEditor::resized()
{
    // Pads 4x4: el pad 1 abajo a la izquierda (como un MPC)
    const int px = 20, py = 90, cell = 135;
    for (int i = 0; i < kNumPads; ++i)
    {
        const int col = i % 4, row = 3 - (i / 4);
        pads[(size_t) i].setBounds (px + col * cell, py + row * cell, cell, cell);
    }

    // Panel derecho
    const int rx = 585;
    sampleBox.setBounds (rx, 134, 420, 28);

    const int kw = 105, kh = 130;
    for (int i = 0; i < kNumKnobs; ++i)
    {
        const int col = i % 4, row = i / 4;
        const int x = rx + col * (kw + 0) , y = 178 + row * (kh + 10);
        knobs[(size_t) i].label.setBounds (x, y, kw, 18);
        knobs[(size_t) i].slider.setBounds (x, y + 18, kw, kh - 18);
    }

    muteButton.setBounds (rx, 465, 90, 28);
    soloButton.setBounds (rx + 100, 465, 90, 28);
    noteLabel.setBounds (rx, 510, 100, 24);
    noteSlider.setBounds (rx + 100, 510, 320, 24);

    // Master
    masterLabel.setBounds (rx, 555, 100, 24);
    masterSlider.setBounds (rx + 100, 555, 320, 24);

    // Barra inferior: kits y presets
    const int by = 645;
    kitLabel.setBounds (20, by, 40, 28);
    kitBox.setBounds (60, by, 210, 28);
    presetLabel.setBounds (290, by, 70, 28);
    presetBox.setBounds (360, by, 210, 28);
    nameEditor.setBounds (590, by, 180, 28);
    saveKitButton.setBounds (780, by, 110, 28);
    savePresetButton.setBounds (900, by, 130, 28);
}
