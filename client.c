#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define SERVER_IP "127.0.0.1"
#define PORT 4449
#define BUFFER_SIZE 1024

int main() {
    int client_fd, len;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    struct pollfd fds[2];

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    printf("Socket created.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if(connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("Connect error.");
        return 1;
    }
    printf("Connected.\n");

    fds[0].fd = client_fd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    printf("[+] Client Running...\n");
    printf("[-] Chat started.\n");
    while (1) {
        poll(fds, 2, -1);

        if (fds[0].revents & POLLIN) {
            len = recv(client_fd, buffer, BUFFER_SIZE, 0);
            if(len == -1){
            printf("Recv error.");
            return 1;
        } 
            printf("Server: %s\n", buffer);
            memset(buffer, '0', BUFFER_SIZE);
        }

        if (fds[1].revents & POLLIN) {
            scanf("%s", buffer);
            send(client_fd, buffer, sizeof buffer, 0);
            memset(buffer, '0', BUFFER_SIZE);
        }
    }

    close(client_fd);
    return 0;
}