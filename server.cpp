#include <iostream>
#include <sys/socket.h>
#include<netinet/in.h>
#include <unistd.h>
using namespace std;
int main (int argc, char * const argv[]) {
    int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ServerSocket <0) {
		cerr << "can't not open socket\n";
	}

	struct sockaddr_in ServerAddress;
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(3000);
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
	int tmp = -1; 
	tmp=accept(ServerSocket,(struct sockaddr *)& cln,&sLen);
	if(tmp==-1){
		cerr<<"Accept Error!!\n";
	}
	dup2(tmp, 0);
//	dup2(tmp, 1);
//	dup2(tmp, 2);
	
//	close(tmp);
	char command[] = "bash";
	
	char * args[2] = { command, 0};
	
	cout << "prepare to exec...\n";
	cout.flush();
	execvp(command, args);
	
	cout << "exec failed!\n";
	string s;
	while (cin >> s) {
		if (s == "exit") {
			break;
		}
		cout << s << '\n';
	}
	
	
	
    return 0;
}
