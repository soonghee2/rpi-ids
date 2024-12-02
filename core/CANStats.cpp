#include "CANStats.h"

// Define the unordered_map
std::unordered_map<uint32_t, CANStats> can_stats;
std::set<uint32_t> sorted_canIDs;

int is_Attack = 0;
