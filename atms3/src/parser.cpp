#include "../json/json.h"
#include "../mxml/mxml.h"
#include "../h/parser.h"
#include "../h/sqlfeeder.h"
#include "../h/sha1.h"
#include <algorithm>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <string.h>
#include "../h/sendarrayobj.h"
#include "../h/config.h"
#include <set>
#include <list>

using namespace std;

extern int backend_fd;

//Binary fields
static const string hGlobalUnknownValue = "";
static const string debug_ip = "81.218.185.126";

char hFieldCpt = 'o';			//CleanSID
char hFieldSetCookie = 'p'; 		//SetCookie
char hFieldAgentID = 'q'; 		//AgentID
char hFieldUserID = 'r';		//UserID
char hFieldUserIP ='a';			//UserIP
char hFieldRespIP ='u';			//RespIP
char hFieldSID = 'e';			//SID
char hFieldCookieStatus = 'n';		//CookieStatus
char hFieldRID = 'j';			//RID
char hFieldReqSeq ='s';			//ReqSeq
char hFieldApp ='f';			//App
char hFieldPage ='c';			//Page
char hFieldStatusCode ='d';		//StatusCode
char hFieldApplicationPool_ID ='g';	//AppID
char hFieldTimeStamp = 'b';		//TimeStamp
char hFieldEncoding = 't';		//Encoding
char hRequest = 'h';			//Request
char hProtocol = 'i';			//Protocol
char hTitle = 'm';			//Title
char hLoginMsg = 'v';			//LoginMsg
char hHTTPAuthType = 'w';		//HTTPAuthType
char hHTTPUser = 'x';			//HTTPUser



#define UNKNOWN_COOKIE 1
#define VALID_COOKIE 2
#define INVALID_COOKIE 3

//Config Fields


#define C_AGENTID "agentid"
//Message Fields
#define REQUEST_TIMESTAMP 1
#define REQUEST_LINE 2
#define REQUEST_HEADER_HOST 3
#define REQUEST_HEADER 4
#define SOURCE_IP 5
#define REUQEST 6
#define REQUEST_HEADER_COOKIE 7
#define RESPONSE_HEADER_SETCOOKIE 8
#define REQUEST_HEADER_USERAGENT 9
#define RESPONSE_CODE 10
#define CONNECTION_TIMESTAMP 11
#define MESSAGE_SEP "&H&"


extern sendArrayObj * sendqueue;
extern int iOperationMode;
extern SQLFeeder *pF;					//pointer to SQLfeeder object
extern unsigned long long iPushed;
extern unsigned long long iRecvMes;
extern unsigned long long iExIPDropped;
extern unsigned long long iHostDropped;
extern unsigned long long iExcParamDropped;
//extern unsigned int iLBDropped=0;
extern unsigned int  iAddUnknownApp;	//add unknown application flag
extern Config* conf_updated;
extern hregex regex;
extern unsigned long long sentCounter; 
u32 iPagesIndexCount=0;
set<string>::iterator itEx;

extern void getSID_new(string& sessionId,string& set_cookie_value,string& cookie_value,int& req_seq,map<string,string>& mheaders,string& real_ip,string& time_stamp,bool has_set_cookie,bool has_cookie);

static const char * get_http_user(std::string & auth, std::string & http_user);

void pushPacket(std::string & request_header_host, std::string & url,
                std::string & title, std::string & login_msg, map<string, string> & mheaders,
                std::string & request_header_cookie, std::string & response_header_setcookie,
                const unsigned char * post, size_t post_size, std::string & source_ip , std::string & resp_ip, int response_code,
		std::string & request_timestamp, const char * request_method, const char * protocol );

//
// This function is used to check if we have a private certificate for specific ip and port
// This function is called from bro. 
//
RSA * get_ssl_cert(const char * server_ip, unsigned int net_port)
{
	if (conf_updated == NULL)
		return NULL;
	return conf_updated->GetRSA(server_ip, net_port);
}


//
// This function is used to check if we need to listen on specific port.
// source_is_server refference is used to say if we handle client of server packet.
// This function is called from bro. 
//
//
bool check_port(unsigned int sport, unsigned int dport, bool & source_is_server)
{
	// no configuration object. Just to load default port 80 and 443
	if (conf_updated == NULL)
	{
		static unsigned int port_80 = ntohs(80);
		static unsigned int port_443 = ntohs(443);
		//static unsigned int port_8080 = ntohs(8080);
		if (dport == port_80 || dport == port_443/* || dport == port_8080*/)
		{
			source_is_server = false;
			return true;
		} else if (sport == port_80 || sport == port_443/* || sport == port_8080*/)
		{
			source_is_server = true;
			return true;
		}
		return false;
	}
	// TODO OPTIMIZE
	set<unsigned int>::iterator itr;
	itr = conf_updated->mSslPorts.find(sport);
	if (itr != conf_updated->mSslPorts.end())
	{
		source_is_server = true;
		return true;
	}
	itr = conf_updated->mSslPorts.find(dport);
	if (itr != conf_updated->mSslPorts.end())
	{
		source_is_server = false;
		return true;
	}
	return false;
}


/*
 * int get_real_ip(string& cUserIP, map<string,string> & headers_map)
 * 		the function takes the header the user has entered in the UI for the real client ip.
 * 		We go over all ips that were marked by the user as a load balancer ip, and if the request ip is a load balancer one,
 * 		we look for the real ip header, and take the ip on that header- that is the real ip
 * Parameters:
 * 		cUserIP - string&, the ip from the request
 * 		headers_map - map<string,string> &, the headers map of the request, this is where we look for the real client ip header
 * Return value:
 * 		int, 1 on found, 0 on not found(will result on drop request)
 */
int get_real_ip(string& cUserIP, map<string,string> & headers_map){
	
	///changing places of excludeIP and loadbalancer
	//==============================load balancer================================================
	//Load Balancing
	//Checking if lb_flag,
	//if so, we check that lb_ip==APP - so we got message from correct server
	//looking for atribute with name lb_header, taking it's, value which is IP, and put it instead of UserIP
	//lb_header in DB should look like "XXX_XXX_XXX"
	if (conf_updated == NULL)
		return 1;

	if (!(conf_updated->lb_flag))
		return 1;
	set<string>::iterator itrIP = conf_updated->sLB_IPaddr.find(cUserIP);  
	if (itrIP == conf_updated->sLB_IPaddr.end())
	{
		return 0;
	}

	vector <string>::iterator itr;
	map <string,string>::iterator itHeaders;
	//this is a request from the load balancer
	for (itr = conf_updated->lb_header.begin(); itr != conf_updated->lb_header.end(); itr++)
	{
		itHeaders = headers_map.find(*itr);
		if( itHeaders != headers_map.end() )
		{
			//found the real ip
			cUserIP = itHeaders->second;
			// remove multiple IP addresses in case of
			// X-Forwarded-For: client, proxy1, proxy2
			unsigned int pos = cUserIP.find_first_not_of("0123456789.");
			if (pos != std::string::npos && pos < cUserIP.size())
			{
				cUserIP.erase(pos);
			}
			if (cUserIP.size() < 7 || cUserIP.size() > 15)
				return 0;
			return 1;
		}
	}	
	//didnt find the real ip, drop request
	//iLBDropped++;
	return 0;
}

/*
 * bool check_exclusion(string& cUserIP)
 * 		The user can insert excluded ip, from which we drop requests
 * Parameters:
 * 		cUserIP - string&, the ip from the request
 * 
 * Return value:
 * 		bool. true if ip is excluded - drop request. false if not excluded. send request to the engine
 */
