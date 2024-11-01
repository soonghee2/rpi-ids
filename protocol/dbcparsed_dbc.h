#ifndef DBCPARSED_H
#define DBCPARSED_H


#include <unordered_map>
#include <string>
#include <vector>

struct CANSignal {
    std::string name;
    int start_bit;
    int length;
    int byte_order;
    int is_signed;
    int LowMinValue;
    double LowMaxValue;
};

struct CANMessage {
    bool Skipable;
    std::string name;
    int dlc;
    std::string source;
    std::vector<CANSignal> signals;
};

extern std::unordered_map<int, CANMessage> message;


#endif // DBC_COMPARE_H
