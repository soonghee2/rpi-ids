#ifndef CANSTATS_H
#define CANSTATS_H

#include <unordered_map>
#include <cstdint>
#include <set>

struct CANStats {
    double periodic = 0;
    double squared_diff_sum = 0;
    double last_timestamp = 0;
    uint8_t last_data[8];
    double prev_timediff = 0;
    bool is_periodic=false;
    int count = 0;

    int event_count = 0;
    double event_last_timestamp = 0;
    double no_event_last_timestamp =0;
   
    int suspected_count = 0;
};

typedef struct qCANMsg {
    double timestamp;      // 타임스탬프 (초 단위)
    uint32_t can_id;     // CAN ID 
    int DLC;               // 데이터 길이 코드 (Data Length Code)
    uint8_t data[8];       // CAN 데이터 (최대 8바이트)
} EnqueuedCANMsg;

extern std::int MIN_CAN_ID = 0x00;
extern std::unordered_map<uint32_t, CANStats> can_stats;
extern std::set<uint32_t> sorted_canIDSet;
#endif
