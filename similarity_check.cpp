#include "similarity_check.h"

unsigned long long Similarity_Check::get_bits_from_hex_string(const std::string &hex_string, int start_bit, int bit_length) {
    // Convert hex string to unsigned long long
    unsigned long long value = 0;
    for (char c : hex_string) {
        value <<= 4; // Shift left by 4 bits to make space for the next hex digit
        value |= (c >= '0' && c <= '9') ? c - '0' : 10 + (tolower(c) - 'a');
    }

    // Extract the required bits using bitwise operations
    unsigned long long mask = (1ULL << bit_length) - 1;
    return (value >> (hex_string.length() * 4 - start_bit - bit_length)) & mask;
}

unsigned long long Similarity_Check::cut_bits(unsigned long long number, int start, int length) {
    unsigned long long mask = (1ULL << length) - 1;
    return (number >> (64 - start - length)) & mask;
}

unsigned long long Similarity_Check::to_little_endian_int(unsigned long long number, int byte_size) {
    unsigned long long result = 0;
    for (int i = 0; i < byte_size; ++i) {
        result |= ((number >> (i * 8)) & 0xFF) << (i * 8);
    }
    return result;
}

bool Similarity_Check::similarity_check(uint32_t can_id, uint8_t data[8], int DLC, /*can id에 해당하는 이전 정상 패이로드*/ ) {

    dbc_file.open("output.json", std::ifstream::binary);
    Json::Value dbc;
    dbc_file >> dbc;
    
    std::stringstream can_id_string;
    can_id_string << std::hex << can_id;
    std::string can_id_hexStr = can_id_string.str();
    
    std::ostringstream payload_string;
    for (int i = 0; i < DLC; ++i) {
        payload_string << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    // 문자열로 저장
    std::string payload_hexStr = payload_string.str();
    //std::cout << payload_hexStr << "\n";

    for (size_t index = 0; index < pair_list.size(); ++index) {
        if ("0x" + can_id_hexStr == message["CAN ID"].asString()) {
            pair_found = true;
            for (const auto &signal : message["Signals"]) {
                total_length += signal['Length']
                unsigned long long old_value, new_value;
                if (signal["Byte Order"].asInt() == 1) {
                    int first = signal["Start Bit"].asInt() / 8;
                    int end = ((signal["Start Bit"].asInt() + signal["Length"].asInt()) / 8);
                    int bit_length = (end - first) * 8;
                    old_value = get_bits_from_hex_string(pair_list[index].second, first * 8, bit_length);
                    new_value = get_bits_from_hex_string(connected_values, first * 8, bit_length);
                    old_value = bitset<64>(to_little_endian_int(old_value, (end - first))).to_ullong();
                    new_value = bitset<64>(to_little_endian_int(new_value, (end - first))).to_ullong();
                    old_value = cut_bits(old_value, ((end - first) * 8) - signal["Length"].asInt() - (signal["Start Bit"].asInt() % 8), signal["Length"].asInt());
                    new_value = cut_bits(new_value, ((end - first) * 8) - signal["Length"].asInt() - (signal["Start Bit"].asInt() % 8), signal["Length"].asInt());
                } else {
                    old_value = get_bits_from_hex_string(pair_list[index].second, signal["Start Bit"].asInt(), signal["Length"].asInt());
                    new_value = get_bits_from_hex_string(connected_values, signal["Start Bit"].asInt(), signal["Length"].asInt());
                }
                if (old_value == new_value) {
                    total_same_percent += signal["Length"].asInt() * 100;
                } else {
                    double diff = abs((double)old_value - (double)new_value) / (double)max(old_value, new_value) * 100;
                    total_same_percent += signal["Length"].asInt() * (100 - diff);
                }
            }

            if ((total_same_percent / total_length) >= 40) { //수용치
                pair_list[index].second = connected_values;
                //정상 페이로드이므로 저장
                return true;std::ostringstream payload_string;
                
    for (int i = 0; i < DLC; ++i) {
        payload_string << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    // 문자열로 저장
    std::string payload_hexStr = payload_string.str();
    //std::cout << payload_hexStr << "\n";
            } else {
                return false;
            }
        }
    }
    if (!pair_found) {
        //정상 페이로드이므로 저장
        pair_list.emplace_back("0x" + data[1], connected_values);
        return true;
    }
}
