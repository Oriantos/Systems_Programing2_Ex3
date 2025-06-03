CXX      = g++
CXXFLAGS = -std=c++17 -Wall -g -Iinclude

SRC_DIR = src
SRCS    = $(filter-out $(SRC_DIR)/Demo.cpp,$(wildcard $(SRC_DIR)/*.cpp))
OBJS    = $(SRCS:$(SRC_DIR)/%.cpp=$(SRC_DIR)/%.o)

# Create a “library” object list that omits main.o
LIB_OBJS = $(filter-out $(SRC_DIR)/main.o, $(OBJS))

TEST_DIR   = tests
TEST_SRCS  = $(wildcard $(TEST_DIR)/test_*.cpp)
TEST_BINS  = $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(TEST_DIR)/%)

TARGET = game

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build each test binary by linking its .cpp with LIB_OBJS (no main.o)
$(TEST_DIR)/%: $(TEST_DIR)/%.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB_OBJS)

.PHONY: test
test: $(TEST_BINS)
	@echo
	@echo "=== Running all tests ==="
	@for T in $(TEST_BINS); do \
	  echo; \
	  echo "--- $$T ---"; \
	  ./$$T; \
	done

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_BINS)
