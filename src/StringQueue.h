#pragma once 

#include <string>
#include <JuceHeader.h>

class StringQueue
{
public:
    StringQueue(int capacity); 

    bool push(const std::string& str);
    bool pop(std::string& result);
    std::vector<std::string> getSnapshot();


private:
    juce::AbstractFifo fifo;
    juce::Array<std::string> buffer;
    mutable std::mutex queueMutex;
};
