cmake_minimum_required(VERSION 3.22)
project(Orage VERSION 4.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

if(DEFINED ENV{CI})
    set(ORAGE_COPY FALSE)   # sur GitHub Actions : pas de copie dans les dossiers systeme
else()
    set(ORAGE_COPY TRUE)
endif()

if(APPLE)
    set(ORAGE_FORMATS VST3 AU Standalone)
else()
    set(ORAGE_FORMATS VST3 Standalone)
endif()

juce_add_plugin(Orage
    COMPANY_NAME "Foudre Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Fdre
    PLUGIN_CODE OrFx
    FORMATS ${ORAGE_FORMATS}
    PRODUCT_NAME "Orage FX"
    COPY_PLUGIN_AFTER_BUILD ${ORAGE_COPY})

target_sources(Orage PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SynthEditor.cpp)

target_compile_definitions(Orage PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Orage
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)


# ------------------------------------------------------------------ Orage (instrument : synthe 6 voix + effets, MIDI)
juce_add_plugin(OrageSynth
    COMPANY_NAME "Foudre Audio"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Fdre
    PLUGIN_CODE OrSy
    FORMATS ${ORAGE_FORMATS}
    PRODUCT_NAME "Orage"
    COPY_PLUGIN_AFTER_BUILD ${ORAGE_COPY})

target_sources(OrageSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SynthEditor.cpp)

target_compile_definitions(OrageSynth PUBLIC
    ORAGE_SYNTH=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(OrageSynth
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
