#include "periodic.h"

// 주기성을 판단할 임계값 (실험적으로 설정)
const double PERIODIC_STD_THRESHOLD = 0.05;  // 표준편차 기준 임계값
const double PERIODIC_MAD_THRESHOLD = 0.0004;  // MAD 기준 임계값
const double PERIODIC_CV_THRESHOLD = 0.02;  // 변동계수 기준 임계값

// // CANStats를 CAN ID별로 저장하기 위한 맵 (CAN ID -> CANStats)
// std::unordered_map<uint32_t, CANStats> can_stats;


double get_standard_deviation(uint32_t can_id) {
    CANStats& stats = can_stats[can_id];
    if (stats.count > 1) {
        return std::sqrt(stats.squared_diff_sum / (stats.count -1));
    }
    return 0.0;  // 데이터가 부족한 경우 표준편차는 0
}


double get_mad(uint32_t can_id) {
    CANStats& stats = can_stats[can_id];
    if (stats.count > 1) {
        // 평균 기반 절대 편차 계산
        double mean = stats.sum_time_diff / (stats.count - 1);
        double absolute_deviation_sum = 0.0;
        double time_diff = stats.last_timestamp - (stats.sum_time_diff / stats.count);
        
        // 이전 주기의 절대 편차로 MAD 추정 (단일 값 사용)
        absolute_deviation_sum += std::abs(time_diff - mean);

        // 평균 절대 편차 반환
        return absolute_deviation_sum / (stats.count - 1);
    }
    return 0.0;  // 데이터가 부족한 경우 MAD는 0
}

double get_coefficient_of_variation(uint32_t can_id) {
    CANStats& stats = can_stats[can_id];
    if (stats.count > 1) {
        // 변동계수 (CV) = 표준편차 / 평균
        double mean = stats.sum_time_diff / (stats.count - 1);
        if (mean == 0.0) {
            return 0.0;  // 평균이 0이면 CV는 0으로 설정
        }
        double stddev = get_standard_deviation(can_id);
        return stddev / mean;
    }
    return 0.0;  // 데이터가 부족한 경우 CV는 0
}

void calc_periodic(uint32_t can_id, double timestamp) {

    CANStats& stats = can_stats[can_id];
    stats.count++;

    if (stats.count > 1) {
        double time_diff = timestamp - stats.last_timestamp;

        // 평균을 바로 계산하지 않고 누적 평균 및 제곱 합계로 계산
        stats.sum_time_diff += time_diff;
        stats.sum_time_diff_squared += time_diff * time_diff;

        double prev_periodic = stats.periodic;
        stats.periodic += (time_diff - prev_periodic) / (stats.count -1);

        double diff = time_diff - prev_periodic;
        stats.squared_diff_sum += diff * (time_diff - stats.periodic);

        // 주기성 판단: 표준편차, MAD, 변동계수 모두 임계값 이하인지 확인
        double stddev = get_standard_deviation(can_id);
        // double mad = get_mad(can_id);
        double cv = get_coefficient_of_variation(can_id);
        // printf("-------------stddev: %lf, mad: %lf, cv: %lf\n", stddev, mad, cv);
        printf("-------------stddev: %lf, cv: %lf\n", stddev, cv);
        
        if (stddev < PERIODIC_STD_THRESHOLD && cv < PERIODIC_CV_THRESHOLD) {
                printf("stats.is_periodic = true\n");
            stats.is_periodic = true;  // 세 가지 기준을 모두 만족하면 주기적
        } else { stats.is_periodic = false; 
                printf("stats.is_periodic = false\n");
        } // 그렇지 않으면 비주기적
        
    } 
    
    else {
        stats.periodic = 0;
        stats.squared_diff_sum = 0;
        stats.is_periodic = false;  // 데이터가 충분하지 않으므로 비주기적으로 초기화
    }
    
    stats.last_timestamp = timestamp;
}

// double get_standard_deviation(uint32_t can_id) {
//     CANStats& stats = can_stats[can_id];
//     if (stats.count > 1) {
//         return std::sqrt(stats.squared_diff_sum / (stats.count -1));
//     }
//     return 0.0;  // 데이터가 부족한 경우 표준편차는 0
// }


// double get_mad(uint32_t can_id) {
//     CANStats& stats = can_stats[can_id];
//     if (stats.count > 1) {
//         // 평균 기반 절대 편차 계산
//         double mean = stats.sum_time_diff / (stats.count - 1);
//         double absolute_deviation_sum = 0.0;
//         double time_diff = stats.last_timestamp - (stats.sum_time_diff / stats.count);
        
//         // 이전 주기의 절대 편차로 MAD 추정 (단일 값 사용)
//         absolute_deviation_sum += std::abs(time_diff - mean);

//         // 평균 절대 편차 반환
//         return absolute_deviation_sum / (stats.count - 1);
//     }
//     return 0.0;  // 데이터가 부족한 경우 MAD는 0
// }

// double get_coefficient_of_variation(uint32_t can_id) {
//     CANStats& stats = can_stats[can_id];
//     if (stats.count > 1) {
//         // 변동계수 (CV) = 표준편차 / 평균
//         double mean = stats.sum_time_diff / (stats.count - 1);
//         if (mean == 0.0) {
//             return 0.0;  // 평균이 0이면 CV는 0으로 설정
//         }
//         double stddev = get_standard_deviation(can_id);
//         return stddev / mean;
//     }
//     return 0.0;  // 데이터가 부족한 경우 CV는 0
// }