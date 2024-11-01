#ifndef DBCPARSED_H
#define DBCPARSED_H


#include <unordered_map>
#include <string>
#include <vector>

struct CANSignal {
    std::string name;
    int start_bit = 0;
    int length = 0;
    int byte_order = 0;
    int is_signed = 0;
    int LowMinValue = 0;
    double LowMaxValue = 0;
};

struct CANMessage {
    bool Skipable = false;
    std::string name;
    int dlc = 0;
    std::string source;
    std::vector<CANSignal> signals;
};

extern std::unordered_map<int, CANMessage> message;


#endif // DBC_COMPARE_H
