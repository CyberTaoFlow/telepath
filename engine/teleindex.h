#ifndef _teleindex_h
#define _teleindex_h

#include <unordered_map>
#include <iomanip> 


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <regex.h>
#include <string>
#include <map>
#include <queue>
#include <set>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp> 
#include <iostream>
#include <algorithm>
#include "teletypes.h"
#include "teleobject.h"


using namespace std;


//! Login Class
/*!
	This class stores automatic detetion of login points that were detected by the loginDetect() function.
	\sa loginDetect(char*,Login&)
*/
class Login{
public:

	//!  The password field name of the POST/GET login page.
	/*!*/
	string password;

	//!  The user field name of the POST/GET login page.
	/*!*/
	string username;

	//!  The application/domain name of the login point.
	/*!*/
	string host;

	//!  The page name of the login point.
	/*!*/
	string orig_uri;

	//!  Sometimes the password and the username belong to another url/page and not to the original url.
	/*!*/
	string action;

	//!  Defult Constrator.
	/*!*/
	Login(){}

	//!  Constrator.
	/*!
		Initializing the object fields with their default values.
		\param password as a C++ string argument.
		\param username as a C++ string argument.
		\param orig_uri as a C++ string argument.
		\param action as a C++ string argument.
	*/
	Login(string & password,string & username,string & action,string & orig_uri){
		this->password = password;
		this->username = username;
		this->orig_uri = orig_uri;
		this->action = action;
	}

	//!  Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\return void
	*/
	void print(){
		cout << "password:" << this->password <<endl;
		cout << "username:" << this->username <<endl;
		cout << "action:" << this->action <<endl;
		cout << "orig_uri:" << this->orig_uri <<endl;
		cout << "host:" << this->host <<endl;
	}

};

//! Logout Class
/*!
	This class stores automatic detetion of logout points (Still not available in the system).
*/
class Logout{
public:

	unsigned int app_id;
	unsigned int page_id;
	unsigned int att_id;

	Logout(){}
	Logout(string & app_id,unsigned int page_id,unsigned int att_id){
		this->app_id = (unsigned int)atoi(app_id.c_str());
		this->page_id = page_id;
		this->att_id = att_id;
	}

	//!  Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\return void
	*/
	void print(){
		cout << "app_id:" << this->app_id <<endl;
		cout << "page_id:" << this->page_id <<endl;
		cout << "att_id:" << this->att_id <<endl;
	}

};

//! AppMode Class
/*!
        This class gets/updates the information from/to the '/telepath-domains' Elasticsearch index.
*/
class AppMode{
public:

	//!  Operation Mode of the application (1/2/3). 
	/*!*/
	unsigned int mode;

	//!  The number of requests that belong to the application(learning_so_for).
	/*!*/
	unsigned int counter;

	//!
	/*!*/
	unsigned int timer;

	//!  The number of requests that need to be inserted to the application before a transition to production made.
	/*!*/
	unsigned int move_to_production;

	//!  Authentication mode value.
	/*!*/
	unsigned short redirect_mode;

	//!  Authentication success criterion.
	/*!*/
	string redirect_page;

	//!     
	/*!*/
	unsigned short redirect_status_code;

	//!  Authentication mode value.
	/*!*/
	unsigned short body_value_mode;

	//!  Authentication mode value.
	/*!*/
	unsigned short basic_mode;

	//!  Authentication mode value.
	/*!*/
	unsigned short digest_mode;

	//!  Authentication mode value.
	/*!*/
	unsigned short ntlm_mode;

	//!  Authentication success criterion.
	/*!*/
	string body_value_html;

	//!  The username parameter field.
	/*!*/
	string form_param_name;

	//!  Authentication mode value.
	/*!*/
	unsigned int cookie_mode;

	//!  Authentication success criterion.
	/*!*/
	string cookie_name;

	//!  Authentication success criterion.
	/*!*/
	string cookie_value;

	//!     
	/*!*/
	unsigned short cookie_value_appearance;

	//!     
	/*!*/
	unsigned short top_level_domain;

	//!     
	/*!*/
	boost::unordered_set <string> subdomains;

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	AppMode(){
		this->mode = 1;
		this->counter = 0;
		this->timer = (unsigned int)time(NULL);
		this->move_to_production = 1000000;
		this->redirect_mode=0;
		this->redirect_page.clear();
		this->redirect_status_code=0;
		this->body_value_mode=0;
		this->basic_mode=0;
		this->digest_mode=0;
		this->ntlm_mode=0;
		this->body_value_html.clear();
		this->form_param_name.clear();
		this->cookie_mode=0;
		this->cookie_name.clear();
		this->cookie_value.clear();
		this->cookie_value_appearance=0;
		this->top_level_domain=0;
	}

