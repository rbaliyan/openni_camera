CPP := g++ -g
CFLAGS :=  -Wall -Wextra $(shell pkg-config --cflags libopenni2) $(shell pkg-config --cflags opencv4) $(pkg-config --libs opencv4)
LDFLAGS := $(shell pkg-config --libs opencv4)  $(shell pkg-config --libs libopenni2) -lpthread
TARGET := webcam
CPP_FILES := $(wildcard *.cpp)
INCLUDE_FILES := $(wildcard *.h)
RM := rm



OBJ_FILES := $(patsubst %.cpp,%.o, $(CPP_FILES))


all: $(TARGET)

$(OBJ_FILES): %.o:%.cpp $(INCLUDE_FILES)  Makefile
	$(CPP) -c $< $(CFLAGS)

$(TARGET): $(OBJ_FILES)
	$(CPP) -o $(TARGET) $(OBJ_FILES) $(LDFLAGS)

clean:
	$(RM) $(TARGET) $(OBJ_FILES)

run: $(TARGET)
	./$(TARGET)

debug:
	gdb $(TARGET)

.PHONY: all clean run