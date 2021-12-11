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

#include "node.h"
#include "Input.h"
#include "ctrlMsg_m.h"

Define_Module(Node);

void Node::initialize()
{
}

void Node::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage())
    {
        EV << "Start Time Now.";
    }
    else if (check_and_cast<CtrlMsg_Base *>(msg))
    {

        CtrlMsg_Base *ctrlMsg = check_and_cast<CtrlMsg_Base *>(msg);

        // EV << "Name: " << ctrlMsg->getName() << endl
        //    << "ID: " << ctrlMsg->getNodeId() << endl
        //    << "fileName: " << ctrlMsg->getFName() << endl
        //    << "Starter: " << ctrlMsg->getStart() << endl
        //    << "StartTime: " << ctrlMsg->getStartTime() << endl;

        ctrlMsg->setName("selfCtrlMsg");
        if (!ctrlMsg->getStart())
        {
            return;
        }

        if (!nodeFileRead)
        {
            scheduleAt(ctrlMsg->getStartTime() + simTime(), ctrlMsg);
            nodeFileRead = true;
        }
    }
}
