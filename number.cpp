#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>

FILE *fd;
using namespace std;

int main(int argc, char *argv[])
{
  char c;
  int counter = 1;

  if(argc == 1)
     fd = stdin;
  else if(argc == 2)
     fd = fopen(argv[1], "r");
  else
  {
     fprintf(stderr,"Usage");
     exit(1);
  }
  
  string str="";
  while((c = fgetc(fd))!=EOF)
  {
    if (c == -1)
      break;
    str+=c;
    if (c == '\n'){
      fprintf(stdout, "%4d %s", counter++, str.c_str());
      str="";
    }
  } 
  if( strcmp( str.c_str(), "") !=0 )
  {
      cout << "   " << counter++ << " " << str;
      cout << endl;
  }
  fclose(fd);
  return 0;

}


 
