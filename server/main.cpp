#include <iostream>
#include <csignal>
#include <thread>
#include "core/Server.h"
#include "network/Listener.h"
#include "auth/AuthManager.h"
#include "routing/MessageRouter.h"
#include "groups/GroupManager.h"
#include "storage/Storage.h"
#include "../common/utils/Logger.h"

// Global pointer to handle graceful shutdown
Server* g_server = nullptr;

void signalHandler(int signum) {
    Logger::info("[SERVER] Signal received. Shutting down...");
    if (g_server) {
        g_server->stop();
    }
    exit(signum);
}

int main() {
    // Register signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    Logger::info("[SERVER] Starting CLI Chat Server...");

    // Server configuration
    const int port = 5000;

    try {
        // Initialize storage
        Storage storage("storage/data/");

        // Initialize auth manager
        AuthManager authManager;

        // Initialize group manager
        GroupManager groupManager;

        // Initialize message router
        MessageRouter router;
        
        // Initialize server
        Server server(port);
        g_server = &server;

        Logger::info("[SERVER] Listening on port " + std::to_string(port));
        
        // Start the server (blocking call)
        server.start();

    } catch (const std::exception& e) {
        Logger::error("[SERVER] Exception: " + std::string(e.what()));
        return 1;
    } catch (...) {
        Logger::error("[SERVER] Unknown exception occurred!");
        return 1;
    }

    return 0;
}
