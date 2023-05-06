#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if(argc != 3) {
        perror("Usage: ./client IP PORT\n");
        return 0;
    }
    const char* ipv6_addr = argv[1];
    int ipv6_port = atoi(argv[2]);
    int client_fd, len;
    struct sockaddr_in6 server_addr;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    struct pollfd fds[2];

    client_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if(client_fd < 0)
    {
        printf("\n Error : Could not create socket \n");
        close(client_fd);
        return 1;
    }
    printf("Socket created.\n");

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(ipv6_port);

    if (inet_pton(AF_INET6, ipv6_addr, &server_addr.sin6_addr) <= 0) {
        perror("Error in the IP Adresse");
        close(client_fd);
        return 1;
    }

    if(connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("Connect error.");
        close(client_fd);
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
                close(client_fd);
                return 1;
        } 
            printf("Server: %s", buffer);
            memset(buffer, '0', BUFFER_SIZE);
        }

        if (fds[1].revents & POLLIN) {
            fgets(buffer,sizeof(buffer), stdin);
            send(client_fd, buffer, sizeof buffer, 0);
            memset(buffer, '0', BUFFER_SIZE);
        }
    }

    close(client_fd);
    return 0;
}