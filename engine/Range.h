#ifndef _Range_h
#define _Range_h

#include <string> 

using namespace std;

unsigned int ipToNum(string &);
bool ipSingleOrRange(string &,string &,string &);

class Range{
public:
	unsigned int from;
	unsigned int to;


	Range();
	Range(unsigned int,unsigned int);
	void init(unsigned int,unsigned int);
	bool inRange(unsigned int);
	void print();
	void print_syslog();
};

#endif
