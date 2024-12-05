#ifndef DOS_DETECTION_H
#define DOS_DETECTION_H

#include "CANStats.h"
#include "ui.h"
#include <cstdint>
#include <cstring>
#include <cstdio>

#define DoS_TIME_THRESHOLD_MS 0.005
#define Non_PERIODIC_DoS_TIME_THRESHOLD_MS 0.003
#define DoS_DETECT_THRESHOLD 5

bool check_DoS(const EnqueuedCANMsg& dequeuedMsg);

extern double DoS_last_time;

#endif // DOS_DETECTION_H

