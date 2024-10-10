#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "header.h"
#include "periodic.h"
#include "all_attack_detection.h"

#define CAN_MSSG_QUEUE_SIZE 100 //큐에 담을수 있는 데이터 사이즈
#define IMPLEMENTATION FIFO //선입선출로 큐를 초기화할때 사용

std::mutex queueMutex;
std::condition_variable queueCondVar;
bool done = false;

Queue_t canMsgQueue; //CAN 데이터를 담을 큐

// 현재 타임스탬프를 초와 마이크로초 단위로 구하는 함수
double get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);  // 현재 시간을 가져옴
    return tv.tv_sec + (tv.tv_usec / 1000000.0);  // 초와 마이크로초를 합쳐서 double로 변환
}

// CAN 패킷을 수신하고 qCANMsg 구조체에 저장하는 함수
int receive_can_frame(int s, EnqueuedCANMsg* msg) {
    while(!done){
	//std::cout <<"queue size:"<<q_getSize(&canMsgQueue)<<std::endl;
	struct can_frame frame;

        size_t nbytes = read(s, &frame, sizeof(struct can_frame));
        if (nbytes < 0) {
            perror("CAN read error");
            return -1;
        }

        msg->timestamp = get_timestamp();  // 타임스탬프를 구조체에 저장
        msg->can_id = frame.can_id;
        msg->DLC = frame.can_dlc;
        memcpy(msg->data, frame.data, frame.can_dlc);  // 수신한 데이터 저장
        msg->timestamp = get_timestamp();  // 타임스탬프를 구조체에 저장
        msg->can_id = frame.can_id;
        msg->DLC = frame.can_dlc;
        memcpy(msg->data, frame.data, frame.can_dlc);  // 수신한 데이터 저장

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if(q_push(&canMsgQueue, msg)){
                queueCondVar.notify_one();
            }else{
                printf("Queue is full.\n");
            }
        }
    }
    return 0;
}

// 큐에서 메시지를 꺼내고 처리하는 함수 
void process_can_msg(double start_time){
    int mal_count = 0;
    FILE *logfile_whole = fopen("../whole_replay.log", "w");
    while(!done){
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondVar.wait(lock, []{return !q_isEmpty(&canMsgQueue)|| done; });
        while ((!q_isEmpty(&canMsgQueue))){
            EnqueuedCANMsg dequeuedMsg;
            q_pop(&canMsgQueue, &dequeuedMsg);
            //std::cout<<"queue pop"<<std::endl;
            lock.unlock();
	    fprintf(logfile_whole, "can0 %03X#", dequeuedMsg.can_id);
	    for (int i = 0; i < dequeuedMsg.DLC; i++) {
                fprintf(logfile_whole, "%02X", dequeuedMsg.data[i]);
            }

            CANStats& stats = can_stats[dequeuedMsg.can_id];
            
	    if(dequeuedMsg.timestamp - start_time <= 40){
                fprintf(logfile_whole, " 0\n");
                calc_periodic(dequeuedMsg.can_id, dequeuedMsg.timestamp);
                //printf("Periodic: %.6f\n", can_stats[dequeuedMsg.can_id].periodic);
            }
            //lowest_can_id(canIDSet);
	    else if (filtering_process(&dequeuedMsg)){
		stats.event_count = -1;
                stats.prev_timediff = 0;
                fprintf(logfile_whole, " 1\n");
                printf("Malicious packet! count: %d\n", mal_count++);
            }
            else {
                //printf("Normal packet!\n");
            }

            stats.prev_timediff = dequeuedMsg.timestamp - stats.last_timestamp;
            stats.last_timestamp = dequeuedMsg.timestamp;
            memcpy(stats.last_data, dequeuedMsg.data, sizeof(stats.last_data));

            lock.lock();
	    fflush(logfile_whole);
        }
    }

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
    
    char filename[256];
    read_dbc("output.json");
    
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
    
    printf("Starting Periodic Calculation 10 seconds\n");
    
    std::thread producerThread(receive_can_frame, s, &can_msg);
    std::thread consumerThread(process_can_msg, start_time);
    
    // Wait for the threads to finish before exiting the program
    producerThread.join();
    consumerThread.join();
    

    close(s);
    return 0;
}

