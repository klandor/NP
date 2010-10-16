/*
 *  mysh.cpp
 *  NP_project
 *
 *  Created by 刁培倫 on 2010/10/15.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
using namespace std;


int main() { 
	chdir("ras/");
	int r = setenv("PATH", "bin:.", 1);
	if( r < 0)
		cerr << "setenv FAIL!\n";
	
//	string ls = "ls";
//	char command[50] = ls.c_str();
//	
//	char * args[2] = { command, 0};
//	
//	cout << "prepare to exec...\n";
//	cout.flush();
//	int execre = execvp(command, args);
//	cout << "exec fail!!\n";
	
	
	string line;
	while (getline(cin, line, '\n')) {
		istringstream iss(line);
		string s;
		vector<string> cmdlist;
		while (iss >> s) {
			cmdlist.push_back(s);
		}
		
		if (cmdlist[0] == "getenv") {
			for (int i=0; i<; <#increment#>) {
				<#statements#>
			}
		}
	}
	
	return 0;
}
