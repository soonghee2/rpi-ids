#ifndef PERIODIC_H
#define PERIODIC_H

#include "cQueue.h"
#include "CANStats.h"

#include <stdio.h>
#include <math.h> 
#include <cstring>

void calc_periodic(uint32_t can_id, double timestamp);
double get_standard_deviation(uint32_t can_id);

#endif // PERIODIC_H