#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define ARGSIZE 10
#define SIZE 104857600 // 100BM chunk of data.
#define TIME_OUT 1
long checkSum = 0;
int qFlag = 0;

long checksum(char *data)
{
    long sum = 0;
    for (int i = 0; i < SIZE; i++)
    {
        sum += data[i];
    }
    return sum;
}

void pipe_client(char *PIPENAME)
{
    sleep(1);
    int pipe_fd;
    char *data = (char *)malloc(SIZE);
    long read_bytes;
    size_t total_bytes = 0;
    if (!qFlag)
    {
        printf("[+] Waiting for connection.\n");
    }
    pipe_fd = open(PIPENAME, O_RDONLY);
    if (!qFlag)
    {
        printf("[+] Connected to the pipe.\n");
    }
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    while ((read_bytes = read(pipe_fd, data + total_bytes, BUFFER_SIZE)) > 0)
    {
        total_bytes += read_bytes;
        if (total_bytes >= SIZE)
        {
            break;
        }
    }
    long calculated_checksum = checksum(data);
    gettimeofday(&end_time, NULL);
    long time_spent_ms = (end_time.tv_sec - start_time.tv_sec) * 1000L + (end_time.tv_usec - start_time.tv_usec) / 1000L;
    if (calculated_checksum == checkSum && !qFlag)
    {
        printf("[+] Valid Checksum !.\n");
    }
    if (!qFlag)
    {
        printf("---------------------------------------\n");
        printf("|    Received a Chunk of bytes         \n");
        printf("| Type : pipe   | Param : %s \n", PIPENAME);
        printf("| Info : Bytes received: %ld\n", total_bytes);
        printf("|  Time spent: %ld milliseconds       \n", time_spent_ms);
        double percentage_received = ((double)total_bytes / SIZE) * 100;
        printf("| Percentage bytes received: %.2f%%    \n", percentage_received);
        printf("| Checksum : %ld                        \n", calculated_checksum);
        printf("---------------------------------------\n");
    }
    else
    {
        printf("pipe,%ld", time_spent_ms);
    }
    close(pipe_fd);
    unlink(PIPENAME);
}

// mmap server and sending 100MB of data and checksum.
void mmap_server(char *FILENAME)
{
    int fd;
    char *shared_memory;
    char *data = (char *)malloc(SIZE);
    long calculated_checksum = 0;
    fd = open(FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        perror("[-] open failed.\n");
        exit(1);
    }
    if (ftruncate(fd, SIZE) < 0)
    {
        perror("[-] ftruncate failed.\n");
        exit(1);
    }

    shared_memory = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_memory == MAP_FAILED)
    {
        perror("[-] mmap failed.\n");
        exit(1);
    }
    if (!qFlag)
    {
        printf("[+] Server ready to receive data.\n");
    }
    // Wait for the client to write data and set the first byte to '1'
    sleep(2);
    if (!qFlag)
    {
        printf("[+] Waiting to the client to finish writing...\n");
    }
    long size = 0;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    memcpy(data, shared_memory, SIZE);

    calculated_checksum = checksum(data);
    gettimeofday(&end_time, NULL);
    long time_spent_ms =(end_time.tv_sec - start_time.tv_sec) * 1000L + (end_time.tv_usec - start_time.tv_usec) / 1000L;
    if (calculated_checksum == checkSum && !qFlag)
    {
        printf("[+] Valid Checksum !.\n");
        size = SIZE;
    }
    if (!qFlag)
    {
        printf("---------------------------------------\n");
        printf("|    Received a Chunk of bytes         \n");
        printf("| Type : mmap   | Param : %s \n", FILENAME);
        printf("| Info : Bytes received: %ld\n", size);
        printf("|  Time spent: %ld milliseconds       \n", time_spent_ms);
        double percentage_received = ((double)size / SIZE) * 100;
        printf("| Percentage bytes received: %.2f%%    \n", percentage_received);
        printf("| Checksum : %ld                        \n", calculated_checksum);
        printf("---------------------------------------\n");
    }
    else
    {
        printf("mmap,%ld", time_spent_ms);
    }

    // Cleanup
    munmap(shared_memory, SIZE);
    close(fd);
    unlink(FILENAME);
    free(data);
}

