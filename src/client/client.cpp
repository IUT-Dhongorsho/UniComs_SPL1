#include "client.h"
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <vector>
#include <sstream>

#define BUFFER_SIZE 4096

// Global state
std::atomic<bool> isRunning(true);
std::string currentUsername;
std::string currentRoom;

// Function declarations
void receiveMessages(int sockfd);
void displayMessage(const std::string& message);
void printWelcome();
void printHelp();
void printPrompt();
bool processCommand(int sockfd, const std::string& input);
std::vector<std::string> splitString(const std::string& str, char delimiter);

// Main function
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        std::cerr << "Example: " << argv[0] << " 127.0.0.1 8080" << std::endl;
        return 1;
    }

    std::string serverIP = argv[1];
    int port = std::stoi(argv[2]);

    try {
        // Create and connect socket
        int sockfd = Socket();
        std::cout << "Connecting to " << serverIP << ":" << port << "..." << std::endl;
        
        Connect(sockfd, serverIP, port);
        std::cout << "Connected successfully!\n" << std::endl;

        // Login process
        std::string username, password;
        std::cout << "Enter username: ";
        std::getline(std::cin, username);
        std::cout << "Enter password: ";
        std::getline(std::cin, password);

        // Send login request
        std::string loginMsg = "LOGIN%%" + username + "%%" + password;
        send(sockfd, loginMsg.c_str(), loginMsg.length(), 0);

        // Receive login response
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesReceived <= 0) {
            std::cerr << "\nError: Failed to receive login response" << std::endl;
            close(sockfd);
            return 1;
        }

        std::string response(buffer);
        
        if (response.find("SUCCESS") == std::string::npos) {
            std::cout << "\nLogin failed: " << response << std::endl;
            close(sockfd);
            return 1;
        }

        // Login successful
        currentUsername = username;
        printWelcome();

        // Start message receiving thread
        std::thread receiveThread(receiveMessages, sockfd);

        // Main command loop
        std::string input;
        while (isRunning) {
            printPrompt();
            
            if (!std::getline(std::cin, input)) {
                isRunning = false;
                break;
            }

            // Skip empty input
            if (input.empty()) {
                continue;
            }

            // Process command
            if (!processCommand(sockfd, input)) {
                break;
            }
        }

        // Cleanup
        isRunning = false;
        
        if (receiveThread.joinable()) {
            receiveThread.join();
        }
        
        // Send logout
        std::string logoutMsg = "LOGOUT%%" + username;
        send(sockfd, logoutMsg.c_str(), logoutMsg.length(), 0);
        
        close(sockfd);
        std::cout << "\nDisconnected. Goodbye!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// Receive messages in separate thread
void receiveMessages(int sockfd) {
    char buffer[BUFFER_SIZE];
    
    while (isRunning) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesReceived <= 0) {
            if (isRunning) {
                std::cout << "\n\n[Server disconnected]" << std::endl;
                isRunning = false;
            }
            break;
        }
        
        buffer[bytesReceived] = '\0';
        displayMessage(std::string(buffer));
    }
}

// Display received message with formatting
void displayMessage(const std::string& message) {
    std::cout << std::endl;
    
    if (message.find("MSG%%") == 0) {
        // Format: MSG%%sender#room%%text
        size_t firstDelim = message.find("%%", 4);
        if (firstDelim != std::string::npos) {
            std::string senderInfo = message.substr(4, firstDelim - 4);
            std::string text = message.substr(firstDelim + 2);
            
            // Parse sender and room
            size_t hashPos = senderInfo.find('#');
            if (hashPos != std::string::npos) {
                std::string sender = senderInfo.substr(0, hashPos);
                std::string room = senderInfo.substr(hashPos + 1);
                std::cout << "[" << room << "] " << sender << ": " << text << std::endl;
            } else {
                std::cout << "[" << senderInfo << "]: " << text << std::endl;
            }
        }
    } 
    else if (message.find("ERROR%%") == 0) {
        std::cout << "[ERROR] " << message.substr(7) << std::endl;
    }
    else if (message.find("info%%") == 0) {
        size_t firstDelim = message.find("%%", 6);
        if (firstDelim != std::string::npos) {
            std::string info = message.substr(6, firstDelim - 6);
            std::string details = message.substr(firstDelim + 2);
            std::cout << "[INFO] " << info << " " << details << std::endl;
        } else {
            std::cout << "[INFO] " << message.substr(6) << std::endl;
        }
    }
    else if (message.find("%%join%%") == 0) {
        std::string room = message.substr(8);
        currentRoom = room;
        std::cout << "✓ Joined room: #" << room << std::endl;
    }
    else if (message.find("%%leave%%") == 0) {
        std::string room = message.substr(9);
        if (currentRoom == room) {
            currentRoom.clear();
        }
        std::cout << "✓ Left room: #" << room << std::endl;
    }
    else if (message.find("%%Chat List%%") == 0) {
        std::cout << "=== Available Chat Rooms ===" << std::endl;
        std::cout << message.substr(15) << std::endl;
    }
    else if (message.find("%%people%%") == 0) {
        size_t delimPos = message.find('\n');
        if (delimPos != std::string::npos) {
            std::string room = message.substr(10, delimPos - 10);
            std::string people = message.substr(delimPos + 1);
            std::cout << "=== People in #" << room << " ===" << std::endl;
            std::cout << people;
        }
    }
    else {
        std::cout << message << std::endl;
    }
    
    printPrompt();
}

