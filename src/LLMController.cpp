#include "LLMController.h"
#include <filesystem>
#include <iostream>

/** call this to switch off llama cpp logging 
 * e.g.     llama_log_set(llama_log_callback_null, NULL);
 * https://github.com/ggml-org/llama.cpp/discussions/1758
*/
static void llama_log_callback_null(ggml_log_level level, const char * text, void * user_data) { (void) level; (void) text; (void) user_data; }


LLMController::LLMController() 
            : model{nullptr}, 
              vocab{nullptr}, 
              ctx{nullptr}, 
              smpl{nullptr}, 
              promptQ{16}, 
              promptThread{promptQ, *this}

{
    // switch off llama logging
    llama_log_set(llama_log_callback_null, NULL);
    status.store(LLMStatus::ModelNotReady);
    promptThread.startThread();
}

LLMController::~LLMController()
{
    if (model != nullptr){
        llama_model_free(model);
    }  

    if (ctx != nullptr){
        llama_free(ctx);
    } 
    if (smpl != nullptr){
        llama_sampler_free(smpl);
    }
    promptThread.stopThread(1000); // Wait max 1 sec for thread to stop
}

bool LLMController::loadModel(const std::string& path)
{
    std::lock_guard<std::mutex> lock(modelMutex);

    std::filesystem::path fp = path;
    if (! std::filesystem::exists(fp)) {
        std::cout << " LLMController::loadModel file not found" <<  path << std::endl;
        return false; 
    }
    
    
    if (model != nullptr){
        llama_model_free(model);
    }
    
    try {
        llama_model_params model_params = llama_model_default_params();
        
        this->model = llama_model_load_from_file(path.c_str(), model_params);
        this->vocab = llama_model_get_vocab(model);

        if (!model) {
            return false; 
        }

        std::cout << "LLMController::loadModel model loaded! " << std::endl;
        status.store(LLMStatus::WaitingForJobs);
        return true; 
    }
    catch (...){
        std::cout << " LLMController::loadModel exception thrown loading model " << std::endl;
        return false; 
    }
    return false; 

}

void LLMController::resetContext(uint32_t ctxLen)
{
    std::lock_guard<std::mutex> lock(modelMutex);

    // code to prepare a context 
    // uint32_t n_ctx = 64; // TODO 64 is way too short - should be enough to store prompt and n predicted tokens
    // int prev_len = 0;
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = ctxLen;
    ctx_params.n_batch = ctxLen;
    ctx_params.no_perf = false;

    this->ctx = llama_init_from_model(model, ctx_params);

}

void LLMController::prepareSampler()
{
    std::lock_guard<std::mutex> lock(modelMutex);

    uint32_t rand_seed = 42;
    float temperature = 0.6;
    float min_p = 5;

    this->smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(smpl, llama_sampler_init_min_p(min_p, 1));
    llama_sampler_chain_add(smpl, llama_sampler_init_temp(temperature));// low is deterministic
    llama_sampler_chain_add(smpl, llama_sampler_init_dist(rand_seed));
}


std::vector<llama_token> LLMController::tokenize(const std::string& prompt)
{
    std::lock_guard<std::mutex> lock(modelMutex);

    if (vocab == nullptr){// vocab not loaded yet 
        return std::vector<llama_token>();
    }

    const int n_prompt = -llama_tokenize(vocab, prompt.c_str(), prompt.size(), NULL, 0, true, true);

    // allocate space for the tokens and tokenize the prompt
    std::vector<llama_token> prompt_tokens(static_cast<size_t>(n_prompt));
    if (llama_tokenize(vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), prompt_tokens.size(), true, true) < 0) {
        return std::vector<llama_token>();
    }
    else{
        // return 
        return prompt_tokens; 
    }
}


