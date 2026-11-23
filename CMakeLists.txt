cmake_minimum_required(VERSION 3.22)
project(MichelDrumLab VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- JUCE (se descarga automaticamente la primera vez) ----
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

# ---- Samples integrados dentro del binario (BinaryData) ----
# Cualquier .wav dentro de Assets/Samples/<CATEGORIA>/ queda incrustado en el VST3.
file(GLOB_RECURSE MDL_SAMPLE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Samples/*.wav")
juce_add_binary_data(MDLSamples SOURCES ${MDL_SAMPLE_FILES})
set_target_properties(MDLSamples PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# ---- Plugin ----
juce_add_plugin(MichelDrumLab
    COMPANY_NAME               "Michel"
    PRODUCT_NAME               "MICHEL DRUM LAB"
    PLUGIN_MANUFACTURER_CODE   Mchl
    PLUGIN_CODE                Mdlb
    FORMATS                    VST3 Standalone
    IS_SYNTH                   TRUE
    NEEDS_MIDI_INPUT           TRUE
    NEEDS_MIDI_OUTPUT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES            Instrument Drum
    COPY_PLUGIN_AFTER_BUILD    FALSE)

target_sources(MichelDrumLab PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SampleLibrary.cpp
    Source/SampleManager.cpp
    Source/PadEngine.cpp
    Source/MIDIManager.cpp
    Source/KitManager.cpp
    Source/MixerEngine.cpp
    Source/PresetManager.cpp)

target_compile_definitions(MichelDrumLab PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(MichelDrumLab
    PRIVATE
        MDLSamples
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
