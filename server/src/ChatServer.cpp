#include "ChatServer.h"
#include "MessageHandler.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // Link Winsock library
#include "../../libs/json.hpp"  // path to your json.hpp file
using json = nlohmann::json;


static std::mutex clientMapMutex;

ChatServer::ChatServer(int port)
    : port_(port), serverSocket_(INVALID_SOCKET), nextClientId_(1), running_(false) {}

ChatServer::~ChatServer() {
    stop();
}

bool ChatServer::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    serverSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "setsockopt failed\n";
        closesocket(serverSocket_);
        WSACleanup();
        return false;
    }

    if (bind(serverSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket_);
        WSACleanup();
        return false;
    }

    if (listen(serverSocket_, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket_);
        WSACleanup();
        return false;
    }

    running_ = true;
    std::thread(&ChatServer::acceptClients, this).detach();
    std::cout << "Server started on port " << port_ << "\n";
    return true;
}

void ChatServer::stop() {
    running_ = false;

    if (serverSocket_ != INVALID_SOCKET) {
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET;
    }

    // Close all client sockets
    std::lock_guard<std::mutex> lock(clientMapMutex);
    for (auto& [id, sock] : clientSockets_) {
        closesocket(sock);
    }
    clientSockets_.clear();

    WSACleanup();
}

void ChatServer::acceptClients() {
    while (running_) {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        SOCKET clientSock = accept(serverSocket_, (sockaddr*)&clientAddr, &addrLen);
        if (clientSock == INVALID_SOCKET) {
            if (running_) std::cerr << "Accept failed\n";
            continue;
        }

        int clientId;
        {
            std::lock_guard<std::mutex> lock(clientMapMutex);
            clientId = nextClientId_++;
            clientSockets_[clientId] = clientSock;
        }

        // Send welcome message with assigned client ID
        json welcomeJson;
        welcomeJson["type"] = "welcome";
        welcomeJson["clientId"] = clientId;
        std::string welcomeStr = welcomeJson.dump();
        send(clientSock, encrypt(welcomeStr).c_str(), (int)encrypt(welcomeStr).length(), 0);

        std::cout << "Client " << clientId << " connected.\n";

        std::thread(&ChatServer::clientHandler, this, clientSock, clientId).detach();
    }
}

void ChatServer::clientHandler(SOCKET clientSocket, int clientId) {
    char buffer[1024];
    while (running_) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cout << "Client " << clientId << " disconnected.\n";
            closesocket(clientSocket);
            std::lock_guard<std::mutex> lock(clientMapMutex);
            clientSockets_.erase(clientId);
            break;
        }
        std::string decrypted = decrypt(std::string(buffer, bytesReceived));

        try {
            json j = json::parse(decrypted);
            if (j.contains("type") && j["type"] == "message" && j.contains("message")) {
                std::string msg = j["message"];
                if (onMessageReceived_) {
                    onMessageReceived_(clientId, msg);
                }
            }
        } catch (...) {
            std::cerr << "JSON parse error from client " << clientId << "\n";
        }
    }
}

void ChatServer::sendMessage(int clientId, const std::string& message) {
    std::lock_guard<std::mutex> lock(clientMapMutex);
    auto it = clientSockets_.find(clientId);
    if (it == clientSockets_.end()) return;

    json j;
    j["type"] = "message";
    j["message"] = message;
    std::string payload = encrypt(j.dump());

    send(it->second, payload.c_str(), (int)payload.size(), 0);
}

void ChatServer::broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(clientMapMutex);
    for (const auto& [id, sock] : clientSockets_) {
        json j;
        j["type"] = "message";
        j["message"] = message;
        std::string payload = encrypt(j.dump());
        send(sock, payload.c_str(), (int)payload.size(), 0);
    }
}

void ChatServer::setOnMessageReceived(MessageCallback callback) {
    onMessageReceived_ = callback;
}

std::string ChatServer::encrypt(const std::string& data) {
    return MessageHandler::encrypt(data);
}

std::string ChatServer::decrypt(const std::string& data) {
    return MessageHandler::decrypt(data);
}

void ChatServer::clearBuffer(char* buffer, int size) {
    memset(buffer, 0, size);
}



