#include <iostream>
#include <string>
#include <sstream>
#include <cstring> // for strlen
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include<unistd.h>
int main() {
    int socket_desc;
    struct sockaddr_in server;

    // ����socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        std::cerr << "Could not create socket" << std::endl;
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // �󶨵����Ȿ�ص�ַ
    server.sin_port = htons(6666); // ���ü����˿�Ϊ6666

    // ��socket
    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    // ��ʼ����
    listen(socket_desc, 3);

    std::cout << "HTTP server is running on port 6666..." << std::endl;

    while (true) 
    {
        int new_socket;
        struct sockaddr_in client;
        socklen_t c = sizeof(struct sockaddr_in);

        // �����µ�����
        new_socket = accept(socket_desc, (struct sockaddr*)&client, &c);
        if (new_socket < 0) 
        {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::cout << "Connection accepted from " << inet_ntoa(client.sin_addr) << std::endl;

        // ��ȡ����
        char buffer[2048] = { 0 };
        ssize_t msg_len = recv(new_socket, buffer, sizeof(buffer), 0);
        if (msg_len <= 0) 
        {
            std::cerr << "Receive failed" << std::endl;
            continue;
        }

        std::cout << "Received message: \n" << buffer << std::endl;

        // ������Ӧ
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";

        if (send(new_socket, response.c_str(), response.size(), 0) < 0)
        {
            std::cerr << "Send failed" << std::endl;
        }

        // �ر�����
        close(new_socket);
    }

    return 0;
} 


//curl http://192.168.0.108:9999  ������������������ip
//ip.addr==192.168.0.108 and tcp.port==9999 and ip.src==192.168.0.108&&ip.dst==192.168.0.102 and http


/*������һ���򵥵�������HTML�������ӣ���ʹ��"GET"������

<!DOCTYPE html>
<html>
<body>

<h1>GET Demo</h1>
<form action="/action_page.php" method="GET">
  <label for="fname">First name:</label><br>
  <input type="text" id="fname" name="fname" value="John"><br>
  <label for="lname">Last name:</label><br>
  <input type="text" id="lname" name="lname" value="Doe"><br><br>
  <input type="submit" value="Submit">
</form>
</body>
</html>

�����ύ��ʽ��GET��������������ݸ�����URL�ϣ���ʽΪ?key1=value1&key2=value2����POST���������ݷ���HTTP����������У�������URL����ʾ��

���ݴ�С������GET���������ݷ���URL�У�����ܵ�URL�������ƣ���������Խ�С����POST�����ύ������������û�����ơ�

��ȫ�ԣ�POST������GET��������ȫ����ΪPOST���������ݲ�����URL����ʾ�����ױ���ȡ����ˣ�������Ϣ�������룩Ӧʹ��POST�����ύ��

*/