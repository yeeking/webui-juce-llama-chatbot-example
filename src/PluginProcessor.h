/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HTTPServer.h"
#include "PromptProcessingThread.h"
#include "StringQueue.h"
#include "LLMController.h"


//==============================================================================
/**
*/
class LlamaPluginProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    LlamaPluginProcessor();
    ~LlamaPluginProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    /** query status */
    std::string apiLLMStatus();
    /** api wants us to load a model */
    bool apiLoadModel(const std::string& modelPath);
    /**  http server calls this when it receives a prompt*/
    void apiPrompt(const std::string& prompt);
    /** http server calls this to get chat history  */
    std::vector<std::pair<std::string,std::string>> apiAllPrompts();

private:
  HttpServerThread apiServer;
  LLMController llmController; 

  /**  */
  void handleNewLLMResultOnGuiThread();
  //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LlamaPluginProcessor)
};
