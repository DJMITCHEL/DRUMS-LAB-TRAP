#include "SampleManager.h"
#include "BinaryData.h"

namespace mdl
{
    void SampleManager::loadAll (const SampleLibrary& library)
    {
        juce::AudioFormatManager formats;
        formats.registerBasicFormats();

        samples.clear();
        samples.resize ((size_t) library.size());

        for (int i = 0; i < library.size(); ++i)
        {
            int dataSize = 0;
            const char* data = BinaryData::getNamedResource (
                library.all()[(size_t) i].resourceName.toRawUTF8(), dataSize);
            if (data == nullptr || dataSize <= 0) continue;

            std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (
                std::make_unique<juce::MemoryInputStream> (data, (size_t) dataSize, false)));
            if (reader == nullptr || reader->lengthInSamples <= 0) continue;

            auto& s = samples[(size_t) i];
            const int numCh  = juce::jmin (2, (int) reader->numChannels);
            const int length = (int) juce::jmin ((juce::int64) 60 * 48000, reader->lengthInSamples);
            s.sampleRate = reader->sampleRate;
            s.buffer.setSize (numCh, length);
            reader->read (&s.buffer, 0, length, 0, true, numCh > 1);
        }
    }
}
