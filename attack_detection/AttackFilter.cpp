#include "AttackFilter.h"
#include "header.h"
#include <map>
#include <iostream>
#include <iomanip> // std::setw, std::setfill 사용
#include <string>
#include <cstring> // memset, memcpy 사용
#include <mutex>   // 멀티스레드 보호를 위한 mutex

// 반복적인 정상 패킷 처리
bool handleNormalPacket(CANStats& stats, EnqueuedCANMsg* dequeuedMsg) {
    memcpy(stats.valid_last_data, dequeuedMsg->data, sizeof(dequeuedMsg->data));
    stats.last_normal_timestamp = dequeuedMsg->timestamp;
    stats.normal_count++;
    if (stats.normal_count >= 5) {
        stats.replay_count = 0;
        is_Attack = 0;
    }
    return false; // 정상 패킷 반환
}

// 탐지 필터링 프로세스
bool filtering_process(EnqueuedCANMsg* dequeuedMsg) {
    bool malicious_packet = true;
    bool normal_packet = false;

    CANStats& stats = can_stats[dequeuedMsg->can_id];

    // Malicious UDS 체크
    if (isMalicousUDS(stats, dequeuedMsg->data, dequeuedMsg->can_id)) {
	stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "UDS", "High", "Malicious UDS packet detected.",stats.mal_count);
        updateAttackMsg("Suspension");
        return malicious_packet;
    }

#ifdef SET_DBC_CHECK
    // DBC 체크
    if (!validation_check(dequeuedMsg->can_id, dequeuedMsg->data, dequeuedMsg->DLC)) {
	stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "DBC", "Medium", "Payload not matching DBC.", stats.mal_count);
        updateAttackMsg("Fuzzing");
        return malicious_packet;
    }

    if (stats.count > 199) {
        if (!check_similarity_with_previous_packet(dequeuedMsg->can_id, dequeuedMsg->data, dequeuedMsg->DLC,
                                                   stats.valid_last_data, stats.similarity_percent - 5, stats.count)) {
            stats.mal_count++;
	    updateIDMsg(dequeuedMsg->can_id, "Replay", "Medium", "Fuzzing or Replay attack detected.", stats.mal_count);
        updateAttackMsg("Replay");
            return malicious_packet;
        }
    }
#endif

    // 비주기 패킷 처리

    if (!stats.is_periodic || stats.count <= 1) {
        if (check_DoS(*dequeuedMsg)) {
	    stats.mal_count++;
            updateIDMsg(dequeuedMsg->can_id, "DoS", "High", "DoS attack detected.", stats.mal_count);
            updateAttackMsg("DoS");
            return malicious_packet;
        }
        return handleNormalPacket(stats, dequeuedMsg);
    }

    // 주기 패킷 처리
    double time_diff = dequeuedMsg->timestamp - stats.last_timestamp;
    if (check_periodic_range(time_diff, stats.periodic) || check_previous_packet_of_avg(time_diff, stats)) {
        if (!check_clock_error(dequeuedMsg->can_id, dequeuedMsg->timestamp)) {
            return handleNormalPacket(stats, dequeuedMsg);
        } else {
	    stats.mal_count++;
            updateIDMsg(dequeuedMsg->can_id, "Clock Skew", "High", "Clock skew detected.", stats.mal_count);
            updateAttackMsg("Masquerade");
            return malicious_packet;
        }
    }

    stats.normal_count = 0;

    // DoS 체크
    if ((is_Attack == 0 || is_Attack == 1) && check_DoS(*dequeuedMsg)) {
	stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "DoS", "High", "DoS attack detected.", stats.mal_count);
        updateAttackMsg("DoS");
        stats.replay_count = 0;
        return malicious_packet;
    }

    // Suspension 체크
    if (check_over_double_periodic(dequeuedMsg->timestamp, stats, dequeuedMsg->can_id)) {
	stats.mal_count++;
        updateIDMsg(dequeuedMsg->can_id, "Suspension", "High", "Suspension attack detected.", stats.mal_count);
        updateAttackMsg("Suspension");
        return malicious_packet;
    }

    // On-Event 체크
    if (check_onEvent(dequeuedMsg->timestamp, stats, dequeuedMsg->can_id, dequeuedMsg->data)) {
        //stats.mal_count++;
	    //updateIDMsg(dequeuedMsg->can_id, "On-Event", "Low", "On-Event packet detected.", stats.mal_count);
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
            return malicious_packet;
        }
    }

    stats.last_normal_timestamp = dequeuedMsg->timestamp;
    return normal_packet;
}

