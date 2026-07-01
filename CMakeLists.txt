cmake_minimum_required(VERSION 3.22)

project(StereoTo714 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(StereoTo714
    COMPANY_NAME "StereoUpmix"
    BUNDLE_ID "com.stereoupmix.stereoto714"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE StUp
    PLUGIN_CODE St71
    FORMATS VST3
    PRODUCT_NAME "Stereo to 7.1.4")

juce_generate_juce_header(StereoTo714)

target_sources(StereoTo714
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Upmix/UpmixEngine.cpp
        Source/Upmix/SpatialEstimator.cpp
        Source/Upmix/Decorrelator.cpp)

target_compile_definitions(StereoTo714
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(StereoTo714
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(BUILD_TESTING)
    add_executable(ChannelLayoutTests
        tests/channel_layout_test.cpp)

    target_include_directories(ChannelLayoutTests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Source)

    target_link_libraries(ChannelLayoutTests
        PRIVATE
            juce::juce_audio_basics)

    add_test(NAME ChannelLayoutTests COMMAND ChannelLayoutTests)
endif()
