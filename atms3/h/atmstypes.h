#ifndef _atmstypes_h
#define _atmstypes_h

#include <stdio.h>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <syslog.h>
#include "hregex.h"
#include "base64.h"

using namespace std;

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long  u64;

enum LOGINSTATE {LOGOUT=0, LOGEDIN, AUTH};
enum ATMSFLAGSTATE {LOAD_BALANCER=24,EXCLUDE_PARAMS=22,SEND_AGENT_CONFIG=34, AGENTADD=40, APPADD=41,AGENT_EDIT=42};

#define EXCEPTION_CATCH
#define MAX_USERNAME 50

struct excluded_param{
	int param_type;	
	string regex;
	int condition;
	regex_t compiled_regex;
};
enum excluded_param_type{
	URL=1,
	HEADERS=2	
};

//Structure to keep parameters for each session.
//Used in teleindex.cpp
struct SessionState
{
	LOGINSTATE loginstate;
	string sCurSID; 		//current SID is changed for every login
	string sUsername;		//username is changed for every login
	string sAgentID;		//mapping agentid for every message
	unsigned long uiSeq;	//current sequence
	time_t timestamp;		//time of last request in session
};

//Structure to keep data from database about each agent.
//Used to keep sock_id and send messages to agents, etc.
//Used in serv.cpp
struct AgentStatus
{
	set<string> ConfigIDs;
	int iSockNum, iTimeout, iStatus, iConfigCounter;
//	string ip;
//	string id;
//	string init_conf;
	int initialized;
};

//Structure to keep data from database about each filter.
//Used to keep sock_id and send messages to agents, etc.
//Used in serv.cpp
struct FilterStatus
{
	set<string> SignalIDs;
	int iSockNum, iTimeout, iStatus, iSignalCounter;
//	string ip;
//	string id;
//	string init_conf;
//	int initialized;
};

struct LoggedIn
{
	string logged_condition;
	char logged_value;
};


void parse_url(string&);					//Trims "http://", "https://", "www." and parameters from string
void parse_quote(string&);					//Adds ' if meets another quote
string parse_str_delim(string& , char );	//Searches for delimeter and breaks to tockens

string int_to_string(int);			//Converts int, long long, unsigned int, unsigned long long to string
string int_to_string(long long);
string int_to_string(unsigned int);
string int_to_string(unsigned long long);

string double_to_string(double);	//Converts double to string

//creates string of comma seperated values from vector
unsigned int utf16_to_utf8(unsigned int utf_16);

void init_lookuptable();// initialization of lookup table
unsigned int to_utf8(string); //returns value from lookup table
unsigned int string_to_utf8(string, vector<unsigned int>& );	//

string xml_node(string ,string );			//Searches for the first tag in str and returns its value
string get_tag_value(string& , const char * , const char *);		//Creates xml node: <nodename>nodevalue</nodename>
string get_tag_value(const char * str, const char * opentag, const char * closetag);


//functional object to sort APP NAMES in descending order
bool sort_app_names(const vector<string>::value_type &,const vector<string>::value_type &);


#endif
