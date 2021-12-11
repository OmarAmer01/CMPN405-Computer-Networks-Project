
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
    int errorNibble = 0;
    string payLoad;
};

class Input
{
public:
    Input();
    virtual ~Input();
    vector<coordFileLine> parseCoordFile(string fileName); // returns a vector of parsed lines as structs.
    vector<nodeFileLine> parseNodeFile(string fileName);   // returns a vector of parsed lines as structs.

private:
    coordFileLine parseCoordLine(string line);
    nodeFileLine parseNodeLine(string line);
};

#endif /* INPUT_H_ */
