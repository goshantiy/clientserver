#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <filesystem>

#pragma comment(lib, "ws2_32.lib")

std::string listFiles(const std::string& directoryPath) {
    WIN32_FIND_DATAA findFileData; // Используем многобайтовую версию структуры
    HANDLE hFind = FindFirstFileA((directoryPath + "\\*").c_str(), &findFileData);

    std::string fileList;
    if (hFind == INVALID_HANDLE_VALUE) {
        fileList += "Error opening directory.";
        return
            fileList;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            fileList += findFileData.cFileName;
            fileList += "\n";
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);

    return fileList;
}

std::string type(const char* fileName) {
    std::ifstream fileStream(fileName);

    if (fileStream.is_open()) {
        // Определяем размер файла
        fileStream.seekg(0, std::ios::end);
        std::streampos fileSize = fileStream.tellg();
        fileStream.seekg(0, std::ios::beg);

        // Читаем содержимое файла в строку
        std::string fileContent;
        fileContent.resize(static_cast<size_t>(fileSize));
        fileStream.read(&fileContent[0], fileSize);

        return fileContent;
    }
    else {
        // Если файл не удалось открыть, отправляем сообщение об ошибке
        return "Error opening file";
    }
}

std::string getCurrentDirectory() {
    wchar_t buffer[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, buffer);

    // Конвертируем Unicode строку в ANSI
    int len = WideCharToMultiByte(CP_ACP, 0, buffer, -1, NULL, 0, NULL, NULL);
    char* currentDirectory = new char[len];
    WideCharToMultiByte(CP_ACP, 0, buffer, -1, currentDirectory, len, NULL, NULL);

    std::cout << "Current working directory: " << currentDirectory << std::endl;

    return std::string(currentDirectory);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    if (InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr) != 1) {
        std::cerr << "Error converting IP address" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }
    serverAddr.sin_port = htons(8080); // Port number


    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Error in listening" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening for incoming connections..." << std::endl;

    SOCKET clientSocket;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error in accepting connection" << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        char buffer[1024];
        int bytesReceived;

        do {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << "Received: " << buffer << std::endl;

                // Распознавание команд и выполнение действий
                if (strncmp(buffer, "ls", 2) == 0) {
                    // Команда ls
                   std::string filePath(buffer + 3); // Предполагаем, что путь начинается после "type "
                   std::string content = listFiles(filePath.c_str());
                   send(clientSocket, content.c_str(), content.size() * sizeof(wchar_t), 0);
                }
                else if (strncmp(buffer, "type", 4) == 0) {
                    // Команда type
                    std::string filePath(buffer + 5); // Предполагаем, что путь начинается после "type "
                    std::string content = type(filePath.c_str());
                    send(clientSocket, content.c_str(), content.size(), 0);
                }
                else if (strncmp(buffer, "pwd", 3) == 0) {
                    // Команда pwd
                    std::string content = getCurrentDirectory();
                    send(clientSocket, content.c_str(), content.size(), 0);
                }
                else {
                    // Неизвестная команда
                    send(clientSocket, "Unknown command", strlen("Unknown command"), 0);
                }
               // send(clientSocket, "Command executed successfully", strlen("Command executed successfully"), 0);
            }
        } while (bytesReceived > 0);
    }
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
