#ifndef _teletype_h
#define _teletype_h

#include <stdio.h>
#include <string>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp> 
#include <vector>
#include <syslog.h>

using namespace std;

//Structure to keep data from database about each filter.
//Used to keep sock_id and send messages to agents, etc.
//Used in serv.cpp

//Structure for parameters from applications table
// to save login param, logout param and it's values

string int_to_string(int);			//Converts int, long long, unsigned int, unsigned long long to string
string int_to_string(long long);
string int_to_string(unsigned int);
string int_to_string(unsigned long long);
string int_to_string(double);

string double_to_string(double);	//Converts double to string

//creates string of comma seperated values from vector
void unicodeParser2(vector <unsigned int>& , string &);


void init_lookuptable();// initialization of lookup table

unsigned int to_utf8(string&); //returns value from lookup table
unsigned int to_utf8(const char *); //returns value from lookup table
void string_to_utf8(string&, vector<unsigned int>& );	//


#endif
