#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

constexpr int MAX_EVENTS = 10;
constexpr int BUFFER_SIZE = 1024;


void handle_client(int client_socket);

void epoll_event_loop(int epoll_fd)
{
    epoll_event events[MAX_EVENTS];
    bool should_continue = true;
    while (should_continue)
    {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) {
            perror("epoll_wait error");
            break;
        }

        for (int i = 0; i < event_count; ++i) 
        {
            int event_fd = events[i].data.fd;
            if (event_fd == STDIN_FILENO) 
            {
                // 处理标准输入事件
                char input_buffer[BUFFER_SIZE];
                ssize_t bytes_read = read(event_fd, input_buffer, BUFFER_SIZE);
                if (bytes_read == -1) 
                {
                    perror("Failed to read from stdin");
                    break;
                }

                // 处理标准输入数据
                std::string input_data(input_buffer, bytes_read);
                //并将 input_buffer 中的前 bytes_read 个字符作为初始内容存储到 input_data 中。
                std::cout << "recive standard date:" << input_data << std::endl;
                if (input_data == "quit\n") //因为 read() 函数读取的数据包含了换行符 \n，
                {
                    std::cout << "Server is shutting down..." << std::endl;
                    should_continue = false;
                    break;
                }

            }
            else if (events[i].events & EPOLLIN)
            {
                // 处理客户端连接事件
                sockaddr_in client_address{};
                socklen_t client_address_len = sizeof(client_address);
                int client_socket = accept(event_fd, reinterpret_cast<sockaddr*>(&client_address), &client_address_len);
                if (client_socket == -1) 
                {
                    perror("Failed to accept client connection");
                    break;
                }

                // 设置客户端套接字为非阻塞模式
                int client_flags = fcntl(client_socket, F_GETFL, 0);
                fcntl(client_socket, F_SETFL, client_flags | O_NONBLOCK);

                // 将客户端套接字注册到 epoll 对象
                epoll_event client_event{};
                client_event.events = EPOLLIN | EPOLLET;
                client_event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &client_event) == -1)
                {
                    perror("Failed to add client socket to epoll");
                    close(client_socket);
                    break;
                }
                std::cout << "New client connected. Socket: " << client_socket << std::endl;
                // 创建一个线程处理客户端连接
                std::thread client_thread(handle_client, client_socket);
                client_thread.detach(); // 在后台运行线程
            }
        }
    }
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);  // 清空接收缓冲区
        ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_read == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 暂时无可读数据，继续下一次循环等待
                continue;
            }
            else
            {
                perror("Failed to read client socket");
                close(client_socket);
                break;
            }
        }
        else if (bytes_read == 0)
        {
            // 客户端连接关闭
            std::cout << "Client disconnected. Socket: " << client_socket << std::endl;
            close(client_socket);
            break;
        }

        std::cout << "Received data from client. Socket: " << client_socket << ", Data: " << buffer << std::endl;

        const char* response = "Server received your message";
        send(client_socket, response, strlen(response), 0);
    }
}

int main() {
    // 创建套接字
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket");
        return -1;
    }
    // 设置套接字地址重用
    int reuseaddr = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1) {
        perror("Failed to set socket options");
        close(server_socket);
        return -1;
    }

    // 绑定和监听套接字
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);
    if (bind(server_socket, reinterpret_cast<const sockaddr*>(&server_address), sizeof(server_address)) == -1) {
        perror("Failed to bind");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        perror("Failed to listen");
        close(server_socket);
        return -1;
    }

    // 创建epoll对象
    int epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        perror("Failed to create epoll");
        close(server_socket);
        return -1;
    }

    // 注册监听套接字到epoll对象
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = server_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
        perror("Failed to add server socket to epoll");
        close(server_socket);
        close(epoll_fd);
        return -1;
    }

    // 注册标准输入到epoll对象
    event.data.fd = STDIN_FILENO;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) == -1) {
        perror("Failed to add stdin to epoll");
        close(server_socket);
        close(epoll_fd);
        return -1;
    }

    // 进入事件循环
    epoll_event_loop(epoll_fd);

    // 关闭套接字和epoll对象
    close(server_socket);
    close(epoll_fd);

    return 0;
}