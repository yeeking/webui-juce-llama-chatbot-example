#pragma once 

#include "../libs/httplib.h"
#include <JuceHeader.h>
#include "Utils.h" 

class LlamaPluginProcessor; // forward declaration to avoid circular include 

class HttpServerThread : public juce::Thread {
public:
    HttpServerThread() = delete;

    HttpServerThread(LlamaPluginProcessor& _pluginProc);
    void run() override;

    void stopServer();
private:

    void initAPI();
    LlamaPluginProcessor& pluginProc; 
    httplib::Server svr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HttpServerThread)

};