bool check_exclusion(string& cUserIP)
{
	if (conf_updated == NULL)
		return false;

	//checking if userIP is in list of excluded IP's
	itEx = conf_updated->excludedIP.find(cUserIP);
	if(itEx != conf_updated->excludedIP.end() ){
		++iExIPDropped;
		return true;
	}
	return false;
}


//int get_page_id(string& url, string sAppID,int req_seq){
//	int cond_val, add_to_db=0;
//	//By default we think, that this page appears for the first time, and we need to save it in index table for pages!
//	int need_to_page=1;
//	regmatch_t match;
//	int page_num;
//	//	checking page for PageContextException
//	//looking for current page in global list for current application
//	for(map<string,struct dynamicPageRegex>::iterator i=mAppData[sAppID].mdynPageRegex2.begin(); i!=mAppData[sAppID].mdynPageRegex2.end();++i)
//	{
//		if ((regexec(&i->second.regex,url.c_str(),1,&match,0)==0))
//		{   //if page is found we assign id from structure to page
//			if(i->second.page_id!=0)
//			{
//				page_num=i->second.page_id;
//				//	log((char*)(string("Multiple context page: ")+sPagePath).c_str(),log_path.c_str());
//			}
//
//			else
//			{
//				map<string, unsigned int>::iterator i_map2;
//				i_map2=mPageIndex.find(sPagePath);
//
//				if(i_map2==mPageIndex.end())
//				{
//					i_map2=mPageIndex.insert(pair<string,int>(sPagePath,iPagesIndexCount)).first;
//					add_to_db=1;
//					page_num=i->second.page_id=iPagesIndexCount;
//					++iPagesIndexCount;
//				}
//				else
//				{
//					page_num=i_map2->second;
//				}
//			}
//
//			need_to_page=0;
//			break;
//		}
//	}
//
//	//if page is not in global list, we check for it in index
//	if(need_to_page)
//	{
//		map<string,unsigned int>::iterator i_map;
//		i_map=mPageIndex.find(sPagePath);		//looking for a page in index table
//		if(i_map==mPageIndex.end())				//if page wasn't found in index, adding it
//		{
//			i_map=mPageIndex.insert(pair<string,int>(sPagePath,iPagesIndexCount)).first;
//			add_to_db=1;
//			page_num=iPagesIndexCount;
//			++iPagesIndexCount;
//		}
//		else//if page was found in index, assigning id to page
//		{
//			page_num=i_map->second;
//		}
//	}
//	if(add_to_db)
//	{
//		string entrypoint="0";
//		if(req_seq==0)
//			entrypoint="1";
//		pF->insert_pages_data((char*)(int_to_string(page_num).c_str()),(char*)sAppID.c_str(),(char*)url.c_str(),(char*)entrypoint.c_str(),iOperationMode);
//	}
//
//
//
//}

/*
 * void getPostData(string& request,string& post_data)
 * 		gets the post data from the request string and assigns it to post_data
 * Parameters:
 * 		request - string&, the request
 * 		post_data - string&, the post data string to put the post data in
 * 
 * Return value:
 *	 	void
 */
void getPostData(string& request,string& post_data){
	size_t start_pos=0;
	//string find = "\r\n\r\n";
	start_pos = request.find("\r\n\r\n");
	if(start_pos == string::npos){
		return;
	}
	start_pos += 4;
	post_data.assign(request.c_str()+start_pos,request.size()-start_pos);	
}
/*
 * void getURL(string& request_line,string& url)
 * 		gets the post data from the request string and assigns it to post_data
 * Parameters:
 * 		request_line - string&, the url header of the reuqest
 * 		url - string&, the url string to return the answer in
 * 
 * Return value:
 *	 	void
 */
void getURL(string& request_line,string& url){
	size_t start_pos=0;
	size_t end_pos=0;
	start_pos = request_line.find(" /");
	if(start_pos == string::npos){
		return;
	}
	start_pos++;
	end_pos = request_line.find(" ",start_pos);
	if(end_pos == string::npos){
		return;
	}
	url.assign(request_line.c_str()+start_pos,end_pos-start_pos);	
}

/*
 * void encode_string_to_binary(const string& str,string * bin)
 * 		converts a string from chars to binary data
 * Parameters:
 * 		str - const string&, the string to convert to binary
 * 		bin - string*, the binary string to return
 * 
 * Return value:
 *	 	void
 */
void encode_string_to_binary(const string& str,string * bin){
	unsigned char buf[8];
	unsigned int str_len = str.size();
	buf[0] = (unsigned char)str_len&0xff;
	buf[1] = (unsigned char)(str_len>>8)&0xff;
	buf[2] = (unsigned char)(str_len>>16) & 0xff;
	buf[3] = (unsigned char)(str_len>>24) & 0xff;
	buf[4] = buf[5] = buf[6] = buf[7] = '\0';
	bin->append((char*)&buf,8);
	bin->append(str);
}
/*
 * void encode_string_to_binary(const string& str,string * bin)
 * 		converts a char* from chars to binary data
 * Parameters:
 * 		str - const char*, the char* to convert to binary
 * 		bin - string*, the binary string to return
 * 
 * Return value:
 *	 	void
 */
void encode_string_to_binary(const char * str,string * bin){
        unsigned char buf[8];
        unsigned int str_len = strlen(str);
        buf[0] = (unsigned char)str_len&0xff;
        buf[1] = (unsigned char)(str_len>>8)&0xff;
        buf[2] = (unsigned char)(str_len>>16) & 0xff;
        buf[3] = (unsigned char)(str_len>>24) & 0xff;
        buf[4] = buf[5] = buf[6] = buf[7] = '\0';
        bin->append((char*)&buf,8);
        bin->append(str);
}

void encode_string_to_binary(char & ch,string * bin){
        unsigned char buf[8];
        unsigned int str_len = 1;
        buf[0] = (unsigned char)str_len&0xff;
        buf[1] = (unsigned char)(str_len>>8)&0xff;
        buf[2] = (unsigned char)(str_len>>16) & 0xff;
        buf[3] = (unsigned char)(str_len>>24) & 0xff;
        buf[4] = buf[5] = buf[6] = buf[7] = '\0';
        bin->append((char*)&buf,8);
        bin->push_back(ch);
}


/*
 * void encode_int_to_binary(int num,string * bin)
 * 		converts an int to binary data
 * Parameters:
 * 		num - int, the int to convert to binary
 * 		bin - string*, the binary string to return
 * 
 * Return value:
 *	 	void
 */
void encode_int_to_binary(int num,string * bin){
	unsigned char buf[4];
	buf[0] = (unsigned char)num&0xff;
	buf[1] = (unsigned char)(num>>8)&0xff;
	buf[2]= (unsigned char)(num>>16)&0xff;
	buf[3]= (unsigned char)(num>>24)&0xff;
	bin->append((char*)&buf,4);	
}
/*
 * void encode_u64_to_binary(int num,string * bin)
 * 		converts an u64 to binary data
 * Parameters:
 * 		num - u64, the u64 to convert to binary
 * 		bin - string*, the binary string to return
 * 
 * Return value:
 *	 	void
 */
void encode_u64_to_binary(u64 num,string * bin){
	unsigned char buf[8];
	buf[0]= (unsigned char)num&0xff;
	buf[1]= (unsigned char)(num>>8)&0xff;
	buf[2]= (unsigned char)(num>>16)&0xff;
	buf[3]= (unsigned char)(num>>24)&0xff;
	buf[4]= (unsigned char)(num>>32)&0xff;
	buf[5]= (unsigned char)(num>>40)&0xff;
	buf[6]= (unsigned char)(num>>48)&0xff;
	buf[7]= (unsigned char)(num>>56)&0xff;
	bin->append((char*)&buf,8);	
}

