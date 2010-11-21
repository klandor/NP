all: server mysh client

server:
	g++ -g server.cpp -o $@
	
mysh:
	g++ -g mysh.cpp -o $@
	
client:
	rm -Rf ras
	cp -r ras-default ras
	g++ -g -o client delayedclient_new.c
	g++ -o ras/bin/noop noop.cpp
	g++ -o ras/bin/number number.cpp
	g++ -o ras/bin/removetag removetag.cpp
	cp /bin/ls /bin/cat ras/bin/

ipcrm:
	ipcrm -M 56523 -S 56523

.PHONY: server mysh client ipcrm
