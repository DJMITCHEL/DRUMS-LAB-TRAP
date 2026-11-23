#pragma once
#include "Common.h"

namespace mdl
{
    struct SampleInfo
    {
        juce::String category;      // "KICKS", "OPEN_HATS", ...
        juce::String name;          // "Pray"
        juce::String fullId;        // "KICKS-Pray"  (identificador estable para kits/presets)
        juce::String resourceName;  // nombre interno en BinaryData
    };

    /** Catalogo (indice) de los samples integrados en el binario. No contiene audio. */
    class SampleLibrary
    {
    public:
        SampleLibrary();

        const std::vector<SampleInfo>& all() const noexcept { return items; }
        int size() const noexcept                           { return (int) items.size(); }
        int indexOfId (const juce::String& fullId) const;
        int findNth (const juce::String& category, int nth) const;   // -1 si no existe
        juce::StringArray getCategories() const;                     // en orden de visualizacion
        static juce::String categoryLabel (const juce::String& category);

    private:
        std::vector<SampleInfo> items;
    };
}
