#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Usage: ./client IP PORT\n");
        return 0;
    }
    const char *ipv4_addr = argv[1];
    int ipv4_port = atoi(argv[2]);
    int client_fd, len;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    struct pollfd fds[2];

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0)
    {
        printf("\n Error : Could not create socket \n");
        close(client_fd);
        return 1;
    }
    printf("Socket created.\n");

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(ipv4_port);

    if (inet_aton(ipv4_addr, &client_addr.sin_addr) == 0)
    {
        perror("Erreur lors de la conversion de l'adresse");
        close(client_fd);
        return 1;
    }

    printf("Address created.\n");

    fds[0].fd = client_fd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    printf("[+] Client Running...\n");
    printf("[-] Chat started.\n");
    while (1)
    {
        poll(fds, 2, -1);

        if (fds[0].revents & POLLIN)
        {
            len = recvfrom(fds[0].fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
            if (len == -1)
            {
                printf("Recvfrom error.");
                close(fds[0].fd);
                return 1;
            }
            printf("Server: %s", buffer);
            memset(buffer, 0, BUFFER_SIZE);
        }

        if (fds[1].revents & POLLIN)
        {
            fgets(buffer, sizeof(buffer), stdin);
            sendto(fds[0].fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, addr_len);
            memset(buffer, 0, BUFFER_SIZE);
        }
    }

    close(client_fd);
    return 0;
}