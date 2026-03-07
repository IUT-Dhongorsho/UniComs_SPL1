CXX      = g++
CXXFLAGS = -std=c++17 -Wall -pthread

SERVER_SRC = \
    src/server/server.cpp \
    src/server/Socket.cpp \
    src/server/getClientInfo.cpp \
    src/server/handleMessage.cpp \
    src/server/login.cpp \
    src/server/cmd_file.cpp \
    src/server/broadcast.cpp \
    src/server/joinChatRoom.cpp \
    src/server/getChatRoomList.cpp \
    src/utils/utils.cpp \
    src/utils/sha256.cpp \
    src/db/Catalog.cpp \
    src/db/Indexer.cpp \
    src/db/Table_Engine.cpp \
    src/db/csv_storage.cpp

CLIENT_SRC = \
    src/client/client.cpp \
    src/client/Connect.cpp \
    src/utils/utils.cpp \
    src/utils/sha256.cpp

all: server client

server: $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) -o server

client: $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) -o client

clean:
	rm -f server client