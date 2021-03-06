 #include "jsoncpp/json.h"
#include "teleindex.h"
#include "mxml/mxml.h"
#include "base64.h"
#include "Enumeration.h"
#include <semaphore.h>
#include "Range.h"
#include <algorithm>
#define MAX_PARAM 128
#define MAX_VALUE 8192

//---------------External variables and functions---------------
extern sem_t sem_insert_logins;
extern boost::unordered_map <string,AppMode> mAppMode;
extern queue <Login> loginQueue;
extern queue <Logout> logoutQueue;
extern queue <Search> searchQueue;
extern void findAppID(string&,string&);
extern int hashCode(string &);
extern void initMethod();
extern unsigned int dropApp;
extern unsigned int dropMethod;
extern unsigned int dropPage;
extern boost::unordered_set <unsigned int> setUserIDs;
extern boost::unordered_set <unsigned int> setHashMasks;
extern unsigned int timeOutFlag;
extern string sSQLBlackList;
extern pthread_mutex_t mutexTeleObjQueue;
extern pthread_mutex_t mutexLogin;
extern pthread_mutex_t mutexLogout;
extern pthread_mutex_t mutexSearch;
extern pthread_mutex_t mutexAppMode;
extern boost::unordered_map<string,unsigned short> global_header;
extern boost::unordered_set<string> domain_header;
extern boost::unordered_set<string> const_method;
extern boost::unordered_set<string> const_content_type;
extern string getDate(unsigned int);
extern boost::unordered_set <string> sLoadbalancerHeaders;
extern boost::unordered_set <string> sFilterExtensions;
extern unsigned short password_masking;
extern vector <Range> loadbalancer_ips;
extern string sha256(const string str);
//--------------------------------------------------------------
extern void es_insert(string,string);

//--------------------------------------------------------------

boost::unordered_map <string,string> usernamePerIP;
boost::unordered_map <string,Enumeration> mDynamicPages;
boost::unordered_map <string,Enumeration>::iterator itDynamic;
boost::unordered_set <string> sDynamicPages;
boost::unordered_set <unsigned int> hashRespBodys;
boost::unordered_map<string,unsigned short>::iterator it_global_header;

unsigned int dynamicCounter=0;
string shard;
unsigned int shard_time=0;

const char HEX2DEC[256] =
{
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

    /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};


TeleCache::TeleCache()
{
	init_lookuptable();
	srand (time(NULL));
}

void findTitle(string & title,char * reply){
	size_t title_len = 0;
	char * pos = strcasestr(reply, "<title");
	char * end;
	if (pos > 0)
	{
		for (pos+= 6; *pos; pos++)
		{
			if (*pos == '>')
			{
				pos++;
				break;
			}
		}
		// skip spaces at the begining
		while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n')
			pos++;
		for (end= pos; *end; end++)
		{
			if (*end == '<')
				break;
		}
		// skip spaces at the end
		while (end>pos && (*(end-1) == ' ' || *(end-1) == '\t' || *(end-1) == '\r' || *(end-1) == '\n'))
			end--;
		title_len = ((end-pos > 200 ) ? 200 : (end-pos));
	}

	if (title_len == 0)
	{
		// look for <h1>
		pos = strcasestr(reply, "<h1>");
		if (pos > 0)
		{
			pos += 4;
			// skip spaces at the begining
			while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n')
				pos++;
			for (end= pos; *end; end++)
			{
				if (*end == '<')
					break;
			}
			// skip spaces at the end
			while (end>pos && (*(end-1) == ' ' || *(end-1) == '\t' || *(end-1) == '\r' || *(end-1) == '\n'))
				end--;
			title_len = ((end-pos > 200 ) ? 200 : (end-pos));
		}
	}
	if (title_len != 0)
	{
		//cout << "title:" << reply << endl;
		title.assign(pos,title_len);
		//syslog(LOG_NOTICE, "Title: %s",title.c_str());
	}
}

string url_encode(const string &value) {
	ostringstream escaped;
	escaped.fill('0');
	escaped << hex;

	for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		//escaped << uppercase;
		escaped << '%' << setw(2) << int((unsigned char) c);
		escaped << nouppercase;
	}

	return escaped.str();
}

void detectPasswords(char * reply,string & host,string & uri){
	unsigned int len = 0;
	char * end;
	char * end2;
	char * pos;
	char * pos2;
	char * new_reply;
	string password;
	string action;
	string hash_str;

	if(*reply != '<'){
		return;
	}

	pos = strcasestr(reply, "type=\"password\"");
	if (pos <= 0){
		pos = strcasestr(reply, "type=\'password\'");
		if (pos <= 0){
			pos = strcasestr(reply, "type=password");
		}
	}

	new_reply = pos;

	if (pos > 0)
	{
		len = ((pos-(reply) > 500 ) ? 500 : (pos-(reply) ));
		for (; *pos != '<'; pos--){
			if(len==0){
				return;
			}
			len--;
		}

		pos = strcasestr(pos, "name=");
		if (pos <= 0)
		{
			return;
		}
		end = pos+5;
		if(*end == '\"' || *end == '\''){
			end++;
			end2 = end;
			for ( ; *end2 != '\"' && *end2 != '\''; end2++){}
		}else{
			end2 = end;
			for ( ; *end2 != ' ' && *end2 != '>'; end2++){}
		}

		if (end2 <= 0){return;}

		len = ((end2-(end) > 100 ) ? 100 : (end2-(end) ));

		password.assign(end,len);

		action.clear();
		pos2 = strcasestr(reply, "action=");
		if(pos2 > 0){
			if(pos2 < end){
				end = pos2+7;
				if(*end == '\"' || *end == '\''){
					end++;
					end2 = end;
					for ( ; *end2 != '\"' && *end2 != '\''; end2++){}
				}else{
					end2 = strcasestr(end, " ");
				}
				if (end2 <= 0){return;}

				len = ((end2-(end) > 200 ) ? 200 : (end2-(end) ));
				action.assign(end,len);
			}
		}

		if(action.size()>0){
			size_t found=action.find('/');
			if (found!=string::npos || uri.size()==0){
				hash_str = host+action+password;
			}else{
				int i;
				for(i=(uri.size()-1);i>=0;i-- ){
					if(uri[i]=='/'){
						break;
					}
				}

				action.insert(action.begin(),uri.begin(),uri.begin()+i+1);
				hash_str = host+action+password;
			}
		}else{
			hash_str = host+uri+password;
		}

		unsigned int hash_value = (unsigned int)hashCode(hash_str);
		setHashMasks.insert(hash_value);
		pos = strcasestr(new_reply, "<");

		if(pos != NULL){
			detectPasswords(pos,host,uri);
		}
	}
}

