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
#include "dataMsg_m.h"
#include "Crc.h"
using namespace std;
Define_Module(Node);

void Node::initialize()
{
}

void Node::handleMessage(cMessage *msg)
{

    // check if the coordinator or the other node is the sender
    // if the coordnator is the sender
        // if (start)
              // readInput("input file name reieved from the coordinator"); ---- should return array of strings
    // else if the other node is the sender
        //increment id
                                // if (Ack)
                                    // if loop idex < inputfile size
                                        //increment loop index
                                    // else
                                        // out put the sending and recieving are all done
                                // else if (nAck)
                                    // it will send the same msg again

    // loop on the array ----- note make the loop index as parameter to increase it at every recieve
        // {
        // cast the message...to dataMsg
        // calculate CRC...................done Amer
        // modify the message.......................a5er haga
        // framing
        // send the msg (Payload + CRC)
        // output a msg is sent ...... output function still under work (look at your notes, yasser)
        //}

    // if loop index == inputfile size
        // output end of sending

    if (msg->isSelfMessage())
    {
        //schedule the first msg
        EV << "Start Time Now."<<endl;
        //dataMsg_Base *dataMsg = check_and_cast<dataMsg_Base *>(msg);
        //dataMsg->setsendingTime(nodeStartingtime + simTime());
       //scheduleAt(nodeStartingtime + simTime(), dataMsg);
        nodeFileRead = true;
       // id++;
       /* DataMsg_Base *dataMsg =new DataMsg_Base("Hi") ;
        //std::string out = "dataIN";
       // std::string gate = out + std::to_string(othernodeID);
        send(dataMsg,"dataOut");
        EV<<"First message is sent successfully"<<endl;*/
        // should handle the first msg here manually as he would exit the loop
    }
    else if (dynamic_cast<CtrlMsg_Base *>(msg))
    {
        EV << "I am a node and I recieved msg from the coord"<<endl;
        // the sender is the coordinator
        CtrlMsg_Base *ctrlMsg = check_and_cast<CtrlMsg_Base *>(msg);

        nodeId=ctrlMsg->getNodeId();
        if (nodeId%2==0)
            othernodeID=nodeId+1;
        else
            othernodeID=nodeId-1;

        ctrlMsg->setName("selfCtrlMsg");
        if (!ctrlMsg->getStart())
        {
            // not starting node
            isStartingNode=false;
            return;
        }
        else
        {
            // the starting node

            Input *input = new Input();
            this->nodeFileVector = input->parseNodeFile(ctrlMsg->getFName());
            isStartingNode=true;
            EV << "I am the starting node, I read my file"<<endl;
            EV<<"First line:"<<nodeFileVector[0].payLoad;
            if (!nodeFileRead)
             {
                 scheduleAt(ctrlMsg->getStartTime() + simTime(), ctrlMsg);
                 nodeFileRead = true;
             }
            output=new Input();
            output->openLogFile(nodeId, othernodeID);
            return;
        }

    }

    if((dynamic_cast<CtrlMsg_Base *>(msg) && id==-1) || dynamic_cast<DataMsg_Base *>(msg)  )
    {

        if (id==-1) //this wwould handle the first message only
        {
            id=-1;
            EV<<"first message is about to be sent "<<endl;
        }

        if (isStartingNode==true) // the sender side
        {
            //sender main loop
            //increment the id to get the next message to be sent , then get that input line
           id++;
           if (dynamic_cast<DataMsg_Base *>(msg))
           {
               DataMsg_Base *dataMsg = check_and_cast<DataMsg_Base *>(msg);
               if (dataMsg->getPiggy()==1)
               {
                   //ack
               }
               else
               {
                   //nAck
               }
           }
           nodeFileLine inputLine;
           int total_num_msg =nodeFileVector.size();
           inputLine =nodeFileVector[id];
           //get all the inf. needed from the inputLine (payload, error bits)
           string errorBits= inputLine.errorNibble;
           string payload =  inputLine.payLoad;
           //calculate the CRC for the msg
           string CRC="0000";//---------------------------should be modified

           if (errorBits=="0000")
           {
            //NO error .... send the msg without modifying it
           }
           else
           {
             //  there is an error...modify accordingly
           }
           //frame and send the msg
           DataMsg_Base * sendMsg = new DataMsg_Base(payload.c_str());
           sendMsg->setSeq_Num(id);
           // we need to frame the payload before sending it
           int size = payload.size();
           int add=0;
           for (int i = 0; i < size; i++)
           {
               if (payload[i] == '$' || payload[i] == '/')
                   add++;
           }
           for (int i = 0; i < size + add; i++)
           {
               if (payload[i] == '$' || payload[i] == '/')
               {
                   payload.insert(i, 1, '/');
                   i++;
               }
           }
           payload="$"+payload+"$";
           sendMsg->setM_Payload(payload.c_str());
           sendMsg->setMycheckbits(CRC.c_str());
           sendMsg->setSendingTime(simTime().dbl());

           send(sendMsg, "dataOut");
           EV<<"sent message with id "<<sendMsg->getSeq_Num()<< " and content of"<<sendMsg->getM_Payload()<<endl;
           output->WriteToFile(nodeId, othernodeID, true, sendMsg->getSeq_Num(),
                   sendMsg->getM_Payload(), sendMsg->getSendingTime(), errorBits, 1);
           // if the sender finishes sending data. output file
           EV<<"id"<<id << "total lines os msg"<<total_num_msg<<endl;
           if (id==total_num_msg-1)
           {

               output->WriteFinishLine(nodeId, othernodeID, true);
               output->WriteStatsLine(nodeId, othernodeID, simTime().dbl(), 10, 1.333);
           }
        }
        else // reciever
        {
            //increase the # of trans and trnastime
            DataMsg_Base *dataMsg = check_and_cast<DataMsg_Base *>(msg);
            std::string out = "out";
            std::string gate = out + std::to_string(othernodeID);
           // send(dataMsg, "dataOut");
            sendDelayed(dataMsg, 0.2, "dataOut");
            EV<<"Recieved message with id "<<dataMsg->getSeq_Num()<< " and content of"<<dataMsg->getM_Payload()<<endl;
            output->WriteToFile(othernodeID, nodeId,false, dataMsg->getSeq_Num(), dataMsg->getM_Payload(), simTime().dbl(), "0000", 1);
        }

    }


}

