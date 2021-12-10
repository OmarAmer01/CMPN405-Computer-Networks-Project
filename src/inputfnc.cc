

#include "inputfnc.h"
#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include<vector>
#include <algorithm>
#include <ostream>
inputfnc::inputfnc() {
    // TODO Auto-generated constructor stub

}

std::vector<std::string> inputfnc::input_function()
{
        std::ifstream infile;

        std::vector<std::string> Msg_array;
        infile.open("C:\\Users\\HP\\Downloads\\Omnet++\\omnetpp-5.7\\samples\\Lab3_student\\src\\input.txt");
        if(infile.fail())
        {
            std::cout<<"Your file didn't work"<<std::endl;
        }
        else
        {
        std::string s;

        while(std::getline(infile,s))
        {
            s.erase(std::remove(s.begin(), s.end(), ' '), s.end());


             Msg_array.push_back(s);
        }

        infile.close();

        }
        return Msg_array;
}
inputfnc::~inputfnc() {
    // TODO Auto-generated destructor stub
}

