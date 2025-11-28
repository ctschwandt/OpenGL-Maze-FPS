CXX := g++
SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LIBS   := $(shell sdl2-config --libs 2>/dev/null)

ifeq ($(SDL_CFLAGS),)
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS   := $(shell pkg-config --libs sdl2 2>/dev/null)
endif

CXXFLAGS := -g -Wall -Iinclude $(SDL_CFLAGS)
LDFLAGS := -lGL -lGLU -lGLEW $(SDL_LIBS)

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
