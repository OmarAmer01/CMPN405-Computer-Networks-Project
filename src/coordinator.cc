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
#include "Crc.h"
#include <bitset>

Define_Module(Coordinator);

void Coordinator::initialize()
{
    Crc *crc = new Crc();
    // TODO - Generated method body
     string testStr = "The Quick Brown Fox Jumps Over the Lazy Dog";
    // cout << "strToNum: " << crc->str2Num(testStr, 2) << endl;
     cout << bitset<8>(crc->crc8(testStr));
}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
