#pragma once
#include "Common.h"
#include "SampleLibrary.h"

namespace mdl
{
    struct LoadedSample
    {
        juce::AudioBuffer<float> buffer;      // 1 o 2 canales
        double sampleRate = 44100.0;
    };

    /** Decodifica los WAV integrados a memoria (una sola vez, en el hilo de interfaz).
        Despues de loadAll() la lista es INMUTABLE, por lo que el hilo de audio puede leerla
        sin bloqueos. (Samples de usuario: etapa futura, se anadiran con doble buffer.) */
    class SampleManager
    {
    public:
        void loadAll (const SampleLibrary& library);
        const LoadedSample* get (int index) const noexcept
        {
            return (index >= 0 && index < (int) samples.size()) ? &samples[(size_t) index] : nullptr;
        }
        int size() const noexcept { return (int) samples.size(); }

    private:
        std::vector<LoadedSample> samples;
    };
}
