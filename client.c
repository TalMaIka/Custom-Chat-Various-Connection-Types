#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define ARGSIZE 10
#define SIZE 104857600 // 100BM chunk of data.

char *generete_random_data() {
    char *data;
    if ((data = malloc(SIZE)) == NULL) {
        perror("Error while allocating memory.\n");
        exit(1);
    }
    for (size_t i = 0; i < SIZE; i++) {
        data[i] = rand() % 100; // random number between 0 and 100.
    }
    return data;
}

long checksum(char *data) {
    long sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += data[i];
    }
    return sum;
}

//Pipeing filename and sending 100MB of data and checksum (FIFO).
void pipe_client(char *data, char *PIPENAME) {
    int pipe_fd;
    ssize_t sent_bytes = 0;
    mkfifo(PIPENAME, 0666);
    pipe_fd = open(PIPENAME, O_WRONLY);
    printf("[+] Pipe created.\n");
    while (sent_bytes < SIZE) {
        sent_bytes += write(pipe_fd, data + sent_bytes, BUFFER_SIZE);
    }
    printf("[+] Data sent.\n");

    close(pipe_fd);
    unlink(PIPENAME);
}


// mmap client and sending 100MB of data and checksum.
void mmap_client(char *data, char *FILENAME) {
    int fd;
    char *shared_memory;
    sleep(1);
    fd = open(FILENAME, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("[-] open failed.\n");
        exit(1);
    }

    shared_memory = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("[-] mmap failed.\n");
        exit(1);
    }
    printf("[+] Sending data...\n");
    // Copy data to shared memory
    memcpy(shared_memory, data, SIZE);

    printf("[+] Data sent.\n");

    // Cleanup
    munmap(shared_memory, SIZE);
    close(fd);
}

// UDS Stream socket (client) and sending 100MB of data and checksum.
void UDSstream(char *socket_path, char *data) {
    int client_fd;
    struct sockaddr_un server_addr;

    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("[-] Socket failed.\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);
    sleep(1);
    if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("[-] Connect failed.\n");
        exit(1);
    }

    printf("[+] Sending data...\n");
    size_t total_bytes_sent = 0;
    int size = 30000;
    while (total_bytes_sent < SIZE) {
        if (total_bytes_sent + size > SIZE) {
            size = SIZE - total_bytes_sent;
        }
        ssize_t bytes_sent = send(client_fd, data + total_bytes_sent, size, 0);
        if (bytes_sent == -1) {
            perror("[-] Error sending data");
            exit(1);
        }
        total_bytes_sent += bytes_sent;
    }
    printf("[+] Data sent.\n");
    close(client_fd);
}

// UDS DGRAM socket (client) and sending 100MB of data and checksum.
void UDSdgram(char *socket_path, char *data) {
    int client_fd;
    struct sockaddr_un server_addr;

    if ((client_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("[-] Socket failed.\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);
    sleep(1);
    printf("[+] Sending data...\n");
    size_t total_bytes_sent = 0;
    int size = 30000;
    while (total_bytes_sent < SIZE) {
        if (total_bytes_sent + size > SIZE) {
            size = SIZE - total_bytes_sent;
        }
        ssize_t bytes_sent = sendto(client_fd, data + total_bytes_sent, size, 0, (struct sockaddr *) &server_addr,sizeof(server_addr));
        if (bytes_sent == -1) {
            perror("[-] Error sending data");
            exit(1);
        }
        total_bytes_sent += bytes_sent;
    }
    printf("[+] Bytes sent: %zd\n", total_bytes_sent);
    printf("[+] Data sent.\n");
    close(client_fd);
}

// Ipv6 UDP socket (client) and sending 100MB of data and checksum.
void UDPipv6(int port, char *ip, char *data) {
    int client_fd;
    struct sockaddr_in6 server_addr;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    if ((client_fd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("[-] Socket failed.\n");
        exit(1);
    }
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port + 1);
    if (inet_pton(AF_INET6, ip, &server_addr.sin6_addr) <= 0) {
        perror("[-] Invalid address.\n");
        exit(1);
    }
    printf("[+] Sending data...\n");
    size_t total_bytes_sent = 0;
    int size = 30000;
    while (total_bytes_sent < SIZE) {
        if (total_bytes_sent + size > SIZE) {
            size = SIZE - total_bytes_sent;
        }
        ssize_t bytes_sent = sendto(client_fd, data + total_bytes_sent, size, 0, (struct sockaddr *) &server_addr,sizeof(server_addr));
        if (bytes_sent == -1) {
            perror("[-] sendto failed");
            exit(1);
        }
        total_bytes_sent += bytes_sent;
    }
    if (total_bytes_sent != SIZE) {
        printf("[-] Failed to send all data. Sent %zd bytes out of %d\n", total_bytes_sent, SIZE);
        exit(1);
    }
    printf("[+] Data sent.\n");
    close(client_fd);
}

// IPv4 TCP socket (client) and sending 100MB of data and checksum.
void UDPipv4(int port, char *ip, char *data) {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[-] Socket failed.\n");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port + 1);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("[-] Invalid address.\n");
        exit(1);
    }
    printf("[+] Sending data...\n");
    size_t total_bytes_sent = 0;
    int size = 30000;
    while (total_bytes_sent < SIZE) {
        if (total_bytes_sent + size > SIZE) {
            size = SIZE - total_bytes_sent;
        }
        ssize_t bytes_sent = sendto(client_fd, data + total_bytes_sent, size, 0, (struct sockaddr *) &server_addr,sizeof(server_addr));
        if (bytes_sent == -1) {
            perror("[-] sendto failed");
            exit(1);
        }
        total_bytes_sent += bytes_sent;
    }
    if (total_bytes_sent != SIZE) {
        printf("[-] Failed to send all data. Sent %zd bytes out of %d\n", total_bytes_sent, SIZE);
        exit(1);
    }
    printf("[+] Data sent.\n");
    close(client_fd);
}

// IPv4 TCP socket (client) and sending 100MB of data and checksum.
void TCPipv4(char *ip, int port, char *data) {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[-] Socket failed.\n");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port + 1);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("[-] Invalid address.\n");
        exit(1);
    }
    sleep(2);
    if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("[-] Connection failed.\n");
        exit(1);
    }
    printf("[+] Sending data...\n");
    send(client_fd, data, SIZE, 0);
    printf("[+] Data sent.\n");
    close(client_fd);
}

