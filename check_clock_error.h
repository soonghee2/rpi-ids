#ifndef CHECK_CLOCK_ERROR_H
#define CHECK_CLOCK_ERROR_H

#include <unordered_map>
#include <cstdint>  // for uint32_t
#include "CANStats.h"  // CANStats 구조체 포함

// 최소 데이터 수 상수 정의
const int MIN_DATA_CNT = 10;

// ClockSkewDetector 클래스 선언
class ClockSkewDetector {
public:
    // 생성자
    ClockSkewDetector(double threshold = 5.0);

    // 복사 생성자 정의
    ClockSkewDetector(const ClockSkewDetector& other);

    // 복사 대입 연산자 정의
    ClockSkewDetector& operator=(const ClockSkewDetector& other);

    // 클럭 스큐 오차를 체크하는 함수
    bool checkClockError(uint32_t can_id, double timestamp);

private:
    double L_plus;
    double L_minus;
    const double kappa;  // CUSUM 계산에 사용할 상수, const이므로 초기화 후 변경 불가
    double threshold;    // 탐지 임계값
};

// CAN ID별로 ClockSkewDetector를 관리하는 맵
extern std::unordered_map<uint32_t, ClockSkewDetector> clockSkewDetectors;

// 클럭 스큐 오류 체크를 수행하는 함수
bool check_clock_error(uint32_t can_id, double timestamp);

#endif // CHECK_CLOCK_ERROR_H