void loginDetect(char * reply,Login & log){
	string tmp_form,tmp;
	unsigned int len = 0;
	char * end;
	char * end2;
	char * pos;
	char * pos2;
	char * pos3;

	if(*reply != '<'){
		return;
	}

	pos = strcasestr(reply, "type=\"password\"");
	if (pos <= 0){
		pos = strcasestr(reply, "type=\'password\'");
		if (pos <= 0){
			pos = strcasestr(reply, "type=password");
		}
	}

	if (pos > 0)
	{
		len = ((pos-(reply) > 500 ) ? 500 : (pos-(reply) ));
		for (; *pos != '<'; pos--){
			if(len==0){
				return;
			}
			len--;
		}

		pos = strcasestr(pos, "name=");
		if (pos <= 0)
		{
			return;
		}
		end = pos+5;
		if(*end == '\"' || *end == '\''){
			end++;
			end2 = end;
			for ( ; *end2 != '\"' && *end2 != '\''; end2++){}
		}else{
			end2 = end;
			for ( ; *end2 != ' ' && *end2 != '>'; end2++){}
		}

		if (end2 <= 0){return;}

		len = ((end2-(end) > 100 ) ? 100 : (end2-(end) ));

		tmp.assign(end,len);
		//log.password = url_encode(tmp);
		log.password = tmp;
		tmp.clear();

		pos2 = strcasestr(reply, "name=");
		pos3=NULL;
		while(pos2 > 0){
			if(pos2 == pos){
				if(pos3==NULL){
					return;
				}

				pos2 = pos3;
				if(*pos2 == '\"' || *pos2 == '\''){
					pos2++;
					pos3 = pos2;
					for ( ; *pos3 != '\"' && *pos3 != '\'' ; pos3++){}
				}else{
					pos3 = pos2;
					for ( ; *pos3 != ' ' && *pos3 != '>' ; pos3++){}
				}

				if (pos3 <= 0){return;}

				len = ((pos3-pos2 > 100 ) ? 100 : (pos3-pos2 ));

				tmp.assign(pos2,len);
				//log.username = url_encode(tmp);
				log.username = tmp;
				tmp.clear();
				break;
			}

			pos2 += 5;
			pos3 = pos2;
			pos2 = strcasestr(pos3, "name=");
		}

		len = ((pos-(reply) > 1000 ) ? 1000 : (pos-(reply) ));
		while(1){
			for (; *pos != '<'; pos--){
				if(len==0){
					return;
				}
				len--;
			}
			tmp_form.assign(pos,5);
			if(tmp_form=="<form"){
				break;
			}
			else if(len==0){
				return;
			}
			pos--;
			len--;
		}

		pos2 = strcasestr(pos, "action=");
		if(pos2 > 0){
			end = pos2+7;
			if(*end == '\"' || *end == '\''){
				end++;
				end2 = end;
				for ( ; *end2 != '\"' && *end2 != '\''; end2++){}
			}else{
				end2 = strcasestr(end, " ");
			}
			if (end2 <= 0){return;}

			len = ((end2-(end) > 200 ) ? 200 : (end2-(end) ));
			tmp.assign(end,len);
			log.action = tmp;
			tmp.clear();
		}

	}
}

void authentication(char * reply,AppMode & am,string & logSuccess){
	if(am.body_value_mode==1){
		char * pos = strcasestr( reply,(char*)am.body_value_html.c_str());
		if (pos > 0){
			#ifdef DEBUG
				syslog(LOG_NOTICE,"Form authentication has been detected!!!");
			#endif
			logSuccess = "y";
		}
	}
	else if(am.basic_mode==1){
		char * pos = strcasestr( reply,(char*)am.body_value_html.c_str());
		if (pos > 0){
			#ifdef DEBUG
				syslog(LOG_NOTICE,"Basic authentication has been detected!!!");
			#endif
			logSuccess = "y";
		}
	}
	else if(am.digest_mode==1){
		char * pos = strcasestr( reply,(char*)am.body_value_html.c_str());
		if (pos > 0){
			#ifdef DEBUG
				syslog(LOG_NOTICE,"Digest authentication has been detected!!!");
			#endif
			logSuccess = "y";
		}
	}
	else if(am.ntlm_mode==1){
	}
}

void loginSuccess(char * reply,string & logSuccess){

	//WordPress
	char * pos = strcasestr( reply,"id=\"wp-admin-bar-user-actions\"" );
	if (pos > 0){
		logSuccess = "y";
		//cout<< "logSuccess==" << logSuccess <<"\n\n";
		return;
	}

	//ownCloud
	pos = strcasestr( reply,"<body id=\"body-user\">" );
	if (pos > 0){
		logSuccess = "y";
		return;
	}

	//redmine
	pos = strcasestr( reply,"<div id=\"loggedas\">Logged in as" );
	if (pos > 0){
		logSuccess = "y";
		return;
	}
}

void logoutDetect(string & app_id,unsigned int page_id,unsigned int att_id){
	Logout logout(app_id,page_id,att_id);

	pthread_mutex_lock(&mutexLogout);
	logoutQueue.push(logout);
	pthread_mutex_unlock(&mutexLogout);
}

void logoutDetect2(string & app_id,unsigned int page_id,string & page_name,unsigned int att_id){
	if (page_name == "/redmine/logout"){
		Logout logout(app_id,page_id,att_id);

		pthread_mutex_lock(&mutexLogout);
		logoutQueue.push(logout);
		pthread_mutex_unlock(&mutexLogout);
	}
}

void searchDetect(char * reply,string & app_id,unsigned int page_id,string & page_name){
	//ownCloud
	if(page_name=="/owncloud/index.php/core/search"){
		Search search(app_id,page_id,"query");

		pthread_mutex_lock(&mutexSearch);
		searchQueue.push(search);
		pthread_mutex_unlock(&mutexSearch);
		return;
	}

	//redmine
	char * pos = strcasestr( reply,"<div id=\"search-results-counts\">" );
	if (pos > 0){
		Search search(app_id,page_id,"q");

		pthread_mutex_lock(&mutexSearch);
		searchQueue.push(search);
		pthread_mutex_unlock(&mutexSearch);
		return;
	}
}

