#include "PresetManager.h"

namespace mdl
{
    PresetManager::PresetManager (juce::File directory, juce::String extension)
        : dir (std::move (directory)), ext (std::move (extension))
    {
        dir.createDirectory();
    }

    juce::File PresetManager::fileFor (const juce::String& name) const
    {
        return dir.getChildFile (juce::File::createLegalFileName (name.trim()) + ext);
    }

    juce::StringArray PresetManager::listNames() const
    {
        juce::StringArray names;
        for (auto& f : dir.findChildFiles (juce::File::findFiles, false, "*" + ext))
            names.add (f.getFileNameWithoutExtension());
        names.sort (true);
        return names;
    }

    bool PresetManager::exists (const juce::String& name) const { return fileFor (name).existsAsFile(); }

    bool PresetManager::save (const juce::String& name, const juce::XmlElement& xml) const
    {
        if (name.trim().isEmpty()) return false;
        return xml.writeTo (fileFor (name));
    }

    std::unique_ptr<juce::XmlElement> PresetManager::load (const juce::String& name) const
    {
        auto f = fileFor (name);
        return f.existsAsFile() ? juce::XmlDocument::parse (f) : nullptr;
    }
}
