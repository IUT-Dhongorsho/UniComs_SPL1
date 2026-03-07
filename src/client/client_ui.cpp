#include "client.h"
#include "../ui/UIManager.hpp"
#include "../ui/Components/FileTransferUI.hpp"
#include "../ui/Animations.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <mutex>
#include <sys/stat.h>
#include <sstream>

// Helper functions for file operations
inline bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

inline size_t getFileSize(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        return buffer.st_size;
    }
    return 0;
}

inline std::string getFileName(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

class UIClient {
private:
    int socket_fd;
    std::atomic<bool> running{true};
    std::atomic<bool> connected{false};
    std::unique_ptr<UIManager> ui;
    std::string username;
    std::map<std::string, std::vector<UIMessage>> chatHistory;
    std::string currentChat;
    std::string incomingFilename;
    std::mutex msgMutex;
    
    // Login state
    bool isLoggedIn = false;
    std::string loginUsername;
    std::string loginPassword;
    bool waitingForLoginResponse = false;
    
    void networkLoop() {
        while (running) {
            std::string line = recvLine(socket_fd);
            if (line.empty()) {
                if (connected) {
                    connected = false;
                    UIMessage msg;
                    msg.id = generateId();
                    msg.sender = "System";
                    msg.content = "Disconnected from server";
                    msg.timestamp = currentTimestamp();
                    msg.isOwn = false;
                    msg.isDelivered = false;
                    msg.isRead = false;
                    msg.type = MessageType::SYSTEM;
                    ui->addMessage(msg);
                }
                break;
            }
            
            if (!connected) {
                connected = true;
                UIMessage msg;
                msg.id = generateId();
                msg.sender = "System";
                msg.content = "Connected to server";
                msg.timestamp = currentTimestamp();
                msg.isOwn = false;
                msg.isDelivered = false;
                msg.isRead = false;
                msg.type = MessageType::SYSTEM;
                ui->addMessage(msg);
                
                // Show login prompt
                showLoginScreen();
            }
            
            processServerMessage(line);
        }
    }
    
    void showLoginScreen() {
        UIMessage msg;
        msg.id = generateId();
        msg.sender = "System";
        msg.content = "Please login or signup:\n"
                     "  /login username password\n"
                     "  /signup username password\n"
                     "  /help for commands";
        msg.timestamp = currentTimestamp();
        msg.isOwn = false;
        msg.isDelivered = false;
        msg.isRead = false;
        msg.type = MessageType::SYSTEM;
        ui->addMessage(msg);
    }
    
    void showHelp() {
        UIMessage msg;
        msg.id = generateId();
        msg.sender = "System";
        msg.content = "Commands:\n"
                     "  /login <user> <pass>\n"
                     "  /signup <user> <pass>\n"
                     "  /logout\n"
                     "  /dm <user> <msg>\n"
                     "  /join <room>\n"
                     "  /leave <room>\n"
                     "  /msg <room> <msg>\n"
                     "  /create <room>\n"
                     "  /list\n"
                     "  /users\n"
                     "  /members <room>\n"
                     "  /history_dm <user>\n"
                     "  /history_room <room>\n"
                     "  /file <user> <path>\n"
                     "  /clear\n"
                     "  /help";
        msg.timestamp = currentTimestamp();
        msg.isOwn = false;
        msg.type = MessageType::SYSTEM;
        ui->addMessage(msg);
    }
    
    void processServerMessage(const std::string& line) {
        auto parts = splitMessage(line, ' ', 3);
        if (parts.empty()) return;
        
        std::string cmd = parts[0];
        
        // Handle login responses
        if (cmd == "OK" && waitingForLoginResponse) {
            waitingForLoginResponse = false;
            isLoggedIn = true;
            username = loginUsername;
            ui->setCurrentUser(username);
            
            UIMessage msg;
            msg.id = generateId();
            msg.sender = "System";
            msg.content = "Login successful! Welcome " + username;
            msg.timestamp = currentTimestamp();
            msg.isOwn = false;
            msg.type = MessageType::SYSTEM;
            ui->addMessage(msg);
            
            // Get online users
            sendLine(socket_fd, "LIST_USERS");
            return;
        }
        
        if (line.find("ERR") == 0 && waitingForLoginResponse) {
            waitingForLoginResponse = false;
            UIMessage msg;
            msg.id = generateId();
            msg.sender = "System";
            msg.content = "Error: " + line;
            msg.timestamp = currentTimestamp();
            msg.isOwn = false;
            msg.type = MessageType::SYSTEM;
            ui->addMessage(msg);
            return;
        }
        
        // Handle user list
        if (line.find("INFO Users:") == 0) {
            updateContactsList(line);
            return;
        }
        
        // Handle DM messages
        if (cmd == "MSG_FROM" && parts.size() >= 3) {
            std::string from = parts[1];
            std::string content = parts[2];
            
            UIMessage msg;
            msg.id = generateId();
            msg.sender = from;
            msg.content = content;
            msg.timestamp = currentTimestamp();
            msg.isOwn = false;
            msg.isDelivered = true;
            msg.isRead = false;
            msg.type = MessageType::TEXT;
            
            // Store in history
            {
                std::lock_guard<std::mutex> lock(msgMutex);
                chatHistory[from].push_back(msg);
            }
            
            // Show message if in this chat
            if (currentChat == from) {
                ui->addMessage(msg);
            }
            
            // Update contact unread count
            std::vector<UIContact> contacts;
            UIContact contact;
            contact.id = from;
            contact.username = from;
            contact.isOnline = true;
            contact.isTyping = false;
            contact.unreadCount = (currentChat != from) ? 1 : 0;
            contacts.push_back(contact);
            ui->updateContacts(contacts);
            
            return;
        }
        
        // Handle room messages
        if (cmd == "ROOM_MSG" && parts.size() >= 4) {
            std::string room = parts[1];
            std::string from = parts[2];
            std::string content = parts[3];
            
            UIMessage msg;
            msg.id = generateId();
            msg.sender = from;
            msg.content = content;
            msg.timestamp = currentTimestamp();
            msg.isOwn = (from == username);
            msg.isDelivered = true;
            msg.isRead = false;
            msg.type = MessageType::TEXT;
            
            {
                std::lock_guard<std::mutex> lock(msgMutex);
                chatHistory[room].push_back(msg);
            }
            
            // Show message if in this room
            if (currentChat == room) {
                ui->addMessage(msg);
            }
            
            return;
        }
        
        // Handle file offers
        if (cmd == "FILE_OFFER" && parts.size() >= 4) {
            std::string from = parts[1];
            std::string filename = parts[2];
            size_t filesize = std::stoull(parts[3]);
            
            UIMessage msg;
            msg.id = generateId();
            msg.sender = from;
            msg.content = "File: " + filename + " (" + std::to_string(filesize) + " bytes)";
            msg.timestamp = currentTimestamp();
            msg.isOwn = false;
            msg.isDelivered = true;
            msg.isRead = false;
            msg.type = MessageType::FILE;
            
            ui->addMessage(msg);
            ui->showFileProgress(filename, 0, 0);
            return;
        }
        
        // Handle room list
        if (line.find("INFO Rooms:") == 0) {
            UIMessage msg;
            msg.id = generateId();
            msg.sender = "System";
            msg.content = line;
            msg.timestamp = currentTimestamp();
            msg.isOwn = false;
            msg.type = MessageType::SYSTEM;
            ui->addMessage(msg);
            return;
        }
        
        // Default: show message in chat
        if (!line.empty()) {
            UIMessage msg;
            msg.id = generateId();
            msg.sender = "Server";
            msg.content = line;
            msg.timestamp = currentTimestamp();
            msg.isOwn = false;
            msg.type = MessageType::SYSTEM;
            ui->addMessage(msg);
        }
    }
    
    void updateContactsList(const std::string& line) {
        std::vector<UIContact> contacts;
        
        // Parse "INFO Users: user1 user2 user3"
        size_t pos = line.find("Users:");
        if (pos != std::string::npos) {
            std::string users = line.substr(pos + 6);
            std::istringstream iss(users);
            std::string user;
            
            while (iss >> user) {
                if (!user.empty() && user != username) {
                    UIContact contact;
                    contact.id = user;
                    contact.username = user;
                    contact.isOnline = true;
                    contact.isTyping = false;
                    contact.unreadCount = 0;
                    contacts.push_back(contact);
                }
            }
        }
        
        ui->updateContacts(contacts);
    }
    
    void handleCommand(const std::string& cmd) {
        auto parts = splitMessage(cmd, ' ', 3);
        if (parts.empty()) return;
        
        std::string command = parts[0];
        
        if (command == "/login" && parts.size() >= 3) {
            loginUsername = parts[1];
            loginPassword = parts[2];
            waitingForLoginResponse = true;
            sendLine(socket_fd, "LOGIN " + loginUsername + " " + loginPassword);
            
            UIMessage msg;
            msg.id = generateId();
            msg.sender = "System";
            msg.content = "Logging in...";
            msg.timestamp = currentTimestamp();
            msg.isOwn = false;
            msg.type = MessageType::SYSTEM;
            ui->addMessage(msg);
        }
        else if (command == "/signup" && parts.size() >= 3) {
            sendLine(socket_fd, "SIGNUP " + parts[1] + " " + parts[2]);
            
            UIMessage msg;
            msg.id = generateId();
            msg.sender = "System";
            msg.content = "Creating account...";
            msg.timestamp = currentTimestamp();
            msg.isOwn = false;
            msg.type = MessageType::SYSTEM;
            ui->addMessage(msg);
        }
        else if (command == "/logout") {
            if (isLoggedIn) {
                sendLine(socket_fd, "LOGOUT");
                isLoggedIn = false;
                username = "";
                ui->setCurrentUser("");
                
                UIMessage msg;
                msg.id = generateId();
                msg.sender = "System";
                msg.content = "Logged out successfully";
                msg.timestamp = currentTimestamp();
                msg.isOwn = false;
                msg.type = MessageType::SYSTEM;
                ui->addMessage(msg);
                
                showLoginScreen();
            }
        }
        else if (command == "/help") {
            showHelp();
        }
        else if (command == "/clear") {
            ui->clearChat();
        }
        else if (command == "/list") {
            sendLine(socket_fd, "LIST_ROOMS");
        }
        else if (command == "/users") {
            sendLine(socket_fd, "LIST_USERS");
        }
        else if (command == "/dm" && parts.size() >= 3) {
            if (!isLoggedIn) {
                ui->addMessage(createSystemMessage("Please login first"));
                return;
            }
            std::string to = parts[1];
            std::string msg = parts[2];
            sendLine(socket_fd, "DM " + to + " " + msg);
            
            // Add to chat history
            UIMessage umsg;
            umsg.id = generateId();
            umsg.sender = username;
            umsg.content = msg;
            umsg.timestamp = currentTimestamp();
            umsg.isOwn = true;
            umsg.isDelivered = false;
            umsg.isRead = false;
            umsg.type = MessageType::TEXT;
            
            {
                std::lock_guard<std::mutex> lock(msgMutex);
                chatHistory[to].push_back(umsg);
            }
            
            // Show in current chat
            if (currentChat == to) {
                ui->addMessage(umsg);
            }
            
            // Set current chat to this person
            currentChat = to;
            ui->setCurrentChat(to);
        }
        else if (command == "/join" && parts.size() >= 2) {
            if (!isLoggedIn) {
                ui->addMessage(createSystemMessage("Please login first"));
                return;
            }
            std::string room = parts[1];
            sendLine(socket_fd, "JOIN " + room);
            currentChat = room;
            ui->setCurrentChat(room);
        }
        else if (command == "/leave" && parts.size() >= 2) {
            if (!isLoggedIn) return;
            sendLine(socket_fd, "LEAVE " + parts[1]);
        }
        else if (command == "/msg" && parts.size() >= 3) {
            if (!isLoggedIn) {
                ui->addMessage(createSystemMessage("Please login first"));
                return;
            }
            std::string room = parts[1];
            std::string msg = parts[2];
            sendLine(socket_fd, "MSG " + room + " " + msg);
            
            // Add to chat history
            UIMessage umsg;
            umsg.id = generateId();
            umsg.sender = username;
            umsg.content = msg;
            umsg.timestamp = currentTimestamp();
            umsg.isOwn = true;
            umsg.isDelivered = false;
            umsg.isRead = false;
            umsg.type = MessageType::TEXT;
            
            {
                std::lock_guard<std::mutex> lock(msgMutex);
                chatHistory[room].push_back(umsg);
            }
            
            if (currentChat == room) {
                ui->addMessage(umsg);
            }
            
            currentChat = room;
            ui->setCurrentChat(room);
        }
        else if (command == "/create" && parts.size() >= 2) {
            if (!isLoggedIn) {
                ui->addMessage(createSystemMessage("Please login first"));
                return;
            }
            sendLine(socket_fd, "CREATE_ROOM " + parts[1]);
        }
        else if (command == "/members" && parts.size() >= 2) {
            if (!isLoggedIn) return;
            sendLine(socket_fd, "LIST_MEMBERS " + parts[1]);
        }
        else if (command == "/history_dm" && parts.size() >= 2) {
            if (!isLoggedIn) return;
            sendLine(socket_fd, "HISTORY_DM " + parts[1]);
        }
        else if (command == "/history_room" && parts.size() >= 2) {
            if (!isLoggedIn) return;
            sendLine(socket_fd, "HISTORY_ROOM " + parts[1]);
        }
        else if (command == "/file" && parts.size() >= 3) {
            if (!isLoggedIn) {
                ui->addMessage(createSystemMessage("Please login first"));
                return;
            }
            std::string to = parts[1];
            std::string filepath = parts[2];
            
            if (!fileExists(filepath)) {
                ui->addMessage(createSystemMessage("File not found: " + filepath));
                return;
            }
            
            size_t filesize = getFileSize(filepath);
            sendLine(socket_fd, "FILE_SEND " + to + " " + 
                     getFileName(filepath) + " " + 
                     std::to_string(filesize));
            
            ui->addMessage(createSystemMessage("Sending file to " + to + "..."));
        }
        else if (command == "/accept") {
            sendLine(socket_fd, "FILE_ACCEPT");
        }
        else if (command == "/reject") {
            sendLine(socket_fd, "FILE_REJECT");
        }
        else if (command[0] == '/') {
            ui->addMessage(createSystemMessage("Unknown command: " + command));
        }
        else if (!command.empty() && isLoggedIn && !currentChat.empty()) {
            // Regular message to current chat
            if (currentChat.find('@') == std::string::npos) {
                // It's a room
                sendLine(socket_fd, "MSG " + currentChat + " " + cmd);
            } else {
                // It's a DM
                sendLine(socket_fd, "DM " + currentChat + " " + cmd);
            }
            
            UIMessage umsg;
            umsg.id = generateId();
            umsg.sender = username;
            umsg.content = cmd;
            umsg.timestamp = currentTimestamp();
            umsg.isOwn = true;
            umsg.isDelivered = false;
            umsg.isRead = false;
            umsg.type = MessageType::TEXT;
            
            {
                std::lock_guard<std::mutex> lock(msgMutex);
                chatHistory[currentChat].push_back(umsg);
            }
            
            ui->addMessage(umsg);
        }
        else if (!isLoggedIn) {
            ui->addMessage(createSystemMessage("Please login first. Use /login or /signup"));
        }
    }
    
    UIMessage createSystemMessage(const std::string& content) {
        UIMessage msg;
        msg.id = generateId();
        msg.sender = "System";
        msg.content = content;
        msg.timestamp = currentTimestamp();
        msg.isOwn = false;
        msg.isDelivered = false;
        msg.isRead = false;
        msg.type = MessageType::SYSTEM;
        return msg;
    }
    
public:
    UIClient(int fd) : socket_fd(fd) {
        ui = std::make_unique<UIManager>();
        
        // Setup callbacks
        ui->setMessageCallback([this](const std::string& msg) {
            handleCommand(msg);
        });
        
        ui->setCommandCallback([this](const std::string& cmd) {
            handleCommand(cmd);
        });
        
        ui->setFileCallback([this](const std::string& username, const std::string& filepath) {
            sendLine(socket_fd, "FILE_SEND " + username + " " + filepath);
        });
        
        // Contact selection callback
        ui->setContactCallback([this](const std::string& username) {
            currentChat = username;
            ui->setCurrentChat(username);
            
            // Load chat history
            auto it = chatHistory.find(username);
            if (it != chatHistory.end()) {
                ui->clearChat();
                for (const auto& msg : it->second) {
                    ui->addMessage(msg);
                }
            }
        });
    }
    
    void run() {
        // Start network thread
        std::thread networkThread(&UIClient::networkLoop, this);
        
        // Small delay to establish connection
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Initialize and run UI
        ui->initialize();
        
        // Show welcome message
        UIMessage welcome;
        welcome.id = generateId();
        welcome.sender = "System";
        welcome.content = "Welcome to UniComs Chat! Type /help";
        welcome.timestamp = currentTimestamp();
        welcome.isOwn = false;
        welcome.type = MessageType::SYSTEM;
        ui->addMessage(welcome);
        
        ui->run();
        
        // Cleanup
        running = false;
        if (connected) {
            shutdown(socket_fd, SHUT_RDWR);
        }
        ::close(socket_fd);
        networkThread.join();
    }
    
    void setUsername(const std::string& name) {
        username = name;
        if (ui) ui->setCurrentUser(name);
    }
};

// ***** MAKE SURE THIS main() FUNCTION IS PRESENT *****
int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 8080;
    
    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);
    
    std::cout << "Connecting to " << host << ":" << port << "...\n";
    
    int fd = clientConnect(host, port);
    if (fd < 0) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        std::cerr << "Error: " << strerror(errno) << std::endl;
        return 1;
    }
    
    UIClient client(fd);
    client.run();
    
    return 0;
}