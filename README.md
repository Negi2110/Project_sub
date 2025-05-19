🧾 Project Documentation: Real-Time Chat System (C++ with Winsock & JSON)
  
  🔷 Overview
   This is a real-time, multi-client chat system implemented in C++ using Winsock for socket programming and nlohmann::json for structured message handling. The system consists of two main components:

   Client Module: Connects to the server, sends/receives encrypted JSON messages, supports auto-reconnect, and provides clean message output.

   Server Module: Accepts multiple clients, assigns client IDs, receives JSON messages, and routes them to specific or all clients.


🔹 Client Module
   📁 Structure

   ChatApp/
    └── client/
        ├── include/
        │   ├── ChatClient.h
        │   └── MessageHandler.h
        ├── src/
        │   ├── ChatClient.cpp
        │   ├── MessageHandler.cpp
        │   └── main.cpp
🧠 Responsibilities
-Connects to the server using IP and port.

-Receives a welcome message with assigned client ID.

-Sends and receives encrypted messages in JSON format.

-Auto-reconnects if connection is lost.

-Uses MessageHandler for encryption/decryption.

-Spawns a receiving thread for asynchronous message handling.   

🛠️ Key Class: ChatClient
   | Method                           | Description                                            |
| -------------------------------- | ------------------------------------------------------ |
| `connectToServer()`              | Initiates socket connection, receives welcome JSON     |
| `sendMessage(msg)`               | Sends a message (internally JSON)                      |
| `startReceiving()`               | Starts a thread that continuously listens for messages |
| `setOnMessageReceived(callback)` | Sets a callback to handle incoming messages            |
| `disconnect()`                   | Closes the socket                                      |
| `enableAutoReconnect(flag)`      | Enables retry if server disconnects                    |

📤 Message Format (Outgoing)
  {
  "type": "message",
  "message": "Hello!"
  }
📥 Message Format (Incoming)
  {
  "type": "message",
  "message": "Welcome to the server!"
  }
📬 Welcome Message
  {
  "type": "welcome",
  "clientId": 1
  }


  🔹 Server Module

   📁 Structure
   ChatApp/
   └── server/
       ├── include/
       │   ├── ChatServer.h
       │   └── MessageHandler.h
       ├── src/
       │   ├── ChatServer.cpp
       │   ├── MessageHandler.cpp
       │   └── main.cpp


🧠 Responsibilities
-Listens on a TCP port and accepts multiple clients.

-Assigns unique client IDs to each connected socket.

-Sends a welcome JSON message upon connection.

-Receives JSON-formatted messages from clients.

-Allows server admin to send messages to any connected client via ID.

-Uses MessageHandler for secure communication.

🛠️ Key Class: ChatServer
| Method                                   | Description                                                    |
| ---------------------------------------- | -------------------------------------------------------------- |
| `start()`                                | Initializes Winsock, binds, listens, accepts clients in thread |
| `handleClient(clientSocket, clientId)`   | Per-client communication handler                               |
| `sendMessageToClient(clientId, message)` | Sends JSON message to specified client                         |
| `broadcastMessage(message)`              | Sends to all clients                                           |
| `disconnectClient(clientId)`             | Gracefully closes a client's connection                        |

📤 Welcome Message Sent to Client
   {
  "type": "welcome",
  "clientId": 2
   }
📥 Expected Client Message Format
   {
  "type": "message",
  "message": "Hey Server!"
   }

🔐 MessageHandler (Used by Both Modules)
    Responsibilities
    -Abstracts message encryption/decryption.
    -Optional XOR-style obfuscation (can be upgraded to AES/RSA).
    -Ensures safe transport of JSON strings.
   
   std::string MessageHandler::encrypt(const std::string& plainText);
   std::string MessageHandler::decrypt(const std::string& cipherText);

🔧 Dependencies
    -Windows-only (Winsock2)
    -nlohmann/json.hpp for JSON parsing
    -C++17 or later
    -Multithreading (<thread>, <mutex>)

🔁 Sample Usage
✅ Starting Server
   > cd ChatApp/server/src
   > server.exe
   Server started on port 5555
   Enter client ID to message (or 'exit'):

✅ Running Client
   > cd ChatApp/client/src
   > client.exe
   [Server] Welcome! Your ID is 1
   > Hello there
   [Server] Got your message!


   
| Feature                        | Server | Client |
| ------------------------------ | ------ | ------ |
| TCP Socket Communication       | ✅      | ✅      |
| JSON Message Parsing           | ✅      | ✅      |
| Encryption/Decryption          | ✅      | ✅      |
| Multi-threading                | ✅      | ✅      |
| Multi-client Support           | ✅      | ✅      |
| Auto Reconnect                 | ❌      | ✅      |
| Buffer Clearing After Messages | ✅      | ✅      |
| Client Identification by ID    | ✅      | ✅      |
| One-to-One Message Routing     | ✅      | ✅      |
| Console Interface              | ✅      | ✅      |


