#pragma once
#include "PresetManager.h"

namespace mdl
{
    /** Un KIT = los 16 samples asignados + todos los parametros de cada pad (volumen, pitch, pan,
        attack, decay, filtro, reverb, saturacion, mute, solo, nota MIDI). */
    class KitManager
    {
    public:
        KitManager();

        juce::StringArray listKits() const              { return store.listNames(); }
        bool saveKit (const juce::String& name, const juce::XmlElement& xml) const { return store.save (name, xml); }
        std::unique_ptr<juce::XmlElement> loadKit (const juce::String& name) const { return store.load (name); }
        void installFactoryKit (const juce::String& name, const juce::XmlElement& xml) const
        {
            if (! store.exists (name)) store.save (name, xml);
        }

    private:
        PresetManager store;
    };
}
