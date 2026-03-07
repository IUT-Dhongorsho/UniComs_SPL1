CXX      = g++
CXXFLAGS = -std=c++17 -Wall -pthread -I./src
LDFLAGS  = -lncurses -lpanel -lmenu -lform

# Source files
SERVER_SRC = \
    src/server/server.cpp \
    src/server/Accept.cpp \
    src/server/Bind.cpp \
    src/server/Listen.cpp \
    src/server/Socket.cpp \
    src/server/getClientInfo.cpp \
    src/server/handleMessage.cpp \
    src/server/login.cpp \
    src/server/cmd_file.cpp \
    src/server/broadcast.cpp \
    src/server/joinChatRoom.cpp \
    src/server/getChatRoomList.cpp

CLIENT_SRC = \
    src/client/Connect.cpp \
    src/client/Socket.cpp \
    src/client/client_ui.cpp

UTILS_SRC = \
    src/utils/utils.cpp \
    src/utils/sha256.cpp \
    src/utils/hostToNetShort.cpp \
    src/utils/memorySet.cpp

DB_SRC = \
    src/db/Catalog.cpp \
    src/db/Indexer.cpp \
    src/db/Table_Engine.cpp \
    src/db/csv_storage.cpp

UI_SRC = \
    src/ui/UIManager.cpp \
    src/ui/Colors.cpp \
    src/ui/Animations.cpp \
    src/ui/Components/ChatBubble.cpp \
    src/ui/Components/ContactList.cpp \
    src/ui/Components/InputField.cpp \
    src/ui/Components/StatusBar.cpp \
    src/ui/Components/FileTransferUI.cpp \
    src/ui/Themes/DarkTheme.cpp

# Combine all sources
ALL_CLIENT_SRC = $(CLIENT_SRC) $(UTILS_SRC) $(DB_SRC) $(UI_SRC)
ALL_SERVER_SRC = $(SERVER_SRC) $(UTILS_SRC) $(DB_SRC)

# Default target
all: server client

# Server build
server: $(ALL_SERVER_SRC)
	$(CXX) $(CXXFLAGS) $^ -o server $(LDFLAGS)

# Client with UI
client: $(ALL_CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $^ -o client $(LDFLAGS)

# Legacy client without UI (for testing)
client_legacy: src/client/client.cpp $(UTILS_SRC) $(DB_SRC)
	$(CXX) $(CXXFLAGS) $^ -o client_legacy

clean:
	rm -f server client client_legacy

.PHONY: all clean