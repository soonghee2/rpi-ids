#!/bin/bash

# 1. Python 파일 실행
echo "Executing Python script..."
python3 cpp_parser_final.py
if [ $? -ne 0 ]; then
    echo "Python script execution failed!"
    exit 1
fi

# 2. Make 명령어 수행
echo "Running make..."
make
if [ $? -ne 0 ]; then
    echo "Make command failed!"
    exit 1
fi

# 3. 현재 날짜와 시간을 기반으로 로그 파일 이름 생성
log_filename=$(date +"%Y-%m-%d_%H-%M-%S.log")

# 4. 컴파일된 ./ids 실행
echo "Executing ./ids with log file: $log_filename"
./ids "$log_filename"
if [ $? -ne 0 ]; then
    echo "./ids execution failed!"
    exit 1
fi

echo "All steps completed successfully."