// UDS DGRAM socket (server) and sending 100MB of data and checksum.
void UDSdgram(char *socket_path)
{
    int server_fd;
    struct sockaddr_un server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_addr_size;
    long calculated_checksum = 0;

    if ((server_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("[-] Socket failed.\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[-] Bind failed.\n");
        exit(1);
    }
    if (!qFlag)
    {
        printf("[+] Waiting for data...\n");
    }
    size_t total_bytes_received = 0;
    int size = 30000;
    char *data = (char *)malloc(SIZE);
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    while (total_bytes_received < SIZE)
    {
        if (total_bytes_received + size > SIZE)
        {
            size = SIZE - total_bytes_received;
        }
        ssize_t bytes_received = recvfrom(server_fd, data + total_bytes_received, size, 0,(struct sockaddr *)&client_addr, &client_addr_size);
        total_bytes_received += bytes_received;
    }
    calculated_checksum = checksum(data);
    gettimeofday(&end_time, NULL);
    long time_spent_ms =(end_time.tv_sec - start_time.tv_sec) * 1000L + (end_time.tv_usec - start_time.tv_usec) / 1000L;
    if (calculated_checksum == checkSum && !qFlag)
    {
        printf("[+] Valid Checksum !.\n");
    }
    if (!qFlag)
    {
        printf("---------------------------------------\n");
        printf("|    Received a Chunk of bytes         \n");
        printf("| Type : UDS   | Param : dgram \n");
        printf("| Info : Bytes received: %zd\n", total_bytes_received);
        printf("|  Time spent: %ld milliseconds       \n", time_spent_ms);
        double percentage_received = ((double)total_bytes_received / SIZE) * 100;
        printf("| Percentage bytes received: %.2f%%    \n", percentage_received);
        printf("| Checksum : %ld                        \n", calculated_checksum);
        printf("---------------------------------------\n");
    }
    else
    {
        printf("uds_dgram,%ld", time_spent_ms);
    }
    close(server_fd);
    unlink(socket_path);
    free(data);
}

// UDS STREAM socket (server) and sending 100MB of data and checksum.
void UDSstream(char *socket_path)
{
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_addr_size;
    long calculated_checksum = 0;

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("[-] Socket failed.\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    // Will free an already bound port that's not in use.
    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[-] Bind failed.\n");
        exit(1);
    }

    if (listen(server_fd, 1) < 0)
    {
        perror("[-] Listen failed.\n");
        exit(1);
    }
    if (!qFlag)
    {
        printf("[+] Server ready to receive data.\n");
    }
    client_addr_size = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_fd < 0)
    {
        perror("[-] Accept failed.\n");
        exit(1);
    }

    size_t total_bytes_received = 0;
    int size = 30000;
    char *data = (char *)malloc(SIZE * sizeof(char));
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    while (total_bytes_received < SIZE)
    {
        if (total_bytes_received + size > SIZE)
        {
            size = SIZE - total_bytes_received;
        }
        ssize_t bytes_received = recv(client_fd, data + total_bytes_received, size, 0);
        total_bytes_received += bytes_received;
    }

    calculated_checksum = checksum(data);
    gettimeofday(&end_time, NULL);
    long time_spent_ms = (end_time.tv_sec - start_time.tv_sec) * 1000L + (end_time.tv_usec - start_time.tv_usec) / 1000L;
    if (calculated_checksum == checkSum && !qFlag)
    {
        printf("[+] Valid Checksum !.\n");
    }
    if (!qFlag)
    {
        printf("---------------------------------------\n");
        printf("|    Received a Chunk of bytes         \n");
        printf("| Type : UDS   | Param : stream \n");
        printf("| Info : Bytes received: %zd\n", total_bytes_received);
        printf("|  Time spent: %ld milliseconds       \n", time_spent_ms);
        double percentage_received = ((double)total_bytes_received / SIZE) * 100;
        printf("| Percentage bytes received: %.2f%%    \n", percentage_received);
        printf("| Checksum : %ld                        \n", calculated_checksum);
        printf("---------------------------------------\n");
    }
    else
    {
        printf("uds_stream,%ld", time_spent_ms);
    }
    close(client_fd);
    close(server_fd);
    unlink(socket_path);
    free(data);
}

