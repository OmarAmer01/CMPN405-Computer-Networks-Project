// This is the class responsible for all CRC calculations.
// We will be using CRC-8 for simplicity.


#ifndef CRC_H_
#define CRC_H_

#include <string>
using namespace std;

class Crc {
public:
    Crc();
    virtual ~Crc();

    string str2Num(string str, int base);
    
    unsigned int crc8(string str);
   

};

#endif /* CRC_H_ */
