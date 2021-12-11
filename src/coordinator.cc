//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "coordinator.h"
#include "Input.h"

Define_Module(Coordinator);

void Coordinator::initialize()
{
    // Read file from coordinator.txt
    Input *input = new Input();
    this->coordFileVector = input->parseCoordFile("coordinator.txt");

    // Send the data to the other nodes.
    sendCtrlMsg2All();
}

void Coordinator::printCoordData()
{
    for (int idx = 0; idx < coordFileVector.size(); idx++)
    {
        coordFileLine line = coordFileVector[idx];
        cout << "NODE ID: " << line.nodeId << endl
             << "NODE NAME: " << line.fName << endl
             << "STARTER?: " << line.start << endl
             << "NODE START: " << line.startTime << endl;
    }
}

CtrlMsg_Base* Coordinator::coordLine2ctrlMsg(coordFileLine line)
{
    CtrlMsg_Base* msg = new CtrlMsg_Base("genericControlMessage");
    msg->setNodeId(line.nodeId);
    msg->setFName(line.fName.c_str());
    msg->setStart(line.start);
    msg->setStartTime(line.startTime);
    return msg;
}

void Coordinator::sendCtrlMsg2All()
{
    for (int idx = 0; idx < coordFileVector.size(); idx++)
    {
        CtrlMsg_Base* msg = coordLine2ctrlMsg(coordFileVector[idx]);
        std::string out = "out";
        std::string gate = out + std::to_string(msg->getNodeId());
        send(msg, gate.c_str());
    }
}



void Coordinator::handleMessage(cMessage *msg)
{
}
