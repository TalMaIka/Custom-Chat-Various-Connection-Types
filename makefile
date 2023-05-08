all: client server stnc
	
server: server_tcp_ipv4.c
	gcc server_tcp_ipv4.c -o server

stnc: stnc.c
	gcc stnc.c -o stnc

client: client_tcp_ipv4.c
	gcc client_tcp_ipv4.c -o client
	
clean: 
	rm -rf *.o server client clientipv6 serveripv6 stnc client_udp_ipv4 server_udp_ipv4

remake: clean all