#include <string>
#include "Input.h"
#include <omnetpp.h>
#include "dataMsg_m.h"
using namespace std;

class frame
{
public:
    string payLoad;
    string errorNibble;
    int ackNack;     // 1 = ack, 0 = nack
    int seqNum; // Relative to window

    bool received; // True if frame has been received correcrtly
                   // Used with the receiving window

    bool acked; // True if frame has been acked

    float timeOut; // Timeout for frame

    frame(string payLoad, string errorNibble, int ackNack, int frameNumber);
    frame(nodeFileLine nodeFL);
    void printFrame();
};

class window
{   // This class is a vector of frames.
    // In the node class, we will have two windows
        // One for the sending window
        // Another for the receiving window
        // Assume sending window has the same size as the receiving window

    int windowSize;
    vector<frame> frames;
    window(int windowSize);
    void printWindow();

    // Gets/sets the frame with the given sequence number
    frame& operator[](int seqNum);


};









