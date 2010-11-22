#include <iostream>

#include <sstream>
#include <sys/socket.h>
#include <signal.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <map>
#include <set>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_PIPE 1200
#define MAX_CLIENT 30
using namespace std;



class clientd {
public:
	clientd()
	{
		nick = "(no name)";
		path = "bin:.";
		for (int i=0; i<MAX_CLIENT+1; i++) {
			in_pipes[i] = -1;
		}
	}
	clientd(int fd, int no)
	{
		this->fd = fd;
		this->no = no;
		nick = "(no name)";
		path = "bin:.";
		for (int i=0; i<MAX_CLIENT+1; i++) {
			in_pipes[i] = -1;
		}
	}
	
	~clientd()
	{
		for (int i=0; i<MAX_CLIENT+1; i++) {
			if (in_pipes[i]>=0) {
				close(in_pipes[i]);
			}
		}
	}
	
	int exec_line(string line);
	int myexec(vector<string> &arglist, int &new_cmdNO, int read_fd, int write_fd);
//private:
	void clearpipe(int cmdNO);
	int fd, no, cmdNO, in_pipes[MAX_CLIENT+1];
	string nick, path, ip;
	map<int, int> pipe_queue;
};

int readline(int fd,char *ptr,int maxlen)
{
	int n,rc;
	char c;
	*ptr = 0;
	for(n=1;n<maxlen;n++)
	{
		if((rc=read(fd,&c,1)) == 1)
		{
			*ptr++ = c;
			if(c=='\n')  
			{
				//read(fd,&c,1);
				break;
			}
		}
		else if(rc==0)
		{
			if(n==1)     return(0);
			else         break;
		}
		else
			return(-1);
	}
	return(n);
}

string ipToString(int ip){
	ostringstream oss;
	oss << (ip & 255);
	oss << '.';
	oss << ((ip >> 8) & 255);
	oss << '.';
	oss << ((ip >> 16) & 255);
	oss << '.';
	oss << ((ip >> 24) & 255);
	
	return oss.str();
	
}

// global varibles
fd_set afds;
map<int, clientd> clients;
set<int> free_client_no;

void broadcast(string s)
{
	for(map<int, clientd>::iterator i = clients.begin(); i!=clients.end(); i++)
	{
		write(i->second.fd, s.c_str(), s.size());
		
	}
}

