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


#define BUFFER_SIZE 100000

void socks_fail(){
	char buff[8] = {0, 0x5b};
	
	write(1, buff, 8);
	exit(1);
}

int main() { 
	signal(SIGCHLD, SIG_DFL);
	char rbuff[BUFFER_SIZE], wbuff[BUFFER_SIZE];
	int read_size=0, write_size=0, header_size;
	while (1) {
		
		int t = read(0, rbuff+read_size, BUFFER_SIZE-read_size);
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
		
		bool tmp=false;
		for (int i=8; i< read_size; i++) {
			if (rbuff[i] == 0) {
				tmp=true;
				header_size = i+1;
				break;
			}
		}
		
		if (tmp) {
			break;
		}
	}
	
	if ( (rbuff[1]!=0x01 && rbuff[1]!=0x02) ) {
		cerr << "wrong command." << endl;
		socks_fail();
	}
	
	
	
	if (rbuff[4] == 0 && rbuff[5] == 0 && rbuff[6] == 0 && rbuff[7] != 0)
	{
		// SOCKS 4a
		cerr << "version 4a, hostname: " ;
		int header2_size;
		while (1) {
			bool tmp=false;
			for (int i=header_size; i< read_size; i++) {
				if (rbuff[i] == 0) {
					tmp=true;
					header2_size = i+1;
					break;
				}
			}
			
			if (tmp) {
				break;
			}
			
			int t = read(0, rbuff+read_size, BUFFER_SIZE-read_size);
			if (t<0) {
				perror("read");
				exit(1);
			}
			else {
				read_size += t;
			}
			
//			if ( rbuff[0] != 0x04 ) {
//				cerr << "version not matched." << endl;
//				socks_fail();
//			}
		}
		
		cerr << rbuff+header_size ;
		
		
		struct hostent *he = gethostbyname(rbuff+header_size);
		
		if (he == NULL) {
			herror("gethostbyname");
			socks_fail();
		}
		
		//int listen_socket = socket(AF_INET,SOCK_STREAM,0);
		memcpy(rbuff+4, he->h_addr, 4);
		header_size = header2_size;
		
	}
	else {
		// SOCKS 4
		cerr << "version 4, IP: ";
		struct in_addr ip;
		ip.s_addr = *((unsigned long *)(rbuff+4));
		cerr << inet_ntoa(ip) ;

	}
	// print Port
	{
		uint16_t port = ntohs(*((uint16_t*)(rbuff+2)));
		cerr << ", PORT: " << port << endl;
	}
	

	bool c_off = 0, s_off = 0;
	int s;
	
	if(rbuff[1]==0x01){ // connect
	
		cerr << "CD CONNECT "; 
	
		s = socket(AF_INET,SOCK_STREAM,0);
		struct sockaddr_in client_sin;
		bzero(&client_sin, sizeof(client_sin));
		client_sin.sin_family = AF_INET;
		client_sin.sin_addr = *((struct in_addr *)(rbuff+4));
		client_sin.sin_port = *((uint16_t *)(rbuff +2));
		
		//cerr << "to " <<  inet_ntoa(client_sin.sin_addr) <<":"<<ntohs(client_sin.sin_port)<<  endl;
		
		if(connect(s,(struct sockaddr *)&client_sin,
				   sizeof(client_sin)) == -1)
		{
			perror("connect");
			socks_fail();
		}
		else {
			cerr << "successed." << endl;
		}

		
		char ac[8] = {0, 0x5a};
		
		write(1, ac, 8);
		
		
	}
	else { // bind
		cerr << "CD BIND ";
		
		int ServerSocket = socket(AF_INET,SOCK_STREAM,0);
		if (ServerSocket <0) {
			perror("socket");
			socks_fail();
		}
		
		struct sockaddr_in ServerAddress;
		ServerAddress.sin_family = AF_INET;
		ServerAddress.sin_port = htons(0);
		ServerAddress.sin_addr.s_addr = INADDR_ANY;
		memset(&ServerAddress.sin_zero, 0, sizeof ServerAddress.sin_zero);
		const int on=1;
		if( setsockopt(ServerSocket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) <0 )
		{
			perror("setsockopt");
			socks_fail();
		}
		
		if(bind(ServerSocket,(struct sockaddr *)&ServerAddress,sizeof(ServerAddress))==-1)
		{
			perror("bind");
			socks_fail();
		}
		
		socklen_t socklen = sizeof(ServerAddress);
		getsockname(ServerSocket, (struct sockaddr *)&ServerAddress, &socklen);
		
		if(listen(ServerSocket,10)==-1)
		{
			perror("listen");
			socks_fail();
		}
		
		char ac[8] = {0, 0x5a, 0, 0, 0, 0, 0, 0};
		memcpy(ac+2, &(ServerAddress.sin_port), 2);
		write(1, ac, 8); // first granted reply
		
		
		cerr << "listening..." << endl;
		//int tmp = -1; 
		socklen = sizeof(ServerAddress);
		s=accept(ServerSocket,(struct sockaddr *)& ServerAddress,&socklen);
		if(s==-1){
			perror("accept");
			socks_fail();
		}
		
		if (ServerAddress.sin_addr.s_addr ==  *(in_addr_t*)(rbuff+4)  ) {
			write(1, ac, 8); // second granted reply
			cerr << "Accepted successfully." << endl;
		}
		else {
			cerr << "Acception failed." << endl;
			socks_fail();
		}
		
		close(ServerSocket);

	}
	
	if (header_size == read_size) {
		read_size = 0;
	}
	else {
		
		
		memcpy(rbuff, rbuff+header_size, read_size-header_size);
		read_size -= header_size;
		cerr << "header shifted. read_size="<<  read_size<< endl;
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
		
		if(select(s+1, &fds, 0, 0, 0) == -1)
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
			int write_out = 0;
			while ( read_size - write_out >0) {
				
				
				t = write(s, rbuff+ write_out, read_size - write_out);
				if (t < 0) {
					perror("write");
					exit(1);
				}
				write_out += t;
			}
			
			read_size = 0;
			
			if (c_off) {
				shutdown(s, SHUT_WR);
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
			
			int write_out = 0;
			while ( write_size - write_out >0) {
				
				
				t = write(1, wbuff+write_out, write_size - write_out);
				if (t < 0) {
					perror("write");
					exit(1);
				}
				write_out += t;
			}
			
			write_size = 0;
			if (s_off) {
				shutdown(1, SHUT_WR);
			}
		}
		
		fds = rfds;
	}

	cerr << "client connection closed" << endl;
	
	return 0;
}
