all: client server
	
server: server.c
	gcc server.c -o server
	
client: client.c
	gcc client.c -o client
	
.PHONY: clean all
	
clean: 
	rm -rf *.o server client