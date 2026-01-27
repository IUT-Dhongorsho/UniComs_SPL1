#include "server.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <functional>
#include <map>

const int BUFFER_SIZE = 4096;

// Global data structures with mutex protection
std::mutex clientsMutex;
std::mutex roomsMutex;
sockList allClients;
sockToName clientNames;
chatRoomToSockList chatRooms;

// Function to broadcast messages to room members
void broadcastToRoom(const std::string& roomName, const std::string& message, int excludeSock = -1) {
    std::lock_guard<std::mutex> lock(roomsMutex);
    
    auto it = chatRooms.find(roomName);
    if (it != chatRooms.end()) {
        for (int clientSock : it->second) {
            if (clientSock != excludeSock) {
                send(clientSock, message.c_str(), message.length(), 0);
            }
        }
    }
}

// Process client commands
void processCommand(int clientSock, const std::string& command, Database<User>& db) {
    std::cout << "Processing command from socket " << clientSock << ": " << command << std::endl;
    
    // Parse command
    size_t delimiter = command.find("%%");
    std::string cmdType;
    std::string payload;
    
    if (delimiter != std::string::npos) {
        cmdType = command.substr(0, delimiter);
        payload = command.substr(delimiter + 2);
    } else {
        cmdType = command;
    }
    
    // Handle different command types
    if (cmdType == "LOGIN") {
        size_t userDelim = payload.find("%%");
        if (userDelim != std::string::npos) {
            std::string username = payload.substr(0, userDelim);
            std::string password = payload.substr(userDelim + 2);
            
            std::cout << "Login attempt: " << username << std::endl;
            
            std::string result = login(db, username, password);
            
            if (result == "SUCCESS") {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clientNames[clientSock] = username;
                std::cout << "User logged in: " << username << std::endl;
            }
            
            send(clientSock, result.c_str(), result.length(), 0);
        }
    }
    else if (cmdType == "LOGOUT") {
        std::string username = payload;
        
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            if (logout(db, username)) {
                clientNames.erase(clientSock);
                std::cout << "User logged out: " << username << std::endl;
            }
        }
        
        // Remove from all rooms
        {
            std::lock_guard<std::mutex> lock(roomsMutex);
            for (auto& room : chatRooms) {
                room.second.erase(clientSock);
            }
            
            // Clean up empty rooms
            for (auto it = chatRooms.begin(); it != chatRooms.end(); ) {
                if (it->second.empty()) {
                    it = chatRooms.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    else if (cmdType == "JOIN") {
        std::string roomName = payload;
        
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::lock_guard<std::mutex> lock2(roomsMutex);
            
            // Check if user is logged in
            if (clientNames.find(clientSock) == clientNames.end()) {
                std::string error = "ERROR%%Please login first";
                send(clientSock, error.c_str(), error.length(), 0);
                return;
            }
            
            std::string username = clientNames[clientSock];
            joinChatRoom(roomName, clientSock, clientNames, chatRooms);
            
            std::string success = "%%join%%" + roomName;
            send(clientSock, success.c_str(), success.length(), 0);
            
            // Notify others in the room
            std::string notification = "info%%" + username + "#" + roomName + "%%joined the room";
            broadcastToRoom(roomName, notification, clientSock);
            
            std::cout << username << " joined room: " << roomName << std::endl;
        }
    }
    else if (cmdType == "LEAVE") {
        std::string roomName = payload;
        
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::lock_guard<std::mutex> lock2(roomsMutex);
            
            if (clientNames.find(clientSock) == clientNames.end()) {
                return;
            }
            
            std::string username = clientNames[clientSock];
            leaveChatRoom(roomName, clientSock, clientNames, chatRooms);
            
            std::string success = "%%leave%%" + roomName;
            send(clientSock, success.c_str(), success.length(), 0);
            
            // Notify others in the room
            if (chatRooms.find(roomName) != chatRooms.end()) {
                std::string notification = "info%%" + username + "#" + roomName + "%%left the room";
                broadcastToRoom(roomName, notification);
            }
            
            std::cout << username << " left room: " << roomName << std::endl;
        }
    }
    else if (cmdType == "LIST_ROOMS") {
        std::lock_guard<std::mutex> lock(roomsMutex);
        std::string roomList = getChatroomList(chatRooms);
        send(clientSock, roomList.c_str(), roomList.length(), 0);
    }
    else if (cmdType == "PEOPLE") {
        std::string roomName = payload;
        
        std::lock_guard<std::mutex> lock(clientsMutex);
        std::lock_guard<std::mutex> lock2(roomsMutex);
        
        if (clientNames.find(clientSock) == clientNames.end()) {
            return;
        }
        
        std::string peopleList = getPeopleList(roomName, clientNames, chatRooms);
        send(clientSock, peopleList.c_str(), peopleList.length(), 0);
    }
    else {
        // Regular message format: roomName%%sender%%text
        size_t firstDelim = command.find("%%");
        if (firstDelim != std::string::npos) {
            std::string roomName = command.substr(0, firstDelim);
            std::string remaining = command.substr(firstDelim + 2);
            
            size_t secondDelim = remaining.find("%%");
            if (secondDelim != std::string::npos) {
                std::string sender = remaining.substr(0, secondDelim);
                std::string text = remaining.substr(secondDelim + 2);
                
                std::lock_guard<std::mutex> lock(clientsMutex);
                std::lock_guard<std::mutex> lock2(roomsMutex);
                
                // Check if sender is logged in
                if (clientNames[clientSock] != sender) {
                    std::string error = "ERROR%%Authentication error";
                    send(clientSock, error.c_str(), error.length(), 0);
                    return;
                }
                
                // Handle the message
                std::string fullMsg = roomName + "%%" + sender + "%%" + text;
                handleMsg(clientSock, chatRooms, clientNames, fullMsg);
                
                std::cout << sender << " sent message to room " << roomName 
                         << ": " << (text.length() > 50 ? text.substr(0, 47) + "..." : text) << std::endl;
            }
        }
    }
}

// Client handler thread
void handleClient(int clientSock) {
    char buffer[BUFFER_SIZE];
    Database<User> db("users.db");
    
    // Initialize database with admin if empty
    DatabaseUtils::initializeDatabase("users.db", "admin", "admin123");
    
    // Get client info
    std::string clientInfo = getClientInfo(clientSock);
    std::cout << "New client connected: " << clientInfo << std::endl;
    
    // Add to global client list
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        allClients.insert(clientSock);
    }
    
    // Client loop
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = recv(clientSock, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesRead <= 0) {
            std::cout << "Client disconnected: " << clientInfo;
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                if (clientNames.find(clientSock) != clientNames.end()) {
                    std::cout << " (" << clientNames[clientSock] << ")";
                }
            }
            std::cout << std::endl;
            break;
        }
        
        buffer[bytesRead] = '\0';
        std::string message(buffer);
        
        // Process the command
        processCommand(clientSock, message, db);
    }
    
    // Cleanup
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        
        // Logout if logged in
        if (clientNames.find(clientSock) != clientNames.end()) {
            std::string username = clientNames[clientSock];
            logout(db, username);
            clientNames.erase(clientSock);
        }
        
        allClients.erase(clientSock);
    }
    
    // Remove from all rooms
    {
        std::lock_guard<std::mutex> lock(roomsMutex);
        for (auto& room : chatRooms) {
            room.second.erase(clientSock);
        }
        
        // Clean up empty rooms
        for (auto it = chatRooms.begin(); it != chatRooms.end(); ) {
            if (it->second.empty()) {
                it = chatRooms.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    close(clientSock);
}

// Main server function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        std::cerr << "Example: " << argv[0] << " 8080" << std::endl;
        return 1;
    }
    
    int port = std::stoi(argv[1]);
    
    // Validate port
    if (port < 1 || port > 65535) {
        std::cerr << "Error: Invalid port number. Must be between 1 and 65535." << std::endl;
        return 1;
    }
    
    std::cout << "Starting LAN Chat Server..." << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Database: users.db" << std::endl;
    
    try {
        // Create server socket
        int serverSock = Socket();
        std::cout << "Socket created successfully." << std::endl;
        
        // Bind to port
        Bind(serverSock, port);
        std::cout << "Bound to port " << port << " successfully." << std::endl;
        
        // Listen for connections
        Listen(serverSock, 10);
        std::cout << "Listening for connections..." << std::endl;
        std::cout << "Server is ready!" << std::endl;
        std::cout << "Press Ctrl+C to stop the server.\n" << std::endl;
        
        // Main server loop
        while (true) {
            // Accept new connection
            int clientSock = Accept(serverSock);
            
            // Handle client in separate thread
            std::thread clientThread(handleClient, clientSock);
            clientThread.detach();
        }
        
    } catch (const std::exception &e) {
        std::cerr << "\nServer Error: " << e.what() << std::endl;
        std::cerr << "Server shutting down..." << std::endl;
        return 1;
    }
    
    return 0;
}