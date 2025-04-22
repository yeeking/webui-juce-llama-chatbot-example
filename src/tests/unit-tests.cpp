#include <JuceHeader.h>
#include "../LLMController.h"
#include "../StringQueue.h"


class PromptProcessorTest : public juce::UnitTest
{
public:
    PromptProcessorTest() : juce::UnitTest("MYK unit tests", "PromptProcessorTest") {}

    void runTest() override
    {
        {
            beginTest("Correctly process queued prompts");
            LLMController llmController{};
            llmController.loadModel("../../../../models/granite-3.3-2b-instruct-Q4_K_M.gguf");
            llmController.prepareSampler();
            llmController.resetContext(32);
            StringQueue q{5};
            PromptProcessingThread pThread{q, llmController};
            pThread.startThread();
            // expectEquals(result, std::string("njam"));  // Adjust expected output accordingly
            q.push("Test input 1");
            q.push("Test input 2");
            q.push("Test input 3");
            
            // pThread.sleep(5000);
            while(llmController.getPromptResponseHistory().size() < 3){
                std::cout << "waiting ... " << std::endl;
                pThread.sleep(1000);
            }
            // pThread.stopThread(30000);// let it finish its work
            std::vector<std::pair<std::string, std::string>> results = llmController.getPromptResponseHistory();
            expectEquals(results.size(), size_t{3});
            // LLM not ready
            for (size_t i=0;i<3;++i){
                beginTest("Item " + std::to_string(i) + " ok ");
                expectGreaterThan(results[i].second.size(), size_t{0});
                expectNotEquals(results[i].second, std::string{"LLM not ready"});
            }

            pThread.stopThread(1000);
            


            
            
        }
    }
};


class LLMControllerTests : public juce::UnitTest
{
public:
    LLMControllerTests() : juce::UnitTest("MYK unit tests", "LLMControllerTests") {}

    void runTest() override
    {
        beginTest("Load bad model");
        LLMController llmController{};
        bool result;

        result = llmController.loadModel("test.gguf");
        expect(result == false);  

        beginTest("Load good model");

        result = llmController.loadModel("../../../../models/Supertinyllama-107K-F16.gguf");
        expect(result == true);  


        beginTest("tokenize string");

        llmController.loadModel("../../../../models/Supertinyllama-107K-F16.gguf");
        llmController.resetContext(32);
        std::vector<llama_token> tokens = llmController.tokenize("hello world!");
        expectGreaterThan(tokens.size(), static_cast<size_t>(0));

        beginTest("detokenize ");
        // use a proper model to ensure detokenisation yields same as input
        // https://huggingface.co/lmstudio-community/granite-3.3-2b-instruct-GGUF/resolve/main/granite-3.3-2b-instruct-Q3_K_L.gguf?download=true
        llmController.loadModel("../../../../models/granite-3.3-2b-instruct-Q4_K_M.gguf");
        llmController.resetContext(32);
        std::string inputStr{"hello world!"};
        tokens = llmController.tokenize(inputStr);
        std::string resultStr = llmController.detokenize(tokens);
        expectEquals(inputStr, resultStr);


        beginTest("infer and detokenize");
        // https://huggingface.co/lmstudio-community/granite-3.3-2b-instruct-GGUF/resolve/main/granite-3.3-2b-instruct-Q3_K_L.gguf?download=true
        llmController.loadModel("../../../../models/granite-3.3-2b-instruct-Q4_K_M.gguf");
        llmController.resetContext(32);
        llmController.prepareSampler();
        std::string inPrompt = "Tell me why C++ is the best language. ";
        std::vector<llama_token> in_tokens = llmController.tokenize(inPrompt);
        // infer 100 tokens from now
        std::vector<llama_token> out_tokens = llmController.infer(in_tokens, static_cast<size_t>(100));
        std::string resultStr2 = llmController.detokenize(out_tokens);
        // std::cout << "Answer to prompt " << inPrompt << " is \n " << resultStr2 << std::endl;

        expectGreaterThan(resultStr2.length(), static_cast<size_t>(0));

        // {
        //     beginTest("Repeated infers work");

        //     LLMController llmController{};
        //     llmController.loadModel("../../../../models/granite-3.3-2b-instruct-Q4_K_M.gguf");
        //     llmController.prepareSampler();
        //     for (int i=0;i<3;i++){
        //         llmController.resetContext(128);// this resets the context 
        //         std::string output = llmController.generate("hello world", 50);
        //         expectGreaterThan(output.length(), static_cast<size_t>(0));
        //     }

        // }

    }
};


class StringQueueTests : public juce::UnitTest
{
public:
    StringQueueTests() : juce::UnitTest("MYK unit tests", "StringQueueTests") {}

    void runTest() override
    {
        {
            beginTest("Snapshot test");
            StringQueue q{5};
            q.push("1");
            q.push("2");
            q.push("3");
            std::vector<std::string> snap = q.getSnapshot();
            expectEquals(snap.size(), static_cast<size_t>(3));
        }
        {
            beginTest("Snapshot test pop");
            StringQueue q{5};
            q.push("1");
            q.push("2");
            q.push("3");
            std::string r;
            q.pop(r);
            std::vector<std::string> snap = q.getSnapshot();
            expectEquals(snap.size(), static_cast<size_t>(2));
        }
            
        
        
    }
};


int main()
{
    
  

    juce::UnitTestRunner runner;
    
    // LLMControllerTests llmControllerTests; 
    // runner.runTestsInCategory("LLMControllerTests");

    // StringQueueTests stringQTests;
    // runner.runTestsInCategory("StringQueueTests");

    PromptProcessorTest promptProcTest;
    runner.runTestsInCategory("PromptProcessorTest");


    return 0;
}
