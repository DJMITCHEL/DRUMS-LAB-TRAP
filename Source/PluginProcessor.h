#pragma once
#include "Common.h"
#include "SampleLibrary.h"
#include "SampleManager.h"
#include "PadEngine.h"
#include "MIDIManager.h"
#include "MixerEngine.h"
#include "KitManager.h"
#include "PresetManager.h"

class MichelDrumLabProcessor : public juce::AudioProcessor
{
public:
    MichelDrumLabProcessor();
    ~MichelDrumLabProcessor() override = default;

    // --- AudioProcessor ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MICHEL DRUM LAB"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- API para la interfaz ---
    juce::AudioProcessorValueTreeState apvts;

    const mdl::SampleLibrary& getLibrary() const noexcept { return library; }
    int  getPadSample (int pad) const noexcept { return padSample[(size_t) pad].load(); }
    void setPadSample (int pad, int sampleIndex) noexcept;

    std::unique_ptr<juce::XmlElement> createStateXml (bool kitOnly) const;
    bool applyStateXml (const juce::XmlElement& xml);

    mdl::KitManager&    getKits()    noexcept { return kits; }
    mdl::PresetManager& getPresets() noexcept { return presets; }

    // Comunicacion lock-free con la interfaz
    std::array<std::atomic<int>,   mdl::kNumPads> uiTrigger;   // velocity 1..127 pendiente (0 = nada)
    std::array<std::atomic<float>, mdl::kNumPads> padFlash;    // para iluminar pads
    float takeOutputPeak() noexcept { return mixer.takePeak(); }

private:
    struct PadParamPtrs
    {
        std::atomic<float> *vol, *pitch, *pan, *attack, *decay, *filter, *reverb, *sat, *mute, *solo, *note;
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void assignDefaultKit();
    bool isPadAudible (int pad, bool anySolo) const noexcept;
    void triggerPad (int pad, float velocity01) noexcept;
    void renderSegment (juce::AudioBuffer<float>& out, int start, int num) noexcept;

    mdl::SampleLibrary  library;
    mdl::SampleManager  samples;
    mdl::PadEngine      padEngine;
    mdl::MIDIManager    midiManager;
    mdl::MixerEngine    mixer;
    mdl::KitManager     kits;
    mdl::PresetManager  presets;

    std::array<std::atomic<int>, mdl::kNumPads> padSample;
    std::array<PadParamPtrs, mdl::kNumPads> ptrs {};
    std::atomic<float>* masterParam = nullptr;
    int maxBlock = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MichelDrumLabProcessor)
};
