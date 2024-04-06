#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

constexpr int BUFFER_SIZE = 1024;

int main() {
    // 创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Failed to create socket");
        return -1;
    }

    // 设置服务器地址
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");  // 替换为实际的服务器IP地址
    server_address.sin_port = htons(8080);  // 替换为实际的服务器端口号

    // 连接服务器
    if (connect(client_socket, reinterpret_cast<const sockaddr*>(&server_address), sizeof(server_address)) == -1) {
        perror("Failed to connect to server");
        close(client_socket);
        return -1;
    }

    std::cout << "Connected to server." << std::endl;

    while (true) {
        std::cout << "Enter message to send (or 'quit' to exit): ";
        std::string input_data;
        std::getline(std::cin, input_data);

        if (input_data == "quit") {
            std::cout << "Exiting..." << std::endl;
            close(client_socket);
            return 0;
        }

        // 发送消息给服务器
        ssize_t bytes_sent = send(client_socket, input_data.c_str(), input_data.size(), 0);
        if (bytes_sent == -1) {
            perror("Failed to send message to server");
            break;
        }

        // 接收服务器的响应
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);  // 清空接收缓冲区
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received == -1) {
            perror("Failed to receive response from server");
            break;
        }

        std::cout << "Received response from server: " << buffer << std::endl;
    }

    close(client_socket);
    return 0;
}
