#include "ClockSkew_detection.h"

std::unordered_map<uint32_t, ClockSkewDetector> clockSkewDetectors;
// 기본 생성자 구현
ClockSkewDetector::ClockSkewDetector()
    : can_id(0), m_detect_cnt(-1), upperLimit(0), lowerLimit(0), diff_average(0) {
    std::fill(std::begin(cum_clock_skew), std::end(cum_clock_skew), 0.0);
}

// 기본 생성자 구현
ClockSkewDetector::ClockSkewDetector(uint32_t can_id)
    : can_id(can_id),m_detect_cnt(-1), upperLimit(0), lowerLimit(0), diff_average(0) {
    std::fill(std::begin(cum_clock_skew), std::end(cum_clock_skew), 0.0);
}

// 로그 출력 함수
void logClockSkewData(const ClockSkewDetector& detector, uint32_t can_id, double time_diff, double error, const std::string& log_file_path) {
    // 파일 열기
    std::ofstream log_file(log_file_path, std::ios_base::app);
    if (log_file.is_open()) {
        log_file << std::fixed << std::setprecision(6)
                //  << detector.m_detect_cnt << " "
                 << can_id
                 << " " << time_diff
                 << " " << error
                 << " " << detector.prev_average
                 << " " << detector.diff_average;

        log_file << "\n"; // 한 줄 종료
        log_file.close();
    } else {
        std::cerr << "Failed to open log file: " << log_file_path << std::endl;
    }
}

void updateCumClockSkew(ClockSkewDetector& detector, double error, int can_id) {

    if (detector.m_detect_cnt < window) {
        detector.cum_clock_skew[detector.m_detect_cnt] = error;  // 배열에 값 추가
    } else {
        for (int i = 0; i < window - 1; ++i) {
            detector.cum_clock_skew[i] = detector.cum_clock_skew[i + 1];  // 현재 위치에 다음 요소 복사
        }
        detector.cum_clock_skew[window - 1] = error;  // 가장 최근 값 저장
    }
}

// 평균값과 차이 계산, upperLimit 및 lowerLimit 업데이트
void calculateAndUpdateLimits(ClockSkewDetector& detector) {
    if (detector.m_detect_cnt<window-1){return;}

    double sum=0;
    for (int i = 0; i < window; ++i) {
        sum+=detector.cum_clock_skew[i];
    }
    double average = sum / window;
    if (detector.m_detect_cnt == window - 1){
        detector.prev_average = average;
        // printf("avg:%lf\n", average);
        return;
    }

    // double diff_average = average - detector.prev_average;
    detector.prev_average = average;
    // printf("avg:%lf, diff_avg: %lf\n", average, diff_average);

    detector.upperLimit = std::max(detector.upperLimit, detector.prev_average);
    detector.lowerLimit = std::min(detector.lowerLimit, detector.prev_average);
}

// 탐지를 수행하고 결과 반환
bool detectAnomaly(ClockSkewDetector& detector, double error, int can_id) {
    updateCumClockSkew(detector, error, can_id);  // cum_clock_skew 업데이트
    double sum=0;
    for (int i = 0; i < window; ++i) {
        sum+=detector.cum_clock_skew[i];
    }
    double average = sum / window;
    double diff_average = average - detector.prev_average;
    detector.diff_average=diff_average;
    
    // 탐지 조건 확인
    detector.prev_average = average;
    // printf("공격 탐지\n");
    return detector.prev_average < detector.lowerLimit || detector.prev_average > detector.upperLimit;
}

bool check_clock_error(uint32_t can_id, double timestamp, CANStats& stats) {

    double time_diff = timestamp - stats.last_normal_timestamp;
    double error = time_diff - stats.periodic;

    if (time_diff > stats.periodic * (1+accept_percentage) || time_diff < stats.periodic * (1-accept_percentage)) {
        return false;
    }

        // ClockSkewDetector가 없으면 새로 생성
    if (clockSkewDetectors.find(can_id) == clockSkewDetectors.end()) {
        clockSkewDetectors[can_id] = ClockSkewDetector(can_id);
    }
    ClockSkewDetector& detector = clockSkewDetectors[can_id];
    detector.m_detect_cnt++;
    // 배열이 다 차지 않았다면 error 채우기만 수행
    if (detector.m_detect_cnt < MIN_DATA_CNT) {
        updateCumClockSkew(detector, error, can_id);
        calculateAndUpdateLimits(detector);  // 초기화 단계 완료 시 평균값 계산
            // 로그 파일 기록(디버그용)
        // if (detector.m_detect_cnt > window){
        //     logClockSkewData(detector, can_id, time_diff, error, "/home/song/YESICAN/canlogs/temp/masq_test.log");
        // }
        return false;
    }
    if (detector.m_detect_cnt==MIN_DATA_CNT){
        // std::ofstream log_file("/home/song/YESICAN/canlogs/temp/masq_test.log", std::ios_base::app);
        // if (log_file.is_open()) {
        //     log_file << std::fixed << std::setprecision(6)
        //             //  << detector.m_detect_cnt << " "
        //             << "final_Limit: "<<detector.can_id << " " << detector.lowerLimit << " "<< detector.upperLimit<<"\n";
        //     log_file.close();
        // } else {
        //     std::cerr << "Failed to open log file: " << "/home/song/YESICAN/canlogs/temp/masq_test.log" << std::endl;
        // }
    }
    //logClockSkewData(detector, can_id, time_diff, error, "/home/song/YESICAN/canlogs/temp/masq_test.log");
    bool result=detectAnomaly(detector, error, can_id);  
    if (result){
        detector.m_detect_cnt+=1;
        if (detector.m_detect_cnt>MIN_DETECT_LIMIT){
            return true;
        }else{  return false;}
    } else{
        detector.m_detect_cnt=0;
    }
    return false;
}