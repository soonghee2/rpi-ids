#include "DoS_detection.h"
#include <cstring>

uint8_t DoS_payload[8];
uint32_t DoS_can_id = 0;

bool check_DoS(const EnqueuedCANMsg& dequeuedMsg) {
    CANStats& stats = can_stats[dequeuedMsg.can_id];
    double time_diff = dequeuedMsg.timestamp - stats.last_timestamp;

    if (time_diff < DoS_TIME_THRESHOLD_MS) {
        if (memcmp(stats.last_data, dequeuedMsg.data, sizeof(stats.last_data)) == 0) {
            stats.suspected_count++;
        } else {
            stats.suspected_count = 1;
        }
        if (stats.suspected_count == DoS_DETECT_THRESHOLD) {
            DoS_can_id = dequeuedMsg.can_id;
            memcpy(DoS_payload, dequeuedMsg.data, sizeof(DoS_payload));
            return true;
        }
    }

    if(DoS_can_id == dequeuedMsg.can_id && memcmp(DoS_payload, dequeuedMsg.data, sizeof(DoS_payload)) == 0){
        return true;
    }
    return false;
}