/*
 * void parseHeaders(string & headers, std::map<string,string> & mheaders )
 * 		parses the request's headers string into a map <string,string>
 * Parameters:
 * 		headers - string &, the requst's headers string
 * 		mheaders - std::map<string,string> &, the headers map
 * 
 * Return value:
 *	 	void
 */
void parseHeaders(string & headers, std::map<string,string> & mheaders )
{
	size_t start_pos = 0;
	size_t end_pos;
	string temp_key;
	string temp_value;	
	const char* p_str = headers.c_str();
	//skip first line..but dont cut it from beginning
	start_pos = headers.find("\n",start_pos);	
	if(start_pos == string::npos)
		return;
	// skip \n
	start_pos++;

	//now erase possible POST data
	end_pos = headers.find("\r\n\r\n");	
	if(end_pos != string::npos)
	{
		headers.erase(end_pos+2);
	}

	//start parsing
	while (p_str[start_pos] != '\0')
	{
		//
		// example of the header field
		// Host: www.google.com
		//

		//take before :
		end_pos = headers.find(":",start_pos);
		if(end_pos == string::npos)
			return;
		temp_key.assign(p_str + start_pos, end_pos-start_pos);
		transform(temp_key.begin(),temp_key.end(),temp_key.begin(),::tolower);
		start_pos = ++end_pos;
		// skip spaces after ":"
		while(p_str[start_pos]==' '){
			start_pos++;
		}
		end_pos = headers.find_first_of("\r\n",start_pos);
		if(end_pos == string::npos)
		{
			mheaders[temp_key] = "";
			return;
		}
		temp_value.assign( p_str+start_pos, end_pos-start_pos);
		mheaders[temp_key] = temp_value;
		//mheaders[temp_key] = headers.substr(start_pos,end_pos-start_pos);
		//mheaders.insert(pair<string,string>(temp_key, headers.substring(start_pos,end_pos-start_pos) ));
		// skip last "\n"
		start_pos = end_pos;
		while (p_str[start_pos]=='\n' || p_str[start_pos]=='\r'){
			start_pos++;
		}
	}
}
/*
 * bool check_excluded_params(map<string,string>& mheaders,map<string,string>& mpost,map<string,string>& mget,string& url)
 * 		goes over the headers, gets, posts and url and looks to see if any of them is in the excluded param regex's
 * Parameters:
 * 		headers - string &, the requst's headers string
 * 		mheaders - std::map<string,string> &, the headers map
 * 
 * Return value:
 *	 	bool. true if one of the headers,gets,posts or url matches the exluded param regex, the parameter will be excluded
 */
bool check_excluded_params(map<string,string>& mheaders,map<string,string>& mpost,map<string,string>& mget,string& url)
{
	if (conf_updated == NULL)
		return false;
	//go over all url regexes and check if this our matches
	vector<excluded_param> exc_parms_vector = conf_updated->excluded_params[conf_updated->mConfig["agent_id"]];
	map<string,string>::iterator itr_headers = mheaders.begin();
	string full_header;
	excluded_param* ex_par;
	for(unsigned int i = 0;i<exc_parms_vector.size();i++){
		ex_par = &(exc_parms_vector.at(i));

		// check if regext is legal !	
		if (!ex_par->regex.size())
			continue;

		//case the excluded param, check if the requests url matches the regex, if so, return true
		if (ex_par->param_type==URL){
			if(regex.regexp(url.c_str(), &ex_par->compiled_regex)){
				return true;
			}
		}
		//if its a header regex, go over all headers and check if any of them matches
		else if (ex_par->param_type==HEADERS){
			for(;itr_headers!=mheaders.end();itr_headers++){
				//unite header_name:header_value to match regex
				full_header.assign(itr_headers->first);
				full_header.append(":");
				full_header.append(itr_headers->second);
				if(hregex::regexp(full_header.c_str(), &ex_par->compiled_regex)){
					return true;
				}
			}
		}
	}
	//go over all headers and check if it matches our headers
	return false;
}

void analyze_dynamic_url(){
	
}

/*
 * void prepareMessage(string& msg,int backend_fd)(FUNCTION NOT IN USE)
 * 		the main function to parse a message before sending it to the engine in binary mode
 * Parameters:
 * 		msg - string &, the message string to prepare
 * 		backend_fd - redundant, not in use
 * 		
 * Return value:
 *	 	void
 */
/*void prepareMessage(string& msg,int backend_fd){
	size_t start_pos = 0;
	size_t end_pos = 0;
	int sep_len = strlen(MESSAGE_SEP);
	string url;
	string post_data;
	string binary;
	/////////////////////////message fields///////////////////// 
	string request_timestamp;
	string request_line;
	string request_header_host;
	string request_header;
	string source_ip;
	string resp_ip;
	string request;
	string request_header_cookie;
	string response_header_setcookie;
	string request_header_useragent;
	string response_code_str;
	string connection_timestamp;
	int response_code;
	map<string, string> mheaders;
	//map<string,string>::iterator const_header_itr;

	const char * p_str = msg.c_str();

	//increment received message
	//iRecvMes++;
	/////////////////////1- request_timestamp //////////////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	request_timestamp.assign(p_str + start_pos, end_pos-start_pos);
	//printf("request_timestamp : %s\n",request_timestamp.c_str());
	/////////////////////2- request.line///////////////////////
	end_pos += sep_len;
	start_pos = end_pos;	
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	request_line.assign(p_str + start_pos, end_pos-start_pos);
	//printf("request_line : %s\n",request_line.c_str());
	/////////////////////3-request_header_host/////////////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	request_header_host.assign(p_str + start_pos, end_pos-start_pos);
	
	if (request_header_host.find("/")!=string::npos){
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FOUND A BAD HOST~~~~~~~~~~~~~~~~~~~\n");
		printf("%s\n",msg.c_str());
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		syslog(LOG_INFO,"~~~~~~~~~~~~~~~~~~~~~~~~~FOUND A BAD HOST == / ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		syslog(LOG_INFO,"%s",msg.c_str());
		syslog(LOG_INFO,"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		return;
	}
	// make hostname lowercase and remove "www." at the beginning
	transform(request_header_host.begin(),request_header_host.end(),request_header_host.begin(),::tolower);
	if (request_header_host.size() > 4)
	{
		if (request_header_host.find("www.") == 0)
			request_header_host.erase(0,4);
	}
	//printf("request_header_host : %s\n",request_header_host.c_str());
	/////////////////////4-request_header////////////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	request_header.assign(p_str + start_pos, end_pos-start_pos);
	//printf("request_header : %s\n",request_header.c_str());
	/////////////////////5-source_ip////////////////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	source_ip.assign(p_str + start_pos, end_pos-start_pos);
	//printf("source_ip : %s\n",source_ip.c_str());
	/////////////////////6-source_ip////////////////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	resp_ip.assign(p_str + start_pos, end_pos-start_pos);
	//printf("resp_ip : %s\n",resp_ip.c_str());
	/////////////////////7-reuqest/////////////////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	request.assign(p_str + start_pos, end_pos-start_pos);	
	getPostData(request,post_data);
	parseHeaders(request, mheaders);
	
	//printf("request : %s\n",request.c_str());
	////////////////////8-request_header_cookie////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	request_header_cookie.assign(p_str + start_pos, end_pos-start_pos);
	//printf("\n\n\nrequest_header_cookie : %s\n\n\n",request_header_cookie.c_str());
	////////////////////9-response_header_setcookie////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	response_header_setcookie.assign(p_str + start_pos, end_pos-start_pos);
	//printf("response_header_setcookie : %s\n",response_header_setcookie.c_str());
	////////////////////10-request_header_useragent////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	request_header_useragent.assign(p_str + start_pos, end_pos-start_pos);
	//printf("request_header_useragent : %s\n",request_header_useragent.c_str());	
	////////////////////11-response_code//////////////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	response_code_str.assign(p_str + start_pos, end_pos-start_pos);
	if (response_code_str == "-")
	{
		response_code = 200;
	} else {
		response_code = atoi(response_code_str.c_str());
	}
	
	//printf("response_code : %s\n",response_code.c_str());
	////////////////////12-connection_timestamp///////////
	end_pos += sep_len;
	start_pos = end_pos;
	end_pos = msg.find(MESSAGE_SEP,start_pos);
	if(end_pos == string::npos)
		return;
	connection_timestamp.assign(p_str + start_pos, end_pos-start_pos);

	//printf("connection_timestamp : %s\n",connection_timestamp.c_str());	
	getURL(request_line,url);
	if (url.size() == 0)
	{
		// broken url
		return;
	}

	static const char * request_method = "GET";
	static std::string title = "title";
	static std::string login_msg;
	pushPacket(request_header_host, url, title, login_msg, mheaders, request_header_cookie, 
		response_header_setcookie, (const unsigned char*)post_data.c_str(), 
		post_data.size(), source_ip, resp_ip, response_code, request_timestamp, request_method, "HTTP");
}*/

