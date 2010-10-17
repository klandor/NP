all: server mysh client

server:
	g++ server.cpp -o $@
	
mysh:
	g++ mysh.cpp -o $@
	
client:
	g++ -o client client.c
	
.PHONY: server mysh client