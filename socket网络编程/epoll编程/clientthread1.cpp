#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[1024] = { 0 };

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    while (true) {
        std::string message;
        std::cout << "Enter message: ";
        std::getline(std::cin, message);

        if (message == "quit") {
            break;
        }

        send(client_fd, message.c_str(), message.size(), 0);

        std::cout << "Message sent to server." << std::endl;

        memset(buffer, 0, sizeof(buffer));
        read(client_fd, buffer, sizeof(buffer));

        std::cout << "Message received from server: " << buffer << std::endl;
    }

    close(client_fd);

    return 0;
}
