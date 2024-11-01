#!/bin/bash

# 로그 파일이 있는 디렉토리
log_dir1=~/dataset/all_log/MIRGU
log_dir=~/dataset/result/
# CAN 인터페이스 이름
can_interface="vcan0"

# 패킷 수신 확인용 로그 파일
can_monitor_log="/tmp/can_monitor.log"

# 수신이 멈춘 시간을 감지할 최대 시간 (60초)
timeout_duration=60

# 로그 파일을 하나씩 처리
for log_file in "$log_dir"/*.log; do
  echo "Processing file: $log_file"

  # ids 프로그램 실행, 로그 파일과 -1 인자를 전달
  ./ids << EOF &
$log_file
-1
EOF

  # canplayer로 로그 파일을 vcan0 인터페이스로 전송
  canplayer -I "$log_file" -I vcan0=can0 &

  # 패킷 수신 여부 확인을 위해 candump로 모니터링 시작
  candump "$can_interface" > "$can_monitor_log" &

  # 패킷 수신 시간 확인을 위한 시간 기록
  last_packet_time=$(date +%s)

  # 무한 루프 시작 - 패킷 수신을 확인
  while true; do
    current_time=$(date +%s)
    
    # 패킷 수신 로그 업데이트 확인
    if [ -s "$can_monitor_log" ]; then
      last_packet_time=$(date +%s)  # 패킷이 수신되면 시간 업데이트
      echo "Packet received from $log_file"
      
      # 모니터 로그 파일을 초기화
      > "$can_monitor_log"
    fi

    # 패킷이 60초 동안 수신되지 않으면 다음 로그로 이동
    if (( current_time - last_packet_time > timeout_duration )); then
      echo "No packets received for $timeout_duration seconds, moving to next log."
      
      # 현재 실행 중인 ids 및 canplayer 프로세스 종료
      pkill -f "ids"
      pkill -f "canplayer"
      pkill -f "candump"
      
      break
    fi

    sleep 5  # 5초 간격으로 체크
  done

  echo "Finished processing $log_file"
  echo "--------------------------------"
done

echo "All log files processed."

