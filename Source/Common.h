#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <memory>
#include <vector>

namespace mdl
{
    constexpr int kNumPads        = 16;
    constexpr int kVoicesPerPad   = 4;
    constexpr int kFirstMidiNote  = 36;   // pad 1 = nota 36, pad 16 = nota 51 (por defecto)

    // Colores de la interfaz
    const juce::Colour kYellow   { 0xffffd400 };
    const juce::Colour kYellowDim{ 0xff4d4300 };
    const juce::Colour kBlack    { 0xff000000 };
    const juce::Colour kPanel    { 0xff0f0f0f };

    inline juce::String padParamId (int pad, const char* name)
    {
        return "pad" + juce::String (pad + 1) + "_" + name;
    }
}
