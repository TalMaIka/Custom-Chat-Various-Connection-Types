all: client server stnc
	
server: server.c
	gcc server.c -o server

stnc: stnc.c
	gcc stnc.c -o stnc

client: client.c
	gcc client.c -o client
	
clean: 
	rm -rf *.o server client stnc

remake: clean all