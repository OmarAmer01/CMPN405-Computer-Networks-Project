#include "window.h"
#include <string>
#include <omnetpp.h>
using namespace omnetpp;

// Ctor
frame::frame(string payLoad, string errorNibble, int ackNack, int seqNum)
{
    this->payLoad = payLoad;
    this->errorNibble = errorNibble;
    this->ackNack = ackNack;
    this->seqNum = seqNum;

    this->received = false;
    this->acked = false;
}

// Prints the frame
void frame::printFrame()
{
    EV << "Frame: " << this->seqNum << endl
       << "Timeout: " << this->timeOut << endl
       << "Payload: " << this->payLoad << endl
       << "ErrorNibble: " << this->errorNibble << endl
       << "AckNack: " << this->ackNack << endl;
}

// Gets/sets the frame with the given sequence number
frame &window::operator[](int seqNum)
{ // if the seqNum is out of bounds, throw an exception
    if (seqNum < 0 || seqNum >= this->windowSize)
        throw "Index out of bounds";

    // if all ok, return the frame
    return this->frames[seqNum];
}


frame::frame(nodeFileLine nodeFL)
{
    this->payLoad = nodeFL.payLoad;
    this->errorNibble = nodeFL.errorNibble;
}


// Ctor
window::window(int windowSize)
{
    this->windowSize = windowSize;
    this->frames.resize(windowSize);
    
    for (int i = 0; i < windowSize; i++)
    {
        this->frames[i].seqNum = 0;
        this->frames[i].timeOut = 0;
        this->frames[i].received = false;
        this->frames[i].acked = false;
        this->frames[i].ackNack = 0;
        this->frames[i].payLoad = "";
        this->frames[i].errorNibble = "";
        

    }
}
