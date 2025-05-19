#include "../include/ChatServer.h"
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") // Only needed if compiling with MSVC


int main() {
    ChatServer server(5555);
    if (!server.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    // Set callback to handle incoming messages
    server.setOnMessageReceived([&server](int clientId, const std::string& msg) {
        std::cout << "[Client " << clientId << "] " << msg << "\n";

        // Example: Echo message back to client
        server.sendMessage(clientId, "Server received: " + msg);
    });

    std::string line;
    while (true) {
        std::cout << "Enter client ID to message (or 'exit'): ";
        std::getline(std::cin, line);
        if (line == "exit") break;

        int clientId = std::stoi(line);
        std::cout << "Enter message: ";
        std::getline(std::cin, line);

        server.sendMessage(clientId, line);
    }

    server.stop();
    return 0;
}
