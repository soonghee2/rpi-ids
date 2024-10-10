#ifndef SIMILARITY_CHECK_H
#define SIMILARITY_CHECK_H

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <json/json.h>
#include <bitset>

class Similarity_Check {
public:
    bool similarity_check(uint32_t can_id, uint8_t data[8], int DLC);
private:
    unsigned long long get_bits_from_hex_string(const std::string &hex_string, int start_bit, int bit_length);
    unsigned long long cut_bits(unsigned long long number, int start, int length);
    unsigned long long to_little_endian_int(unsigned long long number, int byte_size);
    bool is_binary_string_valid(const std::string &binary_string);
    bool parse_csv_line(const std::string &line, std::vector<std::string> &data);

    std::ifstream dbc_file;
    std::ifstream dataset;

    bool is_valid_id = false;
    bool is_valid_range_data = false;
};

#endif // DBC_COMPARE_H