string makeSID(string & cookie){
	char buff_sid[20];
	sprintf(buff_sid,"%u",(unsigned int)hashCode(cookie) );

	return string(buff_sid);
}

unsigned long long longRand(){
	unsigned long long r30 = RAND_MAX*rand()+rand();
	unsigned long long s30 = RAND_MAX*rand()+rand();
	unsigned long long t4  = rand() & 0xf;
	return ( (r30 << 34) + (s30 << 4) + t4 );
}

void parseString(string & str){
	for(unsigned int i=0;i<str.size();i++){
		if(str[i]>0){
			switch( str[i]){
				case '\'':
				case '\"':
					str[i]='`';
					break;
				case '\\':
				case '\a':
				case '\b':
				case '\f':
				case '\n':
				case '\r':
				case '\t':
				case '\v':
				case '\?':
				case 4:
				case 5:
				case 6:
					str[i]=' ';
					break;
			}
		}else{
			str.clear();
			break;
		}
	}
}

void parseMultipart(string & str,vector <struct Attribute> & vAttr){
	struct Attribute tAttr;
	size_t found=0,found2=0,tmp_pos;
	unsigned int i;

	while(1){
		if(tmp_pos > found){
			return;
		}		

		found = str.find("name=\"",found);

		tmp_pos = found;

		if (found!=std::string::npos){
			found += 6;
			found2 = str.find('\"',found);

			if( (found2-found) > MAX_PARAM){
				tAttr.name.assign(str.begin()+found,str.begin()+found+MAX_PARAM);
			}else{
				tAttr.name.assign(str.begin()+found,str.begin()+found2);
			}

			found2++;
			
			found = str.find(13,found2);
			if (found!=std::string::npos){
				found++;
				found2 = str.find(13,found);
				if (found2!=std::string::npos){
					found2++;
					found = str.find(13,found2);
					if (found!=std::string::npos){
						tAttr.value.assign(str.begin()+found2+1,str.begin()+found);
						if( (found-found2) > MAX_VALUE){
							tAttr.value.assign(str.begin()+found2+1,str.begin()+found2+MAX_VALUE);
						}else{
							tAttr.value.assign(str.begin()+found2+1,str.begin()+found);
						}
					}else{
						return;
					}
				}else{
					return;
				}
			}else{
				return;
			}

			parseString(tAttr.name);
			parseString(tAttr.value);

			if(tAttr.name.size()>0){
				tAttr.attribute_source='P';
				vAttr.push_back(tAttr);
			}

		}else{
			return;
		}
	}

}

void parseGetPostParams(string & str,vector <struct Attribute> & vAttr,char attribute_source){
	unsigned int mode=0,pos=0;
	struct Attribute tAttr;

	for(unsigned int i=0;i<str.size();i++){
		if(mode==0){
			switch( str[i]){
				case '=':
					if( (i-pos) > MAX_PARAM){
						tAttr.name.assign(str.begin()+pos,str.begin()+pos+MAX_PARAM);
					}else{
						tAttr.name.assign(str.begin()+pos,str.begin()+i);
					}

					mode++;
					pos=i+1;
					break;
				case '\'':
				case '\"':
					str[i]='`';
					break;
				case '\\':
				case '\a':
				case '\b':
				case '\f':
				case '\n':
				case '\r':
				case '\t':
				case '\v':
				case '\?':
					str[i]=' ';
					break;
			}
		}else{
			switch( str[i]){
				case '\'':
				case '\"':
					str[i]='`';
					break;
				case '\\':
				case '\a':
				case '\b':
				case '\f':
				case '\n':
				case '\r':
				case '\t':
				case '\v':
				case '\?':
					str[i]=' ';
					break;
			}

			if(str[i]!='&' && i!=str.size()-1 ){
			}else{

				if( i!=(str.size()-1) ){
					tAttr.value.assign(str.begin()+pos,str.begin()+i);
					if( (i-pos) > MAX_VALUE){
						tAttr.value.assign(str.begin()+pos,str.begin()+pos+MAX_VALUE);
					}else{
						tAttr.value.assign(str.begin()+pos,str.begin()+i);
					}

					mode=0;
					pos=i+1;
				}else{
					if( (i-pos) > MAX_VALUE){
						tAttr.value.assign(str.begin()+pos,str.begin()+pos+MAX_VALUE);
					}else{
						tAttr.value.assign(str.begin()+pos,str.begin()+i+1);
					}
				}

				parseString(tAttr.name);
				parseString(tAttr.value);
				//syslog(LOG_NOTICE,"parsed obj: %s-%s",tAttr.name.c_str(),tAttr.value.c_str());

				tAttr.attribute_source=attribute_source;
				vAttr.push_back(tAttr);

				if( i==(str.size()-1) ){
					return;
				}
			}
		}
	}
}

void parseCookieParams(string & str,vector <struct Attribute> & vAttr){
	unsigned int mode=0,pos=0;
	struct Attribute tAttr;

	tAttr.name = "cookie_";
	for(unsigned int i=0;i<str.size();i++){
		if(mode==0){
			if(str[i]=='='){
				if( (i-pos) > MAX_PARAM){
					tAttr.name.append(str.begin()+pos,str.begin()+pos+MAX_PARAM);
				}else{
					tAttr.name.append(str.begin()+pos,str.begin()+i);
				}
				//syslog(LOG_NOTICE,"%s",tAttr.name.c_str());

				tAttr.attribute_source='H';

				mode++;
				pos=i+1;
			}
		}
		else if(mode==1){
			if(str[i]!=';' && i!=str.size()-1 ){
				if(str[i]=='&'){
					tAttr.attribute_source='C';
				}
			}else{
				if( i!=(str.size()-1) ){
					tAttr.value.assign(str.begin()+pos,str.begin()+i);
					if( (i-pos) > MAX_VALUE){
						tAttr.value.assign(str.begin()+pos,str.begin()+pos+MAX_VALUE);
					}else{
						tAttr.value.assign(str.begin()+pos,str.begin()+i);
					}

					mode++;
					pos=i+1;
				}else{
					if( (i-pos) > MAX_VALUE){
						tAttr.value.assign(str.begin()+pos,str.begin()+pos+MAX_VALUE);
					}else{
						tAttr.value.assign(str.begin()+pos,str.begin()+i+1);
					}
				}

				vAttr.push_back(tAttr);

				tAttr.name = "cookie_";
				if( i==(str.size()-1) ){
					return;
				}
			}
		}
		else{
			if(str[i]==' '){
				pos++;
			}
			mode=0;
		}
	}
}

