#ifndef NORMAL_PACKET_CHECKER_H
#define NORMAL_PACKET_CHECKER_H

#include "CANStats.h"
#include <cstdint>    

bool check_periodic_range(double time_diff, double periodic);
bool check_previous_packet_of_avg(double current_timediff, CANStats& stats);

#endif
