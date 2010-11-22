all: server mysh client

server:
	g++ -g server.cpp -o $@
	
mysh:
	g++ -g mysh.cpp -o $@
	
client:
	rm -Rf ras
	rm -f *.pipe
	cp -r ras-default ras
	g++ -g -o client delayedclient_new.c
	g++ -o ras/bin/noop noop.cpp
	g++ -o ras/bin/number number.cpp
	g++ -o ras/bin/removetag removetag.cpp
	cp /bin/ls /bin/cat ras/bin/

ipcrm:
	ipcrm -M 56523

push:
	git push github master
pull:
	git pull github master

.PHONY: server mysh client ipcrm push pull
