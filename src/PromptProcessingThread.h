#pragma once 

#include <JuceHeader.h>
#include "StringQueue.h"
// #include "LLMController.h"

class LLMController; // forward declaration 

class PromptProcessingThread : public juce::Thread
{
public:
    PromptProcessingThread(StringQueue& prompts, LLMController& llmC);

    void run() override;


private:
    StringQueue& promptQ;   
    LLMController& llmController; 
};
