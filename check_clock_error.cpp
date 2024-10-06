#include "check_clock_error.h"
#include <unordered_map>

const int MIN_DATA_CNT=100;

// CAN ID별로 ClockSkewDetector를 관리하는 맵
std::unordered_map<uint32_t, ClockSkewDetector> clockSkewDetectors;

// 클럭 스큐를 추정하고, 비정상적인 변화가 발생하는지 확인하는 클래스
class ClockSkewDetector {
public:
    ClockSkewDetector(double threshold = 5.0) : threshold(threshold), L_plus(0.0), L_minus(0.0) {}

    // 클럭 스큐 오차를 체크하는 함수
    bool checkClockError(uint32_t can_id, double timestamp) {
        CANStats& stats = can_stats[can_id];

        if (stats.count < MIN_DATA_CNT) {
            // 데이터가 부족할 경우 검사를 건너뜁니다.
            return false;
        }

        double time_diff = timestamp - stats.last_timestamp;
        double error = time_diff - stats.periodic;

        // CUSUM(누적합) 계산
        L_plus = std::max(0.0, L_plus + error - kappa);
        L_minus = std::max(0.0, L_minus - error - kappa);

        if (L_plus > threshold || L_minus > threshold) {
            return true; // 비정상적인 타이밍 탐지
        }

        return false;
    }

private:
    double L_plus;
    double L_minus;
    const double kappa = 0.5;   // 누적합 계산에 사용할 상수
    double threshold;           // 탐지 임계값
};

// 클럭 스큐 오류 체크를 수행하는 함수
bool check_clock_error(uint32_t can_id, double timestamp) {
    // 해당 CAN ID에 대한 ClockSkewDetector가 없으면 새로 생성
    if (clockSkewDetectors.find(can_id) == clockSkewDetectors.end()) {
        clockSkewDetectors[can_id] = ClockSkewDetector();
    }

    // 해당 CAN ID의 ClockSkewDetector로 클럭 스큐 오류를 체크
    return clockSkewDetectors[can_id].checkClockError(can_id, timestamp);
}
