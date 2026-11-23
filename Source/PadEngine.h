#pragma once
#include "Common.h"
#include "SampleManager.h"

namespace mdl
{
    struct PadVoiceParams
    {
        float pitchSemitones = 0.f;
        float attackMs       = 0.f;
        float decayMs        = 5000.f;   // >= 4999 => sin decay (el sample suena entero)
    };

    /** Reproduce samples (one-shot) con 4 voces por pad. Sin asignaciones de memoria en audio. */
    class PadEngine
    {
    public:
        void prepare (double hostSampleRate) { sampleRate = hostSampleRate; killAll(); }
        void setSampleManager (const SampleManager* m) noexcept { samples = m; }

        void trigger (int pad, int sampleIndex, float velocity01) noexcept;
        void killPad (int pad) noexcept;
        void killAll() noexcept;

        /** SUMA el audio del pad en L/R (los buffers deben venir a cero). */
        void renderPad (int pad, const PadVoiceParams& p, float* L, float* R, int numSamples) noexcept;

    private:
        struct Voice
        {
            const LoadedSample* sample = nullptr;
            double position = 0.0;
            float  velocity = 0.f;
            int    age      = 0;
            bool   active   = false;
        };

        double sampleRate = 44100.0;
        const SampleManager* samples = nullptr;
        std::array<std::array<Voice, kVoicesPerPad>, kNumPads> voices {};
        std::array<int, kNumPads> nextVoice {};
    };
}
