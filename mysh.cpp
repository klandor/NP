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
#include <sys/shm.h>
#include "NP_structs.h"
using namespace std;


#define MAX_PIPE 1200


map<int, int> pipes;
NP_ipc *ipc_data;
int my_no;


void clearpipe(int cmdNO){
	if (pipes.find(cmdNO) != pipes.end()) {
		close(pipes[cmdNO]);
		pipes.erase(cmdNO);
	}
	if (pipes.find(MAX_PIPE + cmdNO) != pipes.end()) {
		close(pipes[MAX_PIPE + cmdNO]);
		pipes.erase(MAX_PIPE + cmdNO);
	}
}

void pop_msg (int param)
{
	while (ipc_data->clients[my_no].buff_size() > 0) {
		cout << ipc_data->clients[my_no].buff_pop() << endl;
	}
}

int myexec(vector<string> &arglist, int &new_cmdNO, int read_fd, int write_fd){
	int cmdNO = new_cmdNO;
	write(3, "*", 1);
	// new_cmdNO advances for 1
	new_cmdNO++;
	new_cmdNO %= MAX_PIPE;
	
	
	//cout << "command #" << cmdNO << endl;
	
	// special commands
	if (arglist[0] == "exit") {
		broadcast(ipc_data, " *** User '" 
				  + string(ipc_data->clients[my_no].nick)+ "' left. ***\n");
		ipc_data->free_client_no[my_no]=1;
		write(3, "\n client exit\n", 14);
		exit(0);
	}
	
	if (arglist[0] == "setenv") {
		setenv(arglist[1].c_str(), arglist[2].c_str(), 1);
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
	}
	
	if (arglist[0] == "printenv") {
		string s = getenv(arglist[1].c_str());
		if (write_fd < 0) {
			cout << arglist[1] << '=' << s << endl;
		}
		else {
			
			write(write_fd, arglist[1].c_str(), arglist[1].size());
			write(write_fd, "=", 1);
			write(write_fd, s.c_str(), s.size());
			write(write_fd, "\n", 1);
			//write()
		}
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
	}
	
	if (arglist[0] == "who"){
		ostringstream oss;
		for(int i = 0; i<MAX_CLIENT; i++)
		{
			if (! (ipc_data->free_client_no[i])) {
				oss << i+1 << '\t' << ipc_data->clients[i].nick << '\t' << ipc_data->clients[i].ip;
				if (i == my_no) {
					oss << "\t<- me";
				}
				oss << '\n';
			}
		}
		
		cout << oss.str();
		
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
        }
	
	if (arglist[0] == "name"){
		if (arglist.size() == 1) {
			cout << "Usage: name new_name\n";
		}
		else {
			string nick = arglist[1];
			for (int i=2; i<arglist.size(); i++) {
				nick += " " + arglist[i];
				
			}
			ipc_data->clients[my_no].setNick(nick);
			broadcast(ipc_data, "*** User from " + string(ipc_data->clients[my_no].ip) +" is named '" + nick + "'. ***");
		}
		
		
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
	}
	
	if (arglist[0] == "yell"){
		if (arglist.size() == 1) {
			cout <<  "Usage: yell <message>\n";
					}
		else {  
			ostringstream oss;
			oss << "*** " << ipc_data->clients[my_no].nick 
				<< " yelled ***: ";
			for(int i=1; i<arglist.size(); i++)
			{
				oss << ' ' << arglist[i];
			}
			broadcast(ipc_data, oss.str());
		}
		
		
		arglist.clear();                clearpipe(cmdNO);
		return 0;
	}
	
	if (arglist[0] == "tell"){
		if (arglist.size() <= 2) {
			cout << "Usage: tell <user_id#> <message>\n";
		}
		else {  
			istringstream iss(arglist[1]);
			int to;
			if ((iss >> to) && (! (ipc_data->free_client_no[to-1] ) ) ) {
				ostringstream oss;
				for(int i=2; i<arglist.size(); i++)
				{
					oss << ' ' << arglist[i];
				}
				string s("*** "+ string(ipc_data->clients[my_no].nick) +" told you ***: " + oss.str());
				ipc_data->clients[to-1].buff_insert(s);
			}
			else {
				cout << "Error: user id '" + arglist[1]  + "' doesn't exit.\n";
				
			}
			
			
		}
		
		
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
	}
	
	//end special commands
	
	// setup argument list for child process
	char **args = new char*[arglist.size()+1];
	
	for (int i=0; i<arglist.size(); i++) {
		args[i] = new char[arglist[i].size()+1];
		strcpy(args[i], arglist[i].c_str());
	}
	args[arglist.size()] = 0;
	
	// forking
	int cpid =fork();
	if (cpid>0) {
		// clear args
		for (int i=0; i<arglist.size(); i++) {
			delete [] args[i]; 
		}
		delete [] args;
		
		clearpipe(cmdNO);
		while(wait(0)!=cpid); // wait for the child
	}
	else {
		if (read_fd >=0) {
			dup2(read_fd, 0);
		}
		if (write_fd >=0) {
			dup2(write_fd, 1);
		}
		
		for(map<int,int>::iterator i = pipes.begin(); i != pipes.end(); i++)
			close( i->second);
		
		execvp(args[0], args);
		cerr <<"Unknown Command: "<< args[0] << endl;
		//cerr << "errno: " << errno << endl; 
		exit(-1);
	}
	
	arglist.clear();	
	
	return 0;
}

