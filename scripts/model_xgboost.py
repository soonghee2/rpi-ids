import numpy as np
from tflite_runtime.interpreter import Interpreter
import time
import os
import sys

# TFLite 모델 로드 및 예측 함수
def load_and_predict_tflite_model(interpreter, input_data):
    """
    TFLite 모델을 로드하고 입력 데이터를 사용하여 예측 수행.
    """
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

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

    while not os.path.exists(log_filename):
        time.sleep(1)

    while True:
        try:
            if os.path.exists(log_filename):
                current_modification_time = os.path.getmtime(log_filename)

                if current_modification_time > last_modification_time:
                    last_modification_time = current_modification_time

                    with open(log_filename, 'r') as log_file, open(output_log_filename, 'a') as output_log:
                        log_file.seek(last_position)
                        lines = log_file.readlines()
                        last_position = log_file.tell()

                        for line in lines:
                            parts = line.strip().split()
                            if len(parts) < 4 or parts[-1] != '0':
                                continue  # 공격으로 감지되지 않은 줄은 무시

                            try:
                                # CAN ID 및 페이로드 처리
                                can_id_hex = parts[2].split('#')[0]
                                payload = parts[2].split('#')[1]

                                can_id = int(can_id_hex, 16)
                                payload_bytes = [int(payload[i:i + 2], 16) for i in range(0, len(payload), 2)]

                                if len(payload_bytes) < payload_size:
                                    payload_bytes += [0] * (payload_size - len(payload_bytes))
                                else:
                                    payload_bytes = payload_bytes[:payload_size]

                                # 추가 피처 생성
                                time_diff = float(parts[0][1:-1])  # 타임스탬프 추출
                                can_id_frequency = 100  # 예시: CAN ID 빈도 값 (실제로는 계산 필요)

                                # 모델 입력 데이터 구성
                                input_data = np.array([[can_id, time_diff, can_id_frequency] + payload_bytes], dtype=np.float32)

                                # 예측 수행
                                predicted_prob = load_and_predict_tflite_model(interpreter, input_data)[0][0]

                                # 임계값으로 침입 여부 판단
                                is_intrusion = predicted_prob > threshold

                                if is_intrusion:
                                    print(f"CAN ID: {can_id_hex}, Intrusion Detected (Probability: {predicted_prob:.2f})")
                                    intrusion_count += 1
                                    parts[-1] = '1'
                                    output_log.write(" ".join(parts) + "\n")  # 공격으로 감지된 경우만 출력
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