/*
 * void pushPacket(std::string & request_header_host, std::string & url,
                std::string & title, std::string & login_msg, map<string, string> & mheaders, 
		std::string & request_header_cookie, std::string & response_header_setcookie,
		const unsigned char * post_data, size_t post_size, std::string & source_ip, std::string & response_code,
		std::string & request_timestamp, const char * request_method, const char * protocol )
 *
 * 		the main function to parse a message before sending it to the engine in binary mode
 * Parameters:
 * 		request_header_host - string &, the host header string
 * 		url - string &, the url
 * 		title - string &, the title
 * 		login_msg - string &, the login message
 * 		mheaders - map<string, string> &, the headers map of the request
 * 		title - string &, the title
 * 		request_header_cookie - string &, the cookie string from the request
 * 		response_header_setcookie - string &, the set-cookie string from the request
 * 		post_data - const unsigned char *, the post string part from the request
 * 		post_size - size_t, the size of the post
 * 		source_ip - std::string &, source ip from the request
 * 		response_code - std::string &, the response code from the server
 * 		request_timestamp - string &, the request_timestamp from the request
 * 		request_method - const char *, the method
 * 		protocol - const char *, the method  
 * 		
 * 
 * Return value:
 *	 	void
 */
void pushPacket(std::string & request_header_host, std::string & url,
                std::string & title, std::string & login_msg, map<string, string> & mheaders, 
		std::string & request_header_cookie, std::string & response_header_setcookie,
		const unsigned char * post_data, size_t post_size, std::string & source_ip, std::string & resp_ip, int response_code,
		std::string & request_timestamp, const char * request_method, const char * protocol )
{

	unsigned int i_lowcase;
        static unsigned long long iRID=0;
	bool has_set_cookie = false;
	bool cookie_found = false;
	int cookie_status,req_seq = 0;
	size_t binary_len,end_pos = 0;
	int num_written = 0;
	int total_written = 0;
	string cookie_value;
	string set_cookie_value;
	string this_cookie_name;
	string sessionId;
	string original_url;
	string http_user;
	const char * http_auth_type = NULL;
	static string * binary = new string();
        map<string, string> mget;
        map<string, string> mpost;
        multimap<string, string> mjson;
        multimap<string, string> mxml1;
	const char * cookieStatus;
        map<string, string>::iterator itr;
        multimap<string, string>::iterator itr2;
	//bool debug_msg = false;
	iRecvMes++;
	sentCounter++;

        //get the real ip if the source_ip is of the load_balancer
        get_real_ip(source_ip,mheaders);

	if (source_ip == debug_ip || conf_updated == NULL)
	{
		original_url = url;
		//debug_msg = true;
		printf("------------------------------------------------------------\n");
		printf("User IP: %s\n", source_ip.c_str());
		printf("Original URL: %s\n", original_url.c_str());
	}	
        //mheaders["hybrid_real_ip"] = source_ip;
        //check if the ip needs to be excluded, if it is drop the message
        if (check_exclusion(source_ip))
	{
		// we increment counter of exclusions iside the function !
		/*if (debug_msg){
			printf("DEBUG IP in exclussion list. Igoring message.\n");
		}*/
                return;
	}

        if (response_code != 401)
        {
                itr = mheaders.find("authorization");
                if (itr != mheaders.end())
                {
			http_auth_type = get_http_user(itr->second, http_user);
			/*if (debug_msg && http_auth_type != NULL){
				printf("HTTP[%d]: %s - %s\n", response_code, http_auth_type, http_user.c_str());
			}*/
		}
	}
	if (request_header_cookie.size() == 0)
                request_header_cookie = mheaders["cookie"];
	parse_get_args(url, mget);
	parse_post_args(post_data, post_size, mheaders, mpost, mjson, mxml1);
	map<string,string>::iterator post_itr = mpost.begin();
	for(;post_itr!=mpost.end();post_itr++){
		if (post_itr->first.find_first_of("<>")!= string::npos){
			//FOUND XML IN POST
			break;
		}
	}
	int app_id = get_app_id(request_header_host,response_code);
	if (app_id == -1){
		/*if (debug_msg){
			printf("No application found, host: %s . Ignoring message.\n", request_header_host.c_str());
		}*/
		return;	
	}
	getCookie(request_header_cookie,cookie_value,response_header_setcookie,set_cookie_value,cookie_status,cookie_found,request_header_host,this_cookie_name,has_set_cookie);
	
	//debugging
	/*if (cookie_status == VALID_COOKIE){
		if (!has_set_cookie && !cookie_found)
			syslog(LOG_ERR,"GOT INVALID(7): hotname:%s,app_cookie_name:%s,cookie_str:%s,cookie_found_flag:%s,cookie_value_found:%s,set_cookie_found_flag:%s,set_cookie_val:%s,cookie_status:%d",request_header_host.c_str(),this_cookie_name.c_str(),request_header_cookie.c_str(),(cookie_found?"True":"False"),cookie_value.c_str(),(has_set_cookie?"True":"False"),set_cookie_value.c_str(),cookie_status);
	}
	if (cookie_status == INVALID_COOKIE){
		if (has_set_cookie || cookie_found)
			syslog(LOG_ERR,"GOT INVALID(8): hotname:%s,app_cookie_name:%s,cookie_str:%s,cookie_found_flag:%s,cookie_value_found:%s,set_cookie_found_flag:%s,set_cookie_val:%s,cookie_status:%d",request_header_host.c_str(),this_cookie_name.c_str(),request_header_cookie.c_str(),(cookie_found?"True":"False"),cookie_value.c_str(),(has_set_cookie?"True":"False"),set_cookie_value.c_str(),cookie_status);
	}*/

	if (conf_updated != NULL)
	{
		getSID_new(sessionId,set_cookie_value,cookie_value,req_seq,mheaders,source_ip,request_timestamp,has_set_cookie,cookie_found);
	}else{
		sessionId = "-debug-";
	}

	if (cookie_status == INVALID_COOKIE)
		cookieStatus = "invalid";
	else if (cookie_status == VALID_COOKIE)
		cookieStatus = "valid";		
	else if (cookie_status == UNKNOWN_COOKIE)
		cookieStatus = "unknown";

	
	if (check_excluded_params(mheaders,mpost,mget,url)){
		iExcParamDropped++;
		/*if (debug_msg){
			printf("Some parameters are in exclude list for this request. Ignoring message.\n");
		}*/
		return;
	}
        // Remove everything after ?
        if ( (end_pos = url.find("?") ) != string::npos)
        {
                url.erase(end_pos);
        }

	//URL to lower case. 
	for(i_lowcase=0;i_lowcase<url.size();i_lowcase++){
		if(url[i_lowcase]<91 && url[i_lowcase]>64){
			url[i_lowcase] +=32;
		}
	}

	/*if (debug_msg){
		if (conf_updated != NULL)
			printf("AgentID: %s\n", conf_updated->mConfig["agent_id"].c_str());
		printf("Host: %s\n", request_header_host.c_str());
		printf("AppID: %d\n", app_id);
		printf("Cookie status: %s\n", cookieStatus);
		printf("Request: %s\n", request_method);
		printf("Protocol: %s\n", protocol);
		printf("URL: %s\n", url.c_str());
		printf("RID: %llu\n", iRID);
		printf("SEQ: %d\n", req_seq);
		printf("SESSION: %s\n", sessionId.c_str());
		printf("Response set-cookie: %s\n", response_header_setcookie.c_str());
		printf("Response: %d\n", response_code);
		printf("TIMESTAMP: %s\n", request_timestamp.c_str());
		printf("HTTP AUTH TYPE: %s\n", http_auth_type);
		printf("HTTP AUTH USER: %s\n", http_user.c_str());
	        itr = mheaders.begin();
        	for (; itr != mheaders.end(); itr++)
        	{
			printf("H: %s => %s\n", itr->first.c_str(), itr->second.c_str());
        	}
        	itr = mget.begin();
		for (; itr != mget.end(); itr++)
		{
			printf("G: %s => %s\n", itr->first.c_str(), itr->second.c_str());
		}
		itr = mpost.begin();
		for (; itr != mpost.end(); itr++)
		{
			printf("P: %s => %s\n", itr->first.c_str(), itr->second.c_str());	
		}
		itr2 = mjson.begin();
		for (; itr2 != mjson.end(); itr2++)
		{
			printf("J: %s => %s\n", itr->first.c_str(), itr->second.c_str());
		}
		itr2 = mxml1.begin();
		for (; itr2 != mxml1.end(); itr2++)
		{
			printf("X: %s => %s\n", itr->first.c_str(), itr->second.c_str());
		}
		if (conf_updated == NULL)
			return;
	}*/
	encode_int_to_binary(0,binary);
	// WE ARE SENDING A COUNTER OF MESSAGES
	if (http_auth_type != NULL)
	{
		// we will send auth type(basic,digest,ntlm) and username
		encode_int_to_binary(22,binary);
	} else {
		encode_int_to_binary(20,binary);
	}
	//Agent Id
	encode_string_to_binary(hFieldAgentID,binary);
	if (conf_updated)
		encode_string_to_binary(conf_updated->mConfig["agent_id"],binary);
	else
		 encode_string_to_binary("1",binary);
	//App
	encode_string_to_binary(hFieldApp,binary);
	encode_string_to_binary(request_header_host,binary);
	//App Id
	encode_string_to_binary(hFieldApplicationPool_ID,binary);	
	encode_string_to_binary(int_to_string(app_id),binary);
	//CleanSid - As for now hasn't been implemented
	encode_string_to_binary(hFieldCpt,binary);
	encode_string_to_binary("1",binary);
	//Cookie Status
	encode_string_to_binary(hFieldCookieStatus,binary);
	encode_string_to_binary(cookieStatus,binary);
	//Encoding
	encode_string_to_binary(hFieldEncoding,binary);
	encode_string_to_binary("unknown",binary);		
	//RespIp
	encode_string_to_binary(hFieldRespIP,binary);
	encode_string_to_binary(resp_ip,binary);
	//Page
	encode_string_to_binary(hFieldPage,binary);
	encode_string_to_binary(url,binary);
	//RID
	encode_string_to_binary(hFieldRID,binary);
	encode_string_to_binary(int_to_string(iRID),binary);
	//printf("Sending RID: %llu\n", iRID);
	iRID++;
	//ReqSeq	
	encode_string_to_binary(hFieldReqSeq,binary);
	encode_string_to_binary(int_to_string(req_seq),binary);
	//SID
	encode_string_to_binary(hFieldSID,binary);
	encode_string_to_binary(sessionId,binary);
	//Set-Cookie

	encode_string_to_binary(hFieldSetCookie,binary);
	encode_string_to_binary(response_header_setcookie,binary);	
	//Status Code
	encode_string_to_binary(hFieldStatusCode,binary);
	encode_string_to_binary(int_to_string(response_code),binary);

	//TimeStamp
	encode_string_to_binary(hFieldTimeStamp,binary);
	encode_string_to_binary(request_timestamp,binary);
	//UserId
	encode_string_to_binary(hFieldUserID,binary);
	encode_string_to_binary("unknown",binary);
	//UserIp
	encode_string_to_binary(hFieldUserIP,binary);
	encode_string_to_binary(source_ip,binary);
	//request method (GET/POST/HEAD/...)
	encode_string_to_binary(hRequest,binary);
	encode_string_to_binary(request_method,binary);
	// request protocol (HTTP/HTTPS)
	encode_string_to_binary(hProtocol,binary);
	encode_string_to_binary(protocol,binary);
	// reply title if present
	encode_string_to_binary(hTitle, binary);
	encode_string_to_binary(title, binary);
	// hLoginMsg - when we find
        encode_string_to_binary(hLoginMsg, binary);
        encode_string_to_binary(login_msg, binary);
	// auth
	if (http_auth_type != NULL)
	{
		encode_string_to_binary(hHTTPAuthType, binary);
        	encode_string_to_binary(http_auth_type, binary);
		encode_string_to_binary(hHTTPUser, binary);
		encode_string_to_binary(http_user, binary);
	}
	//User-Agent
	//encode_string_to_binary(hFieldUserAgent,binary);
	//encode_string_to_binary(request_header_useragent,binary);

	int header_counter_pos = binary->size();
	int header_counter = 0;
	encode_int_to_binary(3,binary);
	//Headers
	//Go over all headers and check if we need to send them

	itr = mheaders.begin();
	u64 i = -1;
	for (; itr != mheaders.end(); itr++)
	{
		encode_string_to_binary(itr->first,binary);
		// encode header value
		encode_string_to_binary(itr->second,binary);
		encode_string_to_binary("H",binary);
		header_counter++;
		i--;
	}

	itr = mget.begin();
	for (; itr != mget.end(); itr++)
	{
		//binary dump
		encode_string_to_binary(itr->first,binary);
		encode_string_to_binary(itr->second,binary);
		encode_string_to_binary("G",binary);
		header_counter++;
		i--;
	}
	itr = mpost.begin();
	for (; itr != mpost.end(); itr++)
	{
		//binary dump		
		encode_string_to_binary(itr->first,binary);
		encode_string_to_binary(itr->second,binary);
		encode_string_to_binary("P",binary);
		header_counter++;
		i--;
	}

	itr2 = mjson.begin();
	for (; itr2 != mjson.end(); itr2++)
	{
 		//binary dump		
		encode_string_to_binary(itr2->first,binary);
		encode_string_to_binary(itr2->second,binary);
		encode_string_to_binary("J",binary);
		header_counter++;
		i--;
	}

        itr2 = mxml1.begin();
        for (; itr2 != mxml1.end(); itr2++)
        {
                //binary dump           
                encode_string_to_binary(itr2->first,binary);
                encode_string_to_binary(itr2->second,binary);
                encode_string_to_binary("X",binary);
                header_counter++;
                i--;
        }


	encode_int_to_binary(0,binary);

	binary_len = binary->size();
	unsigned char* buf = (unsigned char*)binary->c_str();
	buf[0] = (unsigned char)(binary_len&0xff);
	buf[1] = (unsigned char)((binary_len>>8)&0xff);
	buf[2] = (unsigned char)((binary_len>>16)&0xff);
	buf[3] = (unsigned char)((binary_len>>24)&0xff);

	buf = (unsigned char*)binary->c_str() + header_counter_pos;
	buf[0] = (unsigned char)(header_counter&0xff);
	buf[1] = (unsigned char)((header_counter>>8)&0xff);
	buf[2]= (unsigned char)((header_counter>>16)&0xff);
	buf[3]= (unsigned char)((header_counter>>24)&0xff);

	binary->append("</!>");
	binary_len = binary->size();
	//increment pushed counter for stat
	iPushed++;
	if (sentCounter%10000==0){									
		printf("\nSent %llu requests so far\n",sentCounter); 
		syslog (LOG_INFO, "INFO : Sent %llu requests so far\n",sentCounter);
	}
	//if we have something waiting in the queue e
	if (sendqueue->count()>0 || backend_fd==0){	
		sendqueue->enqueue(binary);

		// create new binary !
		binary = new string();
		binary->reserve(980);
		return;
	}	
	while (binary_len!=0){
		num_written = write(backend_fd, binary->c_str()+total_written, binary_len);
		if (num_written < 0)
		{
			printf("Failed to send to engine!\n");
		}
		if (num_written<=0 && binary_len==binary->size()){
			// failed to send. push all packet to outgoing queue
			sendqueue->enqueue(binary);

			// create new binary !
			binary = new string();
			binary->reserve(980);
			return;
		}
		if (num_written<=0){//lost connection-reconnect 						
			binary->erase(0, total_written);
			if (binary->size() > 0)
				sendqueue->enqueue(binary);
			
			// create new binary !
			binary = new string();
			binary->reserve(980);
			return;
		}
		total_written += num_written;
		binary_len = binary_len - num_written;
	}
	// all packet was written
	// no need to allocate new onject
	// we will reuse this object next time !

	binary->clear();
}



