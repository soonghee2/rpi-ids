#ifndef ATTACK_FILTER_H
#define ATTACK_FILTER_H

#include <cstring>
#include <map>
#include <iostream>
#include <iomanip> // std::setw, std::setfill 사용
#include <string>
#include <cstring> // memset, memcpy 사용
#include <mutex>   // 멀티스레드 보호를 위한 mutex

#include "header.h"
#include "AttackDetection.h"
#include "CANStats.h"
#include "ui.h"

// 필터링 프로세스 함수 선언
int filtering_process(EnqueuedCANMsg* dequeuedMsg);
void updateMessage(uint32_t can_id, const std::string& type, const std::string& level, const std::string& message);

extern uint32_t last_can_id;
extern uint8_t last_payload[8];

#endif // ATTACK_FILTER_H

