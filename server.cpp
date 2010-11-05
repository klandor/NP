#include <iostream>
#include <sys/socket.h>
#include <signal.h>
#include<netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <set>

using namespace std;

int main (int argc, char * const argv[]) {
	signal(SIGCHLD, SIG_IGN);
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
	
	set<int> clients;
	int nfds = ServerSocket+1;
	fd_set rfds, afds, wfds;
	FD_ZERO(&afds);
	FD_SET(ServerSocket, &afds);
	while (1) {
		rfds = afds;
		wfds = afds;
		FD_SET(ServerSocket, &rfds);
		
		if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0){
			perror("select");
			exit(-1);
		}
		
		if (clients.size()>0){
			if(select(nfds, (fd_set *)0, &wfds, (fd_set *)0, (struct timeval *)0) < 0){
				perror("select");
				exit(-1);
			}
		}
		for (set<int>::iterator i = clients.begin(); i!=clients.end(); i++)
		{
			if (!FD_ISSET(*i, &wfds)) {
				FD_CLR(*i, &afds);
				close(*i);
				cout << "Client " << *i << " leaved.\n";
				clients.erase(i);
			}
			else {
				write(*i, "Beep!\n", 6);
			}

		}
		
		int tmp = -1; 
		if (FD_ISSET(ServerSocket, &rfds)) {
			
			tmp=accept(ServerSocket,(struct sockaddr *)& cln,&sLen);
			if(tmp<0){
				perror("accept()");
			}
			FD_SET(tmp, &afds);
			clients.insert(tmp);
			if (tmp+1 > nfds) {
				nfds = tmp+1;
			}
		}
		
		
		
		for(set<int>::iterator i = clients.begin(); i!=clients.end(); i++)
		{
			if (FD_ISSET(*i, &rfds) ) {
				char t[5000];
				int len = read(*i, t, 4999);
				if (len == 0) {
					FD_CLR(*i, &afds);
					close(*i);
					cout << "Client " << *i << " leaved.\n";
					clients.erase(i);
					continue;
				}
				t[len] = 0;
				//t[len+1] = 0;
				cout << t ;
			}
		}
//		int cpid = fork();
//		
//		if (cpid>0) {
//			close(tmp);
//			continue;
//		}
//		else {
//			
//			write(tmp, "Bye\n", 6);
//			cout << "Child forked.\n";
//			exit(0);
//
//			dup2(tmp, 0);
//			dup2(tmp, 1);
//			dup2(tmp, 2);
//			
//			close(tmp);
//			close(ServerSocket);
//			char command[] = "./mysh";
//			
//			char * args[2] = { command, 0};
//			
//			//cout << "prepare to exec...\n";
//			cout.flush();
//			execvp(command, args);
//			
//			//cout << "exec failed!\n";
//		}
	}
//	string s;
//	while (cin >> s) {
//		if (s == "exit") {
//			break;
//		}
//		cout << s << '\n';
//	}
	
	
	
    return 0;
}