void parseNestedCookie(string param_name,string param_value,vector <struct Attribute> & vAttr){
	unsigned int mode=0,pos=0;
	struct Attribute tAttr;
	tAttr.name = param_name;
	for(unsigned int i=0;i<param_value.size();i++){
		if(mode==0){
			if(param_value[i]=='='){
				if( (i-pos) > MAX_PARAM){
					tAttr.name.append(param_value.begin()+pos,param_value.begin()+pos+MAX_PARAM);
				}else{
					tAttr.name.append(param_value.begin()+pos,param_value.begin()+i);
				}

				tAttr.attribute_source='H';
				mode++;
				pos=i+1;
			}
		}else{
			if(param_value[i]!='&' && i!=param_value.size()-1 ){
			}else{
				if( i!=(param_value.size()-1) ){
					tAttr.value.assign(param_value.begin()+pos,param_value.begin()+i);
					if(i > MAX_VALUE){
						tAttr.value.assign(param_value.begin()+pos,param_value.begin()+pos+MAX_VALUE);
					}else{
						tAttr.value.assign(param_value.begin()+pos,param_value.begin()+i);
					}


					mode=0;
					pos=i+1;
				}else{
					if(i > MAX_VALUE){
						tAttr.value.assign(param_value.begin()+pos,param_value.begin()+pos+MAX_VALUE);
					}else{
						tAttr.value.assign(param_value.begin()+pos,param_value.begin()+i+1);
					}
				}
				vAttr.push_back(tAttr);
				tAttr.name = param_name;
				if( i==(param_value.size()-1) ){
					return;
				}
			}
		}
	}
}

void parseJsonRecursive(Json::Value & root,vector <struct Attribute> & vAttr){
		string key,val;
        unsigned int j;
        struct Attribute tAttr;
        tAttr.attribute_source = 'J';

        for (Json::Value::iterator it=root.begin(); it!=root.end(); ++it){
        	
        	if((*it).isArray()){
        		Json::Value array = (*it);
        		int printOnceFlag = 1;
        		for (j=0; j < array.size(); j++) {

					if(array[j].isArray()){
						parseJsonRecursive(array[j],vAttr);
						tAttr.name = it.key().asString();
						tAttr.value = "JsonArray";

						if(printOnceFlag){
							vAttr.push_back(tAttr);
							printOnceFlag = 0;
						}
					}else if( array[j].isObject() ){
						parseJsonRecursive(array[j],vAttr);

						tAttr.name = it.key().asString();
						tAttr.value = "JsonObject";
						if(printOnceFlag && tAttr.name !="Arr"){
							vAttr.push_back(tAttr);
							printOnceFlag = 0;
						}
					}else{
						tAttr.name = it.key().asString();
						if ( (*it).isString() ){
							val = (*it).asString();
							parseString(val);
							tAttr.value = val;
						}else{
							val = (*it).toStyledString();
							parseString(val);
							tAttr.value = val;
						}
						if(printOnceFlag){
							vAttr.push_back(tAttr);
							printOnceFlag = 0;
						}
					}
        		}
        	}else if((*it).isObject()){
         		parseJsonRecursive((*it),vAttr);
        		tAttr.name = it.key().asString();
				tAttr.value = "JsonObject";

				vAttr.push_back(tAttr);
				//syslog(LOG_NOTICE,"tAttr- Key: %s , Value: %s",tAttr.name.c_str(),tAttr.value.c_str());
        	}else{ //else it is not an Json Object
				tAttr.name = it.key().asString();
				if ( (*it).isString() ){
					val = (*it).asString();
					parseString(val);
					tAttr.value = val;
				}else{
					val = (*it).toStyledString();
					parseString(val);
					tAttr.value = val;
				}
				vAttr.push_back(tAttr);
				//syslog(LOG_NOTICE,"tAttr- Key: %s , Value: %s",tAttr.name.c_str(),tAttr.value.c_str());
        	}
        }
}

void parseJson(string & name,vector <struct Attribute> & vAttr){
	try{
	Json::Value root;
	Json::Reader reader;

	if(name[0]=='['){
		name.insert(0,1,':');
		name.insert(0,1,'\'');
		name.insert(0,1,'r');
		name.insert(0,1,'r');
		name.insert(0,1,'A');
		name.insert(0,1,'\'');
		name.insert(0,1,'{');
		name.append("}");
		//syslog(LOG_NOTICE, "%s",name.c_str());

	}
	//syslog(LOG_NOTICE, "%s",name.c_str());
	bool parsingSuccessful = reader.parse( name.c_str(), root );     //parse process
	if ( !parsingSuccessful )
	{
		syslog(LOG_NOTICE, "%s",name.c_str());
		syslog(LOG_NOTICE,"Failed to parse %s", reader.getFormattedErrorMessages().c_str());
	}else{
		parseJsonRecursive(root,vAttr);
	}
	}catch(...){
		syslog(LOG_NOTICE, "Json Parsing Failed");
	}
}

