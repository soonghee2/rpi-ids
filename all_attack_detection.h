#ifndef ALL_ATTACK_DETECTION_H
#define ALL_ATTACK_DETECTION_H

#include <cstdint>
#include <cstddef>

#include "CANStats.h" 

bool check_periodic();
bool check_periodic_range(double time_diff, double periodic);
bool check_similarity_with_previous_packet();
bool check_clock_error();
bool check_previous_packet_of_avg(double current_timediff, double prev_timediff, double periodic);
bool check_low_can_id();
bool check_ddos();
bool check_onEvent();
bool check_over_double_periodic();

bool filtering_process(EnqueuedCANMsg* dequeuedMsg);

#endif // ALL_ATTACK_DETECTION_H