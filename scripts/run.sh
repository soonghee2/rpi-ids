#!/bin/bash

# 1. Python 파일 실행 여부 확인
read -p "DBC 파일을 새롭게 적용하시겠습니까? (y/n): " apply_dbc
if [ "$apply_dbc" = "y" ]; then
    python3 cpp_parser.py
    if [ $? -ne 0 ]; then
        echo "DBC 파일 적용 실패!"
        exit 1
    fi

    echo "Running make..."
    make
    if [ $? -ne 0 ]; then
        echo "Make command failed!"
        exit 1
    fi
fi

# 2. AI와 룰셋 통합 IDS 실행 여부 확인
read -p "AI와 룰셋 통합 IDS로 실행하시겠습니까? (y/n): " use_ai

# 3. ./ids 실행
log_filename=$(date +"../log/%Y-%m-%d_%H-%M-%S.log")

if [ "$use_ai" = "n" ]; then
    echo "Executing ./ids with log file: $log_filename"
    echo "Clearing Terminal to Start IDS CLI"
    sleep 3
    clear
    ./ids "$log_filename"
    if [ $? -ne 0 ]; then
        echo "./ids execution failed!"
        exit 1
    fi
else
    # AI 통합 실행
    echo "Clearing Terminal to Start IDS CLI"
    sleep 3
    clear

    echo "AI 통합 IDS 실행 준비 중..."
    tflite_model_path="autoencoder.tflite"
    output_log_file_path=$(date +"../log/%Y-%m-%d_%H-%M-%S_after_AI.log")

    # AI Python 코드 실행
    python3.9 model.py "$log_filename" "$output_log_file_path" &
    model_pid=$!

    echo "Executing ./ids with log file: $log_filename"
    ./ids "$log_filename"
    if [ $? -ne 0 ]; then
        echo "./ids execution failed!"
        kill $model_pid
        exit 1
    fi

    # Python 모델 프로세스 종료
    kill $model_pid
    echo "AI 모델 프로세스를 종료했습니다."
fi

echo "All steps completed successfully."

