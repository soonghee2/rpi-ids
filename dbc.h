#ifndef DBC_H
#define DBC_H

#include <unordered_set>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <bitset>
#include "CANStats.h"
#include "dbcparsed.h"

extern std::unordered_map<int, CANMessage> message;
void read_dbc(const std::string& filename);
uint64_t extractBits(uint64_t data, int start, int length);
uint64_t toLittleEndian(uint64_t data, int byteSize);
bool is_binary_string_valid(const std::string &binary_string);
bool parse_csv_line(const std::string &line, std::vector<std::string> &data);
bool validation_check(uint32_t can_id, uint8_t data[8], int DLC);
bool check_similarity_with_previous_packet(uint32_t can_id, uint8_t data[8], int DLC, uint8_t valid_payload[8], bool& is_initial_data, int percent);

//int total_same_percent;
//int total_lentgh;
#endif // DBC_H
