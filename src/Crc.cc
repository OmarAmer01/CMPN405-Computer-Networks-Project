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
    // This function calculates the CRC8 of the given String.

    unsigned int poly = 0b1001; // x^8 + x^2 + x^1 + 1
    int polyLen = 4;

    //string binStr = str2Num(str);
    string binStr = "10100001";
    int strLen = binStr.length();
    cout << "String is " << binStr << endl
         << "strLen is " << strLen << endl;
    int crc = 0;

    /*
   Steps: MODULO 2 DIVISON binStr / poly
        1. XOR polyLen bits of the divedend "binStr" with poly. <=== Remainder
        2. Append one bit from the divedend to the remainder.
        3. Repeat XORing the remainder with the poly until we have no more bits left to append.
        4. Return Remainder.

   */

    int divedendSlice = -1;
    int remainder = -1;

    // [STEP 1]
    // Take "Slice" first %polyLen% bits of binStr
    divedendSlice = stoi(binStr.substr(0, polyLen), 0, 2);
    cout << "divedend slice is " << bitset<4>(divedendSlice) << endl;

    // XOR the slice with our generator polynomial
    remainder = divedendSlice ^ poly;
    cout << "Remainder no.1 is " << bitset<4>(remainder) << endl;

    int shiftCounter = 0;
    for (int idx = 0; idx < strLen - polyLen + 1; idx++)
    {

        cout << "IDX: " << idx << endl;
        // [STEP 2]
        // Append one bit from the divedend
        shiftCounter = polyLen + idx + 1;
        while (remainder < poly)
        {

            bitset<1> bit(binStr[shiftCounter]);
           

            remainder = (remainder << 1) | bit.to_ulong();

            cout << "Appending BIT [" << bit << "], bit number " << shiftCounter << " to Remainder, ===> " << bitset<4>(remainder) << endl;
            shiftCounter++;
            if (shiftCounter == binStr.length())
            {
                cout << "EXITING FROM LOOP" << endl;
                remainder ^= poly;
                return remainder;
            }
        }

        remainder ^= poly;
        cout << "Remainder no." << idx + 2 << " is " << bitset<4>(remainder) << endl;

        // while (remainder < poly)
        // {

        if (polyLen + idx == binStr.length())
        {
            cout << "exiting from outside" << endl;
            return remainder;
        }

        //     cout << "Appending a bit [" << binStr[polyLen + curr + shiftCounter + 1] << "] from the divedend to the remainder:" << endl;
        //     remainder = (remainder << 1) | bitset<1>(binStr[polyLen + curr + shiftCounter +1]).to_ulong();
        //     shiftCounter++;
        //     /// TODO: Recheck last line lmao
        //     cout << "Remainder After Appending: " << bitset<4>(remainder) << endl;
        // }
        // remainder ^= poly;
        // cout << "Remainder no." << curr + 2 << " is " << bitset<4>(remainder) << endl;

        // [STEP 3]
        // Repeat
    }

    // cout << "SUBSTRING = " << stoi(binStr.substr(0, polyLen)) << endl;

    // cout <<"AFTER CONV:" <<divedend;
    return remainder;
}

Crc::~Crc()
{
    // TODO Auto-generated destructor stub
}
