#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {

    // Socket creating
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create error\n");
        exit(EXIT_FAILURE);
    }

    // Setting socket address parameters
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    // Converting address to the binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        printf("String isn't adress or error\n");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        printf("Connection failed\n");
        exit(EXIT_FAILURE);
    }

    char* message = "Message from client";
    send(sock, message, strlen(message), 0);
    printf("Message sent\n");

    close(sock);
    return 0;
}