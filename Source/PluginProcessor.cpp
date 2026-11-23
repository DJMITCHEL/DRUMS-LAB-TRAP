#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace mdl;

static juce::File appDataDir()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("MichelDrumLab");
}

juce::AudioProcessorValueTreeState::ParameterLayout MichelDrumLabProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "master", 1 }, "Master",
                NormalisableRange<float> (0.f, 1.5f, 0.001f), 0.9f));

    for (int i = 0; i < kNumPads; ++i)
    {
        const String n = "Pad " + String (i + 1) + " ";
        auto id = [i] (const char* s) { return ParameterID { padParamId (i, s), 1 }; };

        layout.add (std::make_unique<AudioParameterFloat> (id ("vol"),    n + "Volume",     NormalisableRange<float> (0.f, 1.5f, 0.001f), 0.85f));
        layout.add (std::make_unique<AudioParameterFloat> (id ("pitch"),  n + "Pitch",      NormalisableRange<float> (-24.f, 24.f, 0.01f), 0.f));
        layout.add (std::make_unique<AudioParameterFloat> (id ("pan"),    n + "Pan",        NormalisableRange<float> (-1.f, 1.f, 0.001f), 0.f));
        layout.add (std::make_unique<AudioParameterFloat> (id ("attack"), n + "Attack",     NormalisableRange<float> (0.f, 500.f, 0.1f, 0.4f), 0.f));
        layout.add (std::make_unique<AudioParameterFloat> (id ("decay"),  n + "Decay",      NormalisableRange<float> (20.f, 5000.f, 1.f, 0.4f), 5000.f));
        layout.add (std::make_unique<AudioParameterFloat> (id ("filter"), n + "Filter",     NormalisableRange<float> (20.f, 20000.f, 1.f, 0.25f), 20000.f));
        layout.add (std::make_unique<AudioParameterFloat> (id ("reverb"), n + "Reverb",     NormalisableRange<float> (0.f, 1.f, 0.001f), 0.f));
        layout.add (std::make_unique<AudioParameterFloat> (id ("sat"),    n + "Saturation", NormalisableRange<float> (0.f, 1.f, 0.001f), 0.f));
        layout.add (std::make_unique<AudioParameterBool>  (id ("mute"),   n + "Mute", false));
        layout.add (std::make_unique<AudioParameterBool>  (id ("solo"),   n + "Solo", false));
        layout.add (std::make_unique<AudioParameterInt>   (id ("note"),   n + "MIDI Note", 0, 127, kFirstMidiNote + i));
    }
    return layout;
}

MichelDrumLabProcessor::MichelDrumLabProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout()),
      kits(),
      presets (appDataDir().getChildFile ("Presets"), ".mdlpreset")
{
    for (int i = 0; i < kNumPads; ++i)
    {
        uiTrigger[(size_t) i].store (0);
        padFlash[(size_t) i].store (0.f);
        padSample[(size_t) i].store (-1);

        auto get = [this, i] (const char* s) { return apvts.getRawParameterValue (padParamId (i, s)); };
        ptrs[(size_t) i] = { get ("vol"), get ("pitch"), get ("pan"), get ("attack"), get ("decay"),
                             get ("filter"), get ("reverb"), get ("sat"), get ("mute"), get ("solo"), get ("note") };
        midiManager.setNoteSource (i, ptrs[(size_t) i].note);
    }
    masterParam = apvts.getRawParameterValue ("master");

    samples.loadAll (library);                 // decodifica los WAV integrados (una vez)
    padEngine.setSampleManager (&samples);

    assignDefaultKit();
    kits.installFactoryKit ("DARK TRAP KIT", *createStateXml (true));
}

void MichelDrumLabProcessor::assignDefaultKit()
{
    static const struct { const char* cat; int nth; } layout[kNumPads] = {
        { "KICKS", 0 },       { "KICKS", 1 },       { "SNARES", 0 },       { "CLAPS", 0 },
        { "HIHATS", 0 },      { "HIHATS", 1 },      { "OPEN_HATS", 0 },    { "RIMS", 0 },
        { "808", 0 },         { "808", 1 },         { "808", 2 },          { "PERCUSSIONS", 0 },
        { "ROLLS", 0 },       { "FX", 0 },          { "PERCUSSIONS", 1 },  { "FX", 1 } };

    for (int i = 0; i < kNumPads; ++i)
    {
        int idx = library.findNth (layout[i].cat, layout[i].nth);
        if (idx < 0 && library.size() > 0) idx = i % library.size();
        padSample[(size_t) i].store (idx);
    }
}

void MichelDrumLabProcessor::setPadSample (int pad, int sampleIndex) noexcept
{
    if (pad >= 0 && pad < kNumPads && sampleIndex >= -1 && sampleIndex < library.size())
        padSample[(size_t) pad].store (sampleIndex);
}

bool MichelDrumLabProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void MichelDrumLabProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    maxBlock = juce::jmax (64, samplesPerBlock);
    padEngine.prepare (sampleRate);
    mixer.prepare (sampleRate, maxBlock);
}