// IPv6 TCP socket (client) and sending 100MB of data and checksum.
void TCPipv6(char *ip, int port, char *data) {
    int client_fd;
    struct sockaddr_in6 server_addr;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    if ((client_fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("[-] Socket failed.\n");
        exit(1);
    }
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port + 1);
    if (inet_pton(AF_INET6, ip, &server_addr.sin6_addr) <= 0) {
        perror("[-] Invalid address.\n");
        exit(1);
    }
    sleep(2);
    if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("[-] Connection failed.\n");
        exit(1);
    }
    printf("[+] Sending data...\n");
    send(client_fd, data, SIZE, 0);
    printf("[+] Data sent.\n");
    close(client_fd);
}

void socketFactory(char *type, char *param, int port, char *data) {
    if (strcmp(type, "pipe") == 0) {
        char *PIPENAME = param;
        pipe_client(data, PIPENAME);
    }
    if (strcmp(type, "uds") == 0) {
        char *socket_path = "/tmp/file";
        if (strcmp(param, "dgram") == 0) {
            UDSdgram(socket_path, data);
        } else {
            UDSstream(socket_path, data);
        }
    }

    if (strcmp(type, "mmap") == 0) {
        mmap_client(data, param);
    }

    if (strcmp(type, "ipv4") == 0) {
        char *ip = "127.0.0.1";
        if (strcmp(param, "tcp") == 0) {
            printf("TCP IPv4\n");
            TCPipv4(ip, port, data);
        } else if (strcmp(param, "udp") == 0) {
            printf("UDP IPv4\n");
            UDPipv4(port, ip, data);
        } else {
            printf("Invalid parameter.\n");
        }
    }
    if (strcmp(type, "ipv6") == 0) {
        char *ip = "::1";
        if (strcmp(param, "tcp") == 0) {
            printf("TCP IPv6\n");

            TCPipv6(ip, port, data);
        } else if (strcmp(param, "udp") == 0) {
            printf("UDP IPv6\n");
            UDPipv6(port, ip, data);
        } else {
            printf("Invalid parameter.\n");
        }
    }
}

void chatTCP(int client_fd) {
    int len;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    struct pollfd fds[2];
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
            if (len == 0) {
                printf("Server closed the connection.\n");
                close(client_fd);
                exit(1);
            }
            printf("Server: %s", buffer);
            memset(buffer, '0', BUFFER_SIZE);
        }

        if (fds[1].revents & POLLIN) {
            fgets(buffer, sizeof(buffer), stdin);
            send(client_fd, buffer, sizeof buffer, 0);
            memset(buffer, '0', BUFFER_SIZE);
        }
    }
}

void sendArgs(int client_fd, char *type, char *param, char *data) {
    printf("[+] Sending the parameters!.[%s] [%s]\n", type, param);
    long checkSum = checksum(data);
    send(client_fd, type, ARGSIZE, 0);
    send(client_fd, param, ARGSIZE, 0);
    send(client_fd, &checkSum, sizeof(long), 0);
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 6) {
        perror("Usage: ./client IP PORT\n");
        perror("Usage: ./client IP PORT -p <type> <param> \n");
        return 0;
    }
    int pFlag = 0;

    // Saving the parameters [5] and [6] in string.
    char *type;
    char *param;
    if (argc == 6 && strcmp(argv[3], "-p") == 0) {
        pFlag = 1;
        type = argv[4];
        param = argv[5];
    }
    const char *ip_addr = argv[1];
    int port = atoi(argv[2]);
    int client_fd, len;
    struct sockaddr_in server_addr;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        printf("\n Error : Could not create socket \n");
        close(client_fd);
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_aton(ip_addr, &server_addr.sin_addr) == 0) {
        perror("Error while converting address \n");
        close(client_fd);
        return 1;
    }

    if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Connect error.");
        close(client_fd);
        return 1;
    }
    printf("[+] Connected.\n");
    if (pFlag == 0) {
        chatTCP(client_fd);
    } else {
        char *data = generete_random_data();
        sendArgs(client_fd, argv[4], argv[5], data);
        close(client_fd);
        socketFactory(argv[4], argv[5], port, data);
        free(data);
    }
    close(client_fd);
    return 0;
}