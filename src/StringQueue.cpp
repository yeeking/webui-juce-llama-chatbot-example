#include "StringQueue.h"

StringQueue::StringQueue(int capacity) : fifo(capacity), buffer()
{
    buffer.ensureStorageAllocated(capacity);
}

bool StringQueue::push(const std::string& str)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    int start1, size1, start2, size2;
    fifo.prepareToWrite(1, start1, size1, start2, size2);
    if (size1 > 0)
    {
        buffer.set(start1, str);
        fifo.finishedWrite(1);
        std::cout << "String Q pushed " << str << std::endl;
        return true;
    }
    return false; // queue full
}

bool StringQueue::pop(std::string& result)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    int start1, size1, start2, size2;
    fifo.prepareToRead(1, start1, size1, start2, size2);
    if (size1 > 0)
    {
        result = buffer[start1];
        fifo.finishedRead(1);
        std::cout << "String Q popped " << result << std::endl;
        return true;
    }
    return false; // queue empty
}

std::vector<std::string> StringQueue::getSnapshot()
{
    std::vector<std::string> snapshot;

    std::lock_guard<std::mutex> lock(queueMutex);

    int numReady = fifo.getNumReady();
    if (numReady == 0)
        return snapshot;

    int start1, size1, start2, size2;
    fifo.prepareToRead(numReady, start1, size1, start2, size2);

    // Copy the first block
    for (int i = 0; i < size1; ++i)
        snapshot.push_back(buffer[start1 + i]);

    // Copy the second block if it exists
    for (int i = 0; i < size2; ++i)
        snapshot.push_back(buffer[start2 + i]);

    // Note: We do not call fifo.finishedRead() here to avoid modifying the queue state

    return snapshot;
}



