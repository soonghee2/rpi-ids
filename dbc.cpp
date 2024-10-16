#include "dbc.h"

//extern std::vector<CANMessage> CANMessages;
    
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
    
    extern std::vector<CANMessage> CANMessages;

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
    for (const auto& message : CANMessages) {
            if (can_id == message.CANID) {
    if(is_initial_data){
        is_initial_data = false;
        return true;
    }else{
        for (const auto& signal : message.Signals) {
          total_length += signal.Length;
          unsigned long long old_value, new_value;
          if (signal.ByteOrder == 1) {
              int first = signal.StartBit / 8;
              int end = ((signal.StartBit + signal.Length) / 8);
              int bit_length = (end - first) * 8;
              old_value = get_bits_from_hex_string(valid_payload_hexStr, first * 8, bit_length);
              new_value = get_bits_from_hex_string(payload_hexStr, first * 8, bit_length);
              old_value = std::bitset<64>(to_little_endian_int(old_value, (end - first))).to_ullong();
              new_value = std::bitset<64>(to_little_endian_int(new_value, (end - first))).to_ullong();
              old_value = cut_bits(old_value, ((end - first) * 8) - signal.Length - (signal.StartBit % 8), signal.Length);
              new_value = cut_bits(new_value, ((end - first) * 8) - signal.Length - (signal.StartBit % 8), signal.Length);
          } else {
              old_value = get_bits_from_hex_string(valid_payload_hexStr, signal.StartBit, signal.Length);
              new_value = get_bits_from_hex_string(payload_hexStr, signal.StartBit, signal.Length);
          }

          if (old_value == new_value) {
              total_same_percent += signal.Length * 100;
          } else {
              double diff = abs((double)old_value - (double)new_value) / (double)std::max(old_value, new_value) * 100;
              total_same_percent += signal.Length * (100 - diff);
          }
      }

      if ((total_same_percent / total_length) >= 40) { //수용치
          return true;
      } else {
          return false;
      }
    }
    }
}
  return false;
}

bool validation_check(uint32_t can_id, uint8_t* data, int DLC) {

    extern std::vector<CANMessage> CANMessages;

    std::ostringstream payload_string;
    for (int i = 0; i < DLC; ++i) {
        payload_string << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    // 문자열로 저장
    std::string payload_hexStr = payload_string.str();

    for (const auto& message : CANMessages) {
        if (can_id == message.CANID) { //can_id check
            if (!message.Skipable) {
                for (const auto& signal : message.Signals) {
                        int start_bit = signal.StartBit;
                        int length = signal.Length;
                        int first = start_bit / 8;
                        int end = ((start_bit + length) / 8);
                        unsigned long long binary_value = 0;
                        if (signal.ByteOrder == 1 && (end - first) > 1) {
                            unsigned long long binary_value = get_bits_from_hex_string(payload_hexStr, first * 8, (end - first) * 8);
                            binary_value = to_little_endian_int(binary_value, (end - first));
                            binary_value = cut_bits(binary_value, ((end - first) * 8) - length - (start_bit % 8), length);
                        } else {
                            binary_value = get_bits_from_hex_string(payload_hexStr, start_bit, length);
                        }
                        
                        if (signal.LowMinValue <= binary_value && binary_value <= signal.LowMaxValue) {

                        } else {
                            return false;
                        }
                }
                return true;
            } else {
	            return true;
            }
        }
    }
    return false;
}
