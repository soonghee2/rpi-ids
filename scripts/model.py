import numpy as np
from tflite_runtime.interpreter import Interpreter
import time
import os
import sys

# TFLite 모델 로드 및 예측 함수
def load_and_predict_tflite_model(model_path, input_data):
    interpreter = Interpreter(model_path=model_path)
    interpreter.allocate_tensors()
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()

    output_data = interpreter.get_tensor(output_details[0]['index'])
    return output_data

# 침입 탐지 함수
def detect_intrusion(model_path, log_filename, output_log_filename, payload_size=8):
    interpreter = Interpreter(model_path=model_path)
    interpreter.allocate_tensors()
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

    last_position = 0  # 파일에서 마지막으로 읽은 위치 저장
    last_modification_time = 0
    intrusion_count = 0  # 침입 횟수 카운터 초기화

    while not os.path.exists(log_filename):  # 로그 파일 생성 대기
        time.sleep(1)


    while True:
        try:
            if os.path.exists(log_filename):
                current_modification_time = os.path.getmtime(log_filename)

                # 파일이 업데이트된 경우에만 진행
                if current_modification_time > last_modification_time:
                    last_modification_time = current_modification_time

                    with open(log_filename, 'r') as log_file, open(output_log_filename, 'a') as output_log:
                        # 마지막으로 읽은 위치로 이동
                        log_file.seek(last_position)
                        lines = log_file.readlines()  # 새로 추가된 줄만 읽기
                        last_position = log_file.tell()  # 현재 파일 끝 위치 저장

                        for line in lines:
                            parts = line.strip().split()
                            # 마지막 항목이 '0'일 경우만 탐지 수행
                            if parts[-1] == '0':
                                can_id_hex = parts[2].split('#')[0]
                                payload = parts[2].split('#')[1]

                                # CAN ID 및 페이로드 처리
                                can_id = int(can_id_hex, 16)
                                payload_bytes = [int(payload[i:i + 2], 16) for i in range(0, len(payload), 2)]

                                # 페이로드 크기 맞추기
                                if len(payload_bytes) < payload_size:
                                    payload_bytes += [0] * (payload_size - len(payload_bytes))
                                else:
                                    payload_bytes = payload_bytes[:payload_size]

                                # 모델 예측을 위한 입력 데이터 구성
                                input_data = np.array([[can_id] + payload_bytes], dtype=np.float32)
                                interpreter.set_tensor(input_details[0]['index'], input_data)
                                interpreter.invoke()

                                # 예측 결과와 임계값 비교
                                reconstructed = interpreter.get_tensor(output_details[0]['index'])
                                error = np.mean((input_data - reconstructed) ** 2)
                                threshold = 2011.54638671875  # 임계값 설정
                                is_intrusion = error > threshold

                                if is_intrusion:
                                    print(f"CAN ID: {can_id_hex}, Intrusion Detected")
                                    intrusion_count += 1  # 침입 감지 시 카운트 증가
                                    parts[-1] = str(1)
                                    output_log.write(" ".join(parts) + "\n")
                                else:
                                    output_log.write(line)

                # 속도 조절을 위해 잠시 대기
                time.sleep(1)
            else:
                time.sleep(1)

        except KeyboardInterrupt:
            break

    # 최종 침입 횟수 출력

# 주요 코드 실행
if __name__ == "__main__":
    tflite_model_path = "autoencoder.tflite"  # TFLite 모델 경로
    log_file_path = sys.argv[1]  # ./ids에서 생성한 로그 파일 경로
    output_log_file_path = sys.argv[2]  # 출력 로그 파일 경로
    detect_intrusion(tflite_model_path, log_file_path, output_log_file_path)

