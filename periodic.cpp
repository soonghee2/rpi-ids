#include <stdio.h>
#include <math.h> 
#include <cstring>
#include "cQueue.h"
#include "CANStats.h"

typedef struct qCANMsg {
    double timestamp;      // 타임스탬프 (초 단위)
    uint8_t can_id[3];     // CAN ID (3바이트 크기 배열)
    int DLC;               // 데이터 길이 코드 (Data Length Code)
    uint8_t data[8];       // CAN 데이터 (최대 8바이트)
} EnqueuedCANMsg;

// Helper function to convert CAN ID array to a single integer
unsigned int convertCANID(const uint8_t* can_id) {
    return (can_id[0] << 16) | (can_id[1] << 8) | can_id[2];
}

void calc_periodic(uint8_t can_id[], double timestamp) {
    unsigned int can_id_int = convertCANID(can_id);

    CANStats& stats = can_stats[can_id_int];
    stats.count++;

    if (stats.count == 1) {
        stats.mean = timestamp;  // 첫 패킷의 타임스탬프를 평균으로
        stats.sum_of_squared_diffs = 0;  // 편차 제곱합 초기화
    } else {
        double prev_mean = stats.mean;
        stats.mean += (timestamp - prev_mean) / stats.count;

        double diff = timestamp - prev_mean;
        stats.sum_of_squared_diffs += diff * (timestamp - stats.mean);
    }
    
}

double get_standard_deviation(unsigned int can_id) {
    CANStats& stats = can_stats[can_id];
    if (stats.count > 1) {
        return std::sqrt(stats.sum_of_squared_diffs / stats.count);
    }
    return 0.0;  // 데이터가 부족한 경우 표준편차는 0
}