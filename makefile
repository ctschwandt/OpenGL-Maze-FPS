CXX := g++
CXXFLAGS := -g -Wall -Iinclude
LDFLAGS := -lGL -lGLU -lGLEW -lglfw -lpng -ljpeg

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

.PHONY: clean c run r asan asanr profile profiler

r run: $(TARGET)
	$(TARGET)

c clean:
	rm -rf $(BUILD_DIR)

asan: CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
asan: LDFLAGS  += -fsanitize=address
asan: clean $(TARGET)

asanr: asan
	$(TARGET)

profile profiler: CXXFLAGS += -pg
profile profiler: LDFLAGS  += -pg
profile profiler: clean $(TARGET)
	$(TARGET)
	gprof $(TARGET) gmon.out > $(BUILD_DIR)/profile.txt
	@echo "Wrote gprof output to $(BUILD_DIR)/profile.txt"
