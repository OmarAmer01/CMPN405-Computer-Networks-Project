

#ifndef INPUTFNC_H_
#define INPUTFNC_H_
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
class inputfnc {
public:
    inputfnc();

    std::vector<std::string> input_function();
    virtual ~inputfnc();
};

#endif /* INPUTFNC_H_ */
