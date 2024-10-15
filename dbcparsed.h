#ifndef DBCPARSED_H
#define DBCPARSED_H

#include <string>
#include <vector>
#include <cstdint>

struct Signal {
    std::string SignalName;
    int StartBit;
    int Length;
    int ByteOrder;
    bool isSigned;
    double LowMinValue;
    double LowMaxValue;
};

// CAN Message structure to represent the entire CAN message
struct CANMessage {
    uint32_t CANID;
    bool Skipable;
    std::string MessageName;
    int DLC;
    std::string Transmitter;
    std::vector<Signal> Signals;
}; 

#endif
