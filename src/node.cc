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

/// TODO: Timer on all messages even if error free, lw m7sl4 loss disbale el timer ( might be changed in SR)
/// TODO: bdl ma heya recieved, htb2a sent , fel wording bta3 el output ... (would be changed in SR)

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

    if (dynamic_cast<CtrlMsg_Base *>(msg))
    {
        ///// read the file at each node
        //cast the msg
        CtrlMsg_Base *ctrlMsg = check_and_cast<CtrlMsg_Base *>(msg);
        //get file name
        nodeId = ctrlMsg->getNodeId();
        if (nodeId % 2 == 0)
            othernodeID = nodeId + 1;
        else
            othernodeID = nodeId - 1;
        ctrlMsg->setName("selfCtrlMsg");
        Input *input = new Input();
        this->nodeFileVector = input->parseNodeFile(ctrlMsg->getFName());

        if (!ctrlMsg->getStart())
        {
            // not starting node
            isStartingNode = false;
            return;
        }
        else
        {
            //starting node
            // send a self msg and return
            isStartingNode = true;
            cMessage *startingNode = new cMessage("startingNode");
            double sartingTime = ctrlMsg->getStartTime();
            scheduleAt(sartingTime + simTime(), startingNode);
            output = new Input();
            output->openLogFile(nodeId);
            return;
        }
    }
    else if (msg->isSelfMessage())
    {

        if (strcmp(msg->getName(), "startingNode") == 0)
        {
            firstmsg = -1;
        }
    }

    { // receive
        if (!(firstmsg == -1))
        {
            if (isStartingNode == true)
                EV << "Starting node ...." << endl;
            firstmsg = 0;
            char bitModded = '0';
            char packetDup = '0';

            // Here we calculate the CRC8 of the message
            int reCalcCrc8 = 0;

            DataMsg_Base *dataMsg = check_and_cast<DataMsg_Base *>(msg);
            string payload = dataMsg->getM_Payload();
            if (dataMsg->getSeq_Num() == -1)
            {
                EV << "The other node has finished" << endl;
                otherNodeDoneSending = 1;
            }
            // Convert the CRC integer into a string.
            //increase the # of trans and trnastime
            std::string out = "out";
            std::string gate = out + std::to_string(othernodeID);

            if (par("CRC").intValue() == 1)
            {
                string CRC = dataMsg->getMycheckbits();
                checkCRC(CRC, payload);
            }
            else
            {
                int t = checkHamming(payload); // Location of the wrong bit
                if (t != 0 && t < int(payload.size()))
                {
                    payload = correctHamming(payload, t - 1);
                    payload = removeHamming(payload);
                    output->printFixedFrame(nodeId, t, payload);
                }
                // check if the error has been removed
                t = checkHamming(payload);
                if (t != 0)
                    EV << "Corrupted frame\n";
            }
            if (prevMessageSeqNum == dataMsg->getSeq_Num())
            {
                // The message is duplicated
                output->writeDropMsg(nodeId, dataMsg->getSeq_Num());
            }
        }
    }

    if (allSent == 0) // send
    {
        id++;

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

        if (id == total_num_msg - 1)
            sendMsg->setSeq_Num(-1); ///TODO:add variable to msg to check sending termination instead of ID
        else
            sendMsg->setSeq_Num(id);

        // we need to frame the payload before sending it
        payload = bitStuffing_Framing(payload);
        //one bit modification
        if (singleBitMod == '1')
            payload = oneBitMod(payload);
        // adding CRC or Hamming
        if (par("CRC").intValue() == 1)
        {
            Crc *crcObj = new Crc();
            string CRC = "CRCBYTE";
            unsigned int CRCInt = crcObj->crc8(payload);
            stringstream ss;
            ss << CRCInt;
            CRC = ss.str();
            sendMsg->setMycheckbits(CRC.c_str());
        }
        else
            payload = addHamming(payload);

        sendMsg->setM_Payload(payload.c_str());
        sendMsg->setSendingTime(simTime().dbl());

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
            ///TODO: For all LOSS cases, we should send  new self cMessage
            losses++;
            id--;
            output->writeTimeOut(nodeId, sendMsg->getSeq_Num(), simTime().dbl() + 0.2); //decrement the id so that it would resend the message
            output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                sendMsg->getM_Payload(), simTime().dbl() + par("timeOutPeriod").doubleValue(), errorBits, 1);
            scheduleAt(simTime() + par("timeOutPeriod").doubleValue(), sendMsg); // wait for a perriod equals the delay at the reciever side
                                                                                 // to send the same message again
        }
        else if (errorBitsWOmod == "101" && lost == false)
        { // Loss & Delay
            losses++;
            id--;
            output->writeTimeOut(nodeId, sendMsg->getSeq_Num(), simTime().dbl() + par("timeOutPeriod").doubleValue() + par("delayPeriod").doubleValue());
            output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                sendMsg->getM_Payload(), simTime().dbl() + par("delayPeriod").doubleValue() + par("timeOutPeriod").doubleValue(), errorBits, 1);
            scheduleAt(simTime() + par("timeOutPeriod").doubleValue() + par("delayPeriod").doubleValue(), sendMsg);
        }
        else if (errorBitsWOmod == "110" && lost == false)
        { // Loss & Dup
            duplicates++;
            losses++;
            id--;
            output->writeTimeOut(nodeId, sendMsg->getSeq_Num(), simTime().dbl() + par("timeOutPeriod").doubleValue());
            output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                sendMsg->getM_Payload(), simTime().dbl() + par("timeOutPeriod").doubleValue(), errorBits, 1);

            scheduleAt(simTime() + par("timeOutPeriod").doubleValue(), sendMsg);
        }

        else if (errorBitsWOmod == "111" && lost == false)
        { // Loss & Dup & Delay
            duplicates++;
            losses++;
            id--;
            output->writeTimeOut(nodeId, sendMsg->getSeq_Num(), simTime().dbl() + par("delayPeriod").doubleValue() + par("timeOutPeriod").doubleValue());
            output->WriteToFile(nodeId, true, sendMsg->getSeq_Num(),
                                sendMsg->getM_Payload(), simTime().dbl() + par("delayPeriod").doubleValue() + par("timeOutPeriod").doubleValue(), errorBits, 1);
            scheduleAt(simTime() + par("delayPeriod").doubleValue() + par("timeOutPeriod").doubleValue(), sendMsg);
        }
        else
        {
            ///TODO: shouldbe send delayed
            send(sendMsg, "dataOut");
        }

        if (id == total_num_msg - 1)
        {
            output->WriteFinishLine(nodeId, true);
            allSent = 1;
        }
    }
    else
    {
        //send Ack or nAck only
        // if you received the last msg from the other node
        if (otherNodeDoneSending == 1)
        //TODO: total number of transmissions should be edited to include transmission of both nodes
        {
            output->WriteStatsLine(nodeId, simTime().dbl(), id + 1 + duplicates + losses, (id + 1) / simTime().dbl());
            return;
        }
        DataMsg_Base *m = new DataMsg_Base("waiting for you to finish");
        send(m, "dataOut");
    }

    //*********************************************************************************//
    /* if (msg->isSelfMessage() && dynamic_cast<DataMsg_Base *>(msg))
    {
        lost = true;
    }
    else if (msg->isSelfMessage())
    {
        nodeFileRead = true;
    }

    else if (dynamic_cast<CtrlMsg_Base *>(msg))
    {
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
        }
        else // not starting node
        {

            ///TODO:: the following parts need to be changed as we would send msg from here
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
    }*/
}

