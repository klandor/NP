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
	int t, read_size=0, write_size=0, header_size;
	while (1) {
		
		t = read(0, rbuff+read_size, BUFFER_SIZE-read_size);
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
				header_size = i+1;
				break;
			}
		}
		
		if (t) {
			break;
		}
	}
	
	if ( (rbuff[1]!=0x01 && rbuff[1]!=0x02) ) {
		cerr << "wrong command." << endl;
		socks_fail();
	}
	
	
	
	if (rbuff[4] == 0 && rbuff[5] == 0 && rbuff[6] == 0)
	{
		// SOCKS 4a
	}
	else {
		if (header_size == read_size) {
			read_size = 0;
		}
	}

	bool c_off = 0, s_off = 0;
	int s;
	
	if(rbuff[1]==0x01){ // connect
	
	
		s = socket(AF_INET,SOCK_STREAM,0);
		struct sockaddr_in client_sin;
		bzero(&client_sin, sizeof(client_sin));
		client_sin.sin_family = AF_INET;
		client_sin.sin_addr = *((struct in_addr *)(rbuff+4));
		client_sin.sin_port = *((uint16_t *)(rbuff +2));
		
		cerr << "Connecting to " <<  inet_ntoa(client_sin.sin_addr) <<":"<<ntohs(client_sin.sin_port)<<  endl;
		
		if(connect(s,(struct sockaddr *)&client_sin,
				   sizeof(client_sin)) == -1)
		{
			perror("connect");
			socks_fail();
		}
		
		char ac[8] = {0, 0x5a};
		
		write(1, ac, 8);
		
		
	}
	else { // bind
		
	}
	
	
	fd_set rfds, fds;
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	FD_SET(s, &rfds);
	fds = rfds;
	while (1) {
		
		if (c_off && s_off) {
			break;
		}
		
		if(select(s+1, &fds, 0, 0, 0) != -1)
			break;
		
		
		if (FD_ISSET(0, &fds)) {
			int t = read(0, rbuff+read_size, BUFFER_SIZE - read_size);
			if (t<0) {
				perror("read");
				exit(1);
			}
			else {
				if(t>0){
					read_size += t;
				}
				else {
					c_off = 1;
					FD_CLR(0, &rfds);
				}
			}
			
			while (read_size >0) {
				
				
				t = write(s, rbuff, read_size);
				if (t < 0) {
					perror("write");
					exit(1);
				}
				read_size -= t;
			}
			
		}
		
		
		if (FD_ISSET(s, &fds)) {
			int t = read(s, wbuff+write_size, BUFFER_SIZE - write_size);
			if (t<0) {
				perror("read");
				exit(1);
			}
			else {
				if(t>0){
					write_size += t;
				}
				else {
					s_off = 1;
					FD_CLR(s, &rfds);
				}
			}
			
			while (write_size >0) {
				
				
				t = write(1, wbuff, write_size);
				if (t < 0) {
					perror("write");
					exit(1);
				}
				write_size -= t;
			}
			
		}
		
		fds = rfds;
	}

	cerr << "client connection closed" << endl;
	
	return 0;
}
