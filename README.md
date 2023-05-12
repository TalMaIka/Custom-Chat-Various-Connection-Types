# Custom Chat Various Connections Based.

The code includes a client program written in C that can communicate with a server using various communication methods, such as:
* **IPv4** and **IPv6** **TCP** sockets
* **IPv4** and **IPv6 UDP** sockets
* **Unix domain** sockets (**STREAM** and **DGRAM**)
* **Pipe**

 The client can preform a preformance test by applying the **"-p"** flag and will send **100MB chunk of data** and by the end of the transer of any **variation** the server side will print the details of the transfer.

### Here are the main methods used by each side client and server with a small summary:

**generete_random_data():** This function generates random data of size 100MB and returns a pointer to the allocated memory.

**checksum():** This function calculates the checksum of the given data by summing all the bytes.

**pipe_client():** This function creates a named pipe (FIFO) with the specified name (PIPENAME) and sends the data through the pipe.

**mmap_client()** This function opens a file with the specified name (FILENAME) and maps it to memory using the mmap() function. It then copies the data to the shared memory and sends it to the server.

**UDSstream():** This function creates a Unix domain stream socket, connects to the server using the specified socket path, and sends the data through the socket.

**UDSdgram():** This function creates a Unix domain datagram socket, sends the data to the server using the specified socket path, and sends the data through the socket.

**UDPipv6():** This function creates an IPv6 UDP socket, constructs the server address using the specified IP address and port, and sends the data to the server.

**UDPipv4():** This function creates an IPv4 UDP socket, constructs the server address using the specified IP address and port, and sends the data to the server.

**TCPipv4():** This function creates an IPv4 TCP socket, connects to the server using the specified IP address and port, and sends the data to the server.

**TCPipv6():** This function creates an IPv6 TCP socket, connects to the server using the specified IP address and port, and sends the data to the server.

**socketFactory():** This function acts as a factory for creating sockets based on the specified type and parameters. It calls the appropriate socket function based on the type and parameters provided.

**chatTCP():** This function handles a simple chat functionality over a TCP connection. It uses the poll() function to monitor the socket and stdin for incoming data. It receives messages from the server and sends messages entered by the user.

**sendArgs():** This function sends the type, parameter, and checksum to the server over the TCP connection.

**main():** The main function of the client program. It parses the command-line arguments to extract the IP address and port. If the -p flag is provided, it also extracts the communication type and parameter. It creates a TCP socket, connects to the server, sends the type and parameter if necessary, generates random data, sends the data to the server using the specified communication method, and then optionally engages in a chat session with the server over TCP.

Overall, the code allows the client to establish a connection with a server and send data using different communication methods. The choice of communication method can be specified via command-line arguments.