void Node::checkCRC(string CRC, string payload)
{ // this only to check the CRC
    // we can't rerequest the corrupted frame as it would be modified again
    Crc *crcObj = new Crc();
    unsigned int reCalcCrc8 = crcObj->crc8(payload);
    stringstream ss;
    ss << reCalcCrc8;
    string reCalcCrc8Str = ss.str();
    if (reCalcCrc8Str == CRC)
    {
        EV << "CRC:the recieved frame has no modification error" << endl;
    }
    else
    {
        EV << "CRC:One bit has been modified" << endl;
    }
}
string Node::calcParity(string msg, int position)
{
    int count = 0;
    int i = position;
    int temp = i;
    while (i < int(msg.size()))
    {
        for (i; (i < temp + position + 1) && (i < msg.size()); i++)
        {
            if (msg[i] == '1')
            {
                count++;
            }
        }
        i = i + (position + 1);
        temp = i;
    }

    if (count % 2 == 0)
        msg[position] = '0';
    else
        msg[position] = '1';
    return msg;
}
string Node::addHamming(string payload)
{
    int r = 0;
    while (payload.size() + r + 1 > pow(2, r))
        r++;

    string newpayload = "";
    int k = 0, l = 0;
    for (unsigned int i = 0; i < payload.size() + r; i++)
    {
        int temp = pow(2, l) - 1;
        if (i == temp)
        {
            newpayload.push_back('0');
            l++;
        }
        else
        {
            newpayload.push_back(payload[k]);
            k++;
        }
    }
    int i = 0;
    l = 0;
    while (i < int(newpayload.size()))
    {
        newpayload = calcParity(newpayload, i);
        l++;
        i = pow(2, l) - 1;
    }
    return newpayload;
}
bool Node::checkParity(string msg, int position)
{
    int count = 0;
    int i = position;
    int temp = i;
    while (i < msg.size())
    {
        for (i; (i < temp + position + 1) && (i < msg.size()); i++)
        {
            if (msg[i] == '1')
            {
                count++;
            }
        }
        i = i + (position + 1);
        temp = i;
    }

    if (count % 2 == 0)
        return false;
    else
        return true;
}
int Node::checkHamming(string msg)
{
    int i = 0;
    int l = 0;
    vector<int> wrong;
    while (i < int(msg.size()))
    {
        int position = i + 1;
        if (checkParity(msg, i))
        {
            wrong.push_back(position);
        }

        l++;
        i = pow(2, l) - 1;
    }
    int sum = 0;
    for (int i = 0; i < int(wrong.size()); i++)
        sum += wrong[i];
    return sum;
}
string Node::correctHamming(string msg, int position)
{
    if (msg[position] == '0')
        msg[position] = '1';
    else
        msg[position] = '0';
    return msg;
}
string Node::removeHamming(string msg)
{
    string newmsg;
    int i = 0, l = 0;
    while (i < int(msg.size()))
    {
        if (i == pow(2, l) - 1)
        {
            i++;
            l++;
            continue;
        }
        newmsg.push_back(msg[i]);
        i++;
    }
    return newmsg;
}
string Node::oneBitMod(string payload)
{
    // generate a random integer between 1 and payload length-1 (select random character)
    int randomIndex = intuniform(1, payload.size() - 1);

    // choose a random number as the bit index for the randomly chosen charachter
    int randomBitIndex = rand() % 8;

    // flip the bit at that index to a random bit
    payload[randomIndex] = payload[randomIndex] ^ (1 << randomBitIndex);
    return payload;
}
string Node::bitStuffing_Framing(string payload)
{
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
    return payload;
}
