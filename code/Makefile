CXX      = g++
CXXFLAGS = -std=c++17 -Wall -pthread -Wno-nonportable-include-path
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
    src/server/Server.cpp \
    src/server/Socket.cpp \
    src/server/GetClientInfo.cpp \
    src/server/HandleMessage.cpp \
    src/server/Login.cpp \
    src/server/CmdFile.cpp \
    src/server/CmdCall.cpp \
    src/server/Broadcast.cpp \
    src/server/JoinChatRoom.cpp \
    src/server/GetChatRoomList.cpp \
    src/utils/Utils.cpp \
    src/utils/Sha256.cpp \
    src/db/Catalog.cpp \
    src/db/Indexer.cpp \
    src/db/TableEngine.cpp \
    src/db/CsvStorage.cpp

CLIENT_SRC = \
    src/client/Client.cpp \
    src/client/Terminal.cpp \
    src/client/NetworkManager.cpp \
    src/client/CryptoHandler.cpp \
    src/client/FileHandler.cpp \
    src/client/VoiceHandler.cpp \
    src/client/ClientController.cpp \
    src/client/Connect.cpp \
    src/voice/VoiceCall.cpp \
    src/utils/Utils.cpp \
    src/utils/Sha256.cpp \
    src/utils/crypto/Aes.cpp \
    src/utils/crypto/DiffieHellman.cpp \
    src/utils/crypto/Random.cpp

all: server client

server: $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) -o server

client: $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) -o client $(LDFLAGS)

clean:
	rm -f server client