int main (int argc, char * const argv[]) {
	//signal(SIGCHLD, SIG_IGN);
	dup2(1, 3);
	
	
    int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ServerSocket <0) {
		cerr << "can't not open socket\n";
	}

	struct sockaddr_in ServerAddress;
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(56523);
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	memset(&ServerAddress.sin_zero, 0, sizeof ServerAddress.sin_zero);
	const int on=1;
	if( setsockopt(ServerSocket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) <0 )
		cerr<<"Set Reuse Error!!\n";
	
	if(bind(ServerSocket,(struct sockaddr *)&ServerAddress,sizeof(ServerAddress))==-1)
	{
		cerr<<"Bind Error!\n";
	}
	
	if(listen(ServerSocket,10)==-1)
	{
		cerr<<"Listen Error!!\n";
	}
	
	struct sockaddr_in  cln;
	socklen_t sLen=sizeof(cln);
	
	
	for (int i=1; i<=MAX_CLIENT; i++) {
		free_client_no.insert(i);
	}
	int nfds = ServerSocket+1;
	fd_set rfds, wfds;
	FD_ZERO(&afds);
	FD_SET(ServerSocket, &afds);
	while (1) {
		rfds = afds;
		wfds = afds;
		FD_SET(ServerSocket, &rfds);
		
		if (select(nfds, &rfds, NULL, NULL, NULL) < 0){
			perror("select");
			exit(-1);
		}
		
		
		
		if (FD_ISSET(ServerSocket, &rfds)) {
			//int tmp = -1; 
			int tmp=accept(ServerSocket,(struct sockaddr *)& cln,&sLen);
			if(tmp<0){
				perror("accept()");
			}
			FD_SET(tmp, &afds);
			
			if (free_client_no.size()>0) {
				clientd c(tmp, *free_client_no.begin());
				c.ip = inet_ntoa(cln.sin_addr);
				clients[c.no]= c;
				free_client_no.erase(free_client_no.begin());
				cout << "Client " << c.no << " just entered.\n";
				broadcast("*** User '(no name)' entered from " + c.ip +". ***\n");
			}
			
			//client_no.insert(tmp);
			if (tmp+1 > nfds) {
				nfds = tmp+1;
			}
		}
		
		
		
		for(map<int, clientd>::iterator i = clients.begin(); i!=clients.end(); i++)
		{
			if (FD_ISSET(i->second.fd, &rfds) ) {
				char t[50000];
				int len = readline(i->second.fd, t, 49999);
				if (len == 0) {
					cout << "Client " << i->second.no << " leaved.\n";
					
					FD_CLR(i->second.fd, &afds);
					close(i->second.fd);
					free_client_no.insert(i->second.no);
					clients.erase(i);
					
					continue;
				}
				t[len] = 0;
				i->second.exec_line(string(t));
				//break;
				if (!FD_ISSET(i->second.fd, &afds)) {
					int no = i->second.no;
					for(map<int, clientd>::iterator j = clients.begin(); j!=clients.end(); j++)
					{
						if (j->second.in_pipes[no] >=0) {
							close( j->second.in_pipes[no] );
							j->second.in_pipes[no] = -1;
						}
					}
					clients.erase(i);
					break;
				}
			}
		}
		

	}
	
    return 0;
}

