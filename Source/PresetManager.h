#pragma once
#include "Common.h"

namespace mdl
{
    /** Guarda/lee archivos XML en una carpeta (en %APPDATA%\MichelDrumLab\...). */
    class PresetManager
    {
    public:
        PresetManager (juce::File directory, juce::String extension);

        juce::StringArray listNames() const;
        bool exists (const juce::String& name) const;
        bool save (const juce::String& name, const juce::XmlElement& xml) const;
        std::unique_ptr<juce::XmlElement> load (const juce::String& name) const;

    private:
        juce::File fileFor (const juce::String& name) const;
        juce::File dir;
        juce::String ext;
    };
}
