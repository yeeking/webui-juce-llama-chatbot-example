#include "HTTPServer.h"
#include "PluginProcessor.h"

HttpServerThread::HttpServerThread(LlamaPluginProcessor& _pluginProc)  : 
   juce::Thread("HTTP Server Thread"), 
   pluginProc{_pluginProc}
{
    initAPI();
}


void HttpServerThread::initAPI()
{
    svr.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        DBG("HttpServerThread::log " << req.method << " " << req.path 
                  << " -> " << res.status);
    });

    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        DBG("Got binary file " << BinaryData::originalFilenames[i]);
    }

#ifdef LOCAL_WEBUI    
    ///** start of server from file system code 
    // this code can be used in the standalone app 
    // to serve files from its 'ui' subdirectory
    // for easy testing where you can edit the 
    std::string workingDir = getBinary().string();
    std::string uiDir = workingDir + "/../../../../src/ui/";
    std::cout << "HTTPServer::  TESTING UI MODE: Serving from " << uiDir << std::endl;
    auto ret = svr.set_mount_point("/", uiDir);
    if (!ret) {
        // The specified base directory doesn't exist...
        std::cout << "Warning: trying to serve from a folder that does not exist " << std::endl;
    }
      
    ///**  end of server from file system code 

#else
    std::cout << "HTTPServer:: serving from memory not disk " << std::endl;

    // Deploy version
    // route for the main index file
    /// *** start of serving files from the linked 'binary'
    svr.Get("/index.html", [](const httplib::Request& req, httplib::Response& res) {
        int size = 0;
        const char* data = BinaryData::getNamedResource(BinaryData::namedResourceList[0], size);
    
        if (data != nullptr) {
            res.set_content(data, static_cast<size_t>(size), "text/html");
        } else {
            res.status = 404;
            res.set_content("404: File not found", "text/plain");
        }
    });
    ///*** end of serving files from the linked 'binary'
#endif



    // 'live' responders for button presses - call to the pluginprocessor 
    svr.Get("/button1", [this](const httplib::Request& req, httplib::Response& res) {
        std::cout << "button 1 clicked" << std::endl;
    });
    svr.Get("/button2", [this](const httplib::Request& req, httplib::Response& res) {
        std::cout << "button 2 clicked" << std::endl;
    });
    svr.Get("/echo", [this](const httplib::Request& req, httplib::Response& res) {
        std::string prompt = req.get_param_value("q");
        res.set_content("echo: " + prompt, "text/html");
    }); 

    svr.Get("/loadModel", [this](const httplib::Request& req, httplib::Response& res) {
        std::string modelPath = req.get_param_value("q");
        bool loadRes = this->pluginProc.apiLoadModel(modelPath);
        res.set_content("{result:"+std::to_string(loadRes)+"}",  "application/json");
    });


    svr.Get("/LLMStatus", [this](const httplib::Request& req, httplib::Response& res) {
        std::string status = this->pluginProc.apiLLMStatus();
        res.set_content(status, "text/html");
    }); 

    svr.Get("/prompt", [this](const httplib::Request& req, httplib::Response& res) {
        std::string prompt = req.get_param_value("q");
        this->pluginProc.apiPrompt(prompt);
        res.set_content("sent to processor: " + prompt, "text/html");
    }); 

    svr.Get("/allPrompts", [this](const httplib::Request& req, httplib::Response& res) {
        // get all the prompts currently stored and convert them to a JSON array as a string
        std::vector<std::pair<std::string, std::string>> prompts = this->pluginProc.apiAllPrompts();
        std::cout << "API server allPrompts prompt history len " << prompts.size() << std::endl;
        juce::Array<juce::var> varArray;
        for (const std::pair<std::string, std::string>& prompt : prompts)
            varArray.add(juce::var("Prompt:" + prompt.first + "\nAnswer:" + prompt.second));
        juce::var jsonVar(varArray);
        juce::String jsonArrayString = juce::JSON::toString(jsonVar, true); // 'true' for compact single-line output   
        res.set_content(jsonArrayString.toStdString(), "application/json");
    });
    
     

}


void HttpServerThread::run()
{

    DBG("API server starting");

    // Run the server in a blocking loop until stopThread() is called
    svr.listen("0.0.0.0", 8080);

}

void HttpServerThread::stopServer()
{
    DBG("API server shutting down");

    svr.stop();
    stopThread(1000); // Gracefully stop thread
}
    

