def compare_files(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        file1_lines = f1.readlines()
        file2_lines = f2.readlines()

        for line_num, (line1, line2) in enumerate(zip(file1_lines, file2_lines), start=1):
            if line1 != line2:
                print(f"Line {line_num} is different:")
                print(f"File 1: {line1}")
                print(f"File 2: {line2}")


def parse_can_line1(line):
    # print(f"line1:{line}")
    parts = line.split()
    can_id, can_payload = parts[2].split('#')
    status = int(parts[3])
    return can_id, can_payload, status

def parse_can_line2(line):
    # print(f"line2:{line}")
    parts = line.split()
    can_id, can_payload = parts[2].split('#')
    status = int(parts[3])
    return can_id, can_payload, status

def check_FNR_FPR(file1, file2, output_file):
    cnt=0
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        cnt+=1
        file1_lines = f1.readlines()
        file2_lines = f2.readlines()

        false_positive = 0    # file2에서 비정상(1), file1에서는 정상(0)
        false_negative = 0    # file2에서 정상(0), file1에서는 비정상(1)
        true_positive = 0     # file2에서 비정상(1), file1에서도 비정상(1)
        true_negative = 0     # file2에서 정상(0), file1에서도 정상(0)
        total_normal = 0      # file1에서의 총 정상 패킷 수
        total_abnormal = 0    # file1에서의 총 비정상 패킷 수

        for line_num, (line1, line2) in enumerate(zip(file1_lines, file2_lines), start=1):
            cnt+=1
            try:
                can_id1, can_payload1, status1 = parse_can_line1(line1)
                can_id2, can_payload2, status2 = parse_can_line2(line2)

                # if can_id1 == can_id2 and can_payload1 == can_payload2:
                if can_id1.lower() == can_id2.lower() and can_payload1.lower() == can_payload2.lower():
                    if status1 == 0 and status2 != 0:
                        false_positive += 1  # file2가 오탐
                    elif status1 == 1 and status2 == 0:
                        false_negative += 1  # file2가 미탐
                    elif status1 == 1 and status2 != 0:
                        true_positive += 1  # file1과 file2 모두 비정상(탐지 성공)
                    elif status1 == 0 and status2 == 0:
                        true_negative += 1  # file1과 file2 모두 정상(탐지 성공)
                else:
                    print(f"line: {cnt}")
                    print(f"File 1: {line1}")
                    print(f"File 2: {line2}")
                    exit(0)
                    # print()

                # file1의 정상/비정상 패킷 수 카운트
                if status1 == 0:
                    total_normal += 1  # file1에서의 정상 패킷
                elif status1 != 0:
                    total_abnormal += 1  # file1에서의 비정상 패킷

            except Exception as e:
                print(f"Error on line {line_num}: {e}")
                print(f"Skipping line {line_num}")
                continue

        total_packets = total_normal + total_abnormal
        # 오탐율, 미탐율, true positive 및 true negative 비율 계산
        false_positive_rate = (false_positive / total_normal) * 100 if total_normal > 0 else 0
        false_negative_rate = (false_negative / total_abnormal) * 100 if total_abnormal > 0 else 0
        true_positive_rate = (true_positive / total_abnormal) * 100 if total_abnormal > 0 else 0
        true_negative_rate = (true_negative / total_normal) * 100 if total_normal > 0 else 0
        
        # # 결과를 파일에 저장
        # with open(output_file, 'w') as f_out:
        #     f_out.write(f"Total packets: {total_packets}\n")
        #     f_out.write(f"False positives: {false_positive} ({false_positive_rate:.6f}%)\n")
        #     f_out.write(f"False negatives: {false_negative} ({false_negative_rate:.6f}%)\n")
        #     f_out.write(f"True positives: {true_positive} ({true_positive_rate:.6f}%)\n")
        #     f_out.write(f"True negatives: {true_negative} ({true_negative_rate:.6f}%)\n")
        #     f_out.write(f"---------------------------------------------------------------\n")
        # 결과를 파일에 저장
        with open(output_file, 'w') as f_out:
            print(f"Total packets: {total_packets}\n")
            print(f"False positives: {false_positive} ({false_positive_rate:.6f}%)\n")
            print(f"False negatives: {false_negative} ({false_negative_rate:.6f}%)\n")
            print(f"True positives: {true_positive} ({true_positive_rate:.6f}%)\n")
            print(f"True negatives: {true_negative} ({true_negative_rate:.6f}%)\n")
            print(f"---------------------------------------------------------------\n")

        #print(f"Results saved to {output_file}")

        return true_positive, true_negative, false_positive, false_negative


# # 파일 경로를 지정하고 호출
# core_path_name=""
# original_log_path="../canlogs/DoS/MIRGU_DoS_attack.log"
# ids_result_path="result.txt"
# ids_checking_performance=f"test1.txt"
# # check_FNR_FPR(original_log_path, ids_result_path, ids_checking_performance)
# tp, tn, fp, fn=check_FNR_FPR(original_log_path, ids_result_path, ids_checking_performance)
# print(tp, fn, fp, fn)
# check_FNR_FPR("/home/pi/dataset/all_log/MIRGU/Fuzzing_valid_IDs.log", "/home/pi/ruleset_ML_performance/ids_logfile.log","/home/pi/ruleset_ML_performance/Fuzzing_valid_IDs_RULESET_perfomance.log")
check_FNR_FPR("../../dataset/all_log/MIRGU/Fuzzing_random_IDs.log", "../log/second_Fuzzing_random_IDs.log","/home/pi/ruleset_ML_performance/Fuzzing_valid_IDs_RULESETnML_perfomance.log")
# python model_xgboost_second.py xgboost_model.tflite ~/dataset/all_log/MIRGU/Fuzzing_valid_IDs_DoS.log ../log/second_Fuzzing_valid_IDs_DoS.log