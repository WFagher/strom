# Ström — vintage analog synth (VST3 + Standalone)
#
# JUCE is fetched automatically at configure time; no SDK installs needed.
# Build:  cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build

cmake_minimum_required(VERSION 3.22)
project(Strom VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.14
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Strom
    COMPANY_NAME              "William Fagher"
    BUNDLE_ID                 se.fagher.strom
    IS_SYNTH                  TRUE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD   FALSE
    PLUGIN_MANUFACTURER_CODE  Wfgh
    PLUGIN_CODE               Strm
    FORMATS                   VST3 Standalone
    VST3_CATEGORIES           "Instrument Synth"
    PRODUCT_NAME              "Strom")

target_sources(Strom PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/engine/SynthVoice.cpp)

target_include_directories(Strom PRIVATE src)

target_compile_definitions(Strom PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Strom
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
