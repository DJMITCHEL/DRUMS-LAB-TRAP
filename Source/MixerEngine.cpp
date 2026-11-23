#include "MixerEngine.h"
#include <cmath>

namespace mdl
{
    void MixerEngine::prepare (double sampleRate, int maxBlockSize)
    {
        sr = sampleRate;
        maxBlock = juce::jmax (1, maxBlockSize);
        scratch.setSize (2, maxBlock); dry.setSize (2, maxBlock); send.setSize (2, maxBlock);

        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlock, 2 };
        for (auto& f : filters)
        {
            f.prepare (spec);
            f.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
            f.setResonance (0.7f);
            f.reset();
        }

        reverb.setSampleRate (sampleRate);
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.55f; rp.damping = 0.45f; rp.wetLevel = 0.3f; rp.dryLevel = 0.f;
        rp.width = 1.f; rp.freezeMode = 0.f;
        reverb.setParameters (rp);
        reverb.reset();
    }

    void MixerEngine::beginBlock (int n) noexcept
    {
        dry.clear (0, n);  dry.clear (1, n);
        send.clear (0, n); send.clear (1, n);
    }

    void MixerEngine::processPad (int pad, int n, const PadMix& m) noexcept
    {
        float* L = scratch.getWritePointer (0);
        float* R = scratch.getWritePointer (1);

        // Filtro paso-bajo (cutoff a tope = bypass)
        if (m.cutoffHz < 19000.f)
        {
            auto& f = filters[(size_t) pad];
            f.setCutoffFrequency (juce::jlimit (20.f, (float) (sr * 0.45), m.cutoffHz));
            float* chans[2] = { L, R };
            juce::dsp::AudioBlock<float> block (chans, 2, (size_t) n);
            f.process (juce::dsp::ProcessContextReplacing<float> (block));
        }
        else
        {
            filters[(size_t) pad].reset();
        }

        const float drive = 1.f + m.saturation * 9.f;
        const float norm  = 1.f / std::tanh (drive);
        const float gl = m.pan > 0.f ? std::cos (m.pan * juce::MathConstants<float>::halfPi) : 1.f;
        const float gr = m.pan < 0.f ? std::cos (-m.pan * juce::MathConstants<float>::halfPi) : 1.f;

        float* dL = dry.getWritePointer (0);  float* dR = dry.getWritePointer (1);
        float* sL = send.getWritePointer (0); float* sR = send.getWritePointer (1);

        for (int i = 0; i < n; ++i)
        {
            float l = L[i], r = R[i];
            if (m.saturation > 0.001f)
            {
                l += m.saturation * (std::tanh (l * drive) * norm - l);
                r += m.saturation * (std::tanh (r * drive) * norm - r);
            }
            l *= m.volume * gl;
            r *= m.volume * gr;
            dL[i] += l; dR[i] += r;
            sL[i] += l * m.reverb; sR[i] += r * m.reverb;
        }
    }

    static inline float softLimit (float x) noexcept
    {
        constexpr float knee = 0.9f;
        const float a = std::abs (x);
        if (a <= knee) return x;
        const float y = knee + (1.f - knee) * std::tanh ((a - knee) / (1.f - knee));
        return x < 0.f ? -y : y;
    }

    void MixerEngine::finish (juce::AudioBuffer<float>& out, int start, int n, float master) noexcept
    {
        float* sL = send.getWritePointer (0); float* sR = send.getWritePointer (1);
        reverb.processStereo (sL, sR, n);

        const float* dL = dry.getReadPointer (0); const float* dR = dry.getReadPointer (1);
        float* oL = out.getWritePointer (0) + start;
        float* oR = out.getNumChannels() > 1 ? out.getWritePointer (1) + start : nullptr;

        float localPeak = 0.f;
        for (int i = 0; i < n; ++i)
        {
            const float l = softLimit ((dL[i] + sL[i]) * master);
            const float r = softLimit ((dR[i] + sR[i]) * master);
            oL[i] = l;
            if (oR != nullptr) oR[i] = r;
            localPeak = juce::jmax (localPeak, std::abs (l), std::abs (r));
        }
        if (localPeak > peak.load()) peak.store (localPeak);
    }
}
