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
#include <random>

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
        EV << "Start Time Now." << endl;
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
        EV << "I am a node and I recieved msg from the coord" << endl;
        // the sender is the coordinator
        CtrlMsg_Base *ctrlMsg = check_and_cast<CtrlMsg_Base *>(msg);

        nodeId = ctrlMsg->getNodeId();
        if (nodeId % 2 == 0)
            othernodeID = nodeId + 1;
        else
            othernodeID = nodeId - 1;

        ctrlMsg->setName("selfCtrlMsg");
        if (!ctrlMsg->getStart())
        {
            // not starting node
            isStartingNode = false;
            return;
        }
        else
        {
            // the starting node

            Input *input = new Input();
            this->nodeFileVector = input->parseNodeFile(ctrlMsg->getFName());
            isStartingNode = true;
            EV << "I am the starting node, I read my file" << endl;
            EV << "First line:" << nodeFileVector[0].payLoad;
            if (!nodeFileRead)
            {
                scheduleAt(ctrlMsg->getStartTime() + simTime(), ctrlMsg);
                nodeFileRead = true;
            }
            output = new Input();
            output->openLogFile(nodeId);
            return;
        }
    }

    if ((dynamic_cast<CtrlMsg_Base *>(msg) && id == -1) || dynamic_cast<DataMsg_Base *>(msg))
    {

        if (id == -1) //this wwould handle the first message only
        {
            id = -1;
            EV << "first message is about to be sent " << endl;
        }

        if (isStartingNode == true) // the sender side
        {
            //sender main loop
            //increment the id to get the next message to be sent , then get that input line
            id++;
            if (dynamic_cast<DataMsg_Base *>(msg))
            {
                DataMsg_Base *dataMsg = check_and_cast<DataMsg_Base *>(msg);
                if (dataMsg->getPiggy() == 1)
                {
                    //ack
                    EV << "I am the sender and I recieved an ack" << endl;
                }
                else
                {
                    //nAck
                    EV << "I am the sender and I recieved a NACK" << endl;
                }
            }
            nodeFileLine inputLine;
            int total_num_msg = nodeFileVector.size();
            inputLine = nodeFileVector[id];
            //get all the inf. needed from the inputLine (payload, error bits)
            string errorBits = inputLine.errorNibble;
            string payload = inputLine.payLoad;

            // Now, for making the errors
            // This error nibble is arranged as follows, with a one meaning that the error will be applied
            // [Modification, Loss, Duplication, Delay]

            char singleBitMod = errorBits[0];
            char packetLoss = errorBits[1];
            char packetDup = errorBits[2];
            char packetDelay = errorBits[3];

            EV << "singleBitMod: " << singleBitMod << endl;
            EV << "packetLoss: " << packetLoss << endl;
            EV << "packetDup: " << packetDup << endl;
            EV << "packetDelay: " << packetDelay << endl;

            /// TODO: Dup Error
            /// TODO: Loss Error
            /// FIXME: delay error ==> only the first message gets delayed
            /// FIXME: error bits dont get assigned correctly, y3ny msln adeelo 0100 y3ml 7aga tanya, etc

            //frame and send the msg
            DataMsg_Base *sendMsg = new DataMsg_Base(payload.c_str());
            sendMsg->setSeq_Num(id);
            // we need to frame the payload before sending it
            int size = payload.size();
            int add = 0;

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
            payload = "$" + payload + "$";
            Crc *crcObj = new Crc();
            string CRC = "CRCBYTE";

            unsigned int CRCInt = crcObj->crc8Alt(payload);

            // convert the CRCint to string
            stringstream ss;
            ss << CRCInt;
            CRC = ss.str();

            if (singleBitMod == '1')
            {

                EV << "CHNAGE BIT ERROR" << endl;
                EV << "payload before error:" << payload << endl;

                // generate a random integer between 1 and payload length-1 (select random character)
                /// TODO: check if this is correct
                int randomIndex = intuniform(1, size - 1);

                // choose a random number as the bit index for the randomly chosen charachter
                int randomBitIndex = rand() % 8;

                // flip the bit at that index to a random bit
                payload[randomIndex] = payload[randomIndex] ^ (1 << randomBitIndex);

                /// TODO: Output that this message was modified

                EV << "Payload after modification:" << payload << endl;
            }

            if (packetDup == '1')
            {
                EV << "PACKET DUPLICATION ERROR" << endl;
            }

            sendMsg->setM_Payload(payload.c_str());
            sendMsg->setMycheckbits(CRC.c_str());
            sendMsg->setSendingTime(simTime().dbl());

            //send(sendMsg, "dataOut");
            output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                sendMsg->getM_Payload(), sendMsg->getSendingTime(), errorBits, 1);
            if (id == total_num_msg - 1)
            {
                output->WriteFinishLine(nodeId, true);
                output->WriteStatsLine(nodeId, simTime().dbl(), id + 1 + duplicates, 1.333);
            }
            else
            {
                //send the next message

                if (packetDelay == '1') // if the error nibble has the delay error
                {
                    // reads the parameter from the node.ned file
                    // and sets the delay time
                    double delayTime = par("delayPeriod").doubleValue();
                    EV << "PACKET DELAY ERROR WITH DELAY = " << delayTime << endl;

                    // send the message with a delay
                    EV << "ERROR HERE" << endl;
                    sendDelayed(sendMsg, delayTime, "dataOut");
                }

                if (packetLoss == '1')
                {
                    // Losing the packet means that we will deadlock-wait for an ack/nack.
                    // So, we will send the message again after a random time.
                    // This is a hack, but it works.
                    /// FIXME: remove the sendDelayed and turn it into a flag instead

                    //get random double
                    int randTime = rand() % 3;
                    sendDelayed(sendMsg, randTime, "dataOut");
                    EV << "PACKET LOSS ERROR" << endl;
                }
                else
                {

                    send(sendMsg, "dataOut");
                }
            }
            // EV << "sent message with id " << sendMsg->getSeq_Num() << endl
            //  << " and content of" << sendMsg->getM_Payload() << endl;

            // if the sender finishes sending data. output file
            //EV << "id" << id << "total lines os msg" << total_num_msg << endl;
        }
        else // reciever
        {
            // The reciever needs to check whether the recicved message is correct or not,
            // we do this by reading the message and comparing the CRC
            // if the CRC is correct, then send an ACK
            // if the CRC is not correct, then send a NACK

            // We also need to check whether the message is duplicated or not
            // if it is duplicated, then we will send an NACK
            // if it is not duplicated, then we will send an ACK


            // Check duplicate:
            
        
            // Here we calculate the CRC8 of the message
            int reCalcCrc8 = 0;

            DataMsg_Base *dataMsg = check_and_cast<DataMsg_Base *>(msg);
            string payload = dataMsg->getM_Payload();
            string CRC = dataMsg->getMycheckbits();

            Crc *crcObj = new Crc();
            reCalcCrc8 = crcObj->crc8Alt(payload);

            stringstream ss;
            ss << reCalcCrc8;
            string reCalcCrc8Str = ss.str();

            

            //increase the # of trans and trnastime

            std::string out = "out";
            std::string gate = out + std::to_string(othernodeID);
            //send(dataMsg, "dataOut");
            EV << "RECIEVED CRC = " << CRC << endl;
            EV << "RECALC CRC = " << reCalcCrc8Str << endl;

            if (reCalcCrc8Str == CRC)
            {
                // send the ACK
                dataMsg->setPiggy(1);
            }
            else
            {
                dataMsg->setPiggy(0);
            }

            sendDelayed(dataMsg, 0.2, "dataOut");
            //EV << "Recieved message with id " << dataMsg->getSeq_Num() << endl
            //   << " and content of" << dataMsg->getM_Payload() << endl
            //   << "With ACK bit " << dataMsg->getPiggy() << endl;

            output->WriteToFile(nodeId, false, dataMsg->getSeq_Num(), dataMsg->getM_Payload(), simTime().dbl(), "0000", 1);
        }
    }
}
