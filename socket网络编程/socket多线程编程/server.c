#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    int clientSocket;
    struct sockaddr_in clientAddress;
} ClientInfo;

void* handleClient(void* arg);


int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(serverSocket, 5) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);

    while (1) 
    {
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength)) == -1) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        pthread_t tid;
        ClientInfo* clientInfo = (ClientInfo*)malloc(sizeof(ClientInfo));
        clientInfo->clientSocket = clientSocket;
        clientInfo->clientAddress = clientAddress;
        pthread_create(&tid, NULL, handleClient, clientInfo);
        pthread_detach(tid);
    }
    close(serverSocket);
    return 0;
}

void* handleClient(void* arg) {
    ClientInfo* clientInfo = (ClientInfo*)arg;
    int clientSocket = clientInfo->clientSocket;
    struct sockaddr_in clientAddress = clientInfo->clientAddress;
    char buffer[BUFFER_SIZE];
    printf("Client connected: IP %s, Port %d\n",inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

    while (1) {
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead == -1) {
            perror("read error");
            break;
        }
        else if (bytesRead == 0) {
            printf("Client disconnected\n");
            break;
        }
        buffer[bytesRead] = '\0';
        printf("Received message from client: %s\n", buffer);
        for (int i = 0; i < bytesRead; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        printf("Modified message: %s\n", buffer);

        if (strcmp(buffer, "QUIT") == 0) {
            printf("Closing connection\n");
            break;
        }
        if (send(clientSocket, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }
    }

    close(clientSocket);
    free(clientInfo);
    return NULL;
}