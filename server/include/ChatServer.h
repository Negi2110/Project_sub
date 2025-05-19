#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <winsock2.h>  // For SOCKET type

class ChatServer {
public:
    using MessageCallback = std::function<void(int clientId, const std::string& message)>;

    explicit ChatServer(int port);
    ~ChatServer();

    bool start();
    void stop();

    void sendMessage(int clientId, const std::string& message);
    void broadcastMessage(const std::string& message);

    void setOnMessageReceived(MessageCallback callback);

private:
    int port_;
    SOCKET serverSocket_;  // Use SOCKET type here
    std::unordered_map<int, SOCKET> clientSockets_;

    int nextClientId_;
    bool running_;

    void acceptClients();
    void clientHandler(SOCKET clientSocket, int clientId);  // SOCKET type here

    std::string encrypt(const std::string& data);
    std::string decrypt(const std::string& data);

    void clearBuffer(char* buffer, int size);

    MessageCallback onMessageReceived_;
};

#endif // CHATSERVER_H