short app_validation(string & app){
	if(app.size() < 4){
		return 0;
	}

	string::size_type pos;
	pos = app.find(':');
	if( pos != string::npos){
		return 0;		
	}

	return 1;

}

/*
 * int get_app_id(const string & appname)
 * 		the function gets the id if the application as it is on the mysql db by its appname
 * Parameters:
 * 		appname - const string &, the application display name
 * 		
 * Return value:
 *	 	int - the app id
 */
int get_app_id(const string & appname,int response_code){
	map<class AppKey,struct AppData,AppMatchPredicate>::iterator itCApp;
	string tmpAppId;
	AppKey app;
	std::size_t found;

	if (conf_updated == NULL)
		return 0;

	app.name = ".";
	app.name += appname;
	app.search_subdomain = true;
	reverse(app.name.begin(), app.name.end());

	//if this application is unknown, it will be dropped or be added to the database.
	for(itCApp = conf_updated->mAppData.begin() ; itCApp != conf_updated->mAppData.end() ; itCApp++ ){
		found = app.name.find(itCApp->first.name);
		if (found!=std::string::npos){
			return itCApp->second.id;
		}
	}

	if( (response_code==200) && (iAddUnknownApp) && ( app_validation(app.name) /*|| appname=="1.1.1.1:8080"*/ ) ){	
		unsigned int maxAppID;
		maxAppID = (unsigned int) pF->add_app(appname);
		if (maxAppID == 0){
			++iHostDropped;
			return -1;
		}

		struct AppData sAD;
		app.search_subdomain = false;
		sAD.id=maxAppID;
		conf_updated->mAppData.insert(pair<AppKey,struct AppData>(app,sAD));
		return maxAppID;
	}else{
		++iHostDropped;
		return -1;
	}
}

