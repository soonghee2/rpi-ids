#ifndef PERIODIC_H
#define PERIODIC_H

#include "cQueue.h"
#include "CANStats.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>
#include <numeric>

#define PERIODIC_SAMPLE_THRESHOLD 30
#define SECOND_PERIODIC_SAMPLE_THRESHOLD 300

const double PERIODIC_STD_THRESHOLD = 0.05;  // 표준편차 기준 임계값
const double PERIODIC_CV_THRESHOLD = 0.1;  // 변동계수 기준 임계값

void calc_periodic(uint32_t can_id, double timestamp);
double get_standard_deviation(uint32_t can_id);
double get_coefficient_of_variation(double mean, double stddev);  // 변동계수 (CV) 계산 함수

#endif // PERIODIC_H
