#include <iostream>
#include <winsock2.h>
#include <string>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")
#define PORT 3030
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{ AF_INET, htons(PORT), INADDR_ANY };
    bind(serverSocket, (sockaddr*)&addr, sizeof(addr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server is listening on port " << PORT << std::endl;

    while (true) {
        SOCKET client = accept(serverSocket, nullptr, nullptr);
        if (client == INVALID_SOCKET) break;

        char buffer[BUFFER_SIZE];
        int received = recv(client, buffer, BUFFER_SIZE, 0);
        std::string request(buffer, received);

        auto firstLineEnd = request.find('\n');
        std::string firstLine = request.substr(0, firstLineEnd);
        size_t start = firstLine.find(" ") + 2;
        size_t end = firstLine.rfind(" ");
        std::string path = firstLine.substr(start, end - start);
        if (path == "/") path = "index.html";

        std::ifstream file(path, std::ios::binary);
        std::string body;
        std::string status;

        if (file) {
            body.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
            status = "HTTP/1.1 200 OK\r\n";
        } else {
            body = "404 Not Found";
            status = "HTTP/1.1 404 Not Found\r\n";
        }

        std::string response = status +
            "Content-Type: text/html\r\n" +
            "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" +
            body;

        send(client, response.c_str(), response.size(), 0);
        closesocket(client);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
