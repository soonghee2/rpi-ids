#include "Malicious_uds_detection.h"

const std::vector<uint8_t> VALID_UDS_REQUESTS = {0x10, 0x11, 0x22, 0x2E, 0x2F, 0x31, 0x34, 0x37};
const std::uint16_t VALID_CAN_ID_MIN = 0x700;
const std::uint16_t VALID_CAN_ID_MAX = 0x7FF;
const uint8_t RESET_SERVICE_CODE = 0x11; // ECU Reset service code
const int HIGH_FREQUENCY_THRESHOLD =5;
const double COUNT_THRESHOLD = 0.5;

bool isValidUDS(uint8_t data[],  uint32_t can_id){
        if(can_id < VALID_CAN_ID_MIN || can_id > VALID_CAN_ID_MAX){
                return false;
        }

        if(std::find(VALID_UDS_REQUESTS.begin(), VALID_UDS_REQUESTS.end(), data[1]) != VALID_UDS_REQUESTS.end()) {
<<<<<<< HEAD:attack_detection/Malicous_uds_detection.cpp
=======
                //printf("Malicious UDS %03x \n", can_id);
>>>>>>> cd075e990a057f5e1da28150d907157f1b3c783c:attack_detection/Malicious_uds_detection_buf.cpp
                return true;
        }

        return false;
}

bool isMaliciousUDS(CANStats& stats, uint8_t data[], uint32_t can_id){
        if(isValidUDS(data, can_id)){
                if(data[1] == RESET_SERVICE_CODE){
                        if(stats.resetcount ==0){
                                stats.resetcount++;
                        } else {
                                if(stats.reset_timestamp - stats.last_timestamp < HIGH_FREQUENCY_THRESHOLD){
                                        stats.resetcount += 1;
                                }else {
                                        stats.resetcount = 0;
                                }
                        }
                }
                if(stats.resetcount >=1){
                        stats.reset_timestamp = stats.last_timestamp;
                }
                if(stats.resetcount >= COUNT_THRESHOLD){
<<<<<<< HEAD:attack_detection/Malicous_uds_detection.cpp
                        //printf("Malcious UDs %03x\n", can_id);
=======
                        printf("Malcious UDs %03x\n", can_id);
			printf("Data: ");
    			for (int i = 0; i < 8; i++) {
        			printf("0x%02X ", data[i]); // Hexadecimal format with leading zeros
    			}
    			printf("\n");
>>>>>>> cd075e990a057f5e1da28150d907157f1b3c783c:attack_detection/Malicious_uds_detection_buf.cpp
                        return true;
                }
                return false;
        }
        return false;
}
