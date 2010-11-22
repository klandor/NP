#include <iostream>
#include <sys/socket.h>
#include <signal.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "NP_structs.h"
using namespace std;
int main (int argc, char * const argv[]) {
	signal(SIGCHLD, SIG_IGN);
	dup2(1, 3);
	
	//cout << "sizeof(NP_ipc): " << sizeof(NP_ipc) << endl;
	int shmid = shmget(SHM_KEY, sizeof(NP_ipc), SHM_R|SHM_W|IPC_CREAT);
	if (shmid<0) {
		perror("shmget");
		exit(-1);
	}
	NP_ipc *ipc_data = (NP_ipc*) shmat(shmid, 0, 0);
	for (int i=0; i<MAX_CLIENT; i++)
		ipc_data->free_client_no[i] = 1;
	
//	ipc_data->semid = semget(SHM_KEY, MAX_CLIENT+1, SEM_R|SEM_A|IPC_CREAT);
//	if (ipc_data->semid<0) {
//		perror("shmget");
//		exit(-1);
//	}
//	else{
//		semun arg;
//		arg.val = 1;
//		for (int i=0; i<MAX_CLIENT+1; i++) {
//			if(semctl(ipc_data->semid, i, SETVAL, arg) <0)
//				perror("semctl");
//		}
//		
//	}
	
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
	while (1) {
		
		
		int tmp = -1; 
		tmp=accept(ServerSocket,(struct sockaddr *)& cln,&sLen);
		if(tmp<0){
			cerr<<"Accept Error!!\n";
		}
		
		int no=0;
		while (no<MAX_CLIENT) {
			if (ipc_data->free_client_no[no]) {
				ipc_data->free_client_no[no] = 0;
				strcpy(ipc_data->clients[no].ip, inet_ntoa(cln.sin_addr)) ;
				
				cout << "Client " << no << " just entered from "
					<<ipc_data->clients[no].ip <<".\n";
				
				break;
			}
			no++;
		}
		
		if (no == MAX_CLIENT) {
			string s = "Sorry, server full.\n";
			write(tmp , s.c_str(), s.size());
			close(tmp);
			continue;
		}
		
		int cpid = fork();
		
		if (cpid>0) {
			close(tmp);
			continue;
		}
		else {
			
			ipc_data->clients[no].init(getpid());

			dup2(tmp, 0);
			dup2(tmp, 1);
			dup2(tmp, 2);
			
			close(tmp);
			close(ServerSocket);
			char command[] = "./mysh";
			
			char * args[3] = { command, new char[10],0};
			
			sprintf(args[1], "%d", no);
			//cout << "prepare to exec...\n";
			cout.flush();
			execvp(command, args);
			perror("exec");
			ipc_data->free_client_no[no] = 1;
			exit(-1);
			//cout << "exec failed!\n";
		}
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
