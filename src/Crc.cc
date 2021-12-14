#include "Crc.h"
#include <string>
#include <iostream>
#include <bitset>
using namespace std;

Crc::Crc()
{
    // TODO Auto-generated constructor stub
}

Crc::~Crc()
{
    // TODO Auto-generated destructor stub
}

unsigned int Crc::crc8(string str){
    // This function calculates the CRC8 of the given String.
    unsigned int crc = 0;
    for(int i = 0; i < str.length(); i++){
        crc ^= str[i];
        for(int j = 0; j < 8; j++){
            if(crc & 1){
                crc = (crc >> 1) ^ 0x8C;
            }
            else{
                crc = crc >> 1;
            }
        }
    }

    return crc;
    
}