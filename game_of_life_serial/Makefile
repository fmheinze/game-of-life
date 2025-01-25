# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -I$(SRC_DIR) -O3

# Directories
SRC_DIR = src
BUILD_DIR = build

# Sources and Object Files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Target Executable
TARGET = game_of_life

# Default Rule
all: $(TARGET)

# Link Objects to Create Executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile Source Files to Object Files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean Build Files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Phony Targets
.PHONY: all clean
