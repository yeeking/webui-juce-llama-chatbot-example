#include "PromptProcessingThread.h"
#include "LLMController.h"

PromptProcessingThread::PromptProcessingThread(StringQueue& proQ, LLMController& llmC) : juce::Thread("Prompt Processor"), promptQ{proQ}, llmController{llmC} {}

void PromptProcessingThread::run() 
{
    while (!threadShouldExit()){
        std::string prompt;
        if (promptQ.pop(prompt)){
            // ===> Do your processing here
            // DBG("PromptProcessingThread::Processing prompt: " + prompt);
            if (llmController.readyToGenerate()){
                // DBG("PromptProcessingThread::Processing llm ready. generating ");

                llmController.resetContext(32);
                std::string output = llmController.generate(prompt, 100);
                llmController.addPromptResponse(prompt, output);
                // DBG("PromptProcessingThread::Processing done generating ");

            }
            else {
                llmController.addPromptResponse(prompt, "LLM not ready");
            }
        }
        else{
            wait(50); // Avoid CPU spin; check every 50 ms
        }
    }
}

