#include "all_attack_detection.h"
#include "CANStats.h"

bool check_periodic(){
	return true;
}
bool check_periodic_range(){
	return false;
}
bool check_similarity_with_previous_packet(){
	//no -> stats.event_count=0;
        return false;
}

bool check_clock_error(){
	//stats.event_count=2;
	//check = true;
        return false;
}

bool check_previous_packet_of_avg(){
        return false;
}

bool check_low_can_id(uint32_t can_id){
	//only check lowest can id
	if(can_id <= lowest_id){
		return false;
	} 
	return false;
}

bool check_ddos(){
        return false;
}

bool check_onEvent(double timestamp, CANStats& stats, uint32_t can_id){
	printf("event count : %d\n", stats.event_count);
	if(stats.no_event_last_timestamp==0) {
            stats.no_event_last_timestamp = timestamp; //last_timestamp에 합칠것인가
        }
	double event_time_diff = 0;
        double time_diff = timestamp - stats.no_event_last_timestamp;

        if(stats.event_count==0){
            stats.event_count=1;
            stats.event_last_timestamp = timestamp;
        }
        else {
            event_time_diff = timestamp - stats.event_last_timestamp;
            if(EVENT_PERIOD *1.2 >= event_time_diff && EVENT_PERIOD*0.8 <= event_time_diff){
                stats.event_count++;
                stats.event_last_timestamp = timestamp;
		printf("%f	%f\n",stats.no_event_last_timestamp , stats.event_last_timestamp);

		if(stats.event_count <10 && stats.event_count>1) {
		    return true;
                }
                else {
                    printf("[On-Event] %03x so many packet with short time(30ms)\n",can_id);
		    return false;
                }
            }
            else if((stats.periodic * 1.2 >= time_diff /2 && stats.periodic*0.8<=time_diff/2)||(stats.periodic*1.2 >= event_time_diff && stats.periodic *0.8 <= event_time_diff)){
                stats.event_count = 0;
                stats.no_event_last_timestamp = timestamp;
            }
        }
        return false;
}

bool check_over_double_periodic(double timestamp, CANStats& stats,uint32_t can_id){
	if(timestamp - stats.last_timestamp > stats.periodic * 5) {
		printf("[Suspension Attack Reason] %03x packet with a cycle time of %f minute arrived %f minutes later than the previous one. >> ", can_id, stats.periodic, timestamp - stats.last_timestamp);
		return true;  
	}	
	return false;
}


bool filtering_process(EnqueuedCANMsg* dequeuedMsg) {
    bool malicious_packet = true;
    bool normal_packet = false;

    CANStats& stats = can_stats[dequeuedMsg->can_id];

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
    if (check_previous_packet_of_avg()) { 
	    //여기서 들어온 패킷이 첫번째 비정상 주기 패킷이면 다음 패킷의 비정상 패킷과 비교해야함. 
	    //그리고 비정상 주기에 대한 범위는 정상 주기에 대략 2배(?) 이하여야함
	    //정상 패킷
        return normal_packet;
    }

    // 2.1 최하위 CAN ID인가?
    if (check_low_can_id(dequeuedMsg->can_id)) {
        // 오차가 5ms 이내로 동일한 패킷이 5번 이상 들어오는가?
        if (check_ddos()) {
            // DDoS 공격
            return malicious_packet;
	}
    }

    // 2.2 On-Event 패킷인가?
    if (check_onEvent(dequeuedMsg->timestamp, stats,dequeuedMsg->can_id)) {
        // 정상 패킷
        printf("ID: %03x\n",dequeuedMsg->can_id);
	return normal_packet;
    }

    // 2.3 오차가 정상 주기의 2배 이상인가?
    if (check_over_double_periodic(dequeuedMsg->timestamp, stats, dequeuedMsg->can_id)) {
	// Suspension 공격
        return malicious_packet;
    }


    // Fuzzing or Replay 공격
    return normal_packet;
}
