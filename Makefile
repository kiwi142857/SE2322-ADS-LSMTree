CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -Wextra

# Directories
SRC_DIR = src
TEST_DIR = tests

# Source files
SRCS = $(wildcard $(SRC_DIR)/core/*.cc) \
       $(wildcard $(SRC_DIR)/memtable/*.cc) \
       $(wildcard $(SRC_DIR)/sstable/*.cc) \
       $(wildcard $(SRC_DIR)/utils/*.cc) \
       $(wildcard $(SRC_DIR)/vlog/*.cc)

# Object files
OBJS = $(SRCS:.cc=.o)

# Include directories
INCLUDES = -I$(SRC_DIR)

# Test targets
TEST_TARGETS = correctness persistence performance

# Default target
all: $(TEST_TARGETS)

# Compile source files
%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link test executables
correctness: $(TEST_DIR)/correctness.cc $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

persistence: $(TEST_DIR)/persistence.cc $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

performance: $(TEST_DIR)/performance.cc $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

# Clean
clean:
	rm -f $(OBJS) $(TEST_TARGETS)
	rm -rf data

.PHONY: all clean