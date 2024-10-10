#include "validation_check.h"

unsigned long long Validation_Check::get_bits_from_hex_string(const std::string &hex_string, int start_bit, int bit_length) {
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

unsigned long long Validation_Check::cut_bits(unsigned long long number, int start, int length) {
    unsigned long long mask = (1ULL << length) - 1;
    return (number >> (64 - start - length)) & mask;
}

unsigned long long Validation_Check::to_little_endian_int(unsigned long long number, int byte_size) {
    unsigned long long result = 0;
    for (int i = 0; i < byte_size; ++i) {
        result |= ((number >> (i * 8)) & 0xFF) << (i * 8);
    }
    return result;
}

bool Validation_Check::validation_check(uint32_t can_id, uint8_t* data, int DLC) {
    bool is_valid_id=false;
    bool is_valid_range_data=false;
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
    
    
    for (const auto &message : dbc) {
        if ("0x" + can_id_hexStr == message["CAN ID"].asString()){ //can_id check
            is_valid_id = true;
            //std::cout << message["CAN ID"].asString() << "\n";
            for (const auto &signal : message["Signals"]) {
                if (!signal["Skipable"].asBool()) {
                    int start_bit = signal["Start Bit"].asInt();
                    int length = signal["Length"].asInt();
                    int first = start_bit / 8;
                    int end = ((start_bit + length) / 8);
                    unsigned long long binary_value = 0;
                    //std::cout << "min:" << signal["Low Min Value"].asDouble() << "\n";
                    //std::cout << "max:" << signal["Low Max Value"].asDouble() << "\n";
                    if (signal["Byte Order"].asInt() == 1 && (end - first) > 1) {
                        //std::cout << "byte order = 1\n";
                        unsigned long long binary_value = get_bits_from_hex_string(payload_hexStr, first * 8, (end - first) * 8);
                        binary_value = to_little_endian_int(binary_value, (end - first));
                        binary_value = cut_bits(binary_value, ((end - first) * 8) - length - (start_bit % 8), length);
                    } else {
                        binary_value = get_bits_from_hex_string(payload_hexStr, start_bit, length);
                    }
                    //std::cout << "payload:" <<binary_value << "\n";
                    if (signal["Low Min Value"].asDouble() <= binary_value && binary_value <= signal["Low Max Value"].asDouble()) {
                        return true;
                    } else {
                        return is_valid_range_data;
                    }
                } else {
                    return true;
                }
            }
            return true;
        }
    }
    return is_valid_id;
}