std::string LLMController::detokenize(const std::vector<llama_token>& tokens)
{
    std::lock_guard<std::mutex> lock(modelMutex);

    if (vocab == nullptr){// vocab not loaded yet 
        return "";
    }
    std::string response{};
    for (auto id : tokens) {
        char buf[256];
        int32_t n = llama_token_to_piece(vocab, id, buf, sizeof(buf), 0, true);
        // if (n < 0) {
        //     fprintf(stderr, "%s: error: failed to convert token to piece\n", __func__);
        //     return 1;
        // }
        std::string piece(buf, static_cast<std::string::size_type>(n));
        response += piece;
    }
    return response; 
    // static_cast<std::string::size_type>
}



std::vector<llama_token> LLMController::infer(std::vector<llama_token>& inTokens, size_t maxLength)
{
    std::lock_guard<std::mutex> lock(modelMutex);

    if (model == nullptr || ctx == nullptr || smpl == nullptr){// model/ ctx not loaded yet 
        return std::vector<llama_token>();
    }

    std::vector<llama_token> outTokens{};
    
    // Prepare a batch for the prompt

    llama_batch batch = llama_batch_get_one(inTokens.data(), inTokens.size());
    llama_token new_token_id;
    
    uint32_t n_ctx = llama_n_ctx(ctx);
    // int n_decode = 0;

    // from 0 up to the length of the context basically
    for (int n_pos = 0; n_pos + batch.n_tokens < static_cast<int32_t>(n_ctx); ) {
        if (llama_decode(ctx, batch)) {
            // problem 
            return outTokens;
        }
        n_pos += batch.n_tokens;

        // Sample the next token
        new_token_id = llama_sampler_sample(smpl, ctx, -1);

        // is it an end of generation?
        if (llama_vocab_is_eog(vocab, new_token_id)) {
            break;
        }
        outTokens.push_back(new_token_id);

        // Prepare the next batch with the sampled token
        batch = llama_batch_get_one(&new_token_id, 1);
    }
    return outTokens;
}
        
std::string LLMController::generate(std::string prompt, size_t maxLength)
{
    
    status.store(LLMStatus::Generating);
    std::vector<llama_token> inTokes = tokenize(prompt);
    std::vector<llama_token> outTokes = infer(inTokes, maxLength);
    status.store(LLMStatus::WaitingForJobs);
    return detokenize(outTokes);
}


bool LLMController::readyToGenerate()
{
    std::lock_guard<std::mutex> lock(modelMutex);

    if (model == nullptr || ctx == nullptr || smpl == nullptr){// model/ ctx not loaded yet 
        std::cout << "LLMController::readyToGenerate not ready" << std::endl;
        if (model == nullptr) std::cout << "Model not loaded yet " << std::endl;
        if (ctx == nullptr) std::cout << "Context not setup yet " << std::endl;
        if (smpl == nullptr) std::cout << "Sampler not loaded yet " << std::endl;
        
        return false;
    }
    return true; 
}



void LLMController::addPromptToQueue(std::string prompt)
{
    promptQ.push(prompt); // has its own mutex
}


// Adds a new prompt-response pair to the history
void LLMController::addPromptResponse(const std::string& prompt, const std::string& response) 
{
    std::lock_guard<std::mutex> lock(historyMutex);
    promptAndResponseHistory.emplace_back(prompt, response);
}

// Retrieves a copy of the entire prompt-response history
std::vector<std::pair<std::string, std::string>> LLMController::getPromptResponseHistory() 
{
    std::lock_guard<std::mutex> lock(historyMutex);
    return promptAndResponseHistory;
}

LLMStatus LLMController::getStatus() const {
    return status.load();  // atomic, thread-safe read
}

std::string LLMController::getStatusString() const {
    switch (status.load()) {
        case LLMStatus::WaitingForJobs: return "Waiting for jobs";
        case LLMStatus::Generating:     return "Generating";
        case LLMStatus::ModelNotReady:  return "Model not ready";
        default: return "Unknown";
    }
}

