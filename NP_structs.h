/*
 *  NP_structs.h
 *  NP_project
 *
 *  Created by 刁培倫 on 2010/11/20.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include <string>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_CLIENT 30
#define MAX_BUFF_SIZE 10
#define SHM_KEY 56523
using namespace std;


class clientd {
public:
	void init(int new_pid){
		f = r =0;
		strcpy(nick , "(no name)");
		pid = new_pid;
	}
	int buff_insert(string s){
		strcpy( buff[r], s.c_str());
		r++;
		r%=MAX_BUFF_SIZE;
		kill(pid, SIGUSR1);
		return 0;
	}
	
	string buff_pop()
	{
		int of = f;
		f = (f+1)%MAX_BUFF_SIZE;
		return buff[of];
	}
	
	int buff_size(){
		return (r-f+MAX_BUFF_SIZE)%MAX_BUFF_SIZE;
	}
	
	void setNick(string s){
		strcpy(nick, s.c_str());
	}
//private:
	char buff[MAX_BUFF_SIZE][1030], nick[30], ip[20];
	int f,r, pid;
};

struct NP_ipc{
	bool free_client_no[MAX_CLIENT];
	int semid;
	clientd clients[MAX_CLIENT];
};

int broadcast(NP_ipc *ipc, string s, int no){
	for (int i=0; i<MAX_CLIENT; i++) {
		if ((!ipc->free_client_no[i]) && no != i) {
			ipc->clients[i].buff_insert(s);
			//kill(ipc->clients[i].pid, SIGUSR1);
		}
	}
	
	return 0;
}
