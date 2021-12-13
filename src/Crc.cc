#include "Crc.h"
#include <string>
#include <iostream>
#include <bitset>
using namespace std;

Crc::Crc()
{
    // TODO Auto-generated constructor stub
}

string Crc::str2Num(string str, int base = 2)
{
    // This function converts a given string to an unsigned int to be further used in the CRC calculation.
    /// TODO: use the bitset implementation
    int strNum = 0;
    int currCharAscii = 0;

    for (int idx = 0; idx < str.length(); idx++)
    {
        currCharAscii = str[idx];               // Automatically casts the character into an integer.
        strNum = (strNum << 8) | currCharAscii; // Shifts the number to the left by 8 bits and then adds the current character to the end of the number.
    }

    if (base == 2)
    {
        // now, we convert from unsigned int to binary string
        string binStr = "";
        while (strNum > 0)
        {
            binStr = (char)(strNum % 2 + '0') + binStr;
            strNum = strNum / 2;
        }
        return binStr;
    }
    else
    {
        // if base != 2, return decimal instead.
        return to_string(strNum);
    }
}

unsigned int Crc::crc8(string str)
{
    return 0b11111111;
    // This function calculates the CRC8 of the given String.

    /*
   Steps: MODULO 2 DIVISON binStr / poly
        1. XOR polyLen bits of the divedend "binStr" with poly. <=== Remainder
        2. Append one bit from the divedend to the remainder.
        3. Repeat XORing the remainder with the poly until we have no more bits left to append.
        4. Return Remainder.

   */

    // we use a test polynomial : X^3 + 1
    const int polyLen = 9;
    bitset<polyLen> poly = 0b100001110;
    //bitset<polyLen> poly = 0b1001;

    // take a slice of the divedend that is polyLen bits long
    string divedend = str2Num(str);
    //string divedend = "10011101";
    //string divedend = "10100001";
    string divedendSlice = divedend.substr(0, polyLen);

    // Convert from binary string to bitset
    bitset<polyLen> divedendSliceBits(divedendSlice);

    // XOR the slice with the polynomial and put the result in remainder
    bitset<polyLen> remainder = divedendSliceBits ^ poly;
    cout << "First Remainder = " << remainder.to_string() << endl;

    // Append a bit from the divedend to the remainder then keep XORing
    int remCtr = 0;
    for (int i = 0; i < divedend.length() - polyLen; i++)
    {

        while (remainder.to_ulong() < poly.to_ulong())
        {

            cout << "Appending Bit: " << (bitset<polyLen>(divedend[i + polyLen])).to_string()[polyLen - 1] << endl;

            remainder = (remainder << 1) | bitset<polyLen>(divedend[i + polyLen]); // appends a bit from the divedend

            cout << "Remainder after appending = " << bitset<polyLen>(remainder).to_string() << endl;
            i++;
        }
        i--;
        remainder = remainder ^ poly;
        cout << "Remainder No. " << remCtr + 2 << " = " << remainder.to_string() << endl;
        remCtr++;
    }

    cout << "Final remainder = " << remainder.to_string().substr(1) << endl;

    return remainder.to_ulong();
}

Crc::~Crc()
{
    // TODO Auto-generated destructor stub
}

unsigned int Crc::crc8Alt(string str){
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