// Establishing a IPv4 UDP connection to be able to recive chuck of 100MB.
void UDPipv6(int port)
{
    int server_fd;
    struct sockaddr_in6 server_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);
    memset(buffer, '0', BUFFER_SIZE);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET6, SOCK_DGRAM, 0)) == 0)
    {
        perror("[-] Socket failed.\n");
        exit(1);
    }

    // Will free an already bound port that's not in use.
    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port + 1);

    // Forcefully attaching socket to the port 8080
    if (!qFlag)
    {
        printf("[+] Binding to port %d\n", port + 1);
    }
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[-] Bind failed.\n");
        exit(1);
    }

    char *data = (char *)malloc(SIZE);
    ssize_t total_bytes_received = 0;
    long calculated_checksum = 0;

    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN;
    if (!qFlag)
    {
        printf("[+] Waiting for data...\n");
    }
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    while (total_bytes_received < SIZE)
    {
        int result = poll(&pfd, 1, TIME_OUT * 1000);
        if (result < 0)
        {
            perror("poll");
            exit(1);
        }
        else if (result == 0)
        {
            if (!qFlag)
            {
                printf("Timeout occurred.\n");
            }
            break;
        }
        else
        {
            ssize_t bytes_received = recvfrom(server_fd, data + total_bytes_received, SIZE - total_bytes_received, 0, (struct sockaddr *)&server_addr, &addr_len);
            if (bytes_received < 0)
            {
                perror("recvfrom");
                exit(1);
            }
            total_bytes_received += bytes_received;
        }
    }
    calculated_checksum = checksum(data);
    gettimeofday(&end_time, NULL);
    long time_spent_ms = (end_time.tv_sec - start_time.tv_sec) * 1000L + (end_time.tv_usec - start_time.tv_usec) / 1000L;
    if (calculated_checksum == checkSum && !qFlag)
    {
        printf("[+] Valid Checksum !.\n");
    }
    if (!qFlag)
    {
        printf("---------------------------------------\n");
        printf("|    Received a Chunk of bytes         \n");
        printf("| Type : Ipv6   | Param : UDP protocol \n");
        printf("| Info : Bytes received: %zd\n", total_bytes_received);
        printf("|  Time spent: %ld milliseconds       \n", time_spent_ms);
        double percentage_received = ((double)total_bytes_received / SIZE) * 100;
        printf("| Percentage bytes received: %.2f%%    \n", percentage_received);
        printf("| Checksum : %ld                        \n", calculated_checksum);
        printf("---------------------------------------\n");
    }
    else
    {
        printf("ipv6_udp,%ld\n", time_spent_ms);
    }
    free(data);
    close(server_fd);
    exit(0);
}

