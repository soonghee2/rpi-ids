#include "CANStats.h"

// set_value.h가 포함된 경우 SET_DBC_CHECK가 정의됨
#ifdef SET_DBC_CHECK
bool dbc_check = true;  // set_value.h가 포함된 경우 초기값을 true로 설정
#else
bool dbc_check = false; // 기본값은 false로 설정
#endif

// Define the unordered_map
std::unordered_map<uint32_t, CANStats> can_stats;
std::set<uint32_t> sorted_canIDs;

uint32_t MIN_CAN_ID = 0;
