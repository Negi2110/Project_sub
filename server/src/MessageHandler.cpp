#include "MessageHandler.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") // Only needed if compiling with MSVC

// Simple XOR cipher for demonstration; replace with real encryption in prod.
static const char XOR_KEY = 'K';

std::string MessageHandler::encrypt(const std::string& plainText) {
    std::string result = plainText;
    for (auto& c : result) c ^= XOR_KEY;
    return result;
}

std::string MessageHandler::decrypt(const std::string& cipherText) {
    std::string result = cipherText;
    for (auto& c : result) c ^= XOR_KEY;
    return result;
}
