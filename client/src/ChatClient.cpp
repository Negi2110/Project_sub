#include "ChatClient.h"
#include "MessageHandler.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>

#include "../../libs/json.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") // Only needed if compiling with MSVC

using json = nlohmann::json;

ChatClient::ChatClient(const std::string& serverIp, int serverPort)
    : serverIp_(serverIp), serverPort_(serverPort), sock_(-1),
      connected_(false), autoReconnect_(false) {}

ChatClient::~ChatClient() {
    disconnect();
}

bool ChatClient::connectToServer() {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort_);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp_.c_str());
if (serverAddr.sin_addr.s_addr == INADDR_NONE) {
    std::cerr << "Invalid address/ Address not supported\n";
    return false;
}


    if (connect(sock_, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sock_);
        return false;
    }

    connected_ = true;

    // Receive welcome message with client ID
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytes = recv(sock_, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        std::string decrypted = decrypt(std::string(buffer, bytes));
        try {
            json j = json::parse(decrypted);
            if (j.contains("type") && j["type"] == "welcome" && j.contains("clientId")) {
                std::cout << "[Server] Welcome! Your ID is " << j["clientId"] << "\n";
            }
        } catch (...) {
            std::cerr << "Failed to parse welcome message\n";
        }
    }
    return true;
}

void ChatClient::disconnect() {
    if (connected_) {
        close(sock_);
        connected_ = false;
    }
}

void ChatClient::sendMessage(const std::string& message) {
    if (!connected_) return;

    json j;
    j["type"] = "message";
    j["message"] = message;
    std::string payload = encrypt(j.dump());

    send(sock_, payload.c_str(), payload.size(), 0);
}

void ChatClient::startReceiving() {
    std::thread([this]() { receiveLoop(); }).detach();
}

void ChatClient::receiveLoop() {
    char buffer[1024];
    while (connected_) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(sock_, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cerr << "[Client] Connection lost.\n";
            if (autoReconnect_) {
                std::cerr << "[Client] Attempting reconnect...\n";
                disconnect();
                while (!connectToServer()) {
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                }
                std::cerr << "[Client] Reconnected.\n";
            } else {
                break;
            }
        } else {
            std::string decrypted = decrypt(std::string(buffer, bytesReceived));
            try {
                json j = json::parse(decrypted);
                if (j.contains("type") && j["type"] == "message" && j.contains("message")) {
                    if (onMessageReceived_) {
                        onMessageReceived_(j["message"]);
                    } else {
                        std::cout << "[Server] " << j["message"] << "\n";
                    }
                }
            } catch (...) {
                std::cerr << "Failed to parse incoming message\n";
            }
        }
    }
}

void ChatClient::enableAutoReconnect(bool enable) {
    autoReconnect_ = enable;
}

void ChatClient::setOnMessageReceived(MessageCallback callback) {
    onMessageReceived_ = callback;
}

std::string ChatClient::encrypt(const std::string& data) {
    return MessageHandler::encrypt(data);
}

std::string ChatClient::decrypt(const std::string& data) {
    return MessageHandler::decrypt(data);
}

void ChatClient::clearBuffer(char* buffer, int size) {
    memset(buffer, 0, size);
}

