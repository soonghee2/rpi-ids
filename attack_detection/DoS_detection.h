#ifndef DOS_DETECTION_H
#define DOS_DETECTION_H

#include "CANStats.h"
#include <cstdint>

#define DoS_TIME_THRESHOLD_MS 0.005
#define DoS_DETECT_THRESHOLD 5

bool check_DoS(const EnqueuedCANMsg& dequeuedMsg);

extern uint8_t DoS_payload[8];
extern uint32_t DoS_can_id;

#endif // DOS_DETECTION_H

