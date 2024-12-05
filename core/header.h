#ifndef HEADER_H
#define HEADER_H

#include <iostream>

#include <thread>
#include <mutex>
#include <utility>
#include <condition_variable>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/if.h> 

#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cmath> 
#include <unistd.h>
#include <errno.h>

#include <vector>
#include <queue>
#include <map>

#include "ui.h"
#include "periodic.h"
#include "AttackFilter.h"

#define CAN_MSSG_QUEUE_SIZE 100 //큐에 담을수 있는 데이터 사이즈
#define IMPLEMENTATION FIFO //선입선출로 큐를 초기화할때 사용
#define INTERFACE_NAME "vcan0"

#endif // HEADER_H