//
// This function looks for login confirmation message inside HTML page.
// This function is called from bro. 
//
void check_login_msg(std::string & appname, const char * reply, size_t len, std::string & result)
{
	map<class AppKey,struct AppData,AppMatchPredicate>::iterator itCApp;
	AppKey app;

	if (conf_updated == NULL)
		return;

	app.name = ".";
	app.name += appname;
	app.search_subdomain = true;
	reverse(app.name.begin(), app.name.end());

	//if this application is unknown, it's dropped or added to list and database.
	itCApp = conf_updated->mAppData.find(app);
	if (itCApp == conf_updated->mAppData.end())
	{
		return;
	}

	if (itCApp->second.login_msg.size()==0){
		return;
	}

	const char* pos = strcasestr( reply,itCApp->second.login_msg.c_str() );
	if (pos==NULL){
		//printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!NO   %s  %s ",itCApp->second.login_msg.c_str(), reply );
		//syslog(LOG_NOTICE,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!NO   %s  %s ",itCApp->second.login_msg.c_str(), reply);
		//syslog(LOG_NOTICE,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!NO   %s  : %s",itAttLogin->second.msg_login.c_str(),teleo->mParams['B'/*ResponseBody*/].c_str());
		return;
	}else{
		//printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!YES   %s  : %s",itCApp->second.login_msg.c_str(),reply);
		//syslog(LOG_NOTICE,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!YES   %s  : %s",itCApp->second.login_msg.c_str(), reply);
		//syslog(LOG_NOTICE,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!YES   %s  : %s",itAttLogin->second.msg_login.c_str(),teleo->mParams['B'/*ResponseBody*/].c_str());
		result.clear();
		result.push_back('y');			
	}
}

static const char * query_delim = "&";


/*
 * void parse_get_args(const string & url, std::map<std::string,std::string> & get)
 * 		the function goes over the get headers string and turns them to a map header_name->header_val
 * Parameters:
 * 		url - const string &, the url string that contains the get params
 * 		get - std::map<std::string,std::string> &, the map to put the args into 
 * Return value:
 *	 	void
 */
void parse_get_args(const string & url, std::map<std::string,std::string> & get)
{
	char * pair;
	char * last = NULL;
	char * eq;
	int size;
	size_t q_pos = url.find("?");

	if (q_pos == string::npos)
		return;
	q_pos++;
	size = strlen(url.c_str()+q_pos);
	if (size == 0)
		return;
	// duplicate query string because apr_strtok function changes query buffer
	char query[size+1];

	memcpy(query, url.c_str()+q_pos, size);
	query[size] = '\0';

	for ( pair = strtok_r(query, query_delim, &last); pair != NULL; 
	pair = strtok_r(NULL, query_delim, &last) )
	{
		for (eq = pair ; *eq ; ++eq)
		{
			if ( *eq == '+' )
				*eq = ' ' ;
		}
		//ap_unescape_url(pair);
		eq = strchr(pair, '=') ;
		if ( eq ) {
			*eq++ = 0 ;
			get[pair] = eq;
		} else {
			get[pair] = "";
		}
	}
	return;
}

/*
 * char * parse_post_multipart(const unsigned char* post_data, const char * boundary, size_t max_size, map<string, string> & post)
 * 		the function goes over the post headers string and turns them to a map header_name->header_val
 * Parameters:
 * 		post_data - const unsigned char*, the post string from the request
 * 		boundary - const unsigned char*, the limit of the string
 * 		max_size - size_t, the size of the string
 * 		post - std::map<std::string,std::string> &, the map to put the args into 
 * Return value:
 *	 	void
 */
