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


#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if(argc != 2) {
        perror("Usage: ./server PORT\n");
        return 0;
    }
    int ipv4_port = atoi(argv[1]);
    int server_fd, client_fd, len;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];
    struct pollfd fds[2];

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_fd < 0)
    {
        printf("\n Error : Could not create socket \n");
        close(server_fd);
        return 1;
    }
    printf("Socket created.\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(ipv4_port);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("Bind done.\n");

    fds[0].fd = client_fd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    printf("[+] Server Running...\n");
    printf("[-] Chat started.\n");
    sleep(1);
    while (1)
    {
        poll(fds, 2, -1);

        if (fds[0].revents & POLLIN)
        {
            len = recvfrom(fds[0].fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
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
            sendto(fds[0].fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, addr_len);
            memset(buffer, 0, BUFFER_SIZE);
        }
    }
    close(client_fd);
    close(server_fd);
    return 0;
}