// Establishing a IPv4 UDP connection to be able to recive chuck of 100MB.
void UDPipv4(int port)
{
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int opt = 1;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    memset(buffer, '0', BUFFER_SIZE);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
    {
        perror("[-] Socket failed.\n");
        exit(1);
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
    if (!qFlag)
    {
        printf("[+] Binding to port %d\n", port + 1);
    }
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[-] Bind failed.\n");
        exit(1);
    }

    char *data = (char *)malloc(SIZE);
    ssize_t total_bytes_received = 0;
    long calculated_checksum = 0;

    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN;

    if (!qFlag)
    {
        printf("[+] Waiting for data...\n");
    }
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    while (total_bytes_received < SIZE)
    {
        int result = poll(&pfd, 1, TIME_OUT * 1000);
        if (result < 0)
        {
            perror("poll");
            exit(1);
        }
        else if (result == 0)
        {
            if (!qFlag)
            {
                printf("Timeout occurred.\n");
            }
            break;
        }
        else
        {
            ssize_t bytes_received = recvfrom(server_fd, data + total_bytes_received, SIZE - total_bytes_received, 0,
                                              (struct sockaddr *)&client_addr, &addr_len);
            if (bytes_received < 0)
            {
                perror("recvfrom");
                exit(1);
            }
            total_bytes_received += bytes_received;
        }
    }

    if (total_bytes_received == SIZE)
    {
        calculated_checksum = checksum(data);
    }

    gettimeofday(&end_time, NULL);
    long time_spent_ms = (end_time.tv_sec - start_time.tv_sec) * 1000L + (end_time.tv_usec - start_time.tv_usec) / 1000L;
    if (calculated_checksum == checkSum && !qFlag)
    {
        printf("[+] Valid Checksum !.\n");
    }
    if (!qFlag)
    {
        printf("---------------------------------------\n");
        printf("|    Received a Chunk of bytes         \n");
        printf("| Type : Ipv4   | Param : UDP protocol \n");
        printf("| Info : Bytes received: %zd\n", total_bytes_received);
        printf("|  Time spent: %ld milliseconds       \n", time_spent_ms);
        double percentage_received = ((double)total_bytes_received / SIZE) * 100;
        printf("| Percentage bytes received: %.2f%%    \n", percentage_received);
        printf("| Checksum : %ld                        \n", calculated_checksum);
        printf("---------------------------------------\n");
    }
    else
    {
        printf("ipv4_udp,%ld\n", time_spent_ms);
    }
    free(data);
    close(server_fd);
    exit(0);
}

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
        exit(1);
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
    if (!qFlag)
    {
        printf("[+] Binding to port %d\n", port + 1);
    }
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[-] Bind failed.\n");
        exit(1);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("[-] Listen failed.\n");
        exit(1);
    }
    client_len = sizeof(client_addr);
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
    {
        perror("[-] Accept failed.\n");
        exit(1);
    }
    if (!qFlag)
    {
        printf("[+] Waiting for data...\n");
    }
    ssize_t bytes_received = 0;
    ssize_t total_bytes_received = 0;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    char *data = (char *)malloc(SIZE);
    int size = 1500;
    while ((bytes_received = recv(client_fd, data + total_bytes_received, size, 0)) > 0)
    {
        if (size + total_bytes_received > SIZE)
        {
            size = SIZE - total_bytes_received;
        }
        total_bytes_received += bytes_received;
    }
    long calculated_checksum = checksum(data);

    gettimeofday(&end_time, NULL);
    long time_spent_ms = (end_time.tv_sec - start_time.tv_sec) * 1000L + (end_time.tv_usec - start_time.tv_usec) / 1000L;
    if (calculated_checksum == checkSum && !qFlag)
    {
        printf("[+] Valid Checksum !.\n");
    }
    if(!qFlag){
        printf("---------------------------------------\n");
        printf("|    Received a Chuck of bytes         \n");
        printf("| Type : Ipv4   | Param : TCP protcol  \n");
        printf("| Info : Bytes received: %zd\n", total_bytes_received);
        printf("|  Time spent: %ld milliseconds       \n", time_spent_ms);
        double percentage_received = ((double)total_bytes_received / SIZE) * 100;
        printf("|Percentage bytes received: %.2f%%    \n", percentage_received);
        printf("| Checksum : %ld                        \n", calculated_checksum);
        printf("---------------------------------------\n");
    }
    else{
        printf("ipv4_tcp,%ld",time_spent_ms);
    }
    free(data);
    close(server_fd);
    close(client_fd);
    exit(0);
}

