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
#include <time.h>

#define BUFFER_SIZE 1024
#define ARGSIZE 10
#define SIZE 104857600 // 100BM chunk of data.

// Establishing a IPv4 TCP connection to be able to recive chuck of 100MB.
void TCPipv4(int port)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int opt = 1;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("[-] Socket failed.\n");
        exit(EXIT_FAILURE);
    }
    // Will free an already bound port that's not in use.
    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port + 1);
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[-] Bind failed.\n");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("[-] Listen failed.\n");
        exit(EXIT_FAILURE);
    }
    client_len = sizeof(client_addr);
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
    {
        perror("[-] Accept failed.\n");
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_received = 0;
    ssize_t total_bytes_received = 0;
    time_t start_time = time(NULL);

    while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        total_bytes_received += bytes_received;
    }
    time_t end_time = time(NULL);
    double time_spent = difftime(end_time, start_time);

    printf("Bytes received: %zd\n", total_bytes_received);
    printf("Time spent: %.2f seconds\n", time_spent);

    double percentage_received = ((double)total_bytes_received / SIZE) * 100;
    printf("Percentage of bytes received: %.2f%%\n", percentage_received);

    close(server_fd);
    close(client_fd);
    exit(0);
}

void socketFactory(char *type, char *param, int port)
{
    if (strcmp(type, "ipv4") == 0)
    {
        if (strcmp(param, "tcp") == 0)
        {
            printf("TCP IPv4\n");
            TCPipv4(port);
        }
        else if (strcmp(param, "ipv6") == 0)
        {
            printf("TCP IPv6\n");
        }
        else
        {
            printf("Invalid parameter.\n");
        }
    }
    else if (strcmp(type, "udp") == 0)
    {
        if (strcmp(param, "ipv4") == 0)
        {
            printf("UDP IPv4\n");
        }
        else if (strcmp(param, "ipv6") == 0)
        {
            printf("UDP IPv6\n");
        }
        else
        {
            printf("Invalid parameter.\n");
        }
    }
    else
    {
        printf("Invalid type.\n");
    }
}

void recArgs(int client_fd, char *type, char *param)
{
    recv(client_fd, type, ARGSIZE, 0);
    recv(client_fd, param, ARGSIZE, 0);
    printf("Reciving the parameters.");
    printf("Type: %s\n", type);
    printf("Param: %s\n", param);
}

void chatTCP(int client_fd, int server_fd)
{
    int len;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    struct pollfd fds[2];
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
                exit(1);
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
}

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
    char *type = (char *)malloc(ARGSIZE);
    char *param = (char *)malloc(ARGSIZE);
    if (argc > 3 && strcmp(argv[2], "-p") == 0)
    {
        pFlag = 1;
    }
    if (argc == 4 && strcmp(argv[3], "-q") == 0)
    {
        qFlag = 1;
    }
    int server_fd, client_fd, len;
    int port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

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
    server_addr.sin_port = htons(port);

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
    if (pFlag == 0)
    {
        recArgs(client_fd, type, param);
        socketFactory(type, param, port);
    }
    else
    {
        chatTCP(client_fd, server_fd);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}