CPP := g++ -g
CFLAGS :=  -Wall -Wextra $(shell pkg-config --cflags libopenni2) $(shell pkg-config --cflags opencv4) $(pkg-config --libs opencv4)
LDFLAGS := $(shell pkg-config --libs opencv4)  $(shell pkg-config --libs libopenni2) -lpthread
TARGET := webcam
CPP_FILES := $(wildcard *.cpp)
INCLUDE_FILES := $(wildcard *.h)
BIN_DIR := bin
RM := rm -f



OBJ_FILES := $(patsubst %.cpp,%.o, $(CPP_FILES))


all: $(BIN_DIR)/$(TARGET)

$(OBJ_FILES): %.o:%.cpp $(INCLUDE_FILES)  Makefile
	$(CPP) -c $< $(CFLAGS)

$(BIN_DIR)/$(TARGET): $(OBJ_FILES)
	$(CPP) -o $(BIN_DIR)/$(TARGET) $(OBJ_FILES) $(LDFLAGS)

clean:
	$(RM) $(BIN_DIR)/$(TARGET) $(OBJ_FILES)

run: $(BIN_DIR)/$(TARGET)
	./$(BIN_DIR)/$(TARGET)

debug:
	gdb $(TARGET)

.PHONY: all clean run