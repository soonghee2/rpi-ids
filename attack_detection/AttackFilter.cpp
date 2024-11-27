#include "AttackFilter.h"

bool filtering_process(EnqueuedCANMsg* dequeuedMsg) {
    bool malicious_packet = true;
    bool normal_packet = false;

    CANStats& stats = can_stats[dequeuedMsg->can_id];
    
    if(isMalicousUDS(stats,dequeuedMsg->data, dequeuedMsg->can_id)){
	    printf("Here\n");
	    return malicious_packet;
    }

    //DBC 검증 체크 
    #ifdef SET_DBC_CHECK
    if(!validation_check(dequeuedMsg->can_id,dequeuedMsg->data,dequeuedMsg->DLC)){
        //printf("Fuzzing or Dos : Not match with DBC %03x\n", dequeuedMsg->can_id);
        return malicious_packet;
    }
    
    if(stats.count > 200){
    //printf("percent : %d\n", stats.similarity_percent);
    if (!check_similarity_with_previous_packet(dequeuedMsg->can_id, dequeuedMsg->data, dequeuedMsg->DLC, stats.valid_last_data, stats.similarity_percent - 10, stats.count)) {
        printf("%f\n", stats.similarity_percent);
        return malicious_packet;
    }
    }
    #endif

    // 비주기 패킷일 경우
    if (!stats.is_periodic || stats.count<=1) {
        if((check_DoS(*dequeuedMsg))){
            //printf("%03x DoS Attack\n", dequeuedMsg->can_id);
            return malicious_packet;
        }
        //std::copy(std::begin(dequeuedMsg->data), std::end(dequeuedMsg->data), std::begin(stats.valid_last_data));
        for (size_t i = 0; i < sizeof(stats.valid_last_data) / sizeof(stats.valid_last_data[0]); ++i) {
            stats.valid_last_data[i] = dequeuedMsg->data[i];
        }
        return normal_packet;
    }
    // 주기 패킷일 경우
    double time_diff = dequeuedMsg->timestamp - stats.last_timestamp;
    if (check_periodic_range(time_diff, stats.periodic) || check_previous_packet_of_avg(time_diff, stats)) {
        if (!check_clock_error(dequeuedMsg->can_id, dequeuedMsg->timestamp, stats)) {
            //memcpy(stats.valid_last_data, dequeuedMsg->data, sizeof(dequeuedMsg->data));
            stats.last_normal_timestamp = dequeuedMsg->timestamp;
            stats.normal_count++;
            if(stats.normal_count >= 5){
                // memset(stats.replay_payload, 0, sizeof(stats.replay_payload));
                stats.replay_count = 0;
                is_Attack = 0;
            }
            //std::copy(std::begin(dequeuedMsg->data), std::end(dequeuedMsg->data), std::begin(stats.valid_last_data));
            for (size_t i = 0; i < sizeof(stats.valid_last_data) / sizeof(stats.valid_last_data[0]); ++i) {
                stats.valid_last_data[i] = dequeuedMsg->data[i];
            }
            return normal_packet;
        } else {
            //printf("%03x Masquarade attack \n",dequeuedMsg->can_id);
            return malicious_packet;
        }
    }

    stats.normal_count = 0;

    // 최하위 CAN ID인가?
    if((is_Attack == 0 || is_Attack == 1) && check_DoS(*dequeuedMsg)) {
        //printf("%03x Dos Attack\n", dequeuedMsg->can_id);
        stats.replay_count = 0;
        return malicious_packet;
    }

    // 오차가 정상 주기의 2배 이상인가?
    if (check_over_double_periodic(dequeuedMsg->timestamp, stats, dequeuedMsg->can_id)) {
        //printf("%03x Suspenstion\n", dequeuedMsg->can_id);
        return malicious_packet;
    }

    // On-Event 패킷인가?
    if (check_onEvent(dequeuedMsg->timestamp, stats,dequeuedMsg->can_id, dequeuedMsg->data)) {
        //std::copy(std::begin(dequeuedMsg->data), std::end(dequeuedMsg->data), std::begin(stats.valid_last_data));
        printf("Event ID: %03x\n",dequeuedMsg->can_id);
        for (size_t i = 0; i < sizeof(stats.valid_last_data) / sizeof(stats.valid_last_data[0]); ++i) {
            stats.valid_last_data[i] = dequeuedMsg->data[i];
        }
        
        return normal_packet;
    }

    //Replay 공격 체크 
    if (stats.prev_timediff == 0 && !check_periodic_range(dequeuedMsg->timestamp - stats.last_normal_timestamp, stats.periodic)){
        if ((is_Attack == 0 || is_Attack == 2) && check_replay(stats, dequeuedMsg->data, dequeuedMsg->can_id)){
            //printf("%03x Replay\n", dequeuedMsg->can_id);
            stats.dos_count = 0;
            return malicious_packet;
        }
    }

    stats.last_normal_timestamp = dequeuedMsg->timestamp;
    //std::copy(std::begin(dequeuedMsg->data), std::end(dequeuedMsg->data), std::begin(stats.valid_last_data));
    for (size_t i = 0; i < sizeof(stats.valid_last_data) / sizeof(stats.valid_last_data[0]); ++i) {
        stats.valid_last_data[i] = dequeuedMsg->data[i];
    }
    return normal_packet;
}
