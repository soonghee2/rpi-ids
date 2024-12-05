#include "Malicious_uds_detection.h"

const std::vector<uint8_t> VALID_UDS_REQUESTS = {0x10, 0x11, 0x22, 0x2E, 0x2F, 0x31, 0x34, 0x37};
const std::uint16_t VALID_CAN_ID_MIN = 0x700;
const std::uint16_t VALID_CAN_ID_MAX = 0x7FF;
const uint8_t RESET_SERVICE_CODE = 0x11; // ECU Reset service code
const int HIGH_FREQUENCY_THRESHOLD = 1; // Seconds
const int RESET_COUNT_THRESHOLD = 3; // Number of consecutive resets to trigger malicious detection

// Helper function: Check if UDS request is valid
bool isValidUDS(uint8_t data[], uint32_t can_id) {
    if (can_id < VALID_CAN_ID_MIN || can_id > VALID_CAN_ID_MAX) {
        return false;
    }

    if (std::find(VALID_UDS_REQUESTS.begin(), VALID_UDS_REQUESTS.end(), data[1]) != VALID_UDS_REQUESTS.end()) {
        return true;
    }

    return false;
}

// Function to check if UDS is malicious
bool isMaliciousUDS(CANStats& stats, uint8_t data[], uint32_t can_id) {
    if (isValidUDS(data, can_id)) {
        // Check for Reset Service Code
        if (data[1] == RESET_SERVICE_CODE) {
            // Check frequency of resets
            if (stats.resetcount == 0) {
                stats.resetcount++;
                stats.reset_timestamp = stats.last_timestamp;
            } else {
                if (stats.last_timestamp - stats.reset_timestamp < HIGH_FREQUENCY_THRESHOLD) {
		    if(data[2] == 0x51){
			    stats.resetcount++;
		    }
                } else {
                    stats.resetcount = 1; // Reset count due to time gap
                }
                stats.reset_timestamp = stats.last_timestamp;
            }
        }

    }

    return false;
}