char * parse_post_multipart(const unsigned char* post_data, const char * boundary, size_t max_size, map<string, string> & post)
{
	bool is_file;
	size_t pos = 0;
	size_t start;
	char * location;
	size_t b_length;
	std::string key;
	std::string val;

	if (!boundary){
		return NULL;
	}

	b_length = strlen(boundary);

	while (pos < max_size)
	{
		location = strstr((char*)post_data+pos, boundary);
		if (location == NULL)
			return 0;
		location += b_length;
		pos = location - (char*) post_data;
		// skip spaces
		while (pos < max_size && (post_data[pos] == '\r' ||
				post_data[pos] == '\n' || post_data[pos] == '\t' || post_data[pos] == ' ') )
			pos++;
		if (pos >= max_size)
		{
			return 0;
		}
		if (strncasecmp((const char*)post_data+pos, "Content-Disposition:", sizeof("Content-Disposition:")-1) != 0)
		{
			// this parameter is broken, try to extract the next one
			continue;
		}
		pos += sizeof("Content-Disposition:")-1;
		// content-disposition: form-data; name="userfile"; filename="$filename"
		//Content-disposition: attachment; filename="file1.txt"
		//content-disposition: form-data; name="field2"
		//
		is_file = false;
		key.clear();
		val.clear();
		while (pos < max_size && is_file == false)
		{
			// look for the next tag
			while (pos < max_size && (post_data[pos] != ';' && post_data[pos] != '\r' && post_data[pos] != '\n'))
				pos++;
			if (pos >= max_size)
				return 0;
			if (post_data[pos] == '\r' || post_data[pos] == '\n')
				break;
			if (post_data[pos] != ';')
				return 0;
			pos++;
			while (pos < max_size && (post_data[pos] == ' ' || post_data[pos] == '\t'))
				pos++;
			if (pos >= max_size)
				return 0;
			if (strncasecmp((const char*)post_data+pos, "name=", 5) ==0)
			{
				pos+=5;
				while (pos < max_size && (post_data[pos] == ' ' || post_data[pos] == '\t'))
					pos++;
				if (pos >= max_size)
					return 0;
				if (post_data[pos] == '"')
				{
					start = ++pos;
					while (pos < max_size && post_data[pos] != '"')
					{pos++;}
				} else if (post_data[pos] == '\'')
				{
					start = ++pos;
					while (pos < max_size && post_data[pos] != '"')
					{pos++;}
				} else {
					start = pos;
					while (pos < max_size && post_data[pos] != ';')
					{pos++;}
				}
				// extract parameter name
				if (pos != start)
				{
					//post_data[pos] = '\0';
					key.assign((const char*)post_data+start, pos-start);
					//printf("parameter: %s\n", key.c_str());
				}
				continue;
			}
			if (strncasecmp((const char *)post_data+pos, "filename", 8) ==0)
			{
				// ignore collected parameter, it is a file
				//printf("it is a filename !\n");
				pos+= 8;
				is_file = true;
				continue;
			}
		}

		// skip to the end of the line
		while (pos < max_size && (post_data[pos] != '\r' && post_data[pos] != '\n'))
			pos++;
		if (pos >= max_size)
			return 0;
		if (post_data[pos] == '\r')
			pos++;
		if (pos >= max_size)
			return 0;
		if (post_data[pos] == '\n')
			pos++;
		if (pos >= max_size)
			return 0;
		while (post_data[pos] != '\r' && post_data[pos] != '\n')
		{
			// now we have atributes, like content-encoding, content-type, etc...
			if (strncasecmp((const char*)post_data+pos, "Content-Type:", sizeof("Content-Type:")-1) == 0)
			{
				// it can be Content-Type: multipart/mixed; boundary=BbC04y
				// it is used to send multiple files, we will ignore it
				// http://www.w3.org/TR/html4/interact/forms.html#h-17.13.4.2
				pos+= sizeof("Content-Type:")-1;
				is_file = true;
				break;
			} else {
				// skip to the end of string
				while (pos < max_size && (post_data[pos] != '\r' && post_data[pos] != '\n'))
					pos++;
				if (pos >= max_size)
					return 0;
				if (post_data[pos] == '\r')
					pos++;
				if (pos >= max_size)
					return 0;
				if (post_data[pos] == '\n')
					pos++;
				if (pos >= max_size)
					return 0;
			}
		}
		if (pos >= max_size)
			return 0;
		if (is_file == true)
			break;
		if (post_data[pos] == '\r')
		{
			pos++;
			if (post_data[pos] == '\n')
				pos++;
		} else {
			pos++;
		}
		// now we are at start of data
		start = pos;
		location = strstr((char*)post_data+pos, boundary);
		pos = location - (char*)post_data;
		if (post_data[pos -2] == '\r')
			val.assign((char*)post_data+start, pos - start -2);
		else
			val.assign((char *)post_data+start, pos - start -1);
		post[key] = val;
	}

	return NULL;
}

// This function parses boundary value, For example:
// Content-type: multipart/form-data, boundary=AaB03x
//
void get_boundary(const string & ctype, string & boundary) {
	size_t b_pos = ctype.find("boundary");
	size_t end_pos;
	if (b_pos == string::npos){
		return ;
	}

	b_pos += 8;
	b_pos = ctype.find("=", b_pos);
	if (b_pos == string::npos){
		return ;
	}

	b_pos++;
	end_pos = ctype.find_first_of("\r\n; \t", b_pos);
	if (end_pos == string::npos){
		return;
	}

	if (end_pos == b_pos)
		return;

	boundary = "--";
	boundary.append(ctype.c_str()+b_pos, end_pos -b_pos);
	return;
}

//Parse JSON values into name and value syntax.

//Input: 
//	name1{
//		name2=value2
//		name3:value3
//	}
//Output:
//	name1::name2::value2
//	name1::name3::value3

void parseJSON(const string & json,string preKey,multimap<string,string> & json_params){
        Json::Value root;
        Json::Reader reader;
        Json::Value::iterator it;
        bool parsed=false;
        string key,strTmp,val;
        unsigned int j,i;

	if(preKey.size()==0){
		if(json[0] == '['){
			string new_json = json;
			new_json.erase(0,1);
			new_json.erase(new_json.size()-1,1);
      			parsed = reader.parse(new_json, root, false);
		}else{
      			parsed = reader.parse(json, root, false);
		}
	}else{
      		parsed = reader.parse(json, root, false);
	}

        if(!parsed){
                return;
        }

        it=root.begin();
        for (i=0; i < root.size() ; i++) {

                if(preKey.size()!=0){
                        key = preKey;
                        key.append("::");
                        key.append(it.key().asString());
                }else{
                        key = it.key().asString();
                }
		if ((*it).isNull())
		{
			json_params.insert(pair<string,string>(key,"null"));
		}
		else if( (*it).isObject() ){
                        parseJSON( (*it).toStyledString(), key,json_params);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
				if ( array[j].isNull() ){
					if (!strTmp.empty())
						strTmp.push_back(',');
					strTmp.append("null");
				}
				else if( array[j].isObject() ){
					parseJSON( array[j].toStyledString(), key,json_params);
				}else{
					if (!strTmp.empty()){
						strTmp.push_back(',');
					}

					if (!strTmp.empty()){
						strTmp.push_back(',');	
					}

				        if ( array[j].isString() ){
				                strTmp.append(array[j].asString());
				        }else{
						strTmp.append(array[j].toStyledString());
				        }

				}
                        }
                        json_params.insert(pair<string,string>(key,strTmp));
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }
                        json_params.insert(pair<string,string>(key,val));
                }
                it++;
        }
}

