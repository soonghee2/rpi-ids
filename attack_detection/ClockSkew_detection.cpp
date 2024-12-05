#include "ClockSkew_detection.h"

std::unordered_map<uint32_t, ClockSkewDetector> clockSkewDetectors;

ClockSkewDetector::ClockSkewDetector(double threshold)
    : m_detect_cnt(0), accumulatedOffset(0),
      upperLimit(0), lowerLimit(0), meanError(0), last_meanError(0), stdError(1.0), threshold(threshold) {}

ClockSkewDetector::ClockSkewDetector(const ClockSkewDetector& other)
    : m_detect_cnt(0),  accumulatedOffset(other.accumulatedOffset),
      upperLimit(other.upperLimit), lowerLimit(other.lowerLimit), meanError(other.meanError), last_meanError(0),
      stdError(other.stdError), threshold(other.threshold) {}

bool ClockSkewDetector::checkClockError(uint32_t can_id, double timestamp) {
    CANStats& stats = can_stats[can_id];

    if (stats.count < MIN_DATA_CNT) {
        return false;
    }

    double time_diff = timestamp - stats.last_timestamp;
    double error = time_diff - stats.periodic;

    accumulatedOffset += (error);

    return detectAnomaly(error, can_id); // 이상이 없는 경우 false 반환
}

bool ClockSkewDetector::detectAnomaly(double error, uint32_t can_id) {
    double scaledError = error * ERROR_SCALING_FACTOR;

    meanError = FORGETTING_FACTOR * meanError + (1 - FORGETTING_FACTOR) * scaledError;
    stdError = std::sqrt(FORGETTING_FACTOR * stdError * stdError + (1 - FORGETTING_FACTOR) * (scaledError - meanError) * (scaledError - meanError));
    upperLimit += (scaledError - meanError) / stdError;

    if (std::abs(last_meanError - meanError) > 1.5) { m_detect_cnt++; } else{ m_detect_cnt=0;}

    if (m_detect_cnt > 5) {
        m_detect_cnt = 0;  // Reset the detection count after detection
        //printf("[Masquerade] [%03x] [High] 연속 5번 이상 1.5 이상의 범위 밖으로 벗어났습니다. 지난 패킷의 meanError은 %.6f으로 정상 %03x의 %.6f보다 %.6f만큼 더 오차가 발생했습니다.\n", can_id, last_meanError, can_id, meanError, last_meanError-meanError);
	return true;       // Return true if anomaly is detected
    }

    last_meanError = meanError;

    return false; // Return false if no anomaly is detected
}

bool check_clock_error(uint32_t can_id, double timestamp) {
    if (clockSkewDetectors.find(can_id) == clockSkewDetectors.end()) {
        clockSkewDetectors[can_id] = ClockSkewDetector(CUSUM_THRESHOLD);
    }
    CANStats& stats = can_stats[can_id];
    stats.count++;

    return clockSkewDetectors[can_id].checkClockError(can_id, timestamp);
}
