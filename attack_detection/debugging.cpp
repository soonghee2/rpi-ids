#include <string>
#include <iostream>

void showRealTimeSimilarityForMultipleIDs(const std::vector<int>& can_ids, const std::vector<string>& difficulty, const std::vector<int>& num, const std::vector<int>& attack) {
    for (int step = 0; step <= steps; ++step) {
        std::cout << "\033[2J\033[H"; // 화면 전체를 지우고 커서를 맨 위로 이동

        for (size_t i = 0; i < can_ids.size(); ++i) {
            std::cout << "[" << std::hex << std::setw(3) << std::setfill('0') << can_ids[i] << "] [" << difficulty[i] << "] " << "[" << num[i] << "] " << ;
            
	    //std::cout << "이전 패킷간의 유사성이 " << std::fixed << std::setprecision(2) << similarities[i] << "% 낮습니다. [";
            }
        }

        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 딜레이 추가
    }

    std::cout << "All processes complete!" << std::endl;
}

int main() {
    // 여러 CAN ID 정의
    std::vector<int> can_ids = {0x123, 0x456, 0x789};
    std::vector<string> difficulty = {"Medium","Hard","Low"};

    std::vector<int> num = {10, 20, 40};
    std::vector<int> attack = {-1, -3, -4};
    // 여러 CAN ID에 대해 실시간 진행 상황 표시
    showRealTimeSimilarityForMultipleIDs(can_ids, difficulty, num, attack);

    return 0;
}
