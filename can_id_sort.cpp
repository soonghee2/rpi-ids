#include "CANStats.h"
#include "can_id_sort.h"
size_t low_canIDs_index = 0; 
uint32_t lowest_id = 0;    
std::set<uint32_t> sorted_canIDSet;  

void can_id_sort(std::unordered_set <uint32_t>& canIDSet){
	sorted_canIDSet = std::set<uint32_t>(canIDSet.begin(), canIDSet.end());
	low_canIDs_index = static_cast<size_t>(sorted_canIDSet.size() * 0.1);
}

void lowest_can_id(std::unordered_set <uint32_t>& canIDSet){
	sorted_canIDSet = std::set<uint32_t>(canIDSet.begin(), canIDSet.end());
	if(!sorted_canIDSet.empty()){
		lowest_id = *sorted_canIDSet.begin();
	} else {
		std::cout << "Error: canIDSet is empty!"<< std::endl;
	}
}
