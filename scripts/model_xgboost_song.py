import numpy as np
from tflite_runtime.interpreter import Interpreter
import time
import os
import sys

# TFLite 모델 로드 및 예측 함수
def load_and_predict_tflite_model(interpreter, input_details, output_details, input_data):
    """
    TFLite 모델을 로드하고 입력 데이터를 사용하여 예측 수행.
    """
   

    # 입력 데이터를 모델 텐서에 설정
    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()

    # 모델 출력 가져오기
    output_data = interpreter.get_tensor(output_details[0]['index'])
    return output_data

def detect_intrusion(model_path, log_filename, output_log_filename, payload_size=8, threshold=0.5):
    """
    공격으로 감지된 로그만 출력 파일에 기록하는 함수.
    """
    interpreter = Interpreter(model_path=model_path)
    interpreter.allocate_tensors()
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

    last_position = 0
    last_modification_time = 0
    intrusion_count = 0

    max_wait_time = 60  # 최대 대기 시간 (초)
    start_time = time.time()

    while not os.path.exists(log_filename):
        if time.time() - start_time > max_wait_time:
            print(f"Log file {log_filename} not found within {max_wait_time} seconds. Exiting.")
            return  # 또는 sys.exit(1)
        time.sleep(1)

    while True:
        try:
            if os.path.exists(log_filename):
                current_modification_time = os.path.getmtime(log_filename)
                print("current_modification_time = os.path.getmtime(log_filename)")
                if current_modification_time > last_modification_time:
                    last_modification_time = current_modification_time
                    print("last_modification_time = current_modification_time")
                    with open(log_filename, 'r') as log_file, open(output_log_filename, 'a') as output_log:
                        log_file.seek(last_position)
                        lines = log_file.readlines()
                        last_position = log_file.tell()
                        # 상태를 저장할 변수 초기화
                        last_timestamps = {}  # 각 CAN ID의 마지막 타임스탬프 저장
                        can_id_count = {}     # 각 CAN ID의 출현 빈도 저장
                        total_message_count = 0  # 전체 메시지 수

                        for line in lines:
                            parts = line.strip().split()
                            if len(parts) < 4 or parts[-1] != '0':
                                continue  # 공격으로 감지되지 않은 줄은 무시
                            try:
                                print(total_message_count)
                                # CAN ID 및 페이로드 처리
                                timestamp = float(parts[0].strip('()'))  # 괄호 제거 후 float으로 변환
                                can_id_hex = parts[2].split('#')[0]
                                payload = parts[2].split('#')[1]

                                can_id = int(can_id_hex, 16)
                                payload_bytes = [int(payload[i:i + 2], 16) for i in range(0, len(payload), 2)]

                                if len(payload_bytes) < payload_size:
                                    payload_bytes += [0] * (payload_size - len(payload_bytes))
                                else:
                                    payload_bytes = payload_bytes[:payload_size]

                                # 추가 피처 생성
                                # 1. time_diff 계산
                                if can_id in last_timestamps:
                                    time_diff = timestamp - last_timestamps[can_id]  # 이전 타임스탬프와의 차이
                                else:
                                    time_diff = 0  # 이전 타임스탬프가 없으면 0으로 초기화
                                last_timestamps[can_id] = timestamp  # 현재 타임스탬프를 저장

                                # 2. can_id_frequency 계산
                                can_id_count[can_id] = can_id_count.get(can_id, 0) + 1
                                # can_id_frequency = can_id_count[can_id]  # 정규화 없이 발생 빈도 그대로 사용
                                total_message_count += 1  # 전체 메시지 수 증가
                                can_id_frequency = can_id_count[can_id] / total_message_count

                                # 모델 입력 데이터 구성
                                input_data = np.array([[can_id, time_diff, can_id_frequency] + payload_bytes], dtype=np.float32)
                                
                                # Input0: Shape [ 1 11], Data Type <class 'numpy.float32'> 1번에 1개 처리, feature는 11개
                                
                                # 예측 수행
                                predicted_prob = load_and_predict_tflite_model(interpreter, input_details, output_details,input_data)

                                # 임계값으로 침입 여부 판단
                                is_intrusion = predicted_prob[0] > threshold

                                if is_intrusion:
                                    print(f"CAN ID: {can_id_hex}, Intrusion Detected (Probability: {predicted_prob:.2f})")
                                    intrusion_count += 1
                                    parts[-1] = '1'
                                    output_log.write(" ".join(parts) + "\n")  # 공격으로 감지된 경우만 출력
                                    exit(0)
                            except Exception as e:
                                print(f"Error processing line: {line}. Error: {e}")

                time.sleep(1)
            else:
                time.sleep(1)

        except KeyboardInterrupt:
            print(f"\nDetection interrupted. Total intrusions detected: {intrusion_count}")
            break


# 주요 코드 실행
if __name__ == "__main__":
    if len(sys.argv) < 3:
        #print("Usage: python detect_intrusion.py <log_file_path> <output_log_file_path>")
        sys.exit(1)

    tflite_model_path = "xgboost_model.tflite"  # TFLite로 변환된 XGBoost 모델 경로
    log_file_path = sys.argv[1]  # ./ids에서 생성한 로그 파일 경로
    output_log_file_path = sys.argv[2]  # 출력 로그 파일 경로

    # 침입 탐지 실행
    detect_intrusion(tflite_model_path, log_file_path, output_log_file_path, threshold=0.5)

