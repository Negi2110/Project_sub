#pragma once
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") // Only needed if compiling with MSVC

class MessageHandler {
public:
    static std::string encrypt(const std::string& plainText);
    static std::string decrypt(const std::string& cipherText);
};
