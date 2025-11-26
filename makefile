CXX := g++
CXXFLAGS := -g -Wall -Iinclude
LDFLAGS := -lGL -lGLU -lglut

SRC_DIR := src
BUILD_DIR := build
TARGET := $(BUILD_DIR)/opengl-maze

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

.PHONY: clean run

r run: $(TARGET)
	$(TARGET)

c clean:
	rm -rf $(BUILD_DIR)
