cmake_minimum_required(VERSION 3.22)
project(R2RMusicVox2Midi VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# Existing vocal-to-MIDI plugin (kept as an independent target).
juce_add_plugin(R2RMusicVox2Midi
  COMPANY_NAME "R2R Music"
  BUNDLE_ID "com.r2rbeats.musicvox2midi"
  PLUGIN_MANUFACTURER_CODE R2RB
  PLUGIN_CODE R2V2
  FORMATS VST3 Standalone
  PRODUCT_NAME "R2R MUSIC VOX 2 MIDI"
  IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT TRUE
  IS_MIDI_EFFECT FALSE EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(R2RMusicVox2Midi)
target_sources(R2RMusicVox2Midi PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_features(R2RMusicVox2Midi PRIVATE cxx_std_17)
target_compile_definitions(R2RMusicVox2Midi PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(R2RMusicVox2Midi PRIVATE
  juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp juce::juce_gui_extra
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

# New optical compressor / limiter plugin.
juce_add_plugin(R2ROpticalTubeCompressor
  COMPANY_NAME "R2R Music"
  BUNDLE_ID "com.r2rbeats.opticaltubecompressor"
  PLUGIN_MANUFACTURER_CODE R2RB
  PLUGIN_CODE ROTC
  FORMATS VST3 Standalone
  PRODUCT_NAME "DJ R2R Optical Tube Compressor"
  IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(R2ROpticalTubeCompressor)
target_sources(R2ROpticalTubeCompressor PRIVATE
  Source/OpticalTube/OpticalTubeProcessor.cpp
  Source/OpticalTube/OpticalTubeEditor.cpp)
target_compile_features(R2ROpticalTubeCompressor PRIVATE cxx_std_17)
target_compile_definitions(R2ROpticalTubeCompressor PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(R2ROpticalTubeCompressor PRIVATE
  juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp juce::juce_gui_extra
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
