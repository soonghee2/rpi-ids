# Define the compilers and flags
CXX = g++
CXXFLAGS = -Wall -O2 -lpthread

# First attempt source files and object files
SRCS = main_with_dbc.cpp periodic.cpp CANStats.cpp cQueue.cpp attack_detection_with_dbc.cpp check_clock_error.cpp dbc.cpp dbcparsed.cpp
OBJS = $(SRCS:.cpp=.o)

# Fallback source files and object files
SRCS2 = main_without_dbc.cpp periodic.cpp CANStats.cpp cQueue.cpp attack_detection_without_dbc.cpp check_clock_error.cpp
OBJS2 = $(SRCS2:.cpp=.o)

# Target executable
TARGET = ids

# Primary rule: try to compile with SRCS
all: try_build clean_objects

# First attempt: build target using the first set of source files
try_build:
	@if $(MAKE) $(TARGET)_primary; then \
		echo "Primary build succeeded"; \
	else \
		echo "Primary build failed, trying fallback"; \
		$(MAKE) fallback; \
	fi

# Rule for building the main target using the first set of source files
$(TARGET)_primary: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -lpthread

# Fallback rule: only triggered if the first set of sources fails to build
fallback: clean_fallback $(OBJS2)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS2)

# Clean up object files before fallback
clean_fallback:
	rm -f $(OBJS)

# Clean up object files after a successful build
clean_objects:
	rm -f $(OBJS) $(OBJS2)

# Rule to compile individual object files from .cpp
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up the entire build (including the executable)
clean:
	rm -f $(OBJS) $(OBJS2)

