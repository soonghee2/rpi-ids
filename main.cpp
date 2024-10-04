#include "header.h"
#include "periodic.h"
#include "all_attack_detection.h"

#define CAN_MSSG_QUEUE_SIZE 100 //큐에 담을수 있는 데이터 사이즈
#define IMPLEMENTATION FIFO //선입선출로 큐를 초기화할때 사용
Queue_t canMsgQueue; //CAN 데이터를 담을 큐

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

    msg->can_id = frame.can_id;
    
    // DLC와 데이터 저장
    msg->DLC = frame.can_dlc;
    memset(msg->data, 0, sizeof(msg->data));  // 데이터 배열을 초기화
    memcpy(msg->data, frame.data, frame.can_dlc);  // 수신한 데이터 저장

    if(!q_push(&canMsgQueue, msg)){ 
	    printf("Queue is full.\n");
    }
    return 0;
}

// 저장된 CAN 메시지 출력 (디버그용)
void debugging_dequeuedMsg(EnqueuedCANMsg* dequeuedMsg){
        printf("Timestamp: %.6f\n", dequeuedMsg->timestamp);
        printf("CAN ID:%03X\n",dequeuedMsg->can_id);
        printf("DLC: %d\n", dequeuedMsg->DLC);
        printf("Data: ");
        for (int i = 0; i < dequeuedMsg->DLC; i++) {
                printf("%02X ", dequeuedMsg->data[i]);
        }
        printf("\n");
}

int main() {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    EnqueuedCANMsg can_msg;  // 수신된 CAN 메시지를 저장할 구조체

    struct timeval tv;
    gettimeofday(&tv, NULL);  // 현재 시간을 가져옴
    double start_time = tv.tv_sec + (tv.tv_usec / 1000000.0);  // 초와 마이크로초를 합쳐서 double로 변환

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

    q_init(&canMsgQueue, sizeof(EnqueuedCANMsg), CAN_MSSG_QUEUE_SIZE, IMPLEMENTATION, false);
    
    // CAN 패킷을 수신하고 구조체에 저장
    while (1) {
        if (receive_can_frame(s, &can_msg) == 0) {
            // 저장된 CAN 메시지 출력 (디버그용)
            EnqueuedCANMsg dequeuedMsg; //canMsgQueue에서 pop한 뒤 데이터를 저장할 공간

            if(q_pop(&canMsgQueue, &dequeuedMsg)){
                debugging_dequeuedMsg(&dequeuedMsg);
                
		//add feature/dbc-detection-criteria
		/*if(check_can_id(dequeuedMsg.can_id) || check_payload(dequeuedMsg)){
                    // need Dos or Fuzzing check
                    detected_malicious = true;
                }*/

                //else {
                    if(start_time - dequeuedMsg.timestamp <= 10){
                        calc_periodic(dequeuedMsg.can_id, dequeuedMsg.timestamp);
                        printf("Periodic: %.6f\n", can_stats[dequeuedMsg.can_id].periodic);
                    } 
                    else if (filtering_process()){
                        printf("Malicious packet!\n");
                    } 
                    else {
                        printf("Normal packet!\n");
                    }
                //}
            }
        }
    }
    close(s);
    return 0;
}

