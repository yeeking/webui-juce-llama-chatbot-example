# Minimal C++ LLM chat bot example

Uses web ui + JUCE + llamacpp CMake C++ 

## Building the project

```
# clone this project
git clone git@github.com:yeeking/webui-juce-llama-chatbot-example.git

# cd into the project libs folder
cd webui-juce-llama-chatbot-example/plugin/libs

# get the JUCE and llama cpp repos
git clone git@github.com:juce-framework/JUCE.git
git clone git@github.com:ggml-org/llama.cpp.git

# generate the project
cd ..
cmake -B build . # don't forget the dot at the end :)

# build
cmake --build build --config Debug   -j 10 # number of threads to use

```


