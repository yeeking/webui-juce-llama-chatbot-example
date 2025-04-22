/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class LlamaPluginEditor  : public juce::AudioProcessorEditor
{
public:
    LlamaPluginEditor (LlamaPluginProcessor&);
    ~LlamaPluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void updateUIFromProcessor(const juce::String& msg);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LlamaPluginProcessor& audioProcessor;
    juce::WebBrowserComponent webView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LlamaPluginEditor)
};
