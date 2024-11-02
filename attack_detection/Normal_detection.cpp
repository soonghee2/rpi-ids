#include "Normal_detection.h"

bool check_periodic_range(double time_diff, double periodic) {
    return (periodic * 0.8 <= time_diff && time_diff <= periodic * 1.2);
}

bool check_previous_packet_of_avg(double current_timediff, CANStats& stats) {
    if (stats.prev_timediff == 0.0 && (current_timediff > stats.periodic * 1.2 && current_timediff <= stats.periodic * 3)) {
        stats.prev_timediff = current_timediff;
        return false;
    }

    if (stats.prev_timediff != 0.0 && check_periodic_range((current_timediff + stats.prev_timediff) / 2, stats.periodic)) {
        stats.prev_timediff = 0; 
        return true;
    }

    stats.prev_timediff = 0;  
    return false;
}

bool check_low_can_id(uint32_t can_id) {
    return can_id <= MIN_CAN_ID;
}