	//!  Constrator.
	/*!
		Initializing the object fields with their argument values.
		\param mode as an unsigned integer argument.
		\param counter as an unsigned integer argument.
		\param move_to_production as an unsigned integer argument.
		\param redirect_mode as an unsigned short argument.
		\param redirect_page as a C++ string argument.
		\param redirect_status_code as an unsigned short argument.
		\param body_value_mode as an unsigned short argument.
		\param basic_mode as an unsigned short argument.
		\param digest_mode as an unsigned short argument.
		\param ntlm_mode as an unsigned short argument.
		\param body_value_html  as a C++ string argument.
		\param form_param_name as a C++ string argument.
		\param cookie_mode as an unsigned short argument.
		\param cookie_name as a C++ string argument.
		\param cookie_value as a C++ string argument.
		\param cookie_value_appearance as an unsigned short argument.
		\param top_level_domain as an unsigned short argument.
	*/

	AppMode(unsigned int & mode,unsigned int & counter,unsigned int & move_to_production,unsigned short & redirect_mode,string & redirect_page,unsigned short & redirect_status_code,unsigned short & body_value_mode,unsigned short & basic_mode,unsigned short & digest_mode,unsigned short & ntlm_mode,string & body_value_html,string & form_param_name,unsigned short & cookie_mode,string & cookie_name,string & cookie_value,unsigned short & cookie_value_appearance,unsigned short & top_level_domain){
		this->mode = mode;
		this->counter = counter;
		this->timer = (unsigned int)time(NULL);
		this->move_to_production = move_to_production;
		this->redirect_mode=redirect_mode;
		this->redirect_page=redirect_page;
		this->redirect_status_code=redirect_status_code;
		this->body_value_mode=body_value_mode;
                this->basic_mode=basic_mode;
                this->digest_mode=digest_mode;
                this->ntlm_mode=ntlm_mode;
		this->body_value_html=body_value_html;
		this->form_param_name=form_param_name;
		this->cookie_mode=cookie_mode;
		this->cookie_name=cookie_name;
		this->cookie_value=cookie_value;
		this->cookie_value_appearance=cookie_value_appearance;
		this->top_level_domain=top_level_domain;
	}

	//! Printing the class variables.
	/*!
		Printing to syslog the object fields.
		\param none
		\return void
	*/
	void print_syslog(){
		syslog(LOG_NOTICE,"op_mode:%u",this->mode);
		syslog(LOG_NOTICE,"counter:%u",this->counter);
		syslog(LOG_NOTICE,"timer:%u",this->timer);
		syslog(LOG_NOTICE,"move_to_production:%u",this->move_to_production);
		syslog(LOG_NOTICE,"redirect_mode:%hu",this->redirect_mode);
		syslog(LOG_NOTICE,"redirect_page:%s",this->redirect_page.c_str());
		syslog(LOG_NOTICE,"redirect_status_code:%hu",this->redirect_status_code);
		syslog(LOG_NOTICE,"body_value_mode:%hu",this->body_value_mode);
		syslog(LOG_NOTICE,"basic_mode:%hu",this->basic_mode);
		syslog(LOG_NOTICE,"digest_mode:%hu",this->digest_mode);
		syslog(LOG_NOTICE,"ntlm_mode:%hu",this->ntlm_mode);
		syslog(LOG_NOTICE,"body_value_html:%s",this->body_value_html.c_str());
		syslog(LOG_NOTICE,"form_param_name:%s",this->form_param_name.c_str());
		syslog(LOG_NOTICE,"cookie_mode:%u",this->cookie_mode);
		syslog(LOG_NOTICE,"cookie_name:%s",this->cookie_name.c_str());
		syslog(LOG_NOTICE,"cookie_value:%s",this->cookie_value.c_str());
		syslog(LOG_NOTICE,"cookie_value_appearance:%hu",this->cookie_value_appearance);
		syslog(LOG_NOTICE,"top_level_domain:%hu",this->top_level_domain);
	}
};

class Search{
public:
	unsigned int app_id;
	unsigned int page_id;
	string att_name;

	Search(){}
	Search(string & app_id,unsigned int page_id,string att_name){
		this->app_id = (unsigned int)atoi(app_id.c_str());
		this->page_id = page_id;
		this->att_name = att_name;
	}

