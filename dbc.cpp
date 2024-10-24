#include "dbc.h"

unsigned long long get_bits_from_hex_string(const std::string &hex_string, int start_bit, int bit_length) {
    unsigned long long value = 0;
    for (char c : hex_string) {
        value <<= 4; 
        value |= (c >= '0' && c <= '9') ? c - '0' : 10 + (tolower(c) - 'a');
    }

    unsigned long long mask = (1ULL << bit_length) - 1;
    return (value >> (hex_string.length() * 4 - start_bit - bit_length)) & mask;
}

unsigned long long cut_bits(unsigned long long number, int start, int length) {
    unsigned long long mask = (1ULL << length) - 1;
    return (number >> (64 - start - length)) & mask;
}

unsigned long long to_little_endian_int(unsigned long long number, int byte_size) {
    unsigned long long result = 0;
    for (int i = 0; i < byte_size; ++i) {
        result |= ((number >> (i * 8)) & 0xFF) << (i * 8);
    }
    return result;
}

bool check_similarity_with_previous_packet(uint32_t can_id, uint8_t data[8], int DLC, uint8_t valid_payload[8], bool is_initial_data) {
    
    extern std::unordered_map<int, CANMessage> message;

    int total_same_percent=0;
    int total_length=0;

    std::ostringstream payload_string;
    for (int i = 0; i < DLC; ++i) {
        payload_string << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    // 문자열로 저장
    std::string payload_hexStr = payload_string.str();

    std::ostringstream valid_payload_string;
    for (int i = 0; i < DLC; ++i) {
        valid_payload_string << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    // 문자열로 저장
    std::string valid_payload_hexStr = valid_payload_string.str();
    if(message.find(can_id) != message.end()) {
    if(is_initial_data){
        is_initial_data = false;
        return true;
    }else{
        for (const auto& signal : message[can_id].signals) {
          total_length += signal.length;
          unsigned long long old_value, new_value;
          if (signal.byte_order == 1) {
              int first = signal.start_bit / 8;
              int end = ((signal.start_bit + signal.length) / 8);
              int bit_length = (end - first) * 8;
              old_value = get_bits_from_hex_string(valid_payload_hexStr, first * 8, bit_length);
              new_value = get_bits_from_hex_string(payload_hexStr, first * 8, bit_length);
              old_value = std::bitset<64>(to_little_endian_int(old_value, (end - first))).to_ullong();
              new_value = std::bitset<64>(to_little_endian_int(new_value, (end - first))).to_ullong();
              old_value = cut_bits(old_value, ((end - first) * 8) - signal.length - (signal.start_bit % 8), signal.length);
              new_value = cut_bits(new_value, ((end - first) * 8) - signal.length - (signal.start_bit % 8), signal.length);
          } else {
              old_value = get_bits_from_hex_string(valid_payload_hexStr, signal.start_bit, signal.length);
              new_value = get_bits_from_hex_string(payload_hexStr, signal.start_bit, signal.length);
          }
          if (old_value == new_value) {
              total_same_percent += signal.length * 100;
          } else {
              double diff = abs((double)old_value - (double)new_value) / (double)std::max(old_value, new_value) * 100;
              total_same_percent += signal.length * (100 - diff);
          }
      }

      if ((total_same_percent / total_length) >= 40) { //수용치
          return true;
      } else {
          return false;
      }
    }
}
  return false;
}

bool validation_check(uint32_t can_id, uint8_t* data, int DLC) {

    extern std::unordered_map<int, CANMessage> message;

    std::ostringstream payload_string;
    for (int i = 0; i < DLC; ++i) {
        payload_string << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    // 문자열로 저장
    std::string payload_hexStr = payload_string.str();

    if (message.find(can_id) != message.end()) {
        if (DLC == message[can_id].dlc){
            if (!message[can_id].Skipable) {
                for (const auto& signal : message[can_id].signals) {
                        int start_bit = signal.start_bit;
                        int length = signal.length;
                        int first = start_bit / 8;
                        int end = ((start_bit + length) / 8);
                        unsigned long long binary_value = 0;
                        if (signal.byte_order == 1 && (end - first) > 1) {
                            unsigned long long binary_value = get_bits_from_hex_string(payload_hexStr, first * 8, (end - first) * 8);
                            binary_value = to_little_endian_int(binary_value, (end - first));
                            binary_value = cut_bits(binary_value, ((end - first) * 8) - length - (start_bit % 8), length);
                        } else {
                            binary_value = get_bits_from_hex_string(payload_hexStr, start_bit, length);
                        }
                        if (static_cast<unsigned long long>(signal.LowMinValue) <= binary_value && 
    binary_value <= static_cast<unsigned long long>(signal.LowMaxValue)) {
                            return true;
                        } else {
                            return false;
                        }
                    }
                } else {
	            return true;
                }
            }else{
                return false;
            }
    }
    return false;
}