int main(int argc, char * const argv[]) { 
	signal(SIGCHLD, SIG_DFL);
	
	
	ifstream wel;
	wel.open("welcome_message.txt");
	
	string line;
	while (getline(wel, line, '\n')){
		cout << line << endl;
	}
	

	int cmdNO = 0;
	if(chdir("ras/")<0)
		perror("chdir");
	
	
	{
		int r = setenv("PATH", "bin:.", 1);
		if( r < 0)
			perror("setenv");
		
		int shmid = shmget(SHM_KEY, sizeof(NP_ipc), SHM_R|SHM_W|IPC_CREAT);
		if (shmid<0) {
			perror("shmget");
			exit(-1);
		}
		ipc_data = (NP_ipc*) shmat(shmid, 0, 0);
	}
	if(argc > 1)
		my_no = atoi(argv[1]);
	else {
		my_no = 0;
		ipc_data->free_client_no[0] = 0;
		ipc_data->clients[0].init(getpid());
	}
	
	signal(SIGUSR1, pop_msg);
	broadcast(ipc_data, 
			  "*** User '(no name)' entered from " + 
			  string(ipc_data->clients[my_no].ip) +". ***");
	
//	cout << "% ";
//	cout.flush();
	
//	string line;
	while (getline(cin, line, '\n')) {
		
		istringstream iss(line);
		string s;
		vector<string> cmdlist;
		while (iss >> s) {
			cmdlist.push_back(s);
		}
		int target_pipe, read_fd=-1, write_fd=-1, file_fd = -1;
		FILE* f;
		vector<string> arglist;
		for (int i=0; i<cmdlist.size(); i++) {
			switch (cmdlist[i][0]) {
				case '|':
					target_pipe = atoi(cmdlist[i].c_str()+1);
					if (pipes.find((cmdNO+1+target_pipe) % MAX_PIPE) 
						== pipes.end()) { 
						//target pipe not found
						int fd[2];
						pipe(fd);
						pipes[((cmdNO+1+target_pipe) % MAX_PIPE)] = fd[1];
						pipes[MAX_PIPE+((cmdNO+1+target_pipe) % MAX_PIPE)] = fd[0];
					}
					
					write_fd = pipes[((cmdNO+1+target_pipe) % MAX_PIPE)];
					
					if (pipes.find(MAX_PIPE + cmdNO) != pipes.end()) {
						read_fd = pipes[MAX_PIPE + cmdNO];
					}
					
					if(arglist.size()>0){
						myexec(arglist, cmdNO, read_fd, write_fd);
					}
					read_fd = -1; write_fd = -1;
					break;
				case '<':
					i++;
					f = fopen(cmdlist[i].c_str(), "r");
					if (f != NULL) {
						pipes[MAX_PIPE + cmdNO] = fileno(f);
					}
					else {
						cerr << "file: " << cmdlist[i] << " can't be opened for read." << endl;
					}
					
					break;
				case '>':
					i++;
					f = fopen(cmdlist[i].c_str(), "w");
					if (f != NULL) {
						file_fd = fileno(f);
					}
					else {
						cerr << "file: " << cmdlist[i] << " can't be opened for write." << endl;
					}
					
					break;
					
					// for HW1 demo
//				case '#':
//					i++;
//					f = fopen(cmdlist[i].c_str(), "r");
//					if (f != NULL) {
//						pipes[MAX_PIPE + cmdNO] = fileno(f);
//					}
//					else {
//						cerr << "file: " << cmdlist[i] << " can't be opened for read." << endl;
//					}
//					f = fopen((cmdlist[i]+".").c_str(), "w");
//					if (f != NULL) {
//						file_fd = fileno(f);
//					}
//					else {
//						cerr << "file: " << cmdlist[i] << " can't be opened for write." << endl;
//					}
//					break;

				default:
					arglist.push_back(cmdlist[i]);
			}

		}
		if (arglist.size()>0) {
			if (pipes.find(MAX_PIPE + cmdNO) != pipes.end()) {
				read_fd = pipes[MAX_PIPE + cmdNO];
			}
			if (file_fd >= 0) {
				write_fd = file_fd;
			}
			myexec(arglist, cmdNO,read_fd, write_fd);
			if (file_fd >= 0) {
				close(file_fd);
			}
			
		}
		
//		cout << "% ";
//		cout.flush();
		
		
	}
	
	
	write(3, "\n client connection closed\n", 27);
	
	return 0;
}
