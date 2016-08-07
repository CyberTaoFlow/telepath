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

class Login{
public:
	string password;
	string username;
	string action;
	string orig_uri;
	string host;

	Login(){}
	Login(string & password,string & username,string & action,string & orig_uri){
		this->password = password;
		this->username = username;
		this->orig_uri = orig_uri;
		this->action = action;
	}

	void print(){
		cout << "password:" << this->password <<endl;
		cout << "username:" << this->username <<endl;
		cout << "action:" << this->action <<endl;
		cout << "orig_uri:" << this->orig_uri <<endl;
		cout << "host:" << this->host <<endl;
	}

};

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

	void print(){
		cout << "app_id:" << this->app_id <<endl;
		cout << "page_id:" << this->page_id <<endl;
		cout << "att_id:" << this->att_id <<endl;
	}

};

class AppMode{
public:
	unsigned int mode;
	unsigned int counter;
	unsigned int timer;
	unsigned int move_to_production;
	unsigned short redirect_mode;
	string redirect_page;
	unsigned short redirect_status_code;
	unsigned short body_value_mode;
	string body_value_html;
	string form_param_name;
	unsigned int cookie_mode;
	string cookie_name;
	string cookie_value;
	unsigned short cookie_value_appearance;
	unsigned short top_level_domain;
	boost::unordered_set <string> subdomains;

	AppMode(){
		this->mode = 1;
		this->counter = 0;
		this->timer = (unsigned int)time(NULL);
		this->move_to_production = 1000000;
		this->redirect_mode=0;
		this->redirect_page.clear();
		this->redirect_status_code=0;
		this->body_value_mode=0;
		this->body_value_html.clear();
		this->form_param_name.clear();
		this->cookie_mode=0;
		this->cookie_name.clear();
		this->cookie_value.clear();
		this->cookie_value_appearance=0;
		this->top_level_domain=0;
	}

	AppMode(unsigned int & mode,unsigned int & counter,unsigned int & move_to_production,unsigned short & redirect_mode,string & redirect_page,unsigned short & redirect_status_code,unsigned short & body_value_mode,string & body_value_html,string & form_param_name,unsigned short & cookie_mode,string & cookie_name,string & cookie_value,unsigned short & cookie_value_appearance,unsigned short & top_level_domain){
		this->mode = mode;
		this->counter = counter;
		this->timer = (unsigned int)time(NULL);
		this->move_to_production = move_to_production;
		this->redirect_mode=redirect_mode;
		this->redirect_page=redirect_page;
		this->redirect_status_code=redirect_status_code;
		this->body_value_mode=body_value_mode;
		this->body_value_html=body_value_html;
		this->form_param_name=form_param_name;
		this->cookie_mode=cookie_mode;
		this->cookie_name=cookie_name;
		this->cookie_value=cookie_value;
		this->cookie_value_appearance=cookie_value_appearance;
		this->top_level_domain=top_level_domain;
	}

	void print_syslog(){
		syslog(LOG_NOTICE,"op_mode:%u",this->mode);
		syslog(LOG_NOTICE,"counter:%u",this->counter);
		syslog(LOG_NOTICE,"timer:%u",this->timer);
		syslog(LOG_NOTICE,"move_to_production:%u",this->move_to_production);
		syslog(LOG_NOTICE,"redirect_mode:%hu",this->redirect_mode);
		syslog(LOG_NOTICE,"redirect_page:%s",this->redirect_page.c_str());
		syslog(LOG_NOTICE,"redirect_status_code:%hu",this->redirect_status_code);
		syslog(LOG_NOTICE,"body_value_mode:%hu",this->body_value_mode);
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

class TeleCache
{
public:
	queue <TeleObject> teleObjQueue;

	void addobject(TeleObject*,std::unordered_map<string,string> &);

	TeleCache();
};




#endif
