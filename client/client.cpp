#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock" << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    if (InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr) != 1) {
        std::cerr << "Error converting IP address" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }
    serverAddr.sin_port = htons(8080); // Port number

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::string command;

    while (true) {
        // Считываем команду из консоли
        std::cout << "Enter command (or 'bye' to exit): ";
        std::getline(std::cin, command);

        // Отправляем команду на сервер
        send(clientSocket, command.c_str(), command.size(), 0);

        // Проверяем условие выхода
        if (command == "bye") {
            break;
        }

        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Server response:\n " << buffer << std::endl;
        }
    }

    // Закрываем сокет и очищаем Winsock
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
