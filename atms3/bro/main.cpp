// icarus.cpp : Defines the entry point for the console application.
//

extern int readfile(const char * fname);

#ifdef WIN32
#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	char * fname = "dump.pcap";
	readfile(fname);
	return 0;
}

#else

int main()
{
  char * fname = "dump.pcap";
  readfile(fname);
}

#endif
