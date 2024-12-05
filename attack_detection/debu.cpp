#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <thread>
#include <chrono>

// 구조체 정의
struct CanData {
    int can_id;                  // CAN ID
    std::string difficulty;      // 난이도
    int detected_count;          // 탐지된 공격 갯수
    int attack_type;             // 공격 유형
};

// 커서 이동 및 줄 업데이트 함수
void updateLine(int row, const CanData& data) {
    // 커서를 특정 줄(row)로 이동
    std::cout << "\033[" << row << ";1H";

    // 해당 줄의 데이터를 덮어쓰기
    std::cout << "[" << std::hex << std::setw(3) << std::setfill('0') << data.can_id << "] ";
    std::cout << "[" << data.difficulty << "] ";
    std::cout << "[공격 탐지: " << std::dec << data.detected_count << "] ";
    std::cout << "[공격 유형: " << std::dec << data.attack_type << "]     ";
    std::cout.flush();
}

// 모든 데이터를 출력하는 함수 (초기 출력)
void printAllData(const std::vector<CanData>& can_data) {
    for (size_t i = 0; i < can_data.size(); ++i) {
        updateLine(i + 1, can_data[i]); // 각 데이터를 출력 (줄 번호는 1부터 시작)
    }
}

// 특정 ID 데이터 갱신 함수
void modifyData(std::vector<CanData>& can_data, int can_id, int detected_increment, int attack_change, const std::string& new_difficulty) {
    for (size_t i = 0; i < can_data.size(); ++i) {
        if (can_data[i].can_id == can_id) {
            // 데이터 갱신
            can_data[i].detected_count += detected_increment;
            can_data[i].attack_type += attack_change;
            can_data[i].difficulty = new_difficulty;

            // 해당 줄만 업데이트
            updateLine(i + 1, can_data[i]);
            break;
        }
    }
}

int main() {
    // CAN 데이터 초기화
    std::vector<CanData> can_data = {
        {0x123, "High", 10, -1},
        {0x456, "Hard", 14, -4},
        {0x789, "Low", 36, -9}
    };

    // 초기 데이터 출력
    printAllData(can_data);

    // 데이터 갱신 반복
    for (int step = 0; step < 5; ++step) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1초 대기

        if (step % 2 == 0) {
            modifyData(can_data, 0x123, 1, 0, "High"); // ID 0x123 갱신
        } else {
            modifyData(can_data, 0x789, 1, 4, "High"); // ID 0x789 갱신
        }
    }

    std::cout << "\n"; // 마지막 출력 이후 줄바꿈
    return 0;
}