// Establishing a IPv6 TCP connection to be able to recive chuck of 100MB.
void TCPipv6(int port)
{
    int server_fd, client_fd;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t client_len;
    int opt = 1;
    char buffer[BUFFER_SIZE];
    memset(buffer, '0', BUFFER_SIZE);
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET6, SOCK_STREAM, 0)) == 0)
    {
        perror("[-] Socket failed.\n");
        exit(1);
    }
    // Will free an already bound port that's not in use.
    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port + 1);
    // Forcefully attaching socket to the port 8080
    if (!qFlag)
    {
        printf("[+] Binding to port %d\n", port + 1);
    }
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[-] Bind failed.\n");
        exit(1);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("[-] Listen failed.\n");
        exit(1);
    }
    client_len = sizeof(client_addr);
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
    {
        perror("[-] Accept failed.\n");
        exit(1);
    }
    if (!qFlag)
    {
        printf("[+] Waiting for data...\n");
    }
    ssize_t bytes_received = 0;
    ssize_t total_bytes_received = 0;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL); // Utilisez gettimeofday au lieu de time
    char *data = (char *)malloc(SIZE);
    while ((bytes_received = recv(client_fd, data + total_bytes_received, BUFFER_SIZE, 0)) > 0)
    {
        total_bytes_received += bytes_received;
    }
    long calculated_checksum = checksum(data);
    printf("Checksum : %ld\n", calculated_checksum);

    gettimeofday(&end_time,
                 NULL); // Utilisez gettimeofday au lieu de time
    long time_spent_ms = (end_time.tv_sec - start_time.tv_sec) * 1000L +
                         (end_time.tv_usec - start_time.tv_usec) / 1000L; // Calculez le temps écoulé en ms
    if (calculated_checksum == checkSum && !qFlag)
    {
        printf("[+] Valid Checksum !.\n");
    }
    if(!qFlag){
        printf("---------------------------------------\n");
        printf("|    Received a Chuck of bytes         \n");
        printf("| Type : Ipv6   | Param : TCP protcol  \n");
        printf("| Info : Bytes received: %zd\n", total_bytes_received);
        printf("|  Time spent: %ld milliseconds       \n", time_spent_ms);
        double percentage_received = ((double)total_bytes_received / SIZE) * 100;
        printf("|Percentage bytes received: %.2f%%    \n", percentage_received);
        printf("| Checksum : %ld                        \n", calculated_checksum);
        printf("---------------------------------------\n");
    }
    else{
        printf("ipv6_tcp,%ld",time_spent_ms);
    }
    free(data);
    close(server_fd);
    close(client_fd);
    exit(0);
}

void socketFactory(char *type, char *param, int port)
{
    if (strcmp(type, "pipe") == 0)
    {
        char *PIPNAME = param;
        pipe_client(PIPNAME);
    }
    if (strcmp(type, "mmap") == 0)
    {
        char *FILENAME = param;
        mmap_server(FILENAME);
    }

    if (strcmp(type, "uds") == 0)
    {
        char *socket_path = "/tmp/socket";
        if (strcmp(param, "dgram") == 0)
        {
            UDSdgram(socket_path);
        }
        else
        {
            UDSstream(socket_path);
        }
    }
    if (strcmp(type, "ipv4") == 0)
    {
        if (strcmp(param, "tcp") == 0)
        {
            TCPipv4(port);
        }
        else if (strcmp(param, "udp") == 0)
        {
            UDPipv4(port);
        }
        else
        {  
            if(!qFlag){
                printf("Invalid parameter.\n");
            }
        }
    }
    if (strcmp(type, "ipv6") == 0)
    {
        if (strcmp(param, "tcp") == 0)
        {
            TCPipv6(port);
        }
        else if (strcmp(param, "udp") == 0)
        {
            UDPipv6(port);
        }
        else
        {   
            if(!qFlag){
                printf("Invalid parameter.\n");
            }
        }
    }
}

void recArgs(int client_fd, char *type, char *param)
{
    recv(client_fd, type, ARGSIZE, 0);
    recv(client_fd, param, ARGSIZE, 0);
    recv(client_fd, &checkSum, sizeof(long), 0);
    if(!qFlag){
        printf("[+] Reciving the parameters [%s] [%s]\n", type, param);
    }
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
    printf("[-->] Chat started.\n");
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
    char *type = (char *)malloc(ARGSIZE);
    char *param = (char *)malloc(ARGSIZE);
    if (argc == 3 && strcmp(argv[2], "-p") == 0)
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
    if(!qFlag){
        printf("[+] Socket created.\n");
    }
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
    if(!qFlag){
        printf("[+] Bind done.\n");
    }
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0)
    {
        printf("\n Error : Connect Failed \n");
        close(server_fd);
        close(client_fd);
        return 1;
    }
    if (pFlag == 1)
    {
        recArgs(client_fd, type, param);
        socketFactory(type, param, port);
    }
    else
    {
        printf("[+] Connecting to the chat...\n");
        chatTCP(client_fd, server_fd);
    }
    close(client_fd);
    close(server_fd);
    return 0;
}