/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "llama.h"


//==============================================================================
LlamaPluginProcessor::LlamaPluginProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
, apiServer{*this}, llmController{}

{
    // do a quick test to see if we can load a model
    // std::string modelPath = "..-/../../../../models/supertinyllama/Supertinyllama-107K-F16.gguf";
    // llama_model_params model_params = llama_model_default_params();
    // llama_model * model = llama_load_model_from_file(modelPath.c_str(), model_params);
    // llama_free_model(model);

    apiServer.startThread();  // Launch server in the background
}

LlamaPluginProcessor::~LlamaPluginProcessor()
{
    apiServer.stopServer();
}

//==============================================================================
const juce::String LlamaPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LlamaPluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LlamaPluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LlamaPluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LlamaPluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LlamaPluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LlamaPluginProcessor::getCurrentProgram()
{
    return 0;
}

void LlamaPluginProcessor::setCurrentProgram (int index)
{
}

const juce::String LlamaPluginProcessor::getProgramName (int index)
{
    return {};
}

void LlamaPluginProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LlamaPluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void LlamaPluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LlamaPluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LlamaPluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // juce::String result;
    // while (resultQ.pop(result))
    // {
    //     // lock for the duration of these brackets...
    //     {
    //         const std::lock_guard<std::mutex> lock(latestLLMOutputMutex);
    //         latestLLMOutput = result;
    //     }// lock released now 

    //     // Now schedule a GUI-safe callback
    //     juce::MessageManager::callAsync([this]() {
    //         handleNewLLMResultOnGuiThread();
    //     });
    // }
}


//==============================================================================
bool LlamaPluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LlamaPluginProcessor::createEditor()
{
    return new LlamaPluginEditor (*this);
}

//==============================================================================
void LlamaPluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void LlamaPluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LlamaPluginProcessor();
}



void LlamaPluginProcessor::handleNewLLMResultOnGuiThread()
{
    // make an update request to the UI 
    // on the messaging thread (I think)

    // if (auto* editor = dynamic_cast<LlamaPluginEditor*>(getActiveEditor()))
    //     editor->updateWithResult(resultCopy);
}


std::string LlamaPluginProcessor::apiLLMStatus()
{
    return llmController.getStatusString();
}

bool LlamaPluginProcessor::apiLoadModel(const std::string& modelPath)
{
    std::cout << "PluginProc: apiLoadModel loading a model " <<  modelPath << std::endl; 
    // send it to the llamaController
    bool res = llmController.loadModel(modelPath);
    llmController.resetContext(32);
    llmController.prepareSampler();

    return res; 
}


void LlamaPluginProcessor::apiPrompt(const std::string& prompt)
{
    std::cout << "PluginProc: apiPrompt q'ing" <<  prompt << std::endl; 
    // send it to the llamaController
    llmController.addPromptToQueue(prompt);

}

std::vector<std::pair<std::string,std::string>> LlamaPluginProcessor::apiAllPrompts()
{
    return llmController.getPromptResponseHistory();
}
