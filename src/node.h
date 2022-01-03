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

#ifndef __MAINPROJECT_NODE_H_
#define __MAINPROJECT_NODE_H_

#include <omnetpp.h>
#include <vector>
#include "Input.h"
#include "ctrlMsg_m.h"
#include "dataMsg_m.h"
#include "Crc.h"
using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  void readNodeFile(CtrlMsg_Base *);
  Input *output;
  string calcParity(string msg, int position);
  string addHamming(string payload);
  string oneBitMod(string payload);
  string bitStuffing_Framing(string payload);
  void checkCRC(string CRC, string payload);
  int checkHamming(string msg);
  bool checkParity(string msg, int position);
  string correctHamming(string msg, int position);
  string removeHamming(string msg);
public:
  int duplicates = 0;
  int losses=0;
  bool nodeFileRead = false;
  bool lost=false;
  //int nodeStartingTime=-1;
  bool isStartingNode = false;
  double totalTrans_Time = 0.0;
  int totalNumberOfTrans = 0;
  int id = -1; //message ID
  int nodeId = 0;
  int othernodeID = 0;
  int prevMessageSeqNum = -1;
  int firstmsg =0;
  int totalMsgNum=0;
  int allSent=0;
  int otherNodeDoneSending=0;
  vector<nodeFileLine> nodeFileVector;
};

#endif
