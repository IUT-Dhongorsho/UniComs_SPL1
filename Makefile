CXX      = g++
CXXFLAGS = -std=c++17 -Wall -pthread
LDFLAGS  =

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
    CXXFLAGS += $(shell pkg-config --cflags portaudio-2.0 2>/dev/null || echo "-I/opt/homebrew/include")
    LDFLAGS  += $(shell pkg-config --libs portaudio-2.0 2>/dev/null || echo "-L/opt/homebrew/lib -lportaudio") \
                -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices
else
    CXXFLAGS += $(shell pkg-config --cflags portaudio-2.0)
    LDFLAGS  += $(shell pkg-config --libs portaudio-2.0)
endif

SERVER_SRC = \
    src/server/server.cpp \
    src/server/Socket.cpp \
    src/server/getClientInfo.cpp \
    src/server/handleMessage.cpp \
    src/server/login.cpp \
    src/server/cmd_file.cpp \
    src/server/cmd_call.cpp \
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
    src/voice/voice_call.cpp \
    src/utils/utils.cpp \
    src/utils/sha256.cpp \
    src/utils/crypto/aes.cpp \
    src/utils/crypto/diffie_hellman.cpp \
    src/utils/crypto/random.cpp

all: server client

server: $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) -o server

client: $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) -o client $(LDFLAGS)

clean:
	rm -f server client