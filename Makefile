CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread -I./Libraries/hdelibc
LDFLAGS = -pthread -L./Libraries/hdelibc -lhdelibc

all: server client

# Build library
libhdelibc.a: SimpleSocket.o BindingSocket.o ConnectingSocket.o ListeningSocket.o
	ar rcs Libraries/hdelibc/libhdelibc.a $^

# Compile socket objects
SimpleSocket.o: Libraries/hdelibc/Networking/Sockets/SimpleSocket.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

BindingSocket.o: Libraries/hdelibc/Networking/Sockets/BindingSocket.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

ConnectingSocket.o: Libraries/hdelibc/Networking/Sockets/ConnectingSocket.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

ListeningSocket.o: Libraries/hdelibc/Networking/Sockets/ListeningSocket.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Build server
server: ChatRoom/server.cpp libhdelibc.a
	$(CXX) $(CXXFLAGS) -o ChatRoom/server $< $(LDFLAGS)

# Build client
client: ChatRoom/client.cpp libhdelibc.a
	$(CXX) $(CXXFLAGS) -o ChatRoom/client $< $(LDFLAGS)

# Run targets
run-server: server
	./ChatRoom/server

run-client: client
	./ChatRoom/client

# Clean
clean:
	rm -f *.o Libraries/hdelibc/libhdelibc.a ChatRoom/server ChatRoom/client

.PHONY: all clean run-server run-client
