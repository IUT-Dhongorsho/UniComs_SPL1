# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I. -I./common -I./server

# Find all source files in server and common
SERVER_SRCS = $(shell find server -name "*.cpp")
COMMON_SRCS = $(shell find common -name "*.cpp")
SRCS = $(SERVER_SRCS) $(COMMON_SRCS)

# Target executable
TARGET = server_app

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -pthread -o $(TARGET)

clean:
	rm -f $(TARGET)