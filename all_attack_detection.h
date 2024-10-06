#ifndef ALL_ATTACK_DETECTION_H
#define ALL_ATTACK_DETECTION_H

#include "periodic.h"

#define DoS_TIME_THRESHOLD_MS 0.005
#define DoS_DETECT_THRESHOLD 5

bool check_periodic();
bool check_periodic_range();
bool check_similarity_with_previous_packet();
bool check_clock_error();
bool check_previous_packet_of_avg();
bool check_low_can_id();
bool check_DoS(EnqueuedCANMsg dequeuedMsg);
bool check_onEvent();
bool check_over_double_periodic();

bool filtering_process(EnqueuedCANMsg* dequeuedMsg);

extern uint8_t DoS_payload[8];   // 전역 변수 선언
extern int suspected_count;      // 전역 변수 선언
extern uint32_t DoS_can_id;      // 전역 변수 선언

#endif