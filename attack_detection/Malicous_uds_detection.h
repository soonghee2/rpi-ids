#ifndef MALICOUS_UDS_DETECTION_H 
#define MALICOUS_UDS_DETECTION_H

#include "CANStats.h"
#include "ui.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm> 

bool isValidUDS(uint8_t data[], uint32_t can_id);
bool isMalicousUDS(CANStats& stats, uint8_t data[], uint32_t can_id);

#endif // REPLAY_DETECTION_H

