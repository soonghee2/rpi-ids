#ifndef REPLAY_DETECTION_H
#define REPLAY_DETECTION_H

#include "CANStats.h"
#include "DoS_detection.h"
#include "ui.h"
#include <cstdint>
#include <cstdio>
#include <cstring>

bool check_replay(CANStats& stats, uint8_t data[], uint32_t can_id);
bool check_over_double_periodic(double timestamp, CANStats& stats, uint32_t can_id);

#endif // REPLAY_DETECTION_H

