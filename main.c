#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/ioctl.h>  // ioctl 함수 사용을 위한 헤더
#include <net/if.h>     // ifreq 구조체와 네트워크 인터페이스 관련 상수를 위한 헤더

// 구조체 정의
typedef struct qCANMsg {
    double timestamp;      // 타임스탬프 (초 단위)
    uint8_t can_id[3];     // CAN ID (3바이트 크기 배열)
    int DLC;               // 데이터 길이 코드 (Data Length Code)
    uint8_t data[8];       // CAN 데이터 (최대 8바이트)
} EnqueuedCANMsg;

// 현재 타임스탬프를 초와 마이크로초 단위로 구하는 함수
double get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);  // 현재 시간을 가져옴
    return tv.tv_sec + (tv.tv_usec / 1000000.0);  // 초와 마이크로초를 합쳐서 double로 변환
}

// CAN 패킷을 수신하고 qCANMsg 구조체에 저장하는 함수
int receive_can_frame(int s, EnqueuedCANMsg *msg) {
    struct can_frame frame;

    // 패킷 수신
    ssize_t nbytes = read(s, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        perror("CAN read error");
        return -1;
    }

    // 타임스탬프 저장
    msg->timestamp = get_timestamp();  // 타임스탬프를 구조체에 저장

    // CAN ID를 3바이트로 변환 (CAN ID는 최대 11비트이므로 3바이트 크기로 사용 가능)
    msg->can_id[0] = (frame.can_id >> 8) & 0xFF;  // 상위 8비트
    msg->can_id[1] = frame.can_id & 0xFF;         // 하위 8비트
    msg->can_id[2] = 0;                           // 남은 1바이트는 0으로 채움

    // DLC와 데이터 저장
    msg->DLC = frame.can_dlc;
    memset(msg->data, 0, sizeof(msg->data));  // 데이터 배열을 초기화
    memcpy(msg->data, frame.data, frame.can_dlc);  // 수신한 데이터 저장

    return 0;
}

int main() {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    EnqueuedCANMsg can_msg;  // 수신된 CAN 메시지를 저장할 구조체

    // 소켓 생성
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    // 인터페이스 이름을 설정 (vcan0 사용)
    strcpy(ifr.ifr_name, "vcan0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("IOCTL error");
        return 1;
    }

    // 소켓 바인딩
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind error");
        return 1;
    }

    // CAN 패킷을 수신하고 구조체에 저장
    while (1) {
        if (receive_can_frame(s, &can_msg) == 0) {
            // 저장된 CAN 메시지 출력 (디버그용)
            printf("Timestamp: %.6f\n", can_msg.timestamp);
            printf("CAN ID: %02X %02X %02X\n", can_msg.can_id[0], can_msg.can_id[1], can_msg.can_id[2]);
            printf("DLC: %d\n", can_msg.DLC);
            printf("Data: ");
            for (int i = 0; i < can_msg.DLC; i++) {
                printf("%02X ", can_msg.data[i]);
            }
            printf("\n");
        }
    }

    close(s);
    return 0;
}

