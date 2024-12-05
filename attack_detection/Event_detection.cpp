#include "Event_detection.h"

bool check_onEvent(double timestamp, CANStats& stats, uint32_t can_id, uint8_t data[]){
    double event_time_diff = 0;
    double time_diff = timestamp - stats.last_normal_timestamp;

    if(stats.event_count == -1){
        memset(stats.event_payload, 0, sizeof(stats.event_payload));
        stats.event_count = 0;
        stats.last_event_timstamp = timestamp;
    } else {
        event_time_diff = timestamp - stats.last_event_timstamp;
        if(EVENT_PERIOD * 1.2 >= event_time_diff && EVENT_PERIOD * 0.8 <= event_time_diff){
            stats.event_count++;
            stats.last_event_timstamp = timestamp;
            memcpy(stats.event_payload, data, sizeof(stats.event_payload));

            if(stats.event_count <= 10 && stats.event_count >= 1) {
                return true;
            }
            else {
		//updateMessage(can_id, "DoS", "Medium", " 짧은 %.5fms 시간동안 너무 많은 패킷이 들어왔습니다.(30ms)",event_time_diff);
                //printf("[On-Event] %03x 짧은 시간동안 너무 많은 패킷이 들어왔습니다.(30ms)\n",can_id);
                stats.event_count = -1;
                return false;
            }
        } else if(check_periodic_range(time_diff / 2, stats.periodic) || memcmp(stats.event_payload, data, sizeof(stats.event_payload)) == 0){
            if(stats.event_count == 0){
                stats.event_count = -1;
                return false;
            }
            stats.event_count = -1;
            stats.last_normal_timestamp = timestamp;
            return true;
        } else {
            stats.event_count = -1;
        }
    }
    return false;
}