bool MichelDrumLabProcessor::isPadAudible (int pad, bool anySolo) const noexcept
{
    const auto& p = ptrs[(size_t) pad];
    if (p.mute->load() > 0.5f) return false;
    if (anySolo && p.solo->load() < 0.5f) return false;
    return true;
}

void MichelDrumLabProcessor::triggerPad (int pad, float velocity01) noexcept
{
    bool anySolo = false;
    for (auto& p : ptrs) if (p.solo->load() > 0.5f) { anySolo = true; break; }
    if (! isPadAudible (pad, anySolo)) return;

    padEngine.trigger (pad, padSample[(size_t) pad].load(), velocity01);
    padFlash[(size_t) pad].store (juce::jmax (0.35f, velocity01));
}

void MichelDrumLabProcessor::renderSegment (juce::AudioBuffer<float>& out, int start, int num) noexcept
{
    bool anySolo = false;
    for (auto& p : ptrs) if (p.solo->load() > 0.5f) { anySolo = true; break; }
    const float master = masterParam->load();

    while (num > 0)
    {
        const int n = juce::jmin (num, maxBlock);
        mixer.beginBlock (n);

        for (int pad = 0; pad < kNumPads; ++pad)
        {
            if (! isPadAudible (pad, anySolo)) { padEngine.killPad (pad); continue; }

            const auto& p = ptrs[(size_t) pad];
            float* L = mixer.scratchL();
            float* R = mixer.scratchR();
            juce::FloatVectorOperations::clear (L, n);
            juce::FloatVectorOperations::clear (R, n);

            PadVoiceParams vp { p.pitch->load(), p.attack->load(), p.decay->load() };
            padEngine.renderPad (pad, vp, L, R, n);

            PadMix mix { p.vol->load(), p.pan->load(), p.filter->load(), p.reverb->load(), p.sat->load() };
            mixer.processPad (pad, n, mix);
        }

        mixer.finish (out, start, n, master);
        start += n;
        num   -= n;
    }
}

void MichelDrumLabProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    // Pads pulsados con el raton en la interfaz
    for (int pad = 0; pad < kNumPads; ++pad)
    {
        const int v = uiTrigger[(size_t) pad].exchange (0);
        if (v > 0) triggerPad (pad, (float) v / 127.f);
    }

    int pos = 0;
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int  at  = juce::jlimit (0, numSamples, metadata.samplePosition);

        if (msg.isNoteOn())
        {
            if (at > pos) { renderSegment (buffer, pos, at - pos); pos = at; }
            const uint32_t mask = midiManager.padsForNote (msg.getNoteNumber());
            for (int pad = 0; pad < kNumPads; ++pad)
                if (mask & (1u << pad)) triggerPad (pad, msg.getFloatVelocity());
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            padEngine.killAll();
        }
    }
    if (pos < numSamples) renderSegment (buffer, pos, numSamples - pos);
}

// ---------------- Estado (proyecto de FL Studio, kits, presets) ----------------

std::unique_ptr<juce::XmlElement> MichelDrumLabProcessor::createStateXml (bool kitOnly) const
{
    auto xml = std::make_unique<juce::XmlElement> ("MichelDrumLab");
    xml->setAttribute ("version", 1);
    xml->setAttribute ("type", kitOnly ? "kit" : "preset");

    for (int i = 0; i < kNumPads; ++i)
    {
        auto* e = xml->createNewChildElement ("PAD");
        e->setAttribute ("index", i);
        const int idx = padSample[(size_t) i].load();
        if (idx >= 0 && idx < library.size())
            e->setAttribute ("sample", library.all()[(size_t) idx].fullId);
    }

    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::AudioProcessorParameterWithID*> (p))
        {
            if (kitOnly && ! rp->paramID.startsWith ("pad")) continue;
            auto* e = xml->createNewChildElement ("P");
            e->setAttribute ("id", rp->paramID);
            e->setAttribute ("v", (double) rp->getValue());
        }
    return xml;
}

bool MichelDrumLabProcessor::applyStateXml (const juce::XmlElement& xml)
{
    if (! xml.hasTagName ("MichelDrumLab")) return false;

    for (auto* e : xml.getChildWithTagNameIterator ("PAD"))
    {
        const int pad = e->getIntAttribute ("index", -1);
        if (pad < 0 || pad >= kNumPads) continue;
        const int idx = library.indexOfId (e->getStringAttribute ("sample"));
        if (idx >= 0) padSample[(size_t) pad].store (idx);   // si el sample ya no existe, se mantiene el actual
    }
    for (auto* e : xml.getChildWithTagNameIterator ("P"))
        if (auto* p = apvts.getParameter (e->getStringAttribute ("id")))
            p->setValueNotifyingHost ((float) e->getDoubleAttribute ("v"));
    return true;
}

void MichelDrumLabProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = createStateXml (false)) copyXmlToBinary (*xml, dest);
}

void MichelDrumLabProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size)) applyStateXml (*xml);
}

juce::AudioProcessorEditor* MichelDrumLabProcessor::createEditor() { return new MichelDrumLabEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MichelDrumLabProcessor(); }
