#include "AttackFilter.h"

int normal_packet = 0;
int dbc_dos_packet = 1;
int time_dos_packet = 2;
int payload_dos_packet = 3;
int dbc_fuzzing_packet = 4;
int similarity_fuzzing_packet = 5;
int replay_packet = 6;
int uds_suspension_packet = 7;
int time_suspension_packet = 8;
int masquerade_packet = 9;

uint32_t last_can_id = 0;
uint8_t last_payload[8] = {0};

int filtering_process(EnqueuedCANMsg* dequeuedMsg) {

    CANStats& stats = can_stats[dequeuedMsg->can_id];

    // Malicious UDS 체크
    if (isMaliciousUDS(stats, dequeuedMsg->data, dequeuedMsg->can_id)) {
        stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "UDS", "High", "Malicious UDS packet detected.",stats.mal_count);
        updateAttackMsg("Suspension");
        return uds_suspension_packet;
    }

    //DBC 검증 체크
    #ifdef SET_DBC_CHECK
    if(!validation_check(dequeuedMsg->can_id,dequeuedMsg->data,dequeuedMsg->DLC)){
        //printf("Fuzzing or Dos : Not match with DBC %03x\n", dequeuedMsg->can_id);
        stats.mal_count++;
        if(dequeuedMsg->timestamp - stats.last_timestamp <= DoS_DETECT_THRESHOLD || (dequeuedMsg->can_id == last_can_id && memcmp(dequeuedMsg->data, last_payload, sizeof(dequeuedMsg->data)))){
            updateIDMsg(dequeuedMsg->can_id, "DoS", "High", "DoS attack detected.", stats.mal_count);
            return dbc_dos_packet;
        } else {
            updateIDMsg(dequeuedMsg->can_id, "Fuzzing", "Medium", "Payload not matching DBC.", stats.mal_count);
            return dbc_fuzzing_packet;
        }

        last_can_id = dequeuedMsg->can_id;
        memcpy(dequeuedMsg->data, last_payload, sizeof(dequeuedMsg->data));
        updateIDMsg(dequeuedMsg->can_id, "DBC", "Medium", "Payload not matching DBC.", stats.mal_count);
        return dbc_dos_packet;
    }

    if(stats.count > 200){
        //printf("percent : %d\n", stats.similarity_percent);
        if(stats.is_periodic){
        //if (!check_similarity_with_previous_packet(dequeuedMsg->can_id, dequeuedMsg->data, dequeuedMsg->DLC, stats.valid_last_data, 100 - ((100 - stats.similarity_percent) * 5), stats.count)) {
            if(!check_similarity_with_previous_packet(dequeuedMsg->can_id, dequeuedMsg->data, dequeuedMsg->DLC, stats.valid_last_data,stats.similarity_percent-13, stats.count)){
                stats.mal_count++;
                updateIDMsg(dequeuedMsg->can_id, "Replay", "Medium", "Fuzzing or Replay attack detected.", stats.mal_count);
                printf("periodic %f\n", stats.similarity_percent);
                updateAttackMsg("Replay");
                return similarity_fuzzing_packet;
            }
        } else{
            //if (!check_similarity_with_previous_packet(dequeuedMsg->can_id, dequeuedMsg->data, dequeuedMsg->DLC, stats.valid_last_data, 100 - ((100 - stats.similarity_percent) * 10), stats.count)) {
            if(!check_similarity_with_previous_packet(dequeuedMsg->can_id, dequeuedMsg->data, dequeuedMsg->DLC, stats.valid_last_data,stats.similarity_percent-10, stats.count)){
                printf("non periodic %f\n", stats.similarity_percent);
                return similarity_fuzzing_packet;
            }
        }
    }
    #endif

    // 주기 패킷일 경우
    double time_diff = dequeuedMsg->timestamp - stats.last_timestamp;

    // 비주기 패킷일 경우
    if (!stats.is_periodic || stats.count<=1) {
        if(stats.fast_periodic == 0 && check_DoS(*dequeuedMsg)){
            // printf("time_diff: %.6lf fast_periodic: %.6lf\n", time_diff, stats.fast_periodic);
            stats.normal_count = 0;
            stats.mal_count++;
            updateIDMsg(dequeuedMsg->can_id, "DoS", "High", "DoS attack detected.", stats.mal_count);
            updateAttackMsg("DoS");
            return (time_diff <= DoS_DETECT_THRESHOLD) ? time_dos_packet : payload_dos_packet;
        }
        //std::copy(std::begin(dequeuedMsg->data), std::end(dequeuedMsg->data), std::begin(stats.valid_last_data));
        for (size_t i = 0; i < sizeof(stats.valid_last_data) / sizeof(stats.valid_last_data[0]); ++i) {
            stats.valid_last_data[i] = dequeuedMsg->data[i];
        }

        stats.normal_count++;
        if(stats.normal_count >= 5){
            stats.dos_count = 0;
        }

        return normal_packet;
    }
    
    if (check_periodic_range(time_diff, stats.periodic) || check_previous_packet_of_avg(time_diff, stats)) {
        if (!check_clock_error(dequeuedMsg->can_id, dequeuedMsg->timestamp, stats)) {
            //memcpy(stats.valid_last_data, dequeuedMsg->data, sizeof(dequeuedMsg->data));
            stats.last_normal_timestamp = dequeuedMsg->timestamp;
            stats.normal_count++;
            if(stats.normal_count >= 5){
                stats.dos_count = 0;
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
            stats.mal_count++;
            updateIDMsg(dequeuedMsg->can_id, "Clock Skew", "High", "Clock skew detected.", stats.mal_count);
            updateAttackMsg("Masquerade");
            return masquerade_packet;
        }
    }

    stats.normal_count = 0;

    // 최하위 CAN ID인가?
    if((is_Attack == 0 || is_Attack == 1) && stats.fast_periodic == 0 && check_DoS(*dequeuedMsg)) {
        //printf("%03x Dos Attack\n", dequeuedMsg->can_id);
        stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "DoS", "High", "DoS attack detected.", stats.mal_count);
        updateAttackMsg("DoS");
        stats.replay_count = 0;
        return (time_diff <= DoS_DETECT_THRESHOLD) ? time_dos_packet : payload_dos_packet;
    }

    // Suspension 체크
    if (check_over_double_periodic(dequeuedMsg->timestamp, stats, dequeuedMsg->can_id)) {
        //printf("%03x Suspenstion\n", dequeuedMsg->can_id);
        stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "Suspension", "High", "Suspension attack detected.", stats.mal_count);
        updateAttackMsg("Suspension");
        return time_suspension_packet;
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

    // Replay 체크
    if (stats.prev_timediff == 0 &&
        !check_periodic_range(dequeuedMsg->timestamp - stats.last_normal_timestamp, stats.periodic)) {
        if ((is_Attack == 0 || is_Attack == 2) && check_replay(stats, dequeuedMsg->data, dequeuedMsg->can_id)) {
            stats.mal_count++;
            updateIDMsg(dequeuedMsg->can_id, "Replay", "Medium", "Replay attack detected.", stats.mal_count);
            updateAttackMsg("Replay");
            stats.dos_count = 0;
            return replay_packet;
        }
    }

    stats.last_normal_timestamp = dequeuedMsg->timestamp;
    //std::copy(std::begin(dequeuedMsg->data), std::end(dequeuedMsg->data), std::begin(stats.valid_last_data));
    for (size_t i = 0; i < sizeof(stats.valid_last_data) / sizeof(stats.valid_last_data[0]); ++i) {
        stats.valid_last_data[i] = dequeuedMsg->data[i];
    }
    return normal_packet;
}

