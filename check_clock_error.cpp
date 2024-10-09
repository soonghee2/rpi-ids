#include "check_clock_error.h"
#include <iostream>
#include <cmath>  // For mathematical operations like sqrt
#include <limits> // For checking numeric bounds
#include "periodic.h"

// CAN ID별로 ClockSkewDetector를 관리하는 전역 맵 정의
std::unordered_map<uint32_t, ClockSkewDetector> clockSkewDetectors;

// ClockSkewDetector 생성자 정의
ClockSkewDetector::ClockSkewDetector(double threshold)
    : S(0), P(INITIAL_P_VALUE), accumulatedOffset(0),
      upperLimit(0), lowerLimit(0), meanError(0), stdError(1.0), threshold(threshold) {}

// 복사 생성자 정의
ClockSkewDetector::ClockSkewDetector(const ClockSkewDetector& other)
    : S(other.S), P(other.P), accumulatedOffset(other.accumulatedOffset),
      upperLimit(other.upperLimit), lowerLimit(other.lowerLimit), meanError(other.meanError), 
      stdError(other.stdError), threshold(other.threshold) {}

// 클럭 스큐 오차를 체크하는 함수 구현
bool ClockSkewDetector::checkClockError(uint32_t can_id, double timestamp) {
    
    // 타임스탬프가 유효한지 확인
    if (timestamp < 0 || timestamp > std::numeric_limits<double>::max()) {
        std::cerr << "Error: Invalid timestamp value detected.\n";
        return false;
    }

    // CANStats 구조체를 사용하여 해당 CAN ID에 대한 통계 가져오기
    CANStats& stats = can_stats[can_id];
    // printf("===============clock skew ==============\n");

    // 데이터가 충분한지 확인
    if (stats.count < MIN_DATA_CNT) {
        std::cout << "CAN ID " << can_id << ": Insufficient data (current data count: " << stats.count << ")\n";
        return false;
    }

    // 타임스탬프 간 시간차 계산
    double time_diff = timestamp - stats.last_timestamp;
    stats.last_timestamp=timestamp;
    double error = time_diff - stats.periodic;

    // RLS 알고리즘을 사용하여 클럭 스큐 추정 업데이트
    updateSkewEstimate(time_diff, error);

    // CUSUM을 사용하여 이상 탐지 수행
    bool isAnomalous = detectAnomaly(error);

    if (isAnomalous) {
        std::cerr << "CAN ID " << can_id << ": Anomaly detected! Possible malicious activity.\n";
        return false; // 이상 탐지 시 true 반환
    }

    return true; // 이상이 없는 경우 false 반환
}

// RLS 알고리즘을 사용하여 클럭 스큐 추정 업데이트
void ClockSkewDetector::updateSkewEstimate(double time_diff, double error) {
    // 이득(Gain) 계산
    double gain = (P * time_diff) / (1 + time_diff * time_diff * P);
    P = (P - gain * time_diff * P); // 공분산 업데이트
    S += gain * error; // 스큐 업데이트
    accumulatedOffset += std::abs(error); // 누적 오프셋 계산
}

// CUSUM을 사용하여 이상 탐지 수행
bool ClockSkewDetector::detectAnomaly(double error) {
    // CUSUM의 평균 오류와 표준 편차 업데이트 (지수 가중 이동 평균 사용)
    meanError = FORGETTING_FACTOR * meanError + (1 - FORGETTING_FACTOR) * error;
    stdError = std::sqrt(FORGETTING_FACTOR * stdError * stdError + (1 - FORGETTING_FACTOR) * (error - meanError) * (error - meanError));


    // printf("Error: %.6f\n", error);
    // CUSUM 상한 및 하한 제어 한계 업데이트
    upperLimit = std::max(0.0, upperLimit + (error - meanError) / stdError - threshold);
    lowerLimit = std::max(0.0, lowerLimit - (error - meanError) / stdError - threshold);
     printf("Error: %.6f\n, meanError: %.6f, stdError: %.6f, \n upperLimit: %.6f, lowerLimit: %.6f, CUSUM_THRESHOLD: %.6f\n", 
        error, meanError, stdError, upperLimit, lowerLimit, CUSUM_THRESHOLD);

    // 임계값 초과 시 이상 탐지
    return (upperLimit > CUSUM_THRESHOLD || lowerLimit > CUSUM_THRESHOLD);
}

// 전역 함수 check_clock_error 구현
bool check_clock_error(uint32_t can_id, double timestamp) {
    // 해당 CAN ID에 대한 ClockSkewDetector가 없으면 새로 생성
    if (clockSkewDetectors.find(can_id) == clockSkewDetectors.end()) {
        std::cout << "CAN ID " << can_id << ": Creating a new ClockSkewDetector instance.\n";
        clockSkewDetectors[can_id] = ClockSkewDetector(CUSUM_THRESHOLD);
    }
    CANStats& stats = can_stats[can_id];
    stats.count++;

    // 해당 CAN ID의 ClockSkewDetector로 클럭 스큐 오류를 체크
    return clockSkewDetectors[can_id].checkClockError(can_id, timestamp);
}
