/*
 *  NP_structs.h
 *  NP_project
 *
 *  Created by 刁培倫 on 2010/11/20.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include <string>
#define MAX_CLIENT 30
#define MAX_BUFF_SIZE 100
#define SHM_KEY 56523
using namespace std;


class clientd {
public:
	void init();
	int insert();
	string get();
	int size();
private:
	char buff[MAX_BUFF_SIZE][1000], nick[50]. ip[20];
	int f,r;
};

struct NP_ipc{
	bool free_client_no[MAX_CLIENT];
	clientd buf[MAX_CLIENT];
};