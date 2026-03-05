CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

DB_SRC = src/db/Catalog.cpp \
         src/db/Indexer.cpp \
         src/db/Table_Engine.cpp \
         src/db/csv_storage.cpp

UTILS_SRC = src/utils/utils.cpp

SERVER_SRC = $(DB_SRC) $(UTILS_SRC) \
             src/server/server.cpp \
             src/server/Socket.cpp \
             src/server/handleMessage.cpp \
             src/server/login.cpp \
             src/server/broadcast.cpp \
             src/server/joinChatRoom.cpp \
             src/server/getChatRoomList.cpp

CLIENT_SRC = $(UTILS_SRC) \
             src/client/client.cpp \
             src/client/Connect.cpp

all: server client

server: $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o server $(SERVER_SRC)

client: $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o client $(CLIENT_SRC)

clean:
	rm -f server client
	rm -f src/db/*.csv

.PHONY: all clean