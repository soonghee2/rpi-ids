#ifndef PERIODIC_H
#define PERIODIC_H

#include "CANStats.h"
// #include "check_clock_error.h"

bool check_periodic();
bool check_periodic_range();
bool check_similarity_with_previous_packet();
// bool check_clock_error();
bool check_previous_packet_of_avg();
bool check_low_can_id();
bool check_ddos();
bool check_onEvent();
bool check_over_double_periodic();

bool filtering_process(const EnqueuedCANMsg* msg);

#endif // PERIODIC_H