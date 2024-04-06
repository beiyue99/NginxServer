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

    // 创建socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        std::cerr << "Could not create socket" << std::endl;
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // 绑定到任意本地地址
    server.sin_port = htons(6666); // 设置监听端口为6666

    // 绑定socket
    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    // 开始监听
    listen(socket_desc, 3);

    std::cout << "HTTP server is running on port 6666..." << std::endl;

    while (true) 
    {
        int new_socket;
        struct sockaddr_in client;
        socklen_t c = sizeof(struct sockaddr_in);

        // 接受新的连接
        new_socket = accept(socket_desc, (struct sockaddr*)&client, &c);
        if (new_socket < 0) 
        {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::cout << "Connection accepted from " << inet_ntoa(client.sin_addr) << std::endl;

        // 读取请求
        char buffer[2048] = { 0 };
        ssize_t msg_len = recv(new_socket, buffer, sizeof(buffer), 0);
        if (msg_len <= 0) 
        {
            std::cerr << "Receive failed" << std::endl;
            continue;
        }

        std::cout << "Received message: \n" << buffer << std::endl;

        // 发送响应
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

        // 关闭连接
        close(new_socket);
    }

    return 0;
} 


//curl http://192.168.0.108:9999  或者用浏览器输入这个ip
//ip.addr==192.168.0.108 and tcp.port==9999 and ip.src==192.168.0.108&&ip.dst==192.168.0.102 and http


/*下面是一个简单的完整的HTML表单的例子，它使用"GET"方法：

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

数据提交方式：GET方法将请求的数据附加在URL上，形式为?key1=value1&key2=value2，而POST方法则将数据放在HTTP请求的主体中，不会在URL中显示。

数据大小：由于GET方法将数据放在URL中，因此受到URL长度限制，数据量相对较小。而POST方法提交的数据理论上没有限制。

安全性：POST方法比GET方法更安全，因为POST方法的数据不会在URL中显示，不易被截取。因此，敏感信息（如密码）应使用POST方法提交。

*/