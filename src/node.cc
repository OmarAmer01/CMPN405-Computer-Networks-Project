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
#include<math.h>
using namespace std;
#include <bitset>
Define_Module(Node);

void Node::initialize()
{
}

void Node::handleMessage(cMessage *msg)
{
    int ack=1;

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
        total_num_msg = nodeFileVector.size();
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

        if (strcmp(msg->getName(), "startingNode") == 0 || strcmp(msg->getName(), "lostFrame") == 0)
        {
            firstmsg = -1;
        }
        if (dynamic_cast<DataMsg_Base *>(msg))
        {
            EV << "Node"<< nodeId<<"has recieved a self msg" << endl;
            DataMsg_Base *sendMsg = check_and_cast<DataMsg_Base *>(msg);
            output->WriteToFile(nodeId, true,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())),
                                           sendMsg->getM_Payload(), simTime().dbl(), "0000", 1, 0);
            //send(sendMsg,"dataOut");
            lost++;
            return;
        }
    }

    { // receive
        if (!(firstmsg == -1))
        {

            firstmsg = 0;
            char bitModded = '0';
            char packetDup = '0';

            // Here we calculate the CRC8 of the message
            int reCalcCrc8 = 0;

            DataMsg_Base *dataMsg = check_and_cast<DataMsg_Base *>(msg);

            string payload = dataMsg->getM_Payload();
            ///TODO: generate expectedFrame according to ack or Nack ***
            //currentSeq-prevMessageSeqNum = 1 ....ack
            // else nack
            if (dataMsg->getSeq_Num() == expectedFrame )
            {
                expectedFrame=(expectedFrame+1); // use it in sending as piggypack
                ack=1;
            }
            else
            {
                expectedFrame=(dataMsg->getSeq_Num() +1);
                lostFrame=dataMsg->getSeq_Num();
                ack=0;
            }
            if (dataMsg->getRecievingTime() == -1)
            {
                EV << "The other node has finished" << endl;
                otherNodeDoneSending = 1;
                otherNodeStat=dataMsg->getKind();

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
                //convert the frame to bitset
                payload = strToBitstring(payload);

                int t = checkHamming(payload); // Location of the wrong bit
                t=3;
                if (t != 0 && t < int(payload.size()))
                {
                    //correct hamming code
                    payload = correctHamming(payload, t - 1);
                    //remove hamming code
                    payload = removeHamming(payload);
                    //convert the string of bits back to ASCII string
                    payload = bitStringToStr(payload);
                    output->printFixedFrame(nodeId, t, payload);
                }

            }
            if (prevMessageSeqNum == dataMsg->getSeq_Num() && dataMsg->getSeq_Num() !=-1)
            {
                // The message is duplicated
                output->writeDropMsg(nodeId, dataMsg->getSeq_Num());
                prevMessageSeqNum = dataMsg->getSeq_Num();
                return;
            }
            prevMessageSeqNum = dataMsg->getSeq_Num();
        }
    }

    firstmsg=1;

    if (allSent == 0) // send
    {
        //send the expected frame
        id++;
        nodeFileLine inputLine;
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
        {
            sendMsg->setRecievingTime(-1);
            sendMsg->setKind( total_num_msg + duplicates + losses);
        }

        sendMsg->setSeq_Num(id);
        sendMsg->setPiggyID(expectedFrame%(2*par("windowSize").intValue()));
        if (ack==1)
            sendMsg->setPiggy(1);
        else
        {
            sendMsg->setPiggy(0);
            sendMsg->setPiggyID((lostFrame-1)%(2*par("windowSize").intValue()));
        }

        // we need to frame the payload before sending it
        payload = bitStuffing_Framing(payload);
        //one bit modification

        // adding CRC or Hamming
        if (par("CRC").intValue() == 1)
        {
            payload = strToBitstring(payload);
            if (singleBitMod == '1')
                payload = oneBitMod(payload);
            Crc *crcObj = new Crc();
            string CRC = "CRCBYTE";
            payload = bitStringToStr(payload);
            unsigned int CRCInt = crcObj->crc8(payload);
            stringstream ss;
            ss << CRCInt;
            CRC = ss.str();
            sendMsg->setMycheckbits(CRC.c_str());
        }
        else
        {

            // convert payload to bit set
            payload = strToBitstring(payload);
            // add the hamming code to the payload
            payload = hammingCode(payload);
            if (singleBitMod == '1') // edit one bit at the string
            {
                payload = oneBitMod(payload);
            }
            //convert the string of bits back to ASCII string
            payload = bitStringToStr(payload);
            EV<<"ASCII payload after modification:"<<payload<<endl;
        }

        sendMsg->setM_Payload(payload.c_str());
        sendMsg->setSendingTime(simTime().dbl());

        // Precedence
        // 1.MOD
        // 2.DUP
        // 3.DELAY
        // 4.LOSS
        ///TODO: don't write to file in case of loss (send msg statment i mean)
        ///TODO: in case of lost frame, edit the error nibbble to retransmit it correctly

        string errorBitsWOmod = errorBits.substr(1, 3);
        if (errorBitsWOmod == "001")
        {
            output->WriteToFile(nodeId, true,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())),
                                            sendMsg->getM_Payload(), simTime().dbl() + par("delayPeriod").doubleValue(), errorBits, ack, sendMsg->getPiggyID());// Delay Only
            sendDelayed(sendMsg, par("delayPeriod").doubleValue(), "dataOut"); // Send the message after the delay specified in the .ini file
        }
        else if (errorBitsWOmod == "010")
        {
            output->WriteToFile(nodeId, true,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())),
                                            sendMsg->getM_Payload(), simTime().dbl() +0.01 , errorBits, ack, sendMsg->getPiggyID());
            duplicates++;
            sendDelayed(sendMsg,par("channelDelay").doubleValue() , "dataOut");                     // Send the message
            sendDelayed(sendMsg->dup(),par("channelDelay").doubleValue()+ 0.01, "dataOut"); // and its duplicate after a small delay
        }
        else if (errorBitsWOmod == "011")
        { // Dup & Delay
            duplicates++;
            output->WriteToFile(nodeId, true,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())),
                                            sendMsg->getM_Payload(), simTime().dbl() + par("delayPeriod").doubleValue() +0.01, errorBits, ack, sendMsg->getPiggyID());
            sendDelayed(sendMsg, par("delayPeriod").doubleValue()+ par("channelDelay").doubleValue() , "dataOut");               // Send the message after the delay specified in the .ini file
            sendDelayed(sendMsg->dup(),par("channelDelay").doubleValue()+ par("delayPeriod").doubleValue() + 0.01, "dataOut"); // and its duplicate after a small delay
        }
        else if (errorBitsWOmod == "100" )
        { // Loss Only
            ///TODO: For all LOSS cases, we should send  new self cMessage
            losses++;
            lost--;
            output->writeTimeOut(nodeId, (sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())), simTime().dbl() + 0.2); //decrement the id so that it would resend the message
        //    output->WriteToFile(nodeId, true, (sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())),
        //                        sendMsg->getM_Payload(), simTime().dbl() + par("timeOutPeriod").doubleValue(), errorBits, ack, sendMsg->getPiggyID());
            cMessage *lostFrame = new cMessage("lostFrame");
            scheduleAt( par("channelDelay").doubleValue()+  simTime(), lostFrame);
            scheduleAt(simTime() + par("timeOutPeriod").doubleValue(), sendMsg); // wait for a perriod equals the delay at the reciever side
                                                                                 // to send the same message again
        }
        else if (errorBitsWOmod == "101")
        { // Loss & Delay
            losses++;
            lost--;
            output->writeTimeOut(nodeId,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())), simTime().dbl() + par("timeOutPeriod").doubleValue() + par("delayPeriod").doubleValue());
          //  output->WriteToFile(nodeId, true,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())),
          //                      sendMsg->getM_Payload(), simTime().dbl() + par("delayPeriod").doubleValue() + par("timeOutPeriod").doubleValue(), errorBits, ack, sendMsg->getPiggyID());
            cMessage *lostFrame = new cMessage("lostFrame");
            scheduleAt( par("channelDelay").doubleValue()+  par("delayPeriod").doubleValue()+simTime(), lostFrame);
            scheduleAt(simTime() + par("timeOutPeriod").doubleValue() + par("delayPeriod").doubleValue(), sendMsg);
        }
        else if (errorBitsWOmod == "110" )
        { // Loss & Dup
            duplicates++;
            losses++;
            lost--;
            output->writeTimeOut(nodeId,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())), simTime().dbl() + par("timeOutPeriod").doubleValue());
          //  output->WriteToFile(nodeId, true,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())),
           //                     sendMsg->getM_Payload(), simTime().dbl() + par("timeOutPeriod").doubleValue()+0.01,errorBits, ack, sendMsg->getPiggyID());
            cMessage *lostFrame = new cMessage("lostFrame");
            scheduleAt( par("channelDelay").doubleValue()+0.01+  simTime(), lostFrame);
            scheduleAt(simTime() + par("timeOutPeriod").doubleValue(), sendMsg);
        }

        else if (errorBitsWOmod == "111" )
        { // Loss & Dup & Delay
            duplicates++;
            losses++;
            lost--;
            output->writeTimeOut(nodeId,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue()))
                            , simTime().dbl() + par("delayPeriod").doubleValue() + par("timeOutPeriod").doubleValue());
         //   output->WriteToFile(nodeId, true,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())),
           //                     sendMsg->getM_Payload(), simTime().dbl() + par("delayPeriod").doubleValue() + par("timeOutPeriod").doubleValue(),errorBits , ack, sendMsg->getPiggyID());
            cMessage *lostFrame = new cMessage("lostFrame");
            scheduleAt(  par("channelDelay").doubleValue()+ simTime(), lostFrame);
            scheduleAt(simTime() + par("delayPeriod").doubleValue() + par("timeOutPeriod").doubleValue(), sendMsg);
        }
        else
        {

            output->WriteToFile(nodeId, true,(sendMsg->getSeq_Num())%( 2* (par("windowSize").intValue())),
                                sendMsg->getM_Payload(), simTime().dbl(), errorBits, ack, sendMsg->getPiggyID() );
            sendDelayed(sendMsg, par("channelDelay").doubleValue(),"dataOut");
        }

        if (id == total_num_msg - 1 )
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
            output->WriteStatsLine(nodeId, simTime().dbl(),otherNodeStat +total_num_msg + duplicates + losses, total_num_msg / simTime().dbl());
            return;
        }
        DataMsg_Base *m = new DataMsg_Base("waiting for you to finish");
        m->setSeq_Num(-1);
        sendDelayed(m,par("channelDelay").doubleValue() , "dataOut");
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

