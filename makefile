CXX = g++
CXXFLAGS = -Wall -O2 -pthread

# dbc_parser.cpp와 dbc_based_ruleset.cpp 파일이 모두 존재할 때만 포함
ifneq ($(wildcard dbcparsed_dbc.cpp dbc_based_ruleset.cpp),)
	CXXFLAGS += -DSET_DBC_CHECK
    SRCS = dbcparsed_dbc.cpp dbc_based_ruleset.cpp
endif

# 기본 소스 파일과 오브젝트 파일
SRCS += main.cpp cQueue.cpp periodic.cpp CANStats.cpp all_attack_detection.cpp check_clock_error.cpp

OBJS = $(SRCS:.cpp=.o)

# 실행 파일 이름
TARGET = ids

# 주 규칙
all: $(TARGET)_primary clean_objects

# 메인 타겟 빌드 규칙
$(TARGET)_primary: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# 성공적인 빌드 후 오브젝트 파일 정리
clean_objects:
	rm -f $(OBJS)

# .cpp 파일에서 개별 오브젝트 파일을 컴파일하는 규칙
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 전체 빌드 정리 (실행 파일 포함)
clean:
	rm -f $(OBJS) $(TARGET)
