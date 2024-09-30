#ifndef CANSTATS_H
#define CANSTATS_H

#include <unordered_map>

struct CANStats {
    double mean = 0;
    double sum_of_squared_diffs = 0;
    int count = 0;
};

extern std::unordered_map<unsigned int, CANStats> can_stats;

#endif
