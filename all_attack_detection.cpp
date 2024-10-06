#include "all_attack_detection.h"

bool check_periodic(){
	return true;
}
bool check_periodic_range(){
	return true;
}
bool check_similarity_with_previous_packet(){
        return true;
}

bool check_clock_error(){
        return true;
}

bool check_previous_packet_of_avg(){
        return true;
}

bool check_low_can_id(){
        return true;
}

bool check_DoS(EnqueuedCANMsg dequeuedMsg){
    CANStats& stats = can_stats[dequeuedMsg.can_id];
    double time_diff = (dequeuedMsg.timestamp - stats.last_timestamp);

    if(time_diff < DoS_TIME_THRESHOLD_MS){
        if(memcmp(stats.last_data, dequeuedMsg.data, sizeof(stats.last_data)) == 0){
            stats.suspected_count++;
        } else{ 
            stats.suspected_count = 1;
        }

        if(stats.suspected_count == DoS_DETECT_THRESHOLD){
            DoS_can_id = dequeuedMsg.can_id;
            memcpy(DoS_payload, dequeuedMsg.data, sizeof(DoS_payload));
            return true;
        }
    }

    if(DoS_can_id == dequeuedMsg.can_id && memcmp(DoS_payload, dequeuedMsg.data, sizeof(DoS_payload)) == 0){
        return true;
    }

    return false;
}

bool check_onEvent(){
        return true;
}

bool check_over_double_periodic(){
        return true;
}


bool filtering_process(EnqueuedCANMsg* dequeuedMsg) {
    bool malicious_packet = true;
    bool normal_packet = false;

    // 비주기 패킷일 경우
    if (!check_periodic()) {
        // 비주기 패킷은 정상 패킷으로 처리
        return normal_packet;
    }

    // 주기 패킷일 경우
    // 1. 정상 주기 범위 내에 있는가?
    if (check_periodic_range()) {
        // 1.1 이전 패킷과 상관관계가 있는가?
        if (check_similarity_with_previous_packet()) {
            // 1.2 시계 오차가 있는가?
            if (check_clock_error()) {
                // 정상 패킷
                return normal_packet;
            } else {
                // Masquerade 공격
                return malicious_packet;
            }
        } else {
            // Fuzzing or Replay 공격
            return malicious_packet;
        }
    }

    // 2. i-1 패킷과의 평균값이 정상 주기 범위 내에 있는가?
    if (check_previous_packet_of_avg()) { //여기서 들어온 패킷이 첫번째 비정상 주기 패킷이면 다음 패킷의 비정상 패킷과 비교해야함. 그리고 비정상 주기에 대한 범위는 정상 주기에 대략 2배(?) 이하여야함
        // 정상 패킷
        return normal_packet;
    }

    // 2.1 최하위 CAN ID인가?
    if (check_low_can_id()) {
        // 오차가 5ms 이내로 동일한 패킷이 5번 이상 들어오는가?
        if (check_DoS(*dequeuedMsg)) {
            // DDoS 공격
            return malicious_packet;
        }
        // On-Event 패킷인가?
        if (check_onEvent()) {
            // 정상 패킷
            return normal_packet;
        }
        if (check_over_double_periodic()) {
            // Suspension 공격
            return malicious_packet;
        }
        // Fuzzing or Replay 공격
        return malicious_packet;
    }

    // 2.2 On-Event 패킷인가?
    if (check_onEvent()) {
        // 정상 패킷
        return normal_packet;
    }

    // 2.3 오차가 정상 주기의 2배 이상인가?
    if (check_over_double_periodic()) {
        // Suspension 공격
        return malicious_packet;
    }

    // Fuzzing or Replay 공격
    return malicious_packet;
}

uint8_t DoS_payload[8];    // 전역 변수 정의
int suspected_count = 0;   // 전역 변수 정의
uint32_t DoS_can_id = 0;   // 전역 변수 정의