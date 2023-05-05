#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>

#define PORT 4449
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

int main() {
    int server_fd, client_fd, len;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    struct pollfd fds[2];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    printf("Socket created.\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(listen(server_fd, 1)==-1){
        perror("Listen error.");
        return 1;
    }
    printf("Bind done.\n");
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if( client_fd < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }
    printf("Accepting....\n");

    fds[0].fd = client_fd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    printf("[+] Server Running...\n");
    printf("[-] Chat started.\n");
    sleep(1);
    while (1) {
        poll(fds, 2, -1);

        if (fds[0].revents & POLLIN) {
            len = recv(client_fd, buffer, BUFFER_SIZE, 0);
            if(len == -1){
            printf("Recv error.");
            return 1;
        } 
            printf("Client: %s\n", buffer);
            memset(buffer, '0', BUFFER_SIZE);
        }

        if (fds[1].revents & POLLIN) {
            scanf("%s", buffer);
            send(client_fd, buffer, sizeof buffer, 0);
            memset(buffer, '0', BUFFER_SIZE);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}