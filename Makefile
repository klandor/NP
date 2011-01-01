all: server socks

server:
	g++ -g server.cpp -o $@
	
socks:
	g++ -g socks.cpp -o $@
	
client:
	rm -Rf ras
	cp -r ras-default ras
	g++ -g -o client client.c
	g++ -o ras/bin/noop noop.cpp
	g++ -o ras/bin/number number.cpp
	g++ -o ras/bin/removetag removetag.cpp
	cp /bin/ls /bin/cat ras/bin/

push:
	git push github hw4
pull:
	git pull github hw4

.PHONY: server mysh client push pull socks
