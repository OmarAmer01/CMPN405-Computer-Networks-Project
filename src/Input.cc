
#include "Input.h"
#include <iostream>
#include<fstream>
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
        if(line.empty())
        {
            break;
        }
        coordFile.push_back(parseCoordLine(line));
    }
    return coordFile;

}

nodeFileLine Input::parseNodeLine(string line){
    nodeFileLine nodeLine;
    
    // we know that the first 4 chars are always of the error nibble.
    nodeLine.errorNibble = stoi(line.substr(0,4));

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
        if(line.empty())
        {
            break;
        }
        nodeFile.push_back(parseNodeLine(line));
    }
    return nodeFile;

}

Input::~Input()
{
    // TODO Auto-generated destructor stub
}
