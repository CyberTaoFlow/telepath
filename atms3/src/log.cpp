#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>

//#define DEBUG

using namespace std;

char * logtime()
{
#ifdef DEBUG	
	printf("Log::logtime()\n");
#endif
	char *s;
	int size;
	time_t rawtime;
	time(&rawtime);
	s=ctime(&rawtime);
	size=strlen(s);
	s[size-1]='\0';
	
	return s;
}

void log(char *str, char const *filename)
{
#ifdef DEBUG	
	printf("Log::log(char *str)\n");
#endif
	ofstream outfile(filename, ios::binary | ios::app);
//*str - gets message to log
//	int size=strlen(str);
	
	char *buf;
	int len=0;
	len=strlen(str);
	buf=new char[len+200];
	
//sprintf - concatenates time and str
	sprintf(buf,"%s> %s%s%s",logtime(),str, "\0","\n");
//outfile - writes all string to file
	outfile.write(buf,strlen(buf));
	delete []buf;
	buf=NULL;
//	printf("%s",buf);
	
	outfile.close();
}