void parseXML(mxml_node_t *tree,string res,vector <struct Attribute> & vAttr){

	if (tree->value.element.name && (tree->child != NULL || tree->next != NULL) ){
		if(tree->next!=NULL){
			parseXML(tree->next,res,vAttr);
		}
		if(tree->child != NULL){
			if (!res.empty()){
				res.append("::") ;
			}
			res.append(tree->value.element.name) ;
			parseXML(tree->child,res,vAttr);
		}
	}

	if( (tree->value.opaque != NULL) && (tree->child == NULL) && (tree->next == NULL) ){
		struct Attribute tAttr;

		tAttr.value = string(tree->value.opaque);
		if( tAttr.value.size() > (MAX_VALUE*2) ){
			string tmp_str = tAttr.value;
			tAttr.value.clear();
			tAttr.value.assign(tmp_str.begin(),tmp_str.begin() + (MAX_VALUE*2) );
		}

		if(tAttr.value.size()==1 && tAttr.value[0]==10){
		}else{
			tAttr.name = res;
			tAttr.attribute_source='X';

			parseString(tAttr.name);
			parseString(tAttr.value);

			vAttr.push_back(tAttr);
		}

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

bool dynamicPageInit(string & page){
	string key,value;
	for (unsigned int i=page.size(); i > 0 ; i--){
		if(page[i]=='/'){
			key.assign(page.begin(),page.begin()+i);
			value.assign(page.begin()+i,page.end());
			itDynamic = mDynamicPages.find(key);
			if(itDynamic != mDynamicPages.end() ){
				itDynamic->second.insertToList( hashCode(value) );
				if( sDynamicPages.count(key) != 0 ){ // The dynamic page was found.
					page = key;
					return true;
				}
			}else{
				Enumeration enumeration;
				enumeration.insertToList( hashCode(value) );
				mDynamicPages.insert(pair<string,Enumeration>(key,enumeration));
			}
			return false;
		}
	}
	return false;
}
void encodeNoSQL (string & req_str){
	replace(req_str.begin(),req_str.end(),'{','\\');
	replace(req_str.begin(),req_str.end(),'}','\\');

	//syslog(LOG_NOTICE,"New req_str: %s",req_str.c_str());
}

void TeleCache::addobject(TeleObject *teleo,std::unordered_map<string,string> & obj)
{
	string flag_loadbalncer="",fullCookie="";
	string ext,fullPageName,attName,postparams,appid,authorization;
	unsigned int i,subDomainID,hash_page;
	unsigned short statusCode;
	std::unordered_map<string,string>::iterator itConvertObj;
	vector <struct Attribute> vAttr;
	struct Attribute attribute;
	size_t found;
	bool multipart_flag=false,xml_content_flag=false,xml_tag_flag=false,post_data=false,json_flag=false;
	double presence=1;

	for(itConvertObj = obj.begin() ; itConvertObj != obj.end() ; itConvertObj++){
		//syslog(LOG_NOTICE,"Old str: %s",itConvertObj->second.c_str());
		//encodeNoSQL(itConvertObj->second);
		
		if(itConvertObj->first.size() == 2){
			if(itConvertObj->first[0] == 'R'){
				if(itConvertObj->first[1] == 'I'){
					teleo->mParams['u'/*RespIP*/] = itConvertObj->second;
				}
				else if(itConvertObj->first[1] == 'C'){
					teleo->mParams['d'/*StatusCode*/] = itConvertObj->second;
				}
				else if(itConvertObj->first[1] == 'B'){
					teleo->mParams['B'/*ResponseBody*/] = itConvertObj->second;
				}
			}
			else if(itConvertObj->first[0] == 'T'){
				if(itConvertObj->first[1] == 'I'){
					teleo->mParams['a'/*UserIP*/] = itConvertObj->second;
				}
				else if(itConvertObj->first[1] == 'M'){
					teleo->mParams['h'/*Method*/] = itConvertObj->second;
				}
				else if(itConvertObj->first[1] == 'T'){
					teleo->mParams['b'/*TimeStamp*/] = itConvertObj->second;
				}
				else if(itConvertObj->first[1] == 'P'){
					teleo->mParams['i'/*Protocol*/] = itConvertObj->second;
				}
				else if(itConvertObj->first[1] == 'Q'){
					parseGetPostParams(itConvertObj->second,vAttr,'G');
				}
				else if(itConvertObj->first[1] == 'S'){
					teleo->mParams['E'/*SHA256_SID*/] = itConvertObj->second;
					teleo->mParams['e'/*SID*/] = makeSID(itConvertObj->second);
				}
				else if(itConvertObj->first[1] == 'B'){					
					postparams = itConvertObj->second;
				}
				else if(itConvertObj->first[1] == 'U'){
					size_t pos = 0;
					string tmpUri = itConvertObj->second;
					std::reverse(tmpUri.begin(),tmpUri.end());
					pos = tmpUri.find(".");
					if(pos != string::npos){
						tmpUri.erase(pos+1,tmpUri.size()-1);
						reverse(tmpUri.begin(),tmpUri.end());
						for (boost::unordered_set <string>::iterator it = sFilterExtensions.begin(); it != sFilterExtensions.end(); ++it ){
							if(tmpUri == (*it)){
								dropApp++;
								//syslog(LOG_NOTICE,"Blocklist: %s our uri extention: %s",(*it).c_str(),tmpUri.c_str());
								return;
							}
						}
					}
					//Max Size for url
					if(itConvertObj->second.size()>MAX_PARAM){
						itConvertObj->second.erase(itConvertObj->second.begin()+MAX_PARAM,itConvertObj->second.end());
					}
					parseString(itConvertObj->second);
					teleo->mParams['c'/*Page*/] = itConvertObj->second;
				}
			}
		}
		else if(itConvertObj->first.size() > 2){
                        if(itConvertObj->first[0] == 'R'){
                                if(itConvertObj->first == "RH_set-cookie"){
                                        teleo->mParams['p'/*SetCookie*/] = itConvertObj->second;
                                }
                        }
			else if(itConvertObj->first[0] == 'T'){
				if(itConvertObj->first[1]=='H'){
					attribute.attribute_source= 'H';
				}else{
					continue;
				}

				// Max parameter name 
				attribute.name.clear();
				if(itConvertObj->first.size() > MAX_PARAM){
					attribute.name.insert(attribute.name.begin(),itConvertObj->first.begin()+3,itConvertObj->first.begin()+MAX_PARAM);
				}else{
					attribute.name.insert(attribute.name.begin(),itConvertObj->first.begin()+3,itConvertObj->first.begin()+itConvertObj->first.size());
				}

				if(attribute.name.size()==0){
					continue;
				}

				// Max parameter value

				attribute.value.clear();
				if(itConvertObj->second.size() > MAX_VALUE){
					attribute.value.insert(attribute.value.begin(),itConvertObj->second.begin(),itConvertObj->second.begin()+MAX_VALUE);
				}else{
					attribute.value.insert(attribute.value.begin(),itConvertObj->second.begin(),itConvertObj->second.begin()+itConvertObj->second.size());
				}

				parseString(attribute.name);
				parseString(attribute.value);

				//loadbalancer check headers then ips
				for (boost::unordered_set <string>::iterator it = sLoadbalancerHeaders.begin(); it != sLoadbalancerHeaders.end(); ++it ){
					if(attribute.name == (*it)){
						flag_loadbalncer = attribute.value;
					}
				}
					
				if(attribute.name=="user-agent"){
					vAttr.push_back(attribute);
					presence -= 0.2;
					teleo->mParams['z'/*user-agent*/] = itConvertObj->second;
				}
				else if(attribute.name=="cookie"){
					//syslog(LOG_NOTICE,"cookie: %s", attribute.value.c_str());
					fullCookie = attribute.value;
					parseCookieParams(attribute.value,vAttr);
				}
				else if(attribute.name=="host"){
					vAttr.push_back(attribute);
					presence -= 0.2;
					teleo->mParams['f'/*App*/] = itConvertObj->second;
					findAppID(teleo->mParams['f'/*App*/],appid);
					teleo->mParams['g'/*AppID*/] = appid;
				}
				else if(attribute.name=="content-type"){
					vAttr.push_back(attribute);
					presence -= 0.1;
					found = attribute.value.find("multipart/");
					if (found!=std::string::npos){
						multipart_flag=true;
						continue;
					}

					found = attribute.value.find("xml");
					if (found!=std::string::npos){
						xml_content_flag=true;
						continue;
					}
					found = attribute.value.find("json");
					if (found!=std::string::npos){
						json_flag = true;
					}
					if(const_content_type.count(attribute.value) != 0){
						//syslog(LOG_NOTICE,"%s",attribute.value.c_str());
						post_data=true;
					}
				}else{
					vAttr.push_back(attribute);
					presence -= 0.1;
				}
			}
		}

	}

	try{
	if(flag_loadbalncer.length()>0){
		for (std::vector<Range>::iterator it_ip = loadbalancer_ips.begin() ; it_ip != loadbalancer_ips.end(); ++it_ip){
			unsigned int int_ip = ipToNum(teleo->mParams['a'/*UserIP*/]);
			if((*it_ip).inRange(int_ip)){
				teleo->mParams['a'/*UserIP*/] = flag_loadbalncer;
			}
		}
	}

	//Finger Print
	string fingerprint = "";
	//if(fullCookie.size() > 0){
	//		fingerprint = fullCookie;
	//}else{
		fingerprint = teleo->mParams['a'/*UserIP*/];

		if(teleo->mParams['z'/*user-agent*/].size() > 0 && teleo->mParams['f'/*App*/].size() > 0){
			fingerprint += teleo->mParams['z'/*user-agent*/] + teleo->mParams['f'/*App*/];
		}
		else{
			fingerprint += teleo->mParams['u'/*RespIP*/];
		}

	//}
	fingerprint = sha256(fingerprint);
	teleo->mParams['E'/*SHA256_SID*/] = fingerprint;
	teleo->mParams['e'/*SID*/] = makeSID(fingerprint);
	
	//syslog(LOG_NOTICE,"srcIP: %s, UserAgent: %s, Host: %s",teleo->mParams['a'/*UserIP*/].c_str(),teleo->mParams['z'/*user-agent*/].c_str(),teleo->mParams['f'/*App*/].c_str());
	//syslog(LOG_NOTICE,"fingerprint: %s", fingerprint.c_str());
	//syslog(LOG_NOTICE,"fingerprintSha256: %s", sha256(fingerprint).c_str());
	}catch(...){ syslog(LOG_NOTICE,"Somting Wrong With Fingerprint Or Loadbalancer");}
	//syslog(LOG_NOTICE,"Getting Obj7 ===> %s:%s:%s:%s:%s:%s:%s:%s:%s",teleo->mParams['a'/*UserIP*/].c_str(),teleo->mParams['b'/*TimeStamp*/].c_str(),teleo->mParams['c'/*Page*/].c_str(),teleo->mParams['i'/*Protocol*/].c_str(),teleo->mParams['d'/*StatusCode*/].c_str(),teleo->mParams['j'/*RID*/].c_str(),teleo->mParams['h'/*Request*/].c_str(),teleo->mParams['z'/*user-agent*/].c_str(),teleo->mParams['f'/*App*/].c_str());

	if(presence < 0){
		presence=0;
	}

	teleo->mParams['l'/*PresenceOrAbsence*/]=int_to_string(presence);
	statusCode = (unsigned short)atoi(teleo->mParams['d'/*StatusCode*/].c_str() );

	if(postparams.size() > 0){
		if(postparams[0]=='<'){
			xml_tag_flag=true;
		}
	}

	if(multipart_flag==true){
		parseMultipart(postparams,vAttr);
	}
	else if(xml_content_flag==true || xml_tag_flag==true){
		if(statusCode!=404){
			unsigned int offset = checkXML( (char*)postparams.c_str() );
			if(offset != 0){
				mxml_node_t *tree = mxmlLoadString(NULL, (const char*)postparams.c_str()+offset ,MXML_OPAQUE_CALLBACK);
				if(tree != NULL){
					parseXML(tree,"",vAttr);
					// free mxml memory !!!!!!!!!
					mxmlDelete( tree );
				}else{
					#ifdef DEBUG
						syslog(LOG_NOTICE,"BROKEN XML");
					#endif
				}
			}
		}else{
			postparams.clear();
		}
	}
	else if(json_flag){
		if(postparams.length() > 0){
			parseJson(postparams,vAttr);
		}
		
	}
	else if(post_data==true){
		parseGetPostParams(postparams,vAttr,'P');
	}

	postparams.clear();

	if(teleo->mParams['i'/*Protocol*/].size() == 0){
		teleo->mParams['i'/*Protocol*/] = "http";
	}

	if(teleo->mParams['f'/*App*/].size()==0){
		teleo->mParams['f'/*App*/] = teleo->mParams['u'/*RespIP*/];
		findAppID(teleo->mParams['f'/*App*/],appid);
		teleo->mParams['g'/*AppID*/] = appid;
	}

	if( teleo->mParams['g'/*AppID*/].size() == 0 ){
		dropApp++;
		return;
	}

	if(teleo->mParams['c'/*Page*/].size() == 0){
		dropPage++;
		return;
	}

	if(const_method.count(teleo->mParams['h'/*Method*/]) == 0){ // Method wasn't found.
		dropMethod++;
		return;
	}

	unsigned int hashResp = (unsigned int)hashCode(teleo->mParams['B'/*ResponseBody*/]);

	// Build ElsticSearch Scheme.
	unsigned int index_time = (unsigned int)atoi(teleo->mParams['b'/*TimeStamp*/].c_str()) / 86400;
	if( index_time == shard_time){
		teleo->mParams['S'/*Shard*/] = shard;
	}else{
		shard_time = index_time;
		shard = getDate( (unsigned int)atoi(teleo->mParams['b'/*TimeStamp*/].c_str()) );
		teleo->mParams['S'/*Shard*/] = shard;

		char curl_buff[200];
		sprintf(curl_buff,"/telepath-%s",&shard[0]);
        es_insert(curl_buff,"{\"settings\":{\"number_of_shards\":1,\"number_of_replicas\":0,\"analysis\":{\"analyzer\":{\"param-value-analyzer\":{\"type\":\"custom\",\"tokenizer\":\"param-value-tokenizer\",\"filter\":[\"lowercase\"]},\"lowercase-analyzer\":{\"type\":\"custom\",\"tokenizer\":\"keyword\",\"filter\":[\"lowercase\"]},\"whitespace-analyzer\":{\"type\":\"custom\",\"tokenizer\":\"whitespace\",\"filter\":[\"lowercase\"]},\"username-analyzer\":{\"type\":\"custom\",\"tokenizer\":\"username-tokenizer\",\"filter\":[\"lowercase\"]},\"dot-analyzer\":{\"type\":\"custom\",\"tokenizer\":\"dot-tokenizer\"},\"slash-analyzer\":{\"type\":\"custom\",\"tokenizer\":\"slash-tokenizer\"},\"search-analyzer\":{\"type\":\"custom\",\"tokenizer\":\"search-tokenizer\",\"filter\":[\"lowercase\"]}},\"tokenizer\":{\"username-tokenizer\":{\"type\":\"pattern\",\"pattern\":\"[-_\\\\.\\\\s]\"},\"slash-tokenizer\":{\"type\":\"pattern\",\"pattern\":\"\/\"},\"dot-tokenizer\":{\"type\":\"pattern\",\"pattern\":\"\\\\.\"},\"param-value-tokenizer\":{\"type\":\"pattern\",\"pattern\":\"[\/\\\\.\\\\s-_\\\\\\\\,\\\\+\\\\;]\"},\"search-tokenizer\":{\"type\":\"pattern\",\"pattern\":\"[-_\/\\\\.\\\\s]\"}}}},\"mappings\":{\"http\":{\"dynamic\":\"true\",\"properties\":{\"host\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"dot-analyzer\"}}},\"uri\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"slash-analyzer\"}}},\"canonical_url\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"slash-analyzer\"}}},\"alerts\":{\"properties\":{\"name\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"simple\"}}},\"score\":{\"type\":\"float\"}}},\"alerts_count\":{\"type\":\"long\"},\"business_actions\":{\"properties\":{\"name\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"simple\"}}},\"status\":{\"type\":\"short\"}}},\"cases_name\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"simple\"}}},\"city\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"whitespace-analyzer\"}}},\"country_code\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"lowercase-analyzer\"}}},\"explanations\":{\"type\":\"long\"},\"explanations_count\":{\"type\":\"long\"},\"index\":{\"type\":\"long\"},\"ip_orig\":{\"type\":\"ip\"},\"ip_resp\":{\"type\":\"ip\"},\"ip_score\":{\"type\":\"double\"},\"location\":{\"type\":\"geo_point\"},\"method\":{\"type\":\"keyword\"},\"parameters\":{\"properties\":{\"name\":{\"type\":\"keyword\"},\"score_data\":{\"type\":\"double\"},\"score_length\":{\"type\":\"double\"},\"type\":{\"type\":\"keyword\"},\"value\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"param-value-analyzer\"}}}}},\"score_average\":{\"type\":\"float\"},\"score_flow\":{\"type\":\"float\"},\"score_geo\":{\"type\":\"float\"},\"score_landing\":{\"type\":\"float\"},\"score_presence\":{\"type\":\"float\"},\"score_query\":{\"type\":\"float\"},\"sid\":{\"type\":\"long\"},\"status_code\":{\"type\":\"short\"},\"sha256_sid\":{\"type\":\"keyword\"},\"title\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\"}}},\"user_scores\":{\"properties\":{\"score_average\":{\"type\":\"float\"},\"score_flow\":{\"type\":\"float\"},\"score_geo\":{\"type\":\"float\"},\"score_landing\":{\"type\":\"float\"},\"score_presence\":{\"type\":\"float\"},\"score_query\":{\"type\":\"float\"}}},\"username\":{\"type\":\"keyword\",\"fields\":{\"search\":{\"type\":\"text\",\"term_vector\":\"yes\",\"analyzer\":\"username-analyzer\"}}},\"ts\":{\"type\":\"double\"}}}}}");
        }


	//Make RID.
	long long uRID = longRand();
	teleo->mParams['j'/*RID*/]=int_to_string(uRID);

	//Get Title
	if(hashRespBodys.count(hashResp) == 0){
		findTitle(teleo->mParams['m'/*Title*/],(char*)teleo->mParams['B'/*ResponseBody*/].c_str());
	}

        if(teleo->mParams['m'/*Title*/].size()>MAX_PARAM){
                teleo->mParams['m'/*Title*/].erase(teleo->mParams['m'/*Title*/].begin()+MAX_PARAM,teleo->mParams['m'/*Title*/].end());
        }

	parseString(teleo->mParams['m'/*Title*/]);

	#ifdef DEBUG
		//Saving fullPageName as APP+PAGE
		fullPageName=teleo->mParams['f'/*App*/]+teleo->mParams['c'/*Page*/];
		if( dynamicPageInit(fullPageName) == true){
			teleo->mParams['C'/*Canonical URL*/] = fullPageName;
		}
		hash_page = (unsigned int)hashCode(fullPageName);
		teleo->mParams['k'/*PageID*/] = int_to_string(hash_page);
	#else
		if(statusCode>=400 && statusCode<500){
			hash_page=0;
			teleo->mParams['k'/*PageID*/] = "0";
		}else{
			//Saving fullPageName as APP+PAGE
			fullPageName=teleo->mParams['f'/*App*/]+teleo->mParams['c'/*Page*/];
			if( dynamicPageInit(fullPageName) == true){
				teleo->mParams['C'/*Canonical URL*/] = fullPageName;
			}
			hash_page = (unsigned int)hashCode(fullPageName);
			teleo->mParams['k'/*PageID*/] = int_to_string(hash_page);
		}
	#endif

	dynamicCounter++;
	if(dynamicCounter % 10000 == 0){
		sDynamicPages.clear();
		for(itDynamic = mDynamicPages.begin() ; itDynamic!=mDynamicPages.end() ; itDynamic++){
			if(itDynamic->second.getCorrelation() == false){
				sDynamicPages.insert( itDynamic->first );
				#ifdef DEBUG
					syslog(LOG_NOTICE,"---------------------------------------------------");
					syslog(LOG_NOTICE,"Dynamic Page -> %s",itDynamic->first.c_str());
					itDynamic->second.print_syslog();
				#endif
			}
			#ifdef DEBUG
			else{
				syslog(LOG_NOTICE,"---------------------------------------------------");
				syslog(LOG_NOTICE,"Regular Page -> %s",itDynamic->first.c_str());
				itDynamic->second.print_syslog();
			}
			#endif
		}

	}

	if(hashRespBodys.count(hashResp) == 0){
		Login login;
		loginDetect((char*)teleo->mParams['B'/*ResponseBody*/].c_str(),login);
		if(login.password.size() != 0){
			login.host = teleo->mParams['f'/*App*/];
			login.orig_uri = teleo->mParams['c'/*Page*/];
			string tmp_login_user = login.host + login.orig_uri + login.username;
			setUserIDs.insert((unsigned int)hashCode(tmp_login_user));
			pthread_mutex_lock(&mutexLogin);
			loginQueue.push(login);
			pthread_mutex_unlock(&mutexLogin);
			sem_post(&sem_insert_logins);

			#ifdef DEBUG
				syslog(LOG_NOTICE,"Login page has been detected!!!");
			#endif
		}
		detectPasswords((char*)teleo->mParams['B'/*ResponseBody*/].c_str(),teleo->mParams['f'/*App*/],teleo->mParams['c'/*Page*/]);
	}

	AppMode am;
	pthread_mutex_lock(&mutexAppMode);
		boost::unordered_map <string,AppMode>::iterator itAppMode = mAppMode.find(appid);
		if(itAppMode != mAppMode.end()){
			am = itAppMode->second;
		}
	pthread_mutex_unlock(&mutexAppMode);

	//loginSuccess((char*)teleo->mParams['B'/*ResponseBody*/].c_str(),teleo->mParams['v'/*LoginMsg*/] );
	//searchDetect((char*)teleo->mParams['B'/*ResponseBody*/].c_str(),teleo->mParams['g'/*AppID*/],hash_page,teleo->mParams['c'/*Page*/]);

	authorization.clear();
	for(i=0 ;i < vAttr.size();i++)
	{
		if(vAttr[i].name.size()==0){
			continue;
		}

		attName.clear();
		if(vAttr[i].attribute_source=='H'){
			if(vAttr[i].name == "authorization"){
				authorization = vAttr[i].value;
			}

			it_global_header=global_header.find(vAttr[i].name);
			if(it_global_header != global_header.end() ){ // Attribute name was found.
				vAttr[i].hash = (unsigned int)it_global_header->second;
			}else{					    // Attribute name wasn't found.
				//if(domain_header.count(vAttr[i].name) != 0){ // Attribute name was found.
				attName.append(teleo->mParams['g'/*AppID*/]);
				attName.push_back('+');
				attName.append(vAttr[i].name);
				//}else{
				//	attName.append(fullPageName);
				//	attName.append(vAttr[i].name);
				//}
				vAttr[i].hash = (unsigned int)hashCode(attName);
			}
		}else{
			attName.append(fullPageName);
			attName.append(vAttr[i].name);
			vAttr[i].hash = (unsigned int)hashCode(attName);

			for(unsigned int i_lower=0;i_lower<vAttr[i].name.size();i_lower++){
				if(vAttr[i].name[i_lower]>64 && vAttr[i].name[i_lower]<91){
					vAttr[i].name[i_lower] += 32;
				}
			}

			if(am.form_param_name==vAttr[i].name){
				usernamePerIP[teleo->mParams['a'/*UserIP*/]] = vAttr[i].value;
			}

			if(vAttr[i].name=="authorization"){
				if(am.digest_mode==1){
					if(vAttr[i].value.size() > 0){
						size_t found=0,found2=0;
						string username;
						username.clear();
						found = vAttr[i].value.find("username=`",found);
						found += 10;

						found2 = vAttr[i].value.find('`',found);
						username.assign(vAttr[i].value.begin()+found,vAttr[i].value.begin()+found2);
						usernamePerIP[teleo->mParams['a'/*UserIP*/]] = username;
                        		}
				}
				else if(am.basic_mode==1){
					if(vAttr[i].value.size() > 0){
						size_t found=0;
						string username,encoded;
						username.clear();
						encoded.clear();
						encoded = vAttr[i].value.assign(vAttr[i].value.begin()+6,vAttr[i].value.end());
						encoded = base64_decode(encoded);
						found =  encoded.find(":",found);
						username.assign(encoded.begin(),encoded.begin()+found);
						usernamePerIP[teleo->mParams['a'/*UserIP*/]] = username;
					}
                		}
                	}
        	}

		if(hash_page != 0){
			if(vAttr[i].name=="logout" ){
				logoutDetect(teleo->mParams['g'/*AppID*/],hash_page,vAttr[i].hash);
			}
			else if(vAttr[i].name=="authenticity_token" && vAttr[i].attribute_source=='P'){
				logoutDetect2(teleo->mParams['g'/*AppID*/],hash_page,teleo->mParams['c'/*Page*/],vAttr[i].hash);
			}
		}

		string_to_utf8(vAttr[i].value, vAttr[i].vec_value);

		teleo->mAttr.insert(pair<unsigned int,struct Attribute>(vAttr[i].hash,vAttr[i]));
	}

	authentication((char*)teleo->mParams['B'/*ResponseBody*/].c_str(),am,teleo->mParams['v'/*LoginMsg*/]);

	if(teleo->mParams['v'/*LoginMsg*/]=="y"){
		teleo->mParams['A'/*Username*/] = usernamePerIP[teleo->mParams['a'/*UserIP*/]];
		usernamePerIP.erase(teleo->mParams['a'/*UserIP*/]);
	}

	hashRespBodys.insert(hashResp);
	teleo->mParams['B'/*ResponseBody*/].clear();
	pthread_mutex_lock(&mutexTeleObjQueue);
	this->teleObjQueue.push(*teleo);
	pthread_mutex_unlock(&mutexTeleObjQueue);

}

