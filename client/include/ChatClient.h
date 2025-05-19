#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") // Only needed if compiling with MSVC

#include <string>
#include <functional>

class ChatClient {
public:
    using MessageCallback = std::function<void(const std::string& message)>;

    ChatClient(const std::string& serverIp, int serverPort);
    ~ChatClient();

    bool connectToServer();
    void disconnect();

    void sendMessage(const std::string& message);
    void startReceiving();

    void enableAutoReconnect(bool enable);

    void setOnMessageReceived(MessageCallback callback);

private:
    std::string serverIp_;
    int serverPort_;
    int sock_;
    bool connected_;
    bool autoReconnect_;

    MessageCallback onMessageReceived_;

    void receiveLoop();
    std::string encrypt(const std::string& data);
    std::string decrypt(const std::string& data);

    void clearBuffer(char* buffer, int size);
};