//Parse XML values into name and value syntax.
//Input: 
//      <name1>
//              <name2>value2</name2>
//              <name3>value3</name3>
//      </name1>
//Output:
//      name1::name2::value2
//      name1::name3::value3

void parseXML(mxml_node_t *tree,string res,multimap<string,string> & xml_params){

	if (tree->value.element.name && (tree->child != NULL || tree->next != NULL) )
	{
		if(tree->next!=NULL){
			parseXML(tree->next,res,xml_params);
		}
		if(tree->child != NULL){
			if (!res.empty()){
				res.append("::") ;
			}
			res.append(tree->value.element.name) ;
			parseXML(tree->child,res,xml_params);
		}
	}
	if(tree->value.opaque && tree->child == NULL && tree->next == NULL )
	{
		xml_params.insert( pair<string,string>(res,tree->value.opaque) );
	}
}

// Check if the input value is a valid XML.
unsigned int checkXML(char * buff){
	unsigned int i;	

	for(i=0;buff[i]!=0 && buff[i]==' ';i++);
	if(buff[i]==0){
		return 0;
	}
	
	if(buff[i+1]=='?' && buff[i+2]=='x' && buff[i+3]=='m' && buff[i+4]=='l'){
		i+=5;
	}else{
		return 0;
	}

	for(;;i++){
		if(buff[i]=='>'){
			break;
		}
		else if(buff[i]==0){
			return 0;
		}
	}
	return (i+1);
}

// This function is used to parse client request contents
void parse_post_args(const unsigned char * post_data, size_t total_post, map<string,string> & headers, map<string,string> & post,multimap<string,string> & json,multimap<string,string> & xml)
{
	char * last = NULL;
	char * pair;
	char * eq;
	size_t max_size;
	if (total_post == 0)
		return;
	map<string, string>::iterator itr;
	//size_t total_post = post_data.size();
	static const string clength = "content-length";
	static const string ctype = "content-type";

	itr = headers.find(clength);
	if (itr != headers.end())
	{
		max_size = atoi(itr->second.c_str());
		// broken packet !
		// we should always get size of post data
		if (max_size == 0)
			return;
		if (total_post > max_size)
		{
			// if received data is bigger than expected length, cut it off
			total_post = max_size;
		}
	} else {
		// post can be trunsmitted using chunk encoding !
		max_size = total_post;
	}
	itr = headers.find(ctype);

	if (itr == headers.end() || strncmp(itr->second.c_str(),"application/x-www-form-urlencoded", 32) == 0)
	{
		for ( pair = strtok_r((char*)post_data, query_delim, &last); pair != NULL; 
			pair = strtok_r(NULL, query_delim, &last) )
		{
			for (eq = pair ; *eq ; ++eq)
			{
				if ( *eq == '+' )
					*eq = ' ' ;
			}
			//ap_unescape_url(pair) ;
			eq = strchr(pair, '=') ;
			if ( eq ) {
				*eq++ = 0 ;
				post[pair] = eq;
			} else {
				post[pair] = "";
			}
		}
	} else if (itr != headers.end() && strncmp(itr->second.c_str(), "multipart/form-data", 19) == 0)
	{

		std::string boundary;
		get_boundary(itr->second, boundary);
		if (boundary.size() == 0)
			return;
		parse_post_multipart(post_data, boundary.c_str(), total_post, post);
	}
	else if (itr != headers.end() && strncmp(itr->second.c_str(), "application/json", 16) == 0)
	{
		if(strlen((const char *)post_data)>0){
			parseJSON((const char*)post_data,"",json);
		}
	}

	else if ( strlen((const char *)post_data)>0 && post_data[0]=='{' )
	{
                parseJSON((const char*)post_data,"",json);
	}

	else if (itr != headers.end() && ( strncmp(itr->second.c_str(), "text/xml", 8) == 0 || strncmp(itr->second.c_str(), "application/xml", 15) == 0 || strncmp(itr->second.c_str(), "application/soap+xml", 20) == 0  ) )
	{
		unsigned int offset = checkXML((char*)post_data);
		mxml_node_t *tree = mxmlLoadString(NULL, (const char*)post_data+offset ,MXML_OPAQUE_CALLBACK);
		if(tree != NULL){
			parseXML(tree,"",xml);
			// free mxml memory !!!!!!!!!
			mxmlDelete( tree );
		} else {
			printf("BROKEN XML:\n%s\n", post_data);
		}
	}
}


// This function is used to parse HTTP Authentication packet and to extract 
// username when Basic or Digest or NTLM authentication us used
static const char * get_http_user(std::string & auth, std::string & http_user)
{
	size_t i, u_len = 0;
	if (auth.compare(0, 5, "Basic") == 0)
	{
		const char * b64 = auth.c_str()+6;
		unsigned char * u = base64_decode(b64, strlen(b64), u_len);
		if (u == 0 || u_len == 0)
		{
			return NULL;
		}
		for (i = 0; i < u_len && u[i] != ':'; i++)
			;
		if (i != 0)
                {
			http_user.assign((char*)u, i);
			//http_auth_type = "basic";
			free(u);
			return "basic";
		}
		free(u);
		return NULL;
	}
	else if ( auth.compare(0, 6, "Digest") == 0)
	{
		//const char * b64 = auth.c_str()+7;
		size_t pos = auth.find("username", 7);
		if (pos == string::npos )
			return NULL;
		pos += 8;
		if (pos < auth.size() && auth[pos] == ' ')
			pos++;
		if (pos < auth.size() && auth[pos] != '=')
			return NULL;
		pos++;
		if (pos < auth.size() && auth[pos] == ' ')
			pos++;
		size_t end = pos+1;
		if (pos < auth.size() && auth[pos] == '\'')
		{
			pos++;
			while (end < auth.size() && auth[end] != '\'')
			{
				end++;
			}
		} else if (pos < auth.size() && auth[pos] == '"')
		{
			pos++;
			while (end < auth.size() && auth[end] != '"')
			{
				end++;
			}
		} else if (pos < auth.size())
		{
			while (end < auth.size() && auth[end] != ',' && auth[end] != ' ')
			{
				end++;
			}
		}
		if (end != pos)
		{
			http_user.assign(auth.c_str() + pos, end-pos);
			//http_auth_type = "digest";
			return "digest";
		}
		return NULL;
	}
	else if (auth.compare(0, 4, "NTLM") == 0)
	{
		// http://www.innovation.ch/personal/ronald/ntlm.html
		const char * b64 = auth.c_str()+5;
		unsigned char * p1 = base64_decode(b64, strlen(b64), u_len);
		if (p1 == NULL || u_len == 0)
		{
			return NULL;
		}
		// check ntlm type
		if (p1[8] != 0x3)
		{
			free(p1);
			return NULL;
		}
		if (u_len < 64)
		{
			free(p1);
			return NULL;
		}
		unsigned int length_user = ((unsigned int)(p1[36]))+((unsigned int)(p1[37]))*256 ;
		if (length_user == 0)
		{
			free(p1);
			return NULL;
		}
		unsigned long long offset_user = ((unsigned long long)(p1[40]))+((unsigned long long)(p1[41]))*256 + ((unsigned long long)(p1[42]))*65536 + ((unsigned long long)(p1[43]))*16777216 ;
		if (offset_user + length_user > u_len)
		{
			free(p1);
			return NULL;
		}
		http_user.assign((char*)p1+offset_user, length_user);
		free(p1);
		//http_auth_type = "ntlm";
		return "ntlm";
	}
	return NULL;
}