int clientd::exec_line(string line){
	istringstream iss(line);
	{
		istringstream iss2(line);
		getline(iss2, line, '\n');
		istringstream iss3(line);
		getline(iss3, line, '\r');
	}
	
	string s;
//	vector<string> cmdlist;
//	while (iss >> s) {
//		cmdlist.push_back(s);
//	}
	int target_pipe, read_fd=-1, write_fd=-1, file_fd = -1;
	FILE* f;
	vector<string> arglist;
	while (iss >> s) {
		switch (s[0]) {
			case '|':
				target_pipe = atoi(s.c_str()+1);
				if (pipe_queue.find((cmdNO+1+target_pipe) % MAX_PIPE) 
					== pipe_queue.end()) { 
					//target pipe not found
					int fd[2];
					pipe(fd);
					pipe_queue[((cmdNO+1+target_pipe) % MAX_PIPE)] = fd[1];
					pipe_queue[MAX_PIPE+((cmdNO+1+target_pipe) % MAX_PIPE)] = fd[0];
				}
				
				write_fd = pipe_queue[((cmdNO+1+target_pipe) % MAX_PIPE)];
				
				if (pipe_queue.find(MAX_PIPE + cmdNO) != pipe_queue.end()) {
					read_fd = pipe_queue[MAX_PIPE + cmdNO];
				}
				
				if(arglist.size()>0){
					myexec(arglist, cmdNO, read_fd, write_fd);
				}
				read_fd = -1; write_fd = -1;
				break;
			case '<':
				target_pipe = atoi(s.c_str()+1);
				if (target_pipe == 0)  // file case
				{
					iss >> s;
					f = fopen(("ras/"+s).c_str(), "r");
					if (f != NULL) {
						pipe_queue[MAX_PIPE + cmdNO] = fileno(f);
					}
					else {
						cerr << "file: " << s << " can't be opened for read." << endl;
					}
				}
				else {		// user pipe case
					if (in_pipes[target_pipe] <0) {
						//*** Error: the pipe #7->#3 does not exist yet. ***
						ostringstream oss;
						oss << "*** Error: the pipe #"<< target_pipe <<"->#"<< no <<" does not exist yet. ***\n";
						string s = oss.str();
						write(fd, s.c_str(), s.size());
						arglist.insert(arglist.begin(), "noop");
					}
					else {
						pipe_queue[MAX_PIPE + cmdNO] = in_pipes[target_pipe];
						in_pipes[target_pipe] = -1;
						//*** student3 (#3) just received from student7 (#7) by 'cat <7' ***
						ostringstream oss;
						oss <<"*** "<< nick <<" (#"<< no <<") just received from ";
						oss << clients.find(target_pipe)->second.nick;
						oss <<" (#"<< target_pipe <<") by '"<< line <<"' ***\n";
						broadcast( oss.str() );
					}

				}

				
				break;
			case '>':
				target_pipe = atoi(s.c_str()+1);
				if (target_pipe == 0)  // file case
				{
					iss >>s;
					f = fopen(("ras/"+s).c_str(), "w");
					if (f != NULL) {
						file_fd = fileno(f);
					}
					else {
						cerr << "file: " << s << " can't be opened for write." << endl;
					}
				}
				else {		// user pipe case
					if (clients.find(target_pipe) == clients.end()) { // target user not found
						ostringstream oss;
						oss << "Error: target user id '" << target_pipe <<  "' not found.\n";
						string s = oss.str();
						write(fd, s.c_str(), s.size());
						arglist.insert(arglist.begin(), "noop");
					}
					else {
						if (clients.find(target_pipe)->second.in_pipes[no] >= 0 ) {
							ostringstream oss;
							oss <<"*** Error: the pipe #"<< no <<"->#"<< target_pipe<<" already exists. ***\n";
							string s = oss.str();
							write(fd, s.c_str(), s.size());
							arglist.insert(arglist.begin(), "noop");
						}
						else {
							int pipe_fd[2];
							pipe(pipe_fd);
							clients.find(target_pipe)->second.in_pipes[no] = pipe_fd[0];
							file_fd = pipe_fd[1];
							//*** student7 (#7) just piped 'cat test.html >3' to student3 (#3) ***
							ostringstream oss;
							oss <<"*** "<< nick <<" (#"<< no <<") just piped '"<< line << "' to ";
							oss << clients.find(target_pipe)->second.nick <<" (#"<< target_pipe <<") ***\n";
							broadcast( oss.str() );
							
						}
					}
					
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
				arglist.push_back(s);
		}
		
	}
	if (arglist.size()>0) {
		if (pipe_queue.find(MAX_PIPE + cmdNO) != pipe_queue.end()) {
			read_fd = pipe_queue[MAX_PIPE + cmdNO];
		}
		if (file_fd >= 0) {
			write_fd = file_fd;
		}
		myexec(arglist, cmdNO,read_fd, write_fd);
		if (file_fd >= 0) {
			close(file_fd);
		}
		
	}
	return 0;
}

int clientd::myexec(vector<string> &arglist, int &new_cmdNO, int read_fd, int write_fd){
	int cmdNO = new_cmdNO;
	write(3, "*", 1);
	// new_cmdNO advances for 1
	new_cmdNO++;
	new_cmdNO %= MAX_PIPE;
	
	
	//cout << "command #" << cmdNO << endl;
	
	// special commands
	if (arglist[0] == "exit") {
		write(3, "\n client exit\n", 14);
		broadcast(" *** User '" + nick+ "' left. ***\n");
		
		FD_CLR(fd, &afds);
		close(fd);
		free_client_no.insert(no);
		//clients.erase(clients.find(no)); // taken care outside
	}
	
	if (arglist[0] == "setenv") {
		if (arglist[1] == "PATH") {
			path = arglist[2];
		}
		else {
			string s = "unknown val: " + arglist[2] + "\n";
			write(fd, s.c_str(), s.size());
		}

		//setenv(arglist[1].c_str(), arglist[2].c_str(), 1);
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
	}
	
	if (arglist[0] == "printenv") {
		//string s = getenv(arglist[1].c_str());
		string s = arglist[1] + "=" + path + "\n";
		if (write_fd < 0) {
			write(fd, s.c_str(), s.size());
		}
		else {
			
//			write(write_fd, arglist[1].c_str(), arglist[1].size());
//			write(write_fd, "=", 1);
//			write(write_fd, path.c_str(), path.size());
//			write(write_fd, "\n", 1);
			
			string s = arglist[1] + "=" + path + "\n";
			write(write_fd, s.c_str(), s.size());
		}
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
	}
	
	if (arglist[0] == "who"){
		ostringstream oss;
		for(map<int, clientd>::iterator i = clients.begin(); i!=clients.end(); i++)
		{
			oss << i->first << '\t' << i->second.nick << '\t' << i->second.ip;
			if (i->first == no) {
				oss << "\t<- me";
			}
			oss << '\n';
			
		}
		
		string s = oss.str();
		
		write(fd, s.c_str(), s.size());
		
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
	}
	
	if (arglist[0] == "name"){
		if (arglist.size() == 1) {
			string s = "Usage: name new_name\n";
			write(fd, s.c_str(), s.size());
		}
		else {
			nick = arglist[1];
			for (int i=2; i<arglist.size(); i++) {
				nick += " " + arglist[i];
				
			}
			broadcast("*** User from " + ip +" is named '" + nick + "'. ***\n");
		}

		
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
	}
	
	if (arglist[0] == "yell"){
		if (arglist.size() == 1) {
			string s = "Usage: yell <message>\n";
			write(fd, s.c_str(), s.size());
		}
		else {
			ostringstream oss;
			for(int i=1; i<arglist.size(); i++)
			{
				oss << ' ' << arglist[i];
			}
			broadcast("*** "+ nick +" yelled ***: " + oss.str() + "\n");
		}
		
		
		arglist.clear();
		clearpipe(cmdNO);
		return 0;
	}
	
	if (arglist[0] == "tell"){
		if (arglist.size() <= 2) {
			string s = "Usage: tell <user_id#> <message>\n";
			write(fd, s.c_str(), s.size());
		}
		else {
			istringstream iss(arglist[1]);
			int to;
			if ((iss >> to) && (clients.find(to) != clients.end() ) ) {
				ostringstream oss;
				for(int i=2; i<arglist.size(); i++)
				{
					oss << ' ' << arglist[i];
				}
				string s("*** "+ nick +" told you ***: " + oss.str() + "\n");
				write(clients.find(to)->second.fd, s.c_str(), s.size() );
			}
			else {
				string s = "Error: user id '" + arglist[1]  + "' doesn't exit.\n";
				write(fd, s.c_str(), s.size());
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
		
		if(chdir("ras/")<0)
			perror("chdir");
		
		int r = setenv("PATH", path.c_str(), 1);
		if( r < 0)
			cerr << "setenv FAIL!\n";
		
		if (read_fd >=0) {
			dup2(read_fd, 0);
		}
		else {
			dup2(fd, 0);
		}

		if (write_fd >=0) {
			dup2(write_fd, 1);
		}
		else {
			dup2(fd, 1);
		}

		
		for(map<int,int>::iterator i = pipe_queue.begin(); i != pipe_queue.end(); i++)
			close( i->second);
		
		execvp(args[0], args);
		//cerr <<"Unknown Command: "<< args[0] << endl;
		//cerr << "errno: " << errno << endl; 
		write(fd, "Unknown Command: ", 17);
		write(fd, args[0], strlen(args[0]) );
		write(fd, "\n", 1);
		exit(-1);
	}
	
	arglist.clear();	
	
	
	return 0;
}

void clientd::clearpipe(int cmdNO){
	if (pipe_queue.find(cmdNO) != pipe_queue.end()) {
		close(pipe_queue[cmdNO]);
		pipe_queue.erase(cmdNO);
	}
	if (pipe_queue.find(MAX_PIPE + cmdNO) != pipe_queue.end()) {
		close(pipe_queue[MAX_PIPE + cmdNO]);
		pipe_queue.erase(MAX_PIPE + cmdNO);
	}
}
