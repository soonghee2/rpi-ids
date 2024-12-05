#include "ui.h"

std::mutex cout_mutex;                
std::map<uint32_t, int> id_to_row;
std::map<uint32_t, int>id_to_num;
std::map<std::string, uint32_t> attack_to_id;
std::map<std::string, int> attack_to_num;
int ID_row;

// 특정 CAN ID에 대한 메시지를 출력
void updateReasonMsg(uint32_t can_id, const std::string& type, const std::string& level, const char* format, ...) {
    std::lock_guard<std::mutex> lock(cout_mutex);

    // 새로운 CAN ID라면 줄 번호 할당
    if (id_to_row.find(can_id) == id_to_row.end()) {
        id_to_row[can_id] = current_row++;
    }

    int row = id_to_row[can_id]; // 해당 CAN ID의 줄 번호

    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::cout << "\033[" << row << ";1H"; // 커서를 해당 줄(row)로 이동
    std::cout << "\033[K";                // 현재 줄 지우기
    std::cout << "[CAN ID: " << std::setw(3) << std::setfill(' ') << can_id
              << "] [Type: " << type
              << "] [Severity: " << level
              << "] " << buffer << std::flush;
}

/*// 특정 CAN ID에 대한 메시지를 출력
void updateAttackMsg(uint32_t can_id, const std::string& type, const std::string& level, const std::string& message, const int num) {
    std::lock_guard<std::mutex> lock(cout_mutex);

    if (id_to_row.find(can_id) == id_to_row.end()) {
        id_to_row[can_id] = current_row++;  // 새로운 줄 번호 할당 후 current_row 증가
    } 

    int row = id_to_row[can_id]; // 해당 CAN ID의 줄 번호

    // 커서를 해당 줄로 이동 후 메시지 출력
    std::cout << "\033[" << row << ";1H"; // 커서를 특정 줄(row)로 이동
    std::cout << "\033[K";                // 현재 줄 지우기
    std::cout << "[CAN ID: " << std::setw(3) << std::setfill('0') << can_id
              << "] [Type: " << type
              << "] [Severity: " << level
              << "] " << message << ">> " << num << std::flush;
}*/

// 특정 CAN ID에 대한 메시지를 출력
void updateIDMsg(uint32_t can_id, const std::string& type, const std::string& level, const std::string& message, const int num) {
    std::lock_guard<std::mutex> lock(cout_mutex);

    // id_to_num에 CAN ID에 대한 num 값 업데이트
    id_to_num[can_id] = num;

    // 현재 CAN ID들을 num 값에 따라 정렬
    std::vector<std::pair<uint32_t, int>> sorted_can_ids(id_to_num.begin(), id_to_num.end());
    std::sort(sorted_can_ids.begin(), sorted_can_ids.end(), [](const auto& a, const auto& b) {
        return a.second > b.second; // num 값이 큰 순서대로 정렬
    });
    
    int max_display = 10;
    int row = current_row;
    
    for (size_t i = 0; i < sorted_can_ids.size() && i < max_display; ++i) {
        const auto& [id, num_value] = sorted_can_ids[i];
        id_to_row[id] = row++;

        // ANSI 커서 이동 및 줄 지우기
        std::string ansi_cursor = "\033[" + std::to_string(id_to_row[id]) + ";1H";
        std::cout << ansi_cursor << std::flush; // 커서를 해당 줄로 이동
        std::cout << "\033[K";                  // 해당 줄 지우기

        // 메시지 출력
        std::cout << "[CAN ID: 0x" << std::hex << std::uppercase << std::setw(3) << std::setfill(' ') << id
                  << "] [Type: " << type
                  << "] [Severity: " << level
                  << "] " << message << " >> " << std::dec << num_value << std::flush; // Reset to decimal after num_value
    }

    // 출력되지 않는 ID들에 대한 출력 지우기
    for (size_t i = max_display; i < sorted_can_ids.size(); ++i) {
        const auto& [id, num_value] = sorted_can_ids[i];
        if (id_to_row.find(id) != id_to_row.end()) {
            int row_to_clear = id_to_row[id];
            std::string ansi_cursor = "\033[" + std::to_string(row_to_clear) + ";1H";
            std::cout << ansi_cursor << "\033[K" << std::flush; // 해당 줄 지우기
            id_to_row.erase(id); // 해당 줄 번호 매핑 제거
        }
    }
}

