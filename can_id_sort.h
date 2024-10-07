#include <unordered_set>  
#include <set>             
#include <cstdint>                  
#include <cstddef>
#include <iostream>
#include <ostream>
#include "CANStats.h"     

void can_id_sort(std::unordered_set<uint32_t>& canIDSet);
void lowest_can_id(std::unordered_set <uint32_t>& canIDSet);

extern size_t low_canIDs_index;
extern uint32_t lowest_id;
extern std::set<uint32_t> sorted_canIDSet;