string Node::hammingCode(string msg)
{
    int r = 0;
    while (msg.size() + r + 1 > pow(2, r))
        r++;

    string newmsg = "";
    int k = 0, l = 0;
    for (unsigned int i = 0; i < msg.size() + r; i++)
    {
        int temp = pow(2, l) - 1;
        if (i == temp)
        {
            newmsg.push_back('0');
            l++;
        }
        else
        {
            newmsg.push_back(msg[k]);
            k++;
        }
    }
    int i = 0;
    l = 0;
    while (i < int(newmsg.size()))
    {
        newmsg = calcParity(newmsg, i);
        l++;
        i = pow(2, l) - 1;
    }
    return newmsg;
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
           if (checkParity(msg, i) == true)
           {
               wrong.push_back(position);
           }

           l++;
           i = pow(2, l) - 1;
       }
       int sum = 0;
       for (int i = 0; i < int(wrong.size()); i++)
       {
           sum += wrong[i];
       }
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
    char x = payload[3];
    if (x == '0')
    payload[2] = '1';
    else
        payload[3] = '0';
    return payload;
}

char Node::strToChar(string subStr) {
    char parsed = 0;
    for (int i = 0; i < 8; i++) {
        if (subStr[i] == '1') {
            parsed |= 1 << (7 - i);
        }
    }
    return parsed;
}

string Node::strToBitstring(string payload)
{
    string payloadBits;
    for (int i = 0; i < payload.size(); i++)
    {
        std::bitset<8> x(payload[i]);
        payloadBits.append(x.to_string());
    }
    return payloadBits;
}
string Node::bitStringToStr(string str)
{
    char c;
    string originalString;
    int m = str.size() % 8;
    m = 8 - m;
    for (int i = 0; i<m ; i++)
    {
        str.push_back('0');
    }
    for (int i = 0; i < str.size() / 8; i++)
    {
        string substring = str.substr(i * 8, 8);
        c = strToChar(substring);
        originalString.push_back(c);
    }
    return originalString;
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
