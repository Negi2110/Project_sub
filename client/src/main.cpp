#include "../include/ChatClient.h"
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") // Only needed if compiling with MSVC

int main() {
       WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (wsaResult != 0) {
        std::cerr << "WSAStartup failed: " << wsaResult << "\n";
        return 1;
    }
    ChatClient client("127.0.0.1", 5555);
    client.enableAutoReconnect(true);

    if (!client.connectToServer()) {
        std::cerr << "Initial connection failed. Will retry...\n";
    }

    client.setOnMessageReceived([](const std::string& msg) {
        std::cout << "[Server] " << msg << "\n";
    });

    client.startReceiving();

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "exit") break;
        client.sendMessage(input);
    }

    client.disconnect();
    WSACleanup();
    return 0;
}

