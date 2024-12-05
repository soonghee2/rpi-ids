#ifndef EVENT_DETECTION_H
#define EVENT_DETECTION_H

#include "CANStats.h"
#include "Normal_detection.h"
#include "ui.h"
#include <cstdint>
#include <cstdio>
#include <cstring>

#define EVENT_PERIOD 0.03

bool check_onEvent(double timestamp, CANStats& stats, uint32_t can_id, uint8_t data[]);

#endif // EVENT_DETECTION_H

