import subprocess
import check_FNR_FPR

import subprocess
import time
import os
import signal
from datetime import datetime
# Set the log file path
origin_dataset_path="/home/pi/dataset/all_log/MIRGU/"

model_path = "./model_xgboost_second.py"
dataset_path="/home/pi/dataset/all_log/MIRGU/" #모델에 돌릴 위치
model_log_path="/home/pi/dataset/test/MIRGU_only_ml/" #모델 돌리고 결과 위치
model_performance_path ="../ml_performance/"
model_flite_path = "./xgboost_model.tflite"

def ensure_directory_exists(path):
    os.makedirs(path, exist_ok=True)

def run_model_in_background(command):
    # model.py를 백그라운드에서 실행
    model_process = subprocess.Popen(command, shell=True)
    return model_process

def start_model(command, log_path):
    model_process = run_model_in_background(command)
    print(f"Started model.py (PID: {model_process.pid})")

    model_process.terminate()  # ids_process를 종료시킴
    time.sleep(1)
    #subprocess.run(["pkill", "-f", "ids"], check=True)
    model_process.wait()
    # ids_process가 아직 실행 중인지 확인, 종료
    if model_process.poll() is None:
        # 2초 대기 후 process_process에 Ctrl+C와 같은 SIGINT 신호를 보냄
        os.kill(ids_process.pid, signal.SIGINT)  # Ctrl+C와 동일하게 SIGINT 신호를 보냄
        print("SIGINT sent to model_process")
    else:
        print("model_process already terminated")
    time.sleep(5)

def rating_performance(attack, original_log_names):
    print(f"공격 유형:{attack}")
    print("성능을 측정할 log 파일들")
    print(original_log_names)

    # 필요한 폴더들 생성
    ensure_directory_exists(model_log_path + attack)
    ensure_directory_exists(model_performance_path + attack)

    all_tp=0
    all_tn=0
    all_fp=0
    all_fn=0

    #성능 평가
    for name in original_log_names:
        # ids의 log를 저장할 경로 + ids 재생 명령어
        model_log_file_paths=model_log_path + name
        ids_log_paths = dataset_path + name
        process_commands="python3.9 " + model_path + " " + model_flite_path+" " + ids_log_paths + " " + model_log_file_paths
        print(process_commands)

        #성능 결과를 저장할 경로
        performance_file_paths=model_performance_path + name

        print(f"{attack} - {name} IDS 탐지 중")
        start_model(process_commands, ids_log_paths)

        print(f"{attack} - {name} 성능 평가 중")
        print("===============================================")

        tp, tn, fp, fn=check_FNR_FPR.check_FNR_FPR(ids_log_paths, model_log_file_paths, performance_file_paths)
        all_tp+=tp
        all_tn+=tn
        all_fp+=fp
        all_fn+=fn

    print(f"tp: {all_tp} tn: {all_tn} fp: {all_fp} fn: {all_fn}")
    current_datetime = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(f"./record_performance_summury.txt", "a") as file:
        file.write(f"{current_datetime}-{attack}\ttp: {all_tp}\ttn: {all_tn}\tfp: {all_fp}\tfn: {all_fn}\n")

attack=""

# 폴더 안의 모든 로그 파일들을 원할 때
#original_log_names = [f for f in os.listdir(dataset_path+attack)]
original_log_names=["Fuzzing_random_IDs.log", "Fuzzing_valid_IDs.log", "Fuzzing_valid_IDs_DoS.log"]

rating_performance(attack, original_log_names)