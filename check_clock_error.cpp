#include "check_clock_error.h"
#include <iostream>
#include "periodic.h"

// CAN ID별로 ClockSkewDetector를 관리하는 전역 맵 정의
std::unordered_map<uint32_t, ClockSkewDetector> clockSkewDetectors;

// ClockSkewDetector 생성자 정의
ClockSkewDetector::ClockSkewDetector(double threshold)
    : L_plus(0.0), L_minus(0.0), kappa(0.5), threshold(threshold) {}

// 복사 생성자 정의
ClockSkewDetector::ClockSkewDetector(const ClockSkewDetector& other)
    : L_plus(other.L_plus), L_minus(other.L_minus), kappa(0.5), threshold(other.threshold) {}


// 복사 대입 연산자 정의
ClockSkewDetector& ClockSkewDetector::operator=(const ClockSkewDetector& other) {
    if (this != &other) {
        L_plus = other.L_plus;
        L_minus = other.L_minus;
        threshold = other.threshold;
        // kappa는 const이므로 복사하지 않음
    }
    return *this;
}

// 클럭 스큐 오차를 체크하는 함수 구현
bool ClockSkewDetector::checkClockError(uint32_t can_id, double timestamp) {
    calc_periodic(can_id, timestamp);
    CANStats& stats = can_stats[can_id];

    if (stats.count < MIN_DATA_CNT) {
        std::cout << "CAN ID " << can_id << ": leat of data(current data cnt): " << stats.count << ")\n";
        return false;
    }

    // 타임스탬프 간 시간차 계산
    double time_diff = timestamp - stats.last_timestamp;
    double error = time_diff - stats.periodic;

    // CUSUM(누적합) 계산
    L_plus = std::max(0.0, L_plus + error - kappa);
    L_minus = std::max(0.0, L_minus - error - kappa);

    // 디버그 출력: 오차와 CUSUM 값들 출력
    std::cout << "CAN ID " << can_id << ": "
              << "time_diff = " << time_diff << ", "
              << "error = " << error << ", "
              << "L_plus = " << L_plus << ", "
              << "L_minus = " << L_minus << "\n";

    if (L_plus > threshold || L_minus > threshold) {
        std::cout << "CAN ID " << can_id << ": abnormal timing detection\n";
        return true;
    }

    return false;
}

// 전역 함수 check_clock_error 구현
bool check_clock_error(uint32_t can_id, double timestamp) {
    // 해당 CAN ID에 대한 ClockSkewDetector가 없으면 새로 생성
    if (clockSkewDetectors.find(can_id) == clockSkewDetectors.end()) {
        std::cout << "CAN ID " << can_id << ": Create new ClockSkewDetector\n";
        clockSkewDetectors[can_id] = ClockSkewDetector();
    }

    // 해당 CAN ID의 ClockSkewDetector로 클럭 스큐 오류를 체크
    return clockSkewDetectors[can_id].checkClockError(can_id, timestamp);
}
