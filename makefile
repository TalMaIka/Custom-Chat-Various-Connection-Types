all: client server stnc clientipv6 serveripv6 client_udp_ipv4 server_udp_ipv4
	
server: server_tcp_ipv4.c
	gcc server_tcp_ipv4.c -o server

stnc: stnc.c
	gcc stnc.c -o stnc

client: client_tcp_ipv4.c
	gcc client_tcp_ipv4.c -o client

clientipv6: client_ipv6.c
	gcc client_ipv6.c -o clientipv6

serveripv6: server_ipv6.c
	gcc server_ipv6.c -o serveripv6

client_udp_ipv4: client_udp_ipv4.c
	gcc client_udp_ipv4.c -o client_udp_ipv4

server_udp_ipv4: server_udp_ipv4.c
	gcc server_udp_ipv4.c -o server_udp_ipv4
	
clean: 
	rm -rf *.o server client clientipv6 serveripv6 stnc client_udp_ipv4 server_udp_ipv4