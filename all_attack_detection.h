#ifndef ALL_ATTACK_DETECTION_H
#define ALL_ATTACK_DETECTION_H

#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <iostream>
#include "periodic.h"
#include "CANStats.h" 


#define EVENT_PERIOD 0.03
#define DoS_TIME_THRESHOLD_MS 0.005
#define DoS_DETECT_THRESHOLD 5

const double FORGETTING_FACTOR = 0.95;  
const double CUSUM_THRESHOLD = 5.0;     
const int MIN_DATA_CNT = 100;           
const double ERROR_SCALING_FACTOR = 1000.0; 

bool check_periodic_range(double time_diff, double periodic);
bool check_clock_error();
bool check_previous_packet_of_avg(double current_timediff, CANStats& stats);
bool check_low_can_id(uint32_t can_id);
bool check_DoS(EnqueuedCANMsg dequeuedMsg);
bool check_onEvent(double timestamp, CANStats& stats, uint32_t can_id, uint8_t data[]);
bool check_over_double_periodic(double timestamp, CANStats& stats, uint32_t can_id);
bool check_replay(CANStats& stats, uint8_t data[]);
bool filtering_process(EnqueuedCANMsg* dequeuedMsg);
bool check_clock_error(uint32_t can_id, double timestamp);

class ClockSkewDetector {
public:
    explicit ClockSkewDetector(double threshold = CUSUM_THRESHOLD);  // 생성자
    ClockSkewDetector(const ClockSkewDetector& other);      //복사 생성

    bool checkClockError(uint32_t can_id, double timestamp);  // 클럭 스큐 오류를 체크하는 함수
private:
    int m_detect_cnt;
    double accumulatedOffset; // 누적된 클럭 오프셋
    double upperLimit;        // CUSUM 상한 제어 값
    double lowerLimit;        // CUSUM 하한 제어 값
    double meanError;         // CUSUM용 평균 오류
    double last_meanError;      //직전 meanError
    double stdError;          // CUSUM용 표준 편차
    double threshold;         // CUSUM 임계값

    bool detectAnomaly(double error, uint32_t can_id);  // CUSUM을 사용한 이상 탐지
};

extern std::unordered_map<uint32_t, ClockSkewDetector> clockSkewDetectors;
extern uint8_t DoS_payload[8];   // 전역 변수 선언
extern int suspected_count;      // 전역 변수 선언
extern uint32_t DoS_can_id;      // 전역 변수 선언


#endif // ALL_ATTACK_DETECTION_H
