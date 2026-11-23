#include "PadEngine.h"
#include <cmath>

namespace mdl
{
    void PadEngine::trigger (int pad, int sampleIndex, float velocity01) noexcept
    {
        if (samples == nullptr || pad < 0 || pad >= kNumPads) return;
        auto* s = samples->get (sampleIndex);
        if (s == nullptr || s->buffer.getNumSamples() < 2) return;

        auto& padVoices = voices[(size_t) pad];
        int slot = -1;
        for (int i = 0; i < kVoicesPerPad; ++i)
            if (! padVoices[(size_t) i].active) { slot = i; break; }

        if (slot < 0)   // todas ocupadas: roba la mas antigua (round-robin)
        {
            slot = nextVoice[(size_t) pad];
            nextVoice[(size_t) pad] = (slot + 1) % kVoicesPerPad;
        }

        auto& v = padVoices[(size_t) slot];
        v.sample = s; v.position = 0.0; v.velocity = juce::jlimit (0.f, 1.f, velocity01);
        v.age = 0; v.active = true;
    }

    void PadEngine::killPad (int pad) noexcept
    {
        for (auto& v : voices[(size_t) pad]) v.active = false;
    }

    void PadEngine::killAll() noexcept
    {
        for (int p = 0; p < kNumPads; ++p) killPad (p);
    }

    void PadEngine::renderPad (int pad, const PadVoiceParams& p, float* L, float* R, int n) noexcept
    {
        const float attackSamples = p.attackMs * 0.001f * (float) sampleRate;
        const bool  useDecay      = p.decayMs < 4999.f;
        const float decaySamples  = juce::jmax (1.f, p.decayMs * 0.001f * (float) sampleRate);
        const double pitchRatio   = std::pow (2.0, (double) p.pitchSemitones / 12.0);

        for (auto& v : voices[(size_t) pad])
        {
            if (! v.active) continue;

            const auto& buf = v.sample->buffer;
            const int   len = buf.getNumSamples();
            const float* sl = buf.getReadPointer (0);
            const float* sr = buf.getNumChannels() > 1 ? buf.getReadPointer (1) : sl;
            const double inc = pitchRatio * v.sample->sampleRate / sampleRate;

            for (int i = 0; i < n; ++i)
            {
                const int idx = (int) v.position;
                if (idx >= len - 1) { v.active = false; break; }

                float env = 1.f;
                if (attackSamples > 1.f && (float) v.age < attackSamples)
                    env = (float) v.age / attackSamples;
                if (useDecay)
                {
                    const float t = (float) v.age / decaySamples;
                    if (t >= 1.f) { v.active = false; break; }
                    env *= (1.f - t) * (1.f - t);
                }

                const float frac = (float) (v.position - (double) idx);
                const float a = sl[idx] + (sl[idx + 1] - sl[idx]) * frac;
                const float b = sr[idx] + (sr[idx + 1] - sr[idx]) * frac;
                const float g = v.velocity * env;
                L[i] += a * g;
                R[i] += b * g;

                v.position += inc;
                ++v.age;
            }
        }
    }
}
