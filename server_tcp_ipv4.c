#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define ARGSIZE 10

int server_fd, client_fd, len;

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 4)
    {
        perror("Usage: ./server PORT\n");
        perror("Usage: ./server PORT -p\n");
        perror("Usage: ./server PORT -p -q\n");
        return 0;
    }
    int pFlag = 0;
    int qFlag = 0;
    char *type;
    char *param;
    if (argc > 3 && strcmp(argv[2], "-p") == 0)
    {
        pFlag = 1;
    }
    if (argc == 4 && strcmp(argv[3], "-q") == 0)
    {
        qFlag = 1;
    }
    int ipv4_port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    struct pollfd fds[2];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        printf("\n Error : Could not create socket \n");
        close(server_fd);
        return 1;
    }
    printf("Socket created.\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(ipv4_port);

    // Will free an already bound port that's not in use.
    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (listen(server_fd, 1) == -1)
    {
        perror("Listen error.");
        close(server_fd);
        return 1;
    }
    printf("Bind done.\n");
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0)
    {
        printf("\n Error : Connect Failed \n");
        close(server_fd);
        close(client_fd);
        return 1;
    }
    printf("Accepting....\n");
    if (pFlag == 1)
        {
            len = recv(client_fd, type, 10, 0);
            len = recv(client_fd, param, 10, 0);
            if (len == 0)
            {
                printf("Connection closed by the client.\n");
                close(server_fd);
                close(client_fd);
                return 1;
            }
        }
    fds[0].fd = client_fd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    printf("[+] Server Running...\n");
    printf("[-] Chat started.\n");
    while (1)
    {
        // Receive two parameters from the client.
        // The first one is the type and the second is param.
        poll(fds, 2, -1);

        if (fds[0].revents & POLLIN)
        {
            len = recv(client_fd, buffer, BUFFER_SIZE, 0);
            if (len == 0)
            {
                printf("Connection closed by the client.\n");
                close(server_fd);
                close(client_fd);
                return 1;
            }
            printf("Client: %s", buffer);
            memset(buffer, '0', BUFFER_SIZE);
        }

        if (fds[1].revents & POLLIN)
        {
            fgets(buffer, sizeof(buffer), stdin);
            send(client_fd, buffer, sizeof buffer, 0);
            memset(buffer, '0', BUFFER_SIZE);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}