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

/// TODO: Add piggyback ID to the output and correct acknowledge number .

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
    if (msg->isSelfMessage() && dynamic_cast<DataMsg_Base *>(msg))
    {
        lost = true;
    }
    else if (msg->isSelfMessage())
    {

        nodeFileRead = true;
    }

    else if (dynamic_cast<CtrlMsg_Base *>(msg))
    {

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
    else
    {
        lost = false;
    }

    if ((dynamic_cast<CtrlMsg_Base *>(msg) && id == -1) || dynamic_cast<DataMsg_Base *>(msg))
    {

        if (id == -1) //this wwould handle the first message only
        {
            id = -1;
        }

        if (isStartingNode == true) // the sender side
        {
            //sender main loop
            //increment the id to get the next message to be sent , then get that input line
            id++;
            if (dynamic_cast<DataMsg_Base *>(msg))
            {
                DataMsg_Base *dataMsg = check_and_cast<DataMsg_Base *>(msg);
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

            unsigned int CRCInt = crcObj->crc8(payload);

            // convert the CRCint to string
            stringstream ss;
            ss << CRCInt;
            CRC = ss.str();

            if (singleBitMod == '1')
            {

                // generate a random integer between 1 and payload length-1 (select random character)
                int randomIndex = intuniform(1, size - 1);

                // choose a random number as the bit index for the randomly chosen charachter
                int randomBitIndex = rand() % 8;

                // flip the bit at that index to a random bit
                payload[randomIndex] = payload[randomIndex] ^ (1 << randomBitIndex);
            }

            sendMsg->setM_Payload(payload.c_str());
            sendMsg->setMycheckbits(CRC.c_str());
            sendMsg->setSendingTime(simTime().dbl());

            if (id == total_num_msg - 1)
            {
                output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                    sendMsg->getM_Payload(), sendMsg->getSendingTime(), errorBits, 1);
                output->WriteFinishLine(nodeId, true);
                output->WriteStatsLine(nodeId, simTime().dbl(), id + 1 + duplicates + losses, (id + 1) / simTime().dbl());
            }
            else
            {

                // Precedence
                // 1.MOD
                // 2.DUP
                // 3.DELAY
                // 4.LOSS

                if (lost == false)
                    output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                        sendMsg->getM_Payload(), sendMsg->getSendingTime(), errorBits, 1);
                string errorBitsWOmod = errorBits.substr(1, 3);
                if (errorBitsWOmod == "001")
                {                                                                      // Delay Only
                    sendDelayed(sendMsg, par("delayPeriod").doubleValue(), "dataOut"); // Send the message after the delay specified in the .ini file
                }
                else if (errorBitsWOmod == "010")
                {
                    duplicates++;                                 // Dup only
                    send(sendMsg, "dataOut");                     // Send the message
                    sendDelayed(sendMsg->dup(), 0.01, "dataOut"); // and its duplicate after a small delay
                }
                else if (errorBitsWOmod == "011")
                { // Dup & Delay
                    duplicates++;
                    sendDelayed(sendMsg, par("delayPeriod").doubleValue(), "dataOut");               // Send the message after the delay specified in the .ini file
                    sendDelayed(sendMsg->dup(), par("delayPeriod").doubleValue() + 0.01, "dataOut"); // and its duplicate after a small delay
                }
                else if (errorBitsWOmod == "100" && lost == false)
                { // Loss Only
                    losses++;
                    id--;
                    output->writeTimeOut(nodeId, sendMsg->getSeq_Num(), simTime().dbl() + 0.2); //decrement the id so that it would resend the message
                    output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                        sendMsg->getM_Payload(), simTime().dbl() + 0.2, errorBits, 1);
                    scheduleAt(simTime() + 0.2, sendMsg); // wait for a perriod equals the delay at the reciever side
                                                          // to send the same message again
                }
                else if (errorBitsWOmod == "101" && lost == false)
                { // Loss & Delay
                    losses++;
                    id--;
                    output->writeTimeOut(nodeId, sendMsg->getSeq_Num(), simTime().dbl() + 0.2 + par("delayPeriod").doubleValue());
                    output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                        sendMsg->getM_Payload(), simTime().dbl() + par("delayPeriod").doubleValue() + 0.2, errorBits, 1);
                    scheduleAt(simTime() + 0.2 + par("delayPeriod").doubleValue(), sendMsg);
                }
                else if (errorBitsWOmod == "110" && lost == false)
                { // Loss & Dup
                    duplicates++;
                    losses++;
                    id--;
                    output->writeTimeOut(nodeId, sendMsg->getSeq_Num(), simTime().dbl() + 0.2);
                    output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                        sendMsg->getM_Payload(), simTime().dbl() + 0.2, errorBits, 1);

                    scheduleAt(simTime() + 0.2, sendMsg);
                }
                else if (errorBitsWOmod == "111" && lost == false)
                { // Loss & Dup & Delay
                    duplicates++;
                    losses++;
                    id--;
                    output->writeTimeOut(nodeId, sendMsg->getSeq_Num(), simTime().dbl() + par("delayPeriod").doubleValue() + 0.2);
                    output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                        sendMsg->getM_Payload(), simTime().dbl() + par("delayPeriod").doubleValue() + 0.2, errorBits, 1);
                    scheduleAt(simTime() + par("delayPeriod").doubleValue() + 0.2, sendMsg);
                }
                else
                {
                    send(sendMsg, "dataOut");
                }
            }
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

            char bitModded = '0';
            char packetDup = '0';

            // Here we calculate the CRC8 of the message
            int reCalcCrc8 = 0;

            DataMsg_Base *dataMsg = check_and_cast<DataMsg_Base *>(msg);
            string payload = dataMsg->getM_Payload();
            string CRC = dataMsg->getMycheckbits();

            Crc *crcObj = new Crc();
            reCalcCrc8 = crcObj->crc8(payload);

            // Convert the CRC integer into a string.

            stringstream ss;
            ss << reCalcCrc8;
            string reCalcCrc8Str = ss.str();

            //increase the # of trans and trnastime

            std::string out = "out";
            std::string gate = out + std::to_string(othernodeID);

            if (reCalcCrc8Str == CRC)
            {
                // send the ACK
                // Because there is no modification
                dataMsg->setPiggy(1);
                bitModded = '0';
            }
            else
            {
                // There was atleast one bit error.

                bitModded = '1';
                dataMsg->setPiggy(0);
            }

            if (prevMessageSeqNum == dataMsg->getSeq_Num())
            {
                // The message is duplicated
                // send the NACK
                dataMsg->setPiggy(0);
                packetDup = '1';
                output->writeDropMsg(nodeId, dataMsg->getSeq_Num());
            }
            else
            {
                // The message is not duplicated
                // send the ACK
                //dataMsg->setPiggy(1);
                packetDup = '0';
                sendDelayed(dataMsg, 0.2, "dataOut");

                string errorString = "0000";
                errorString[0] = bitModded;
                errorString[2] = packetDup;

                output->WriteToFile(nodeId,
                                    false,
                                    dataMsg->getSeq_Num(),
                                    dataMsg->getM_Payload(),
                                    simTime().dbl(),
                                    errorString,
                                    dataMsg->getPiggy());
            }

            prevMessageSeqNum = dataMsg->getSeq_Num(); // in the first time, prevMessageSeqNum = -1, so the first message will be sent with ACK always
        }
    }
}
