#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <cstring>
#include <fcntl.h>  // for making socket non-blocking
#include <mutex>    // for std::mutex

#define MAX_EVENTS 10
#define PORT 8080

std::mutex cout_mutex;  // Mutex for std::cout access

void handle_client(int client_fd) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int n = read(client_fd, buffer, sizeof(buffer));

        if (n > 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Received: " << buffer << std::endl;
            write(client_fd, buffer, strlen(buffer)); // Echo back
        }
        else if (n == 0) {
            break;  // Break the loop if connection is closed by the client
        }
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;  // Continue the loop if there's no data to read
            }
            else {
                perror("read");  // Print the error message if read failed
                break;
            }
        }
    }
    close(client_fd);
}

int main() {
    int server_fd, client_fd, epoll_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct epoll_event ev, events[MAX_EVENTS];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    listen(server_fd, 10);

    epoll_fd = epoll_create1(0);

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == server_fd) {
                client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                // Make the accepted socket non-blocking
                int flags = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
                std::thread t(handle_client, client_fd);
                t.detach();
            }
        }
    }

    close(server_fd);

    return 0;
}