void initializeAttackTypes() {
    std::lock_guard<std::mutex> lock(cout_mutex);
    const int type_width = 35; // `>>`// Type 출력 폭 (고정된 너비)
    static int next_available_row = current_row + 13; // 초기 줄 번호 관리

    // 미리 정의된 공격 유형 리스트
    std::vector<std::string> attack_types = {
        "Replay", "Fuzzing", "Masquerade", "DoS", "Suspension"
    };

    // 각 공격 유형을 미리 출력
    for (const auto& type : attack_types) {
        attack_to_id[type] = next_available_row++; // 고유 Row 번호 할당
        attack_to_num[type] = 0;                  // 초기 Count 값 설정

        // 출력
        int row = attack_to_id[type];
        std::cout << "\033[" << row << ";1H"; // 고유 줄로 커서 이동
        std::cout << std::left << std::setw(type_width) 
                  << ("[Type: " + type + "] >>") // `>>`까지를 고정 폭으로 정렬
                  << "Count: " << std::dec << attack_to_num[type] <<  std::flush;
    }
}

void updateAttackMsg(const std::string& type) {
    std::lock_guard<std::mutex> lock(cout_mutex);

    // 새로운 Type을 허용하지 않고, 미리 정의된 Type만 업데이트
    if (attack_to_id.find(type) == attack_to_id.end()) {
        return; // 미리 정의되지 않은 Type은 무시
    }

    // 메시지 업데이트
    attack_to_num[type]++;

    // 고유 Row 번호 가져오기
    int row = attack_to_id[type];

    const int type_width = 35; // Type 출력 폭 (고정된 너비)

    // ANSI 커서 이동 및 줄 지우기
    std::string ansi_cursor = "\033[" + std::to_string(row) + ";1H";
    std::cout << ansi_cursor << std::flush; // 커서를 고유 줄로 이동
    std::cout << "\033[K";                  // 해당 줄 지우기

    std::cout << std::left << std::setw(type_width) 
                << ("[Type: " + type + "] >>") // `>>`까지를 고정 폭으로 정렬
                << "Count: " << std::dec << attack_to_num[type] <<  std::flush;
}


int getCursorPosition(int& rows, int& cols) {
    // 터미널로부터 커서 위치를 요청하기 위해 '\033[6n'을 보냄
    std::cout << "\033[6n";
    std::cout.flush();

    // 터미널 응답 읽기
    char buffer[32];
    int i = 0;

    // 응답을 읽기 위해 터미널에서 raw 모드로 설정
    struct termios orig_term, raw_term;
    tcgetattr(STDIN_FILENO, &orig_term);
    raw_term = orig_term;
    raw_term.c_lflag &= ~(ICANON | ECHO); // 캐논 모드와 에코를 끄기
    tcsetattr(STDIN_FILENO, TCSANOW, &raw_term);

    // '\033[rows;colsR' 형식의 응답 읽기
    while (i < sizeof(buffer) - 1) {
        if (read(STDIN_FILENO, &buffer[i], 1) != 1) {
            break;
        }
        if (buffer[i] == 'R') {
            break;
        }
        ++i;
    }
    buffer[i] = '\0';

    // 원래의 터미널 설정으로 복구
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);

    // 응답 파싱
    if (buffer[0] != '\033' || buffer[1] != '[') {
        return -1; // 예상치 않은 응답 형식
    }

    if (sscanf(&buffer[2], "%d;%d", &rows, &cols) != 2) {
        return -1; // 실패 시 오류 반환
    }

    return 0; // 성공
}
