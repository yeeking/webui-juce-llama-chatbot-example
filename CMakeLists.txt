cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(myk-llama VERSION 0.0.1)

add_subdirectory(./libs/JUCE JUCE)  
# uncomment for static libraries build, which is slower but easier to share your plugin :) 
# set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build llama as a static library")                
# makes the static library build work with JUCE
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(./libs/llama.cpp llama.cpp)


## JUCE plugin
##

juce_add_plugin(myk-llama-chatbot
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME Yee-King                          # Specify the name of the plugin's author
    IS_SYNTH TRUE                       # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE               # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    NEEDS_MIDI_OUTPUT TRUE
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD TRUE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Yeek               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE abc1                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS AU VST3 Standalone                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "myk-llama-chatbot"        # The name of the final executable, which can differ from the target name
   ## for the web-based ui
    NEEDS_WEB_BROWSER TRUE
    NEEDS_WEBVIEW2 TRUE    )

juce_generate_juce_header(myk-llama-chatbot)

target_sources(myk-llama-chatbot
    PRIVATE
    src/PluginEditor.cpp
    src/PluginProcessor.cpp
    src/LLMController.cpp
    src/HTTPServer.cpp
    src/PromptProcessingThread.cpp
    src/StringQueue.cpp
)

target_compile_definitions(myk-llama-chatbot
    PUBLIC # 
        JUCE_ALSA=1
        JUCE_DIRECTSOUND=1
        JUCE_DISABLE_CAUTIOUS_PARAMETER_ID_CHECKING=1
        #JUCE_PLUGINHOST_LADSPA=1
        #JUCE_PLUGINHOST_LV2=1
        #JUCE_PLUGINHOST_VST3=1
        JUCE_USE_OGGVORBIS=1
        #JUCE_VST3_HOST_CROSS_PLATFORM_UID=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=1  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

juce_add_binary_data(AudioPluginData SOURCES src/ui/index.html)

target_link_libraries(myk-llama-chatbot
    PRIVATE
        AudioPluginData           # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils
        juce::juce_gui_extra  
        llama
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

juce_add_console_app(myk-llama-chatbot-tests)

juce_generate_juce_header(myk-llama-chatbot-tests)

target_sources(myk-llama-chatbot-tests
    PRIVATE
        src/tests/unit-tests.cpp
        src/LLMController.cpp
        src/PromptProcessingThread.cpp
        src/StringQueue.cpp
)

target_compile_definitions(myk-llama-chatbot-tests
    PRIVATE
        JUCE_ALSA=1
        JUCE_DIRECTSOUND=1
        JUCE_USE_OGGVORBIS=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_UNIT_TESTS=1  # Good to enable JUCE's built-in unit-testing support
)

target_link_libraries(myk-llama-chatbot-tests
    PRIVATE
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_events
        juce::juce_audio_basics
        llama  # if needed
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)



# set this to ON and the http server will serve the UI files from disk instead of memory
# which makes it easier to do quick UI iterations
set(LOCAL_WEBUI ON) # or ON

if(LOCAL_WEBUI)
    target_compile_definitions(myk-llama-chatbot PRIVATE LOCAL_WEBUI)
    message(STATUS "Serving UI from disk: LOCAL_WEBUI is ${LOCAL_WEBUI}")
else()
    message(STATUS "Serving UI from memory: LOCAL_WEBUI is ${LOCAL_WEBUI}")   
endif()


