/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LlamaPluginEditor::LlamaPluginEditor (LlamaPluginProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), 
    webView{
      // no idea if this stuff is needed... but it looks useful
      juce::WebBrowserComponent::Options{}
      .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
      .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
      .withBackgroundColour(juce::Colours::blue)
          // this may be necessary for some DAWs; include for safety
      .withUserDataFolder(juce::File::getSpecialLocation(
          juce::File::SpecialLocationType::tempDirectory)))
  }
{
  juce::ignoreUnused (audioProcessor);
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.

  setSize (800, 600);

  // Load the webpage from the local server
  webView.goToURL("http://127.0.0.1:8080/index.html");  // Adjust port if needed

  // Add WebView component to the editor
  addAndMakeVisible(webView);
  
}

LlamaPluginEditor::~LlamaPluginEditor()
{
}

//==============================================================================
void LlamaPluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void LlamaPluginEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    webView.setBounds(getLocalBounds()); // Make web view fill entire UI
}

void LlamaPluginEditor::updateUIFromProcessor(const juce::String& msg)
{  
    // create an object and convert it to a json string 
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("msg", msg);
    juce::var data(obj);
    juce::String json = juce::JSON::toString(data);
    // eval some javascript in the webui which causes it to call
    // a function called handleCppMessage with the data
    webView.evaluateJavascript("handleCppMessage(" + json + ");", 
        [](juce::WebBrowserComponent::EvaluationResult result) {
            std::cout << "PluginEditor::updateUIFromProcessor js eval completed with res : " << result.getResult() << std::endl;
            // code to run once the javascript has been evaluated ... 
            // e.g. maybe re-enable the animation loop if you are waiting
            // for a UI update 
        }
    );
    
    // std::string msg_as_json = "{\"msg\":\""+ msg + "\"}";
    // juce::JSON::toString({"msg":msg});
    // webView.evaluateJavascript("backendCall('"+msg_as_json+"')");
}
