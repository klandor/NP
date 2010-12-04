all: server mysh client

server:
	g++ -g server.cpp -o $@
	
mysh:
	g++ -g mysh.cpp -o $@
	
client:
	rm -Rf ras
	cp -r ras-default ras
	g++ -g -o client client.c
	g++ -o ras/bin/noop noop.cpp
	g++ -o ras/bin/number number.cpp
	g++ -o ras/bin/removetag removetag.cpp
	cp /bin/ls /bin/cat ras/bin/

push:
	git push github hw3-2

pull:
	git pull github hw3-2
.PHONY: server mysh client push pull
