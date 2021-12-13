
#include "Input.h"
#include <iostream>
#include <fstream>
#include <string>
Input::Input()
{
    // TODO Auto-generated constructor stub
}

coordFileLine Input::parseCoordLine(string line)
{
    // coord stands for coordinator not coordinates
    // Assuming the input string is error free and is always
    // in the expected format
    // first we count how many spaces there are
    // then we plug the values into the struct

    coordFileLine coordLine;

    // we also know for sure that the first charachter is the node id.
    if (line[0] == '0')
        coordLine.nodeId = 0;
    else
        coordLine.nodeId = line[0] - '0';
    // skip a space, then we have the input file name.
    int i = 2;
    while (line[i] != ' ')
    {
        i++;
    }

    coordLine.fName = line.substr(2, i - 2); // i - 2 because we want to skip the space

    int oldI = i;
    i++;
    while (line[i] != ' ') // Fetch next word if exists
    {
        i++;
        if (i >= line.length() - 1)
        {
            return coordLine;
        }
    }

    if (line.substr(oldI + 1, 5) == "start")
    {
        coordLine.start = true;
    }
    else
    {
        coordLine.start = false;
        return coordLine;
    }

    oldI = i;
    i++;
    while (line[i] != ' ') // Fetch next word
    {
        i++;
    }

    coordLine.startTime = stoi(line.substr(oldI, i));
    return coordLine;
}

vector<coordFileLine> Input::parseCoordFile(string fileName)
{
    // reads the file line by line
    // and parses the line into a struct
    // then pushes the struct into a vector

    vector<coordFileLine> coordFile;
    ifstream coordFileStream(fileName);
    string line;
    while (getline(coordFileStream, line))
    {
        if (line.empty())
        {
            break;
        }
        coordFile.push_back(parseCoordLine(line));
    }
    return coordFile;
}

nodeFileLine Input::parseNodeLine(string line)
{
    nodeFileLine nodeLine;

    // we know that the first 4 chars are always of the error nibble.
    nodeLine.errorNibble = line.substr(0, 4); // maybe use stoi?

    // starting from the 5th charachter, everything that follows is the payload.
    nodeLine.payLoad = line.substr(5);

    // done.
    return nodeLine;
}

vector<nodeFileLine> Input::parseNodeFile(string fileName)
{
    // reads the file line by line
    // and parses the line into a struct
    // then pushes the struct into a vector

    vector<nodeFileLine> nodeFile;
    ifstream nodeFileStream(fileName);
    string line;
    while (getline(nodeFileStream, line))
    {
        if (line.empty())
        {
            break;
        }
        nodeFile.push_back(parseNodeLine(line));
    }
    return nodeFile;
}

void Input::openLogFile(int nodeID)
{
    fstream my_file;
    string sender;
    string receiver;
    if (nodeID % 2 == 0)
    {
        sender = to_string(nodeID);
        receiver = to_string(nodeID + 1);
    }
    else
    {
        sender = to_string(nodeID-1);
        receiver = to_string(nodeID);
    }
    string FileName = "pair" + sender + receiver + ".txt";

    my_file.open(FileName, ios::out);
    if (!my_file)
    {
        cout << "File not created!";
    }
    else
    {
        cout << "File created successfully!";
    }
}

void Input::WriteToFile(int nodeID, bool isSender, int msgID, string msg, double time, string e, int ack)
{
    //otherNode ID might be used
    fstream my_file;
    string sender;
    string receiver;
    if (nodeID % 2 == 0)
    {
        sender = to_string(nodeID);
        receiver = to_string(nodeID + 1);
    }
    else
    {
        sender = to_string(nodeID-1);
        receiver = to_string(nodeID);
    }
    string FileName = "pair" + sender + receiver + ".txt";
    my_file.open(FileName, std::ios::app);
    string sendorNOT;
    string error;
    string mod = "";
    string dup = "";
    string delay = "";
    string loss = "";
    if (e[0] == 1)
        mod = "modification,";
    if (e[1] == 1)
        dup = " loss,";
    if (e[2] == 1)
        delay = " duplicated,";
    if (e[3] == 1)
        loss = " delay,";
    error = mod + loss + dup + delay;

    //int nodeID;
    if (isSender)
    {
        sendorNOT = "sends";
       // nodeID = senderID;
    }
    else
    {
        sendorNOT = "received";
     //   nodeID = receiverID;
    }
    my_file << "- node" << nodeID << " " << sendorNOT << " message with id=" << msgID
            << " and content= " << msg << " at " << time << " with " << error
            << " and piggybacking Ack number " << ack << endl;
    my_file.close();
}

// print error message (timeout , drop)
// print finish msg
void Input::WriteFinishLine(int nodeID, bool isSender)
{
    fstream my_file;
    string sender;
    string receiver;
    if (nodeID % 2 == 0)
    {
        sender = to_string(nodeID);
        receiver = to_string(nodeID + 1);
    }
    else
    {
        sender = to_string(nodeID-1);
        receiver = to_string(nodeID);
    }
    string FileName = "pair" + sender + receiver + ".txt";
    my_file.open(FileName, std::ios::app);
    if (isSender)
        my_file << "node" << nodeID << " end of input file" << endl;
    else
        my_file << "node" << nodeID << " end of input file" << endl;
    my_file.close();
}
// print stats
void Input::WriteStatsLine(int nodeID, double totalTime, int totalTranNum, double throuput)
{
    fstream my_file;
    string sender;
    string receiver;
    if (nodeID % 2 == 0)
    {
        sender = to_string(nodeID);
        receiver = to_string(nodeID + 1);
    }
    else
    {
        sender = to_string(nodeID-1);
        receiver = to_string(nodeID);
    }
    string FileName = "pair" + sender + receiver + ".txt";
    my_file.open(FileName, std::ios::app);
    my_file << "------------------------------------------------------" << endl;
    my_file << "total transmission time= " << totalTime << endl
            << "total number of transmissions= " << totalTranNum << endl
            << "the network throughput=" << throuput << endl;
    my_file.close();
}

void Input::closeLogFile(int nodeID)
{
    fstream my_file;
    string sender;
    string receiver;
    if (nodeID % 2 == 0)
    {
        sender = to_string(nodeID);
        receiver = to_string(nodeID + 1);
    }
    else
    {
        sender = to_string(nodeID-1);
        receiver = to_string(nodeID);
    }
    string FileName = "pair" + sender + receiver + ".txt";
    my_file.close();
}
Input::~Input()
{
    // TODO Auto-generated destructor stub
}
