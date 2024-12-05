#include "AttackFilter.h"
#include "header.h"
#include <map>
#include <iostream>
#include <iomanip> // std::setw, std::setfill 사용
#include <string>
#include <cstring> // memset, memcpy 사용
#include <mutex>   // 멀티스레드 보호를 위한 mutex

int normal_packet = 0;
int dos_packet = 1;
int fuzzing_packet = 2;
int replay_packet = 3;
int suspension_packet = 4;
int masquerade_packet = 5;

uint32_t last_can_id = 0;
uint8_t last_payload[8] = {0};

int filtering_process(EnqueuedCANMsg* dequeuedMsg) {

    CANStats& stats = can_stats[dequeuedMsg->can_id];

    // Malicious UDS 체크
    if (isMaliciousUDS(stats, dequeuedMsg->data, dequeuedMsg->can_id)) {
        stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "UDS", "High", "Malicious UDS packet detected.",stats.mal_count);
        updateAttackMsg("Suspension");
        return suspension_packet;
    }

    //DBC 검증 체크
    #ifdef SET_DBC_CHECK
    if(!validation_check(dequeuedMsg->can_id,dequeuedMsg->data,dequeuedMsg->DLC)){
        stats.mal_count++;
        if(is_Attack == 1 && dequeuedMsg->can_id == last_can_id && memcmp(dequeuedMsg->data, last_payload, sizeof(dequeuedMsg->data))){
            updateIDMsg(dequeuedMsg->can_id, "DoS", "High", "DoS attack detected.", stats.mal_count);
            updateAttackMsg("DoS");
	    return dos_packet;
        } else {
            updateIDMsg(dequeuedMsg->can_id, "Fuzzing", "Medium", "Payload not matching DBC.", stats.mal_count);
            updateAttackMsg("Fuzzing");
	    return fuzzing_packet;
        }

        last_can_id = dequeuedMsg->can_id;
        memcpy(dequeuedMsg->data, last_payload, sizeof(dequeuedMsg->data));
        is_Attack = 1;
        updateIDMsg(dequeuedMsg->can_id, "DBC", "Medium", "Payload not matching DBC.", stats.mal_count);
        updateAttackMsg("DBC");
	return dos_packet;
    }

    if(stats.count > 200){
        if(stats.is_periodic){
            if(!check_similarity_with_previous_packet(dequeuedMsg->can_id, dequeuedMsg->data, dequeuedMsg->DLC, stats.valid_last_data,stats.similarity_percent-13, stats.count)){
                stats.mal_count++;
                updateIDMsg(dequeuedMsg->can_id, "Replay", "Medium", "Fuzzing or Replay attack detected.", stats.mal_count);
                printf("periodic %f\n", stats.similarity_percent);
                updateAttackMsg("Replay");
                return fuzzing_packet;
            }
        } else{
            if(!check_similarity_with_previous_packet(dequeuedMsg->can_id, dequeuedMsg->data, dequeuedMsg->DLC, stats.valid_last_data,stats.similarity_percent-10, stats.count)){
                printf("non periodic %f\n", stats.similarity_percent);
                return fuzzing_packet;
            }
        }
    }
#endif
    // 비주기 패킷일 경우
    if (!stats.is_periodic || stats.count<=1) {
        if((dequeuedMsg->can_id == 0x000 && check_DoS(*dequeuedMsg, false)) || (check_DoS(*dequeuedMsg, true))){
            //printf("%03x DoS Attack\n", dequeuedMsg->can_id);
            stats.mal_count++;
            updateIDMsg(dequeuedMsg->can_id, "DoS", "High", "DoS attack detected.", stats.mal_count);
            updateAttackMsg("DoS");
            return dos_packet;
        } else {

        }
        for (size_t i = 0; i < sizeof(stats.valid_last_data) / sizeof(stats.valid_last_data[0]); ++i) {
            stats.valid_last_data[i] = dequeuedMsg->data[i];
        }
        return normal_packet;
    }

    // 주기 패킷 처리
    double time_diff = dequeuedMsg->timestamp - stats.last_timestamp;
    if (check_periodic_range(time_diff, stats.periodic) || check_previous_packet_of_avg(time_diff, stats)) {
        if (!check_clock_error(dequeuedMsg->can_id, dequeuedMsg->timestamp, stats)) {
            stats.last_normal_timestamp = dequeuedMsg->timestamp;
            stats.normal_count++;
            if(stats.normal_count >= 5){
                stats.replay_count = 0;
                is_Attack = 0;
            }
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
    if((is_Attack == 0 || is_Attack == 1) && check_DoS(*dequeuedMsg, false)) {
        stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "DoS", "High", "DoS attack detected.", stats.mal_count);
        updateAttackMsg("DoS");
        stats.replay_count = 0;
        return dos_packet;
    }

    // Suspension 체크
    if (check_over_double_periodic(dequeuedMsg->timestamp, stats, dequeuedMsg->can_id)) {
        stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "Suspension", "High", "Suspension attack detected.", stats.mal_count);
        updateAttackMsg("Suspension");
        return suspension_packet;
    }

    // On-Event 패킷인가?
    if (check_onEvent(dequeuedMsg->timestamp, stats,dequeuedMsg->can_id, dequeuedMsg->data)) {
        // printf("Event ID: %03x\n",dequeuedMsg->can_id);
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
    for (size_t i = 0; i < sizeof(stats.valid_last_data) / sizeof(stats.valid_last_data[0]); ++i) {
        stats.valid_last_data[i] = dequeuedMsg->data[i];
    }
    return normal_packet;
}