// Print welcome message
void printWelcome() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "       Welcome to LAN Chat!             " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\nLogged in as: " << currentUsername << std::endl;
    printHelp();
}

// Print help information
void printHelp() {
    std::cout << "\n--- Commands -------------------" << std::endl;
    std::cout << " /join <room>       Join a chat room" << std::endl;
    std::cout << " /leave <room>      Leave a room" << std::endl;
    std::cout << " /rooms             List all rooms" << std::endl;
    std::cout << " /people <room>     List room users" << std::endl;
    std::cout << " /msg <room> <text> Send to room" << std::endl;
    std::cout << " @user <text>       Private message" << std::endl;
    std::cout << "                    (use in /msg)" << std::endl;
    std::cout << " /help              Show this help" << std::endl;
    std::cout << " /quit              Exit chat" << std::endl;
    std::cout << "--------------------------------\n" << std::endl;
    std::cout << "Tip: After joining a room, type messages directly without /msg\n" << std::endl;
}

// Print command prompt
void printPrompt() {
    if (!currentRoom.empty()) {
        std::cout << "[#" << currentRoom << "] > " << std::flush;
    } else {
        std::cout << "> " << std::flush;
    }
}

// Process user command
bool processCommand(int sockfd, const std::string& input) {
    // Check for quit command
    if (input == "/quit" || input == "/exit") {
        isRunning = false;
        return false;
    }
    
    // Help command
    if (input == "/help" || input == "?") {
        printHelp();
        return true;
    }
    
    // List rooms
    if (input == "/rooms" || input == "/list") {
        std::string msg = "LIST_ROOMS";
        send(sockfd, msg.c_str(), msg.length(), 0);
        return true;
    }
    
    // Join room
    if (input.find("/join ") == 0) {
        if (input.length() <= 6) {
            std::cout << "Usage: /join <room_name>" << std::endl;
            return true;
        }
        
        std::string room = input.substr(6);
        // Remove leading/trailing spaces
        size_t start = room.find_first_not_of(" \t");
        size_t end = room.find_last_not_of(" \t");
        
        if (start == std::string::npos) {
            std::cout << "Usage: /join <room_name>" << std::endl;
            return true;
        }
        
        room = room.substr(start, end - start + 1);
        std::string msg = "JOIN%%" + room;
        send(sockfd, msg.c_str(), msg.length(), 0);
        return true;
    }
    
    // Leave room
    if (input.find("/leave ") == 0) {
        if (input.length() <= 7) {
            std::cout << "Usage: /leave <room_name>" << std::endl;
            return true;
        }
        
        std::string room = input.substr(7);
        size_t start = room.find_first_not_of(" \t");
        size_t end = room.find_last_not_of(" \t");
        
        if (start == std::string::npos) {
            std::cout << "Usage: /leave <room_name>" << std::endl;
            return true;
        }
        
        room = room.substr(start, end - start + 1);
        std::string msg = "LEAVE%%" + room;
        send(sockfd, msg.c_str(), msg.length(), 0);
        return true;
    }
    
    // List people in room
    if (input.find("/people ") == 0) {
        if (input.length() <= 8) {
            std::cout << "Usage: /people <room_name>" << std::endl;
            return true;
        }
        
        std::string room = input.substr(8);
        size_t start = room.find_first_not_of(" \t");
        size_t end = room.find_last_not_of(" \t");
        
        if (start == std::string::npos) {
            std::cout << "Usage: /people <room_name>" << std::endl;
            return true;
        }
        
        room = room.substr(start, end - start + 1);
        std::string msg = "PEOPLE%%" + room;
        send(sockfd, msg.c_str(), msg.length(), 0);
        return true;
    }
    
    // Send message to room
    if (input.find("/msg ") == 0) {
        if (input.length() <= 5) {
            std::cout << "Usage: /msg <room> <message>" << std::endl;
            return true;
        }
        
        size_t spacePos = input.find(' ', 5);
        if (spacePos == std::string::npos) {
            std::cout << "Usage: /msg <room> <message>" << std::endl;
            return true;
        }
        
        std::string room = input.substr(5, spacePos - 5);
        std::string text = input.substr(spacePos + 1);
        
        if (text.empty()) {
            std::cout << "Usage: /msg <room> <message>" << std::endl;
            return true;
        }
        
        std::string msg = room + "%%" + currentUsername + "%%" + text;
        send(sockfd, msg.c_str(), msg.length(), 0);
        return true;
    }
    
    // Send message to current room (shortcut)
    if (!currentRoom.empty() && input[0] != '/') {
        std::string msg = currentRoom + "%%" + currentUsername + "%%" + input;
        send(sockfd, msg.c_str(), msg.length(), 0);
        return true;
    }
    
    // Unknown command
    std::cout << "Unknown command: " << input << std::endl;
    std::cout << "Type /help for available commands" << std::endl;
    return true;
}

// Utility function to split string
std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}