	void print(){
		cout << "app_id:" << this->app_id <<endl;
		cout << "page_id:" << this->page_id <<endl;
		cout << "att_name:" << this->att_name <<endl;
	}

};

//!  TeleCache Class.
/*!
	This important class stores the main queue(teleObjQueue) of the system.
*/
class TeleCache
{
public:

	//!
	/*!
		The main queue of the system that holds objects(TeleObject).
	*/
	queue <TeleObject> teleObjQueue;

	//!  TeleObjects Parsing
	/*!
		This function gets an unordered_map(after unpacking msgpeck from Redis). And do the following action: \n \n
		1) Parsing and transfering data from the redis style object to the 'TeleObject.mParam' unordered_map.\n
		2) Parsing the request body parameters(XML/multipart/regular).\n
		3) Finding/Giving the ID to of the subdomain.\n
		4) Droping illeagal or irrelevent requests because application,page or method.\n
		5) Creating mapping for a new index ('telepath-<date>').\n
		6) Creating a random request id(RID).\n
		7) Getting the title of the page from the response body.\n
		8) Canonical URLs detection.\n
		9) Logins detection.\n
		10) Passwords detection and masking.\n
		11) Inserting all the parameter(Headers,Gets,Posts ..) to 'TeleObject.mAttr' unordered_map.\n
		12) Authentication by success criteria.\n
		13) Pushing the TeleObject into the TeleCache queue(teleObjQueue).\n
		\param teleo as a pointer to the TeleObject object.
		\param obj as a C++ STL unordered_map argument.
		\sa parseMultipart() checkXML() mxmlLoadString() parseXML() parseGetPostParams() findAppID() getDate() dynamicPageInit() findTitle()
	 */
	void addobject(TeleObject*,std::unordered_map<string,string> &);

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	TeleCache();
};

//!  Fatching the title string from the response body.
/*!
	\param title as a C++ string argument.
	\param reply as a pointer to character argument.
*/
void findTitle(string &,char *);

//!  URL Encoding
/*!
	\param value as a C++ const string argument.
	\return
*/
string url_encode(const string &);

//!  Fatching all the passwords in the page from the response body.
/*!
	\param reply as a pointer to character argument.
	\param host as a C++ string argument.
	\param uri as a C++ string argument.
*/
void detectPasswords(char *,string &,string &);

//!  Fatching the username and password in the login page from the response body.
/*!
	\param reply as a pointer to character argument.
	\param log
*/
void loginDetect(char *,Login &);

//!  Checking the success criteria(basic, digest or form) of the response body.
/*!
	\param reply as a pointer to character argument.
	\param am as a AppMode object argument.
	\param logSuccess as a C++ string argument.
*/
void authentication(char *,AppMode &,string &);

//!  Generating a Random Number.
/*!
	\return The unsigned long long random number .
*/
unsigned long long longRand();

//!
/*!
	\param str as a C++ string argument.
*/
void parseString(string &);

//!  Parsing multipart parameters.
/*
	\param str as a C++ string argument.
	\param vAttr as a C++ STL vector of Attribute structs.
	\sa https://www.w3.org/Protocols/rfc1341/7_2_Multipart.html 
*/
void parseMultipart(string &,vector <struct Attribute> &);

//!  Parsing GET/POST parameters.
/*!
	For example:\n str = "username=roi&password=123456&save_pass=true" \n
		       vAttr[1].name="username"\n
		       vAttr[1].value="roi"\n
                       vAttr[2].name="password"\n
                       vAttr[2].value="123456"\n
                       vAttr[3].name="save_pass"\n
                       vAttr[3].value="true"\n
                     
	\param str as a C++ string argument.
	\param vAttr as a C++ STL vector of Attribute structs.
	\param attribute_source
*/
void parseGetPostParams(string &,vector <struct Attribute> &,char);

//!
/*!
	\param str as a C++ string argument.
	\param vAttr as a C++ STL vector of Attribute structs.
*/
void parseCookieParams(string &,vector <struct Attribute> &);

//!
/*!
	\param param_name as a C++ string argument.
	\param param_value as a C++ string argument.
	\param vAttr as a C++ STL vector of Attribute structs.
*/
void parseNestedCookie(string,string,vector <struct Attribute> &);

//  Check if the input value is a valid XML.
//!
/*!
	\param buff as a pointer to character argument.
*/
unsigned int checkXML(char *);

//!
/*!
	\param page as a C++ string argument.
*/
bool dynamicPageInit(string &);

#endif
