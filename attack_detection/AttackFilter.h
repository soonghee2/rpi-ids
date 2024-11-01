#ifndef ATTACK_FILTER_H
#define ATTACK_FILTER_H

#include "AttackDetection.h"
#include "CANStats.h"
#include <cstring>

// 필터링 프로세스 함수 선언
bool filtering_process(EnqueuedCANMsg* dequeuedMsg);

#endif // ATTACK_FILTER_H

