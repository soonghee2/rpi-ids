#include "periodic.h"

double get_standard_deviation(uint32_t can_id) {
    CANStats& stats = can_stats[can_id];
    if (stats.count > 1) {
        return std::sqrt(stats.squared_diff_sum / (stats.count -1));
    }
    return 0.0;  // 데이터가 부족한 경우 표준편차는 0
}

double get_coefficient_of_variation(double mean, double stddev) {
    if (mean!=0) {
        return stddev / mean;
    }
    return 0.0;  // 데이터가 부족한 경우 CV는 0
}

void calc_periodic(uint32_t can_id, double timestamp) {

    CANStats& stats = can_stats[can_id];
    stats.count++;

    if (stats.count > 1) {
        double time_diff = timestamp - stats.last_timestamp;

        double prev_periodic = stats.periodic;
        stats.periodic += (time_diff - prev_periodic) / (stats.count -1);

        double diff = time_diff - prev_periodic;
        stats.squared_diff_sum += diff * (time_diff - stats.periodic);

        if (stats.count == PERIODIC_SAMPLE_THRESHOLD){
            double stddev = get_standard_deviation(can_id);
            double cv = get_coefficient_of_variation(stats.periodic, stddev);
            if (stddev < PERIODIC_STD_THRESHOLD && cv < PERIODIC_CV_THRESHOLD) {
                stats.is_periodic = true;

            } else { 
                stats.is_periodic = false;
            }
        } else if (stats.count == SECOND_PERIODIC_SAMPLE_THRESHOLD){
            double stddev = get_standard_deviation(can_id);
            double cv = get_coefficient_of_variation(stats.periodic, stddev);
            if (stddev < PERIODIC_STD_THRESHOLD && cv < PERIODIC_CV_THRESHOLD) {
                stats.is_periodic = true;  

            } else { 
                stats.is_periodic = false;
            }
        }    
    } 
    
    else {
        stats.periodic = 0;
        stats.squared_diff_sum = 0;
        stats.is_periodic = false;  // 데이터가 충분하지 않으므로 비주기적으로 초기화
    }
    
    stats.last_timestamp = timestamp;
}
