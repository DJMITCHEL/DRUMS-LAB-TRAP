#include "SampleLibrary.h"
#include "BinaryData.h"

namespace mdl
{
    static const char* kCategoryOrder[] = { "808", "KICKS", "SNARES", "CLAPS", "HIHATS", "OPEN_HATS",
                                            "ROLLS", "PERCUSSIONS", "RIMS", "VOCALS", "FX" };

    static int categoryRank (const juce::String& c)
    {
        for (int i = 0; i < (int) std::size (kCategoryOrder); ++i)
            if (c == kCategoryOrder[i]) return i;
        return 1000;
    }

    SampleLibrary::SampleLibrary()
    {
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            const char* resName = BinaryData::namedResourceList[i];
            const char* orig    = BinaryData::getNamedResourceOriginalFilename (resName);
            if (orig == nullptr) continue;

            juce::String file = juce::String::fromUTF8 (orig);
            if (! file.endsWithIgnoreCase (".wav")) continue;

            file = file.dropLastCharacters (4);                 // quita ".wav"
            const juce::String category = file.upToFirstOccurrenceOf ("-", false, false);
            const juce::String name     = file.fromFirstOccurrenceOf ("-", false, false);
            if (category.isEmpty() || name.isEmpty()) continue;

            SampleInfo s;
            s.category     = category;
            s.name         = name.replaceCharacter ('_', ' ');
            s.fullId       = file;
            s.resourceName = juce::String (resName);
            items.push_back (std::move (s));
        }

        std::sort (items.begin(), items.end(), [] (const SampleInfo& a, const SampleInfo& b)
        {
            const int ra = categoryRank (a.category), rb = categoryRank (b.category);
            if (ra != rb) return ra < rb;
            if (ra == 1000 && a.category != b.category) return a.category < b.category;
            return a.name.compareNatural (b.name) < 0;
        });
    }

    int SampleLibrary::indexOfId (const juce::String& fullId) const
    {
        for (int i = 0; i < size(); ++i)
            if (items[(size_t) i].fullId == fullId) return i;
        return -1;
    }

    int SampleLibrary::findNth (const juce::String& category, int nth) const
    {
        int found = -1, count = 0;
        for (int i = 0; i < size(); ++i)
            if (items[(size_t) i].category == category)
            {
                found = i;
                if (count++ == nth) return i;
            }
        return found;   // si hay menos de nth+1, devuelve el ultimo de la categoria
    }

    juce::StringArray SampleLibrary::getCategories() const
    {
        juce::StringArray cats;
        for (auto& s : items) cats.addIfNotAlreadyThere (s.category);
        return cats;
    }

    juce::String SampleLibrary::categoryLabel (const juce::String& category)
    {
        return category.replaceCharacter ('_', ' ');
    }
}
