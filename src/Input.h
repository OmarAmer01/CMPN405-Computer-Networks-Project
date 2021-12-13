
#ifndef INPUT_H_
#define INPUT_H_

#include <string>
#include <vector>
using namespace std;

struct coordFileLine
{
    int nodeId;
    string fName;
    bool start = false;
    int startTime = -1; // -1 if is not the starting node.
};

struct nodeFileLine
{
    string errorNibble ="0000";
    string payLoad;
};

class Input
{
public:
    Input();
    virtual ~Input();
    vector<coordFileLine> parseCoordFile(string fileName); // returns a vector of parsed lines as structs.
    vector<nodeFileLine> parseNodeFile(string fileName);   // returns a vector of parsed lines as structs.
    void openLogFile(int senderID, int receiverID);
    void WriteToFile(int nodeID,  int otherNodeID,bool isSender, int msgID, string msg, double time, string errors, int ack);
    void WriteStatsLine(int senderID, int receiverID, double totalTime, int totalTranNum, double throuput);
    void WriteFinishLine(int senderID, int receiverID , bool isSender);
    void closeLogFile(int senderID, int receiverID);
    string fname;

private:
    coordFileLine parseCoordLine(string line);
    nodeFileLine parseNodeLine(string line);
};

#endif /* INPUT_H_ */
