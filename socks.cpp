/*
 *  mysh.cpp
 *  NP_project
 *
 *  Created by 刁培倫 on 2010/10/15.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
using namespace std;


#define BUFFER_SIZE 5000

void socks_fail(){
	char buff[8] = {0, 0x5b};
	
	write(1, buff, 8);
	exit(1);
}

int main() { 
	signal(SIGCHLD, SIG_DFL);
	char rbuff[BUFFER_SIZE], wbuff[BUFFER_SIZE];
	int t, read_size=0, write_size=0;
	while (1) {
		
		t = read(1, rbuff+read_size, BUFFER_SIZE-read_size);
		if (t<0) {
			perror("read");
			exit(1);
		}
		else {
			read_size += t;
		}
		
		if ( rbuff[0] != 0x04 ) {
			cerr << "version not matched." << endl;
			socks_fail();
		}
		
		bool t=false;
		for (int i=8; i< read_size; i++) {
			if (rbuff[i] == 0) {
				t=true;
			}
		}
		
		if (t) {
			break;
		}
	}
	
	if (read_size < 9 )  {
		cerr << "protocol error: "<< read_size << "-byte header." << endl;
		socks_fail();
	}
	
	
	
	if ( (rbuff[1]!=0x01 && rbuff[1]!=0x02) ) {
		cerr << "wrong command." << endl;
		socks_fail();
	}
	
	
	
	if (rbuff[4] == 0 && rbuff[5] == 0 && rbuff[6] == 0)
	{
		// SOCKS 4a
	}
	
	
	
	int s = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in client_sin;
	bzero(&client_sin, sizeof(client_sin));
	client_sin.sin_family = AF_INET;
	client_sin.sin_addr = *((struct in_addr *)(rbuff+4));
	client_sin.sin_port = *((uint16_t *)(rbuff +2));
	
	cerr << "Connecting to " <<  inet_ntoa(client_sin.sin_addr) << endl;
	
	if(connect(s,(struct sockaddr *)&client_sin,
			   sizeof(client_sin)) == -1)
	{
		perror("connect");
		socks_fail();
	}
	
	char ac[8] = {0, 0x5a};
	
	write()
	
	cerr << "\nclient connection closed" << endl;
	
	return 0;
}
