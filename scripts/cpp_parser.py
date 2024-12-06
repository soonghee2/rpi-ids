import re

# Define regex patterns for extracting CAN messages and signals
BO_PATTERN = re.compile(r"^BO_\s+(\d+)\s+(\w+):\s+(\d+)\s+(\w+)")
# Refined regex to handle the "@" symbol, +/- signs, and possible variations in formatting
# SG_PATTERN = re.compile(r"^SG_\s+([\w\s]+)\s*:\s+(\d+)\|(\d+)@(\d+)([+-])\s+\(([\d\.E\-]+),\s*([\d\.E\-]+)\)\s+\[([\d\.\-]+)\|([\d\.\-]+)\]\s*\"([^\"]*)\"\s*([\w,]*)")
SG_PATTERN = re.compile(
    r"^SG_\s+([\w\s]+)\s*:\s+(\d+)\|(\d+)@(\d+)([+-])\s+\(([\d\.E\-]+),\s*([\d\.E\-]+)\)\s+\[([\d\.\-]+)\|([\d\.\-]+)\]\s*\"([^\"]*)\"\s*([\w,]*)")


# CAN 메시지와 신호를 저장할 클래스 정의
class Signal:
    def __init__(self, name, start_bit, length, byte_order, byte_order_sign, factor, offset, min_val, max_val, unit,
                 receiver, min_range, max_range):
        self.name = name
        self.start_bit = start_bit
        self.length = length
        self.byte_order = byte_order
        self.byte_order_sign = byte_order_sign  # 부호 저장
        self.factor = factor
        self.offset = offset
        self.min_val = min_val
        self.max_val = max_val
        self.unit = unit
        self.receiver = receiver
        self.min = min_range
        self.max = max_range
        self.calculate_min_max()  # 계산을 바로 적용

        self.need_to_check = False
        self.is_need_to_check()

    # 신호의 최소/최대 값을 계산하는 함수
    def calculate_min_max(self):
        if self.factor < 0:
            self.min = int(round((self.max_val - self.offset) / self.factor))
            self.max = int(round((self.min_val - self.offset) / self.factor))
        elif self.factor > 0:
            self.min = int(round((self.min_val - self.offset) / self.factor))
            self.max = int(round((self.max_val - self.offset) / self.factor))
        else:
            print(f"ERROR: Factor is zero for signal {self.name}")

    def is_need_to_check(self):
        if self.min != 0 or self.max < ((2 ** self.length) - 1): self.need_to_check = True
        # if 'checksum' in self.name.lower():
        #     self.need_to_check = True


class CANMessage:
    def __init__(self, can_id, message_name, dlc, transmitter):
        self.can_id = can_id
        self.skipable = True
        self.message_name = message_name
        self.dlc = dlc
        self.transmitter = transmitter
        self.signals = []

    def add_signal(self, signal):
        self.signals.append(signal)

    def func_skipable(self):
        for signal in self.signals:
            if signal.need_to_check: self.skipable = False


def write_cpp(messages, cpp_file):
    # Sort the messages by CAN ID in ascending order
    sorted_messages = sorted(messages, key=lambda msg: msg.can_id)

    # Prepare the data to be written as C++ code
    cpp_data = "#include <unordered_map>\n#include <string>\n#include <vector>\n#include \"dbcparsed_dbc.h\"\n"

    cpp_data += "std::unordered_map<int, CANMessage> message = {\n"

    for message in sorted_messages:
        message.func_skipable()
        cpp_data += f'    {{0x{message.can_id:x}, {{true, "{message.message_name}", {message.dlc}, "{message.transmitter}", {{\n'

        for signal in message.signals:
            cpp_data += f'        {{"{signal.name}", {signal.start_bit}, {signal.length}, {signal.byte_order}, {1 if signal.byte_order_sign == "-" else 0}, {signal.min}, {signal.max}}},\n'

        cpp_data = cpp_data.rstrip(",\n") + "\n    }}},\n"  # Closing the signal list and message
    cpp_data = cpp_data.rstrip(",\n") + "\n};\n"  # Closing the CANMessages map

    # Write the C++ formatted data to a file
    with open(cpp_file, 'w') as f:
        f.write(cpp_data)

    return len(sorted_messages)


def parse_dbc_file(file_path):
    messages = []
    current_message = None

    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()

            # Ignore specific lines like BU_SG_REL_, SG_MUL_VAL_, and comments
            if line.startswith("BU_SG_REL_") or line.startswith("SG_MUL_VAL_") or line.startswith("CM_ SG_"):
                continue

            # Match CAN message line
            bo_match = BO_PATTERN.match(line)
            if bo_match:
                can_id = int(bo_match.group(1))
                message_name = bo_match.group(2)
                dlc = int(bo_match.group(3))
                transmitter = bo_match.group(4)

                # If a message was already in progress, store it
                if current_message:
                    messages.append(current_message)

                # Create a new CANMessage object
                current_message = CANMessage(can_id, message_name, dlc, transmitter)
                continue

            # Match CAN signal line
            sg_match = SG_PATTERN.match(line)
            if sg_match and current_message:
                signal_name = sg_match.group(1).strip()  # Remove any extra spaces
                start_bit = int(sg_match.group(2))
                length = int(sg_match.group(3))
                byte_order = int(sg_match.group(4))
                byte_order_sign = sg_match.group(5)  # 부호 (+/-) 캡처
                factor = float(sg_match.group(6))  # Adjusted group for factor
                offset = float(sg_match.group(7))  # Adjusted group for offset
                min_val = float(sg_match.group(8))
                max_val = float(sg_match.group(9))
                unit = sg_match.group(10)
                receiver = sg_match.group(11)

                # Create a Signal object and add it to the current message
                signal = Signal(signal_name, start_bit, length, byte_order, byte_order_sign, factor, offset, min_val,
                                max_val, unit, receiver, min_val, max_val)
                current_message.add_signal(signal)

            else:
                # Debug log to identify unparsed signals
                if "SG_" in line:
                    print(f"Unparsed signal: {line}")

        # Add the last message to the list
        if current_message:
            messages.append(current_message)

    return messages


def main():
    # 사용자로부터 C++ 파일 경로를 입력받기
    cpp_file = "../protocol/dbcparsed_dbc.cpp"

    # DBC 파일 경로를 입력받기
    dbc_file = input("DBC 파일 경로를 입력하세요 (예: ../protocol/dbc.dbc): ")

    # DBC 파일 파싱 및 CAN 메시지 목록 생성
    messages = parse_dbc_file(dbc_file)

    # CAN ID 기준으로 메시지를 오름차순 정렬하여 JSON 파일에 기록
    write_cpp(messages, cpp_file)

    # DBC 파일에서 CAN ID의 수를 출력
    print(f"DBC 파일에서 CAN ID 갯수: {len(messages)}")


if __name__ == "__main__":
    main()

