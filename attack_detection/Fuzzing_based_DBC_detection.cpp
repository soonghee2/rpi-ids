#include "Fuzzing_based_DBC_detection.h"

uint64_t extractBits(uint64_t data, int start, int length, int DLC) {
    int adjustedStart = DLC*8 - start - length;
    uint64_t mask = ((1ULL << length) - 1) << adjustedStart;
    return (data & mask) >> adjustedStart;
}

uint64_t toLittleEndian(uint64_t data, int byteSize) {
    uint64_t result = 0;
    for (int i = 0; i < byteSize; ++i) {
        result |= ((data >> (8 * i)) & 0xFF) << (8 * (byteSize - 1 - i));
    }
    
    return result;
}

bool check_similarity_with_previous_packet(uint32_t can_id, uint8_t data[8], int DLC, uint8_t valid_payload[8], int percent, int& count) {   
    
    double total_same_percent=0;
    int total_length=0;

    uint64_t payload_combined = 0;
    uint64_t valid_payload_combined = 0;
    for (int i = 0; i < DLC; ++i) {
        payload_combined |= ((uint64_t)data[i] << (8 * (DLC - 1 - i)));
        valid_payload_combined |= ((uint64_t)valid_payload[i] << (8 * (DLC - 1 - i)));
    }
    if(message.find(can_id) != message.end()) {
        if(count == 0){
            count++;
            return true;
        }
        for (const auto& signal : message[can_id].signals) {
        if (!message[can_id].signals.empty()){
            total_length += signal.length;
            uint64_t old_value, new_value;
            new_value = extractBits(payload_combined, signal.start_bit, signal.length, DLC);
            old_value = extractBits(valid_payload_combined, signal.start_bit, signal.length, DLC);
            if((int)old_value == (int)new_value){
                total_same_percent += signal.length * 100;
            }else{
                double diff = std::min(std::pow(2,total_length) - fabs((double)old_value - (double)new_value),fabs((double)old_value - (double)new_value)) / std::pow(2,total_length) * 100;
                total_same_percent += signal.length * (100 - diff);
            }
        }else{
        return true;
    }
    }
        if ((total_same_percent / total_length) >= percent) { //수용치
            return true;
        } else {
	    return false;
        }
    }

    is_Attack = 3;
    return false;
}

bool validation_check(uint32_t can_id, uint8_t* data, int DLC) {

    uint64_t payload_combined = 0;
    
    for (int i = 0; i < DLC; ++i) {
        payload_combined |= ((uint64_t)data[i] << (8 * (DLC - 1 - i)));
    }

    
    if (message.find(can_id) != message.end()) {
        if (DLC == message[can_id].dlc){
            if (!message[can_id].Skipable) {
                if (!message[can_id].signals.empty()){
                for (const auto& signal : message[can_id].signals) {
                    int start_bit = signal.start_bit;
                    int length = signal.length;
                    int first = start_bit / 8;
                    int end = ((start_bit + length) / 8);
                    uint64_t binary_value = 0;
                    if (signal.byte_order == 1 && (end - first) > 1) {
                        binary_value = extractBits(payload_combined, first * 8, (end - first) * 8, DLC);
                        binary_value = toLittleEndian(binary_value, (end - first));
                        binary_value = extractBits(binary_value, ((end - first) * 8) - length - (start_bit % 8), length, (end - first));
                    } else {
                        binary_value = extractBits(payload_combined, start_bit, length, DLC);
                    }
                    if ((uint64_t)signal.LowMinValue <= binary_value && binary_value <= (uint64_t)signal.LowMaxValue) {
			            return true;
                    } else {
                        is_Attack = 3;
			return false;
                    }
                }
            }else{
                return true;
            }
            }else{
                return true;
            }
        } else {
            is_Attack = 3;
	    return false;
        }
    }
    is_Attack = 3;
    return false;
}

void calc_similarity(uint32_t can_id, uint8_t data[8], int DLC, uint8_t valid_payload[8], float& similarity_percent, int count) {
    double total_same_percent=0;
    int total_length=0;

    uint64_t payload_combined = 0;
    uint64_t valid_payload_combined = 0;
    for (int i = 0; i < DLC; ++i) {
        payload_combined |= ((uint64_t)data[i] << (8 * (DLC - 1 - i)));
        valid_payload_combined |= ((uint64_t)valid_payload[i] << (8 * (DLC - 1 - i)));
    }
    
    if(message.find(can_id) != message.end()) {
        if(count == 0){
            return;
        }
        if (!message[can_id].signals.empty()){
        for (const auto& signal : message[can_id].signals) {
            total_length += signal.length;
            uint64_t old_value, new_value;
            new_value = extractBits(payload_combined, signal.start_bit, signal.length, DLC);
            old_value = extractBits(valid_payload_combined, signal.start_bit, signal.length, DLC);
            if (old_value == new_value) {
                total_same_percent += signal.length * 100;
            }else{
                double diff = std::min(std::pow(2,total_length) - fabs((double)old_value - (double)new_value),fabs((double)old_value - (double)new_value)) / std::pow(2,total_length) * 100;
                total_same_percent += signal.length * (100 - diff);
            }
        }
        
        similarity_percent = ((similarity_percent*count)+(total_same_percent/total_length))/(count+1);
    }else{
        return;
    }
      return;
    }
    return;
}

