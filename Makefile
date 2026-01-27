# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall -Wextra -I.

# Directories
SRC_DIR = src
CLIENT_DIR = $(SRC_DIR)/client
SERVER_DIR = $(SRC_DIR)/server
DB_DIR = $(SRC_DIR)/db
ENTRY_DIR = $(DB_DIR)/entry
UTILS_DIR = $(SRC_DIR)/utils

# Source files
CLIENT_SRCS = $(CLIENT_DIR)/Connect.cpp $(CLIENT_DIR)/Socket.cpp $(CLIENT_DIR)/client.cpp
SERVER_SRCS = $(SERVER_DIR)/server.cpp \
              $(SERVER_DIR)/Accept.cpp \
              $(SERVER_DIR)/Bind.cpp \
              $(SERVER_DIR)/Listen.cpp \
              $(SERVER_DIR)/Socket.cpp \
              $(SERVER_DIR)/broadcast.cpp \
              $(SERVER_DIR)/getChatRoomList.cpp \
              $(SERVER_DIR)/getClientInfo.cpp \
              $(SERVER_DIR)/getPeopleList.cpp \
              $(SERVER_DIR)/handleMessage.cpp \
              $(SERVER_DIR)/joinChatRoom.cpp \
              $(SERVER_DIR)/leaveChatRoom.cpp \
              $(SERVER_DIR)/login.cpp \
              $(SERVER_DIR)/logout.cpp

DB_SRCS = $(DB_DIR)/database.cpp \
          $(DB_DIR)/databaseUtils.cpp \
          $(DB_DIR)/user.cpp \
          $(ENTRY_DIR)/entry.cpp

UTILS_SRCS = $(UTILS_DIR)/hostToNetShort.cpp \
             $(UTILS_DIR)/memorySet.cpp

# Object files
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
DB_OBJS = $(DB_SRCS:.cpp=.o)
UTILS_OBJS = $(UTILS_SRCS:.cpp=.o)

# Executables
CLIENT_EXEC = client
SERVER_EXEC = server

# Default target
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Build client
$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build server
$(SERVER_EXEC): $(SERVER_OBJS) $(DB_OBJS) $(UTILS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Pattern rule for .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Client specific object files
$(CLIENT_DIR)/%.o: $(CLIENT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Server specific object files
$(SERVER_DIR)/%.o: $(SERVER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Database specific object files
$(DB_DIR)/%.o: $(DB_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(ENTRY_DIR)/%.o: $(ENTRY_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Utils specific object files
$(UTILS_DIR)/%.o: $(UTILS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean build artifacts
clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC) \
	      $(CLIENT_OBJS) $(SERVER_OBJS) $(DB_OBJS) $(UTILS_OBJS)

# Phony targets
.PHONY: all clean

# Help message
help:
	@echo "Available targets:"
	@echo "  all     - Build both client and server (default)"
	@echo "  client  - Build only the client"
	@echo "  server  - Build only the server"
	@echo "  clean   - Remove all executables and object files"
	@echo "  help    - Show this help message"

# Build only client
client: $(CLIENT_EXEC)

# Build only server
server: $(SERVER_EXEC)