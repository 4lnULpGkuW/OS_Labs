#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

const int port = 8080;
const int backlog = 5;

volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int sigNumber) {
    wasSigHup = 1;
}

int main() {

    // Socket creating
    int sock;
    if (!(sock = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("Creation error");
        exit(EXIT_FAILURE);
    }

    // Setting socket address parameters
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = INADDR_ANY;
    sock_addr.sin_port = htons(port);

    // Socket binding to the address
    if (bind(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    // Started socket listenig
    if (listen(sock, backlog) == -1) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }
    printf("Server started on port %d \n", port);

    // Signal handler registration
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    // Setting up signal blocking
    sigset_t blocked_mask, orig_mask;
    sigemptyset(&blocked_mask);
    sigemptyset(&orig_mask);
    sigaddset(&blocked_mask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blocked_mask, &orig_mask);
   
    int client_socket = 0;

    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds); 
        FD_SET(sock, &read_fds); 
        
        if (client_socket > 0) { 
            FD_SET(client_socket, &read_fds); 
        } 
        
        int maxSd = (client_socket > sock) ? client_socket : sock; 
 
        if (pselect(maxSd + 1, &read_fds, NULL, NULL, NULL, &orig_mask) == -1) { 
            if (errno != EINTR) {
                perror("pselect error");
                exit(EXIT_FAILURE);
            }
            else if (wasSigHup == 1) {
            	printf("SIGHUP received.\n");
  		    	wasSigHup = 0;
                continue;
            }
        }

        // Reading incoming bytes
        if (client_socket > 0 && FD_ISSET(client_socket, &read_fds)) {
            char buffer[1024] = { 0 };
            int readBytes = read(client_socket, buffer, 1024);
            if (readBytes > 0) {
                printf("Received data: %d bytes\n", readBytes); 
            }
            else if (readBytes == 0) {
                close(client_socket); 
                client_socket = 0; 
            }
            else {
                perror("read error"); 
            }
            continue;
        }
        
        // Check of incoming connections
        if (FD_ISSET(sock, &read_fds)) {
            socklen_t sock_addr_len = sizeof(sock_addr);
            if ((client_socket = accept(sock, (struct sockaddr*)&sock_addr, &sock_addr_len)) == -1) {
                perror("accept error");
                exit(EXIT_FAILURE);
            }
            printf("New connection.\n");
        }
    }

    close(sock);
    return 0;
}