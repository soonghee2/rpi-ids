#ifndef ATTACK_FILTER_H
#define ATTACK_FILTER_H


#include <cstring>

#include "AttackDetection.h"
#include "CANStats.h"
#include "ui.h"

// 필터링 프로세스 함수 선언
bool filtering_process(EnqueuedCANMsg* dequeuedMsg);
void updateMessage(uint32_t can_id, const std::string& type, const std::string& level, const std::string& message);

#endif // ATTACK_FILTER_H

