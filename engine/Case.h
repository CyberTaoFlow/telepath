#ifndef _Case_h
#define _Case_h

#include <string>
#include <vector>
#include <set>
#include "Range.h"

using namespace std;

class SubCase{
public:
	char type;
	bool neg;

	string vals;
	vector <string> values;
	vector <Range> ips;
	set <string> countries;
	set <string> apps;
	set <unsigned int> rules;

	SubCase();
	void clean();
	void init();
	bool isMatch(unsigned int,string &,string &/*,unsigned int*/);
	void print();
	void print_syslog();
};

class Case{
public:
	string case_name;
	vector <SubCase> subcase;

	Case();

	void clean();
	void init();
	void print();
	void print_syslog();

};

#endif

