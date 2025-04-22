#pragma once
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include "llama.h"
#include "StringQueue.h"
#include "PromptProcessingThread.h"

struct llama_model;

// class PromptProcessingThread;// forward declaration 

enum class LLMStatus
{
    WaitingForJobs,
    Generating,
    ModelNotReady
};


class LLMController{
    public:
        LLMController();
        ~LLMController();
        /** returns true if model successfully loaded , false otherwise  */
        bool readyToGenerate();
        /** load a gguf model from sent path. If fails, return false, else return true */
        bool loadModel(const std::string& path);
        /** sets up a context for use in tokenizing and inference */
        void resetContext(uint32_t ctxLen);
        /** setup a top n sampler thingy */
        void prepareSampler();
        /** calls tokenize, infer and detokenize */
        std::string generate(std::string prompt, size_t maxLength);
        /** convert sent string to tokens ready to feed to the model */
        std::vector<llama_token> tokenize(const std::string& prompt);
        /** feed tokens to model, generate output as tokens */
        std::vector<llama_token> infer(std::vector<llama_token>& inTokens, size_t maxLength);
        /** convert tokens (from the model/ tokenize) to a string */
        std::string detokenize(const std::vector<llama_token>& tokens);

        /** (thread safe) add a prompt to the prompt Q for background thread to pick up*/
        void addPromptToQueue(std::string prompt);
        /** (thread safe) add a prompt and response to the p and r history */
        void addPromptResponse(const std::string& prompt, const std::string& response) ;
        /** (thread safe) retrieve a copy of the prompt and response history */
        std::vector<std::pair<std::string, std::string>> getPromptResponseHistory();
        LLMStatus getStatus() const ;
        std::string getStatusString() const ;
    
    private:
        // no dirty old pointers mate
        // std::unique_ptr<llama_model> model;
        llama_model* model;
        const llama_vocab * vocab;
        llama_context* ctx;
        llama_sampler * smpl;
        std::mutex modelMutex;


        StringQueue promptQ; // has its own mutex
        PromptProcessingThread promptThread;
        std::vector<std::pair<std::string,std::string>> promptAndResponseHistory;
        /** mutex to control access to the promptandresponse history  */
        std::mutex historyMutex;
        std::atomic<LLMStatus> status = LLMStatus::WaitingForJobs;

};