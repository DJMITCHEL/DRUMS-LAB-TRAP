#pragma once
#include "Common.h"

namespace mdl
{
    struct PadMix
    {
        float volume = 1.f, pan = 0.f, cutoffHz = 20000.f, reverb = 0.f, saturation = 0.f;
    };

    /** Filtro, saturacion, volumen, pan y envio de reverb por pad + limitador de salida. */
    class MixerEngine
    {
    public:
        void prepare (double sampleRate, int maxBlockSize);

        float* scratchL() noexcept { return scratch.getWritePointer (0); }
        float* scratchR() noexcept { return scratch.getWritePointer (1); }

        void beginBlock (int numSamples) noexcept;
        void processPad (int pad, int numSamples, const PadMix& mix) noexcept;   // procesa el scratch
        void finish (juce::AudioBuffer<float>& out, int startSample, int numSamples, float masterGain) noexcept;

        float takePeak() noexcept { return peak.exchange (0.f); }

    private:
        double sr = 44100.0;
        int    maxBlock = 512;
        juce::AudioBuffer<float> scratch, dry, send;
        std::array<juce::dsp::StateVariableTPTFilter<float>, kNumPads> filters;
        juce::Reverb reverb;
        std::atomic<float> peak { 0.f };
    };
}
