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
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <map>
using namespace std;


#define MAX_PIPE 1200


map<int, int> pipes;

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

int myexec(vector<string> &arglist, int &new_cmdNO, int read_fd, int write_fd){
	int cmdNO = new_cmdNO;
	write(3, "*", 1);
	// new_cmdNO advances for 1
	new_cmdNO++;
	new_cmdNO %= MAX_PIPE;
	
	
	//cout << "command #" << cmdNO << endl;
	
	// special commands
	if (arglist[0] == "exit") {
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

int main() { 
	signal(SIGCHLD, SIG_DFL);
	string http_path = "/net/gcs/98/9856523/public_html";

	string method, request;
	
	chdir("/net/gcs/98/9856523/public_html");
	getline(cin, method, ' ');
	getline(cin, request, ' ');
	
	if (method == "GET") {
		istringstream iss(request);
		string query_path, query_string;
		getline(iss, query_path, '?');
		getline(iss, query_string);
		
		struct stat q_stat;
		
		if(stat((http_path+query_path).c_str(), &q_stat) <0 )
		{
			cout << "HTTP/1.1 404 Not Found\n";
			cout << "Content-type: text/html\n\n";
			cout << "<html><head></head><body><h1>404 Not Found</h1></body></html>\n";
			return 0;
		}
		
		if (query_path.substr(query_path.size()-4, 4) == ".cgi") {
			int found = query_path.find_last_of("/");
			chdir(query_path.substr(1, found-1).c_str());
			write(1,"HTTP/1.1 200 OK\n", 16);
			setenv("REQUEST_METHOD", method.c_str(), 1);
			setenv("QUERY_STRING", query_string.c_str(), 1);
			
			char** args = new char*[2];
			
			args[0] = new char[query_path.substr(found).size()];
			strcpy(args[0], query_path.substr(found).c_str());
			args[1] = 0;
			execvp(args[0], args);
			
			return 0;
		}
		
			
		
		cout << "HTTP/1.1 200 OK\n";
		
		cout << "Content-type: text/html\n\n";
		ifstream ifs( (http_path+query_path).c_str() );
		
		string s;
		while ( getline(ifs, s, '\0') ) {
			cout << s;
		}
	
	}
	else {
		cout << "HTTP/1.1 501 Not Implemented\n";
		cout << "Content-type: text/html\n\n";
		cout << "<html><head></head><body><h1>501 Not Implemented</h1><BR>Method not supported."
			<< " Only 'GET' is supported.</body></html>\n";
	}

		
	return 0;


	
}
