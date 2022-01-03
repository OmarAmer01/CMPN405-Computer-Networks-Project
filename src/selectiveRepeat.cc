#include "selectiveRepeat.h"
vector<frame> Window::castAllNodeFileToFrame(vector<nodeFileLine> nodeFileLineV)
{
    /*Simple data transformation, loop on the given nodeFileVector and convert each nodeFileLine to a frame struct.*/
    for (int i = 0; i < nodeFileLineV.size(); i++)
    {
        frame newFrame;
        newFrame.errorNibble = nodeFileLineV[i].errorNibble;
        newFrame.msgTxt = nodeFileLineV[i].payLoad;
        newFrame.seqNum = -1;            // This is the default value for the seqNum. [Uninitialized]
        newFrame.timeOut = frameTimeOut; // The timeout initial value is constant for all frames.
        allMsgs.push_back(newFrame);
    }
    return allMsgs;
}

Window::Window(int windowSize, vector<nodeFileLine> nodeFileVector, float frameTimeOut)
{

    this->windowSize = windowSize;
    this->nodeFileVector = nodeFileVector;
    this->windowStart = 0;
    this->windowEnd = windowSize - 1;
    this->frameTimeOut = frameTimeOut;

    this->allMsgs = castAllNodeFileToFrame(nodeFileVector); // This fills the allMsgs vector with the frames after transformation from nodeFileLine to frames.
                                                            // Our window is a subset of the allMsgs vector, and will slide across said vector.

    // This fills the window vector with the first windowSize frames.
    for (int i = 0; i < windowSize; i++)
    {
        window.push_back(allMsgs[i]);
    }
}



void Window::slideWindow()
{
    this->windowStart++;
    this->windowEnd++;

    // If we cant slide anymore, we then we have finished sending. End the simulation.

    /// TODO: Check if the we actually end the simulation here
    /// TODO: Check id this is the right way to do it.
    if (windowEnd >= allMsgs.size())
    {
        EV << "REACHED THE END";
        omnetpp::cSimulation::getActiveSimulation()->getSystemModule()->callFinish();
    }

    // Slide the window vector
    for (int i = 0; i < windowSize; i++)
    {
        window[i] = allMsgs[windowStart + i];
    }

    
}

frame Window::operator[](int index)
{
    // if the index is out of bounds throw an exception
    if (index < 0 || index >= windowSize)
    {
        throw std::out_of_range("Index out of bounds");
    }
    else
    {
        return window[index];
    }
}
void Window::printWindow()
{
    for (int i = 0; i < windowSize; i++)
    {
        EV << "Window[" << i << "] [PAYLOAD] = " << window[i].msgTxt << endl
             << "Window[" << i << "] [SEQ NUM] = " << window[i].seqNum << endl
             << "Window[" << i << "] [TIM OUT] = " << window[i].timeOut << endl
             << endl;
    }
}