#ifndef CHECK_CLOCK_ERROR_H
#define CHECK_CLOCK_ERROR_H

#include "CANStats.h"
#include <cstdint>
#include <cmath>
#include <fstream>
#include <iomanip> // setprecision을 사용하기 위해 추가
#include <iostream>
#include <numeric> // std::accumulate 사용
#include <unordered_map>
#include <algorithm>

#define window 30
const int MIN_DATA_CNT = 100;              // 최소 수데이터 
const double accept_percentage=0.9;


class ClockSkewDetector {
public:
    ClockSkewDetector();  // 기본 생성자 추가
    int m_detect_cnt;
    double upperLimit;        // CUSUM 기울기 상한 제어 값
    double lowerLimit;        // CUSUM 기울기 하한 제어 값
    double prev_average;
    double cum_clock_skew[window];
    double diff_average;
};

extern std::unordered_map<uint32_t, ClockSkewDetector> clockSkewDetectors;

bool check_clock_error(uint32_t can_id, double timestamp, CANStats& stats);

#endif // CHECK_CLOCK_ERROR_H