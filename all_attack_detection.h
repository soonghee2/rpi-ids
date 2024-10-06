#include <cstdint>
#include <cstddef>

#include "can_id_sort.h"

#define EVENT_PERIOD 0.03

bool check_periodic();
bool check_periodic_range();
bool check_similarity_with_previous_packet();
bool check_clock_error();
bool check_previous_packet_of_avg();
bool check_low_can_id(uint32_t can_id);
bool check_ddos();
bool check_onEvent(double timestamp, CANStats& stats,uint32_t can_id);
bool check_over_double_periodic(double timestamp, CANStats& stats, uint32_t can_id);

bool filtering_process(EnqueuedCANMsg* dequeuedMsg);
