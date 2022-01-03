// Make a new class called window, which will be used to store the frames
#include <vector>
#include "Input.h"
#include <omnetpp.h>

struct frame
{
    string errorNibble;
    string msgTxt;
    int seqNum; // sequence number in the window
    int msgID; // message ID (relative to all other messages)
    float timeOut;
};

class Window
{
public:
    // Data members
    int windowSize;
    int windowStart;
    int windowEnd;
    float frameTimeOut;
    vector<nodeFileLine> nodeFileVector; // This is the vector that will store the text of the messages. (called nodeFileVector)
                                         // It is taken as an input to the class

    vector<frame> allMsgs; // This is the vector that will store the all the frames.
                           // our windows is a subset of this vector, and will slide across the whole vector.
    vector<frame> window;

    // Methods
public:
    vector<frame> castAllNodeFileToFrame(vector<nodeFileLine> nodeFileVector);
    Window(int windowSize, vector<nodeFileLine> window, float frameTimeOut);
    void slideWindow();
    frame operator[](int);
    void printWindow();
};
