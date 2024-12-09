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

    last_position = 0  # Track the last read position
    intrusion_count = 0  # Intrusion count tracker

    while not os.path.exists(log_filename):  # Wait for the log file to be created
        print(f"Waiting for log file: {log_filename}")
        time.sleep(1)

    while True:
        try:
            if os.path.exists(log_filename):
                print("Processing new log entries...")
                 # 상태를 저장할 변수 초기화
                last_timestamps = {}  # 각 CAN ID의 마지막 타임스탬프 저장
                can_id_count = {}     # 각 CAN ID의 출현 빈도 저장
                total_message_count = 0  # 전체 메시지 수

                with open(log_filename, 'r') as log_file, open(output_log_filename, 'a') as output_log:
                    log_file.seek(last_position)
                    lines = log_file.readlines()  # Read new lines only
                    last_position = log_file.tell()

                    for line in lines:
                        #print(f"Processing line: {line.strip()}")
                        parts = line.strip().split()
                        if len(parts) < 4:  # Ensure the line has enough parts
                            print(f"Skipping invalid line: {line.strip()}")
                            continue
                        try:
                            # Parse CAN ID and payload
                            timestamp = float(parts[0].strip('()'))  # 괄호 제거 후 float으로 변환
                            can_id_hex = parts[2].split('#')[0]
                            payload = parts[2].split('#')[1]
                            label = int(parts[-1])  # Extract label (last part)

                            # Debug only for CAN ID 000
                            if can_id_hex == "000":  # Compare as string
                                print(f"Debugging CAN ID 000: Line: {line.strip()}")

                            can_id = int(can_id_hex, 16)
                            payload_bytes = [int(payload[i:i+2], 16) for i in range(0, len(payload), 2)]

                            # Ensure payload has 8 bytes
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

                            # Construct input features
                            # input_features = np.array([[can_id] + payload_bytes], dtype=np.float32)
                            input_features = np.array([[can_id, time_diff, can_id_frequency] + payload_bytes], dtype=np.float32)

                            # Perform model prediction
                            prediction = load_and_predict_tflite_model(model_path, input_features)
                            is_intrusion = prediction[0] > 0.5  # Example threshold

                            # Update the line with intrusion detection result
                            if is_intrusion:
                                if can_id_hex == "000":
                                    print(f"Intrusion detected: CAN ID {can_id_hex}, Prediction: {prediction}")
                                intrusion_count += 1
                                parts[-1] = "1"  # Update label to 1 for intrusion
                            else:
                                if can_id_hex == "000":
                                    print(f"No Intrusion detected: CAN ID {can_id_hex}, Prediction: {prediction}")
                                parts[-1] = "0"  # Update label to 0 for no intrusion

                            # Write the updated line to the output log
                            output_log.write(" ".join(parts) + "\n")
                        except Exception as e:
                            print(f"Error processing line: {line.strip()}")
                            print(f"Exception: {e}")


                time.sleep(1)  # Polling interval
            else:
                time.sleep(1)
        except KeyboardInterrupt:
            print("Stopping intrusion detection.")
            break

# 주요 코드 실행
if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python detect_intrusion.py <model_path> <log_file_path> <output_log_file_path>")
        sys.exit(1)

    tflite_model_path = sys.argv[1]
    log_file_path = sys.argv[2]
    output_log_file_path = sys.argv[3]
    payload_size = 8  # Update this with the number of features used in the XGBoost model
    print(f"first")
    detect_intrusion(tflite_model_path, log_file_path, output_log_file_path, payload_size)
