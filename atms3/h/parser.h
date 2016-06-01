																																										#ifndef PARSER_H_
#define PARSER_H_
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "netdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <fstream>
//#include <queue>
//#include <pthread.h>
//#include <sys/timeb.h>
#include <map>
//#include <algorithm>
//#include <math.h>
#include <syslog.h>
//#include <signal.h>
//#include <asm-generic/errno-base.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
//Syslog libraries.
#include <sys/wait.h>
#include <ctime>
#include <list>

#include "../h/atmstypes.h"
using namespace std;

struct s_sessioninfo;
typedef map<string, string> MapStringKey;
typedef map<string, s_sessioninfo> MapStringKeySessionsValue;
typedef map<string, s_sessioninfo>::iterator index_to_msessioninfo;
typedef map<string,map<string,s_sessioninfo>::iterator>::iterator index_to_msessionkeys;

//Session Struct.
struct s_sessioninfo{
	long count;
	time_t last_time;
	time_t start_time;	
	list < map<string, map<string,s_sessioninfo>::iterator>::iterator> keys_pointed_by;//list of iterators to the map of iterators, msessionkeys
	string validated_cookie_value;	
	string SID;
	list< index_to_msessioninfo >::iterator iter_to_fifo;
};

index_to_msessioninfo update_msessioninfo_table(string& key,const string& sid,bool sid_already_exists,index_to_msessioninfo session_struct_index);
int get_real_ip(string& cUserIP, map<string,string> & headers_map);
int get_page_id(string& url, string sAppID,int req_seq);
void encode_string_to_binary(const string& str,string *bin);
void getCookie(string& cookie_str,string& cookie_val,string& set_cookie_str,string& set_cookie_val,int& cookie_status,bool& has_cookie,string& hostname,string& cookie_name,bool& has_set_cookie);
void getURL(string& request_line,string& ans);
void getPostData(string& request,string& post_data);
void prepareMessage(string& msg,int);
short app_validation(string &);
int get_app_id(const string & app,int response_code);
void parseJSON(const string & json,string preKey,multimap<string,string> & json_params);
//void parseXML(mxml_node_t *tree,string res,multimap<string,string> & xml_params);
//unsigned int checkXML(char * buff);
void parse_post_args(const unsigned char * post_data, size_t post_size, map<string,string> & headers, map<string,string> & post,multimap<string,string> & json,multimap<string,string> & xml);
void parse_get_args(const string & url, map<string,string> & get);

#endif /*PARSER_H_*/
