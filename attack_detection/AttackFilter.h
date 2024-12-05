#ifndef ATTACK_FILTER_H
#define ATTACK_FILTER_H


#include <cstring>

#include "AttackDetection.h"
#include "CANStats.h"
#include "ui.h"

// 필터링 프로세스 함수 선언
int filtering_process(EnqueuedCANMsg* dequeuedMsg);
void updateMessage(uint32_t can_id, const std::string& type, const std::string& level, const std::string& message);

extern int normal_packet;
extern int dos_packet;
extern int fuzzing_packet;
extern int replay_packet;
extern int suspension_packet;
extern int masquerade_packet;

extern uint32_t last_can_id;
extern uint8_t last_payload[8];

#endif // ATTACK_FILTER_H

