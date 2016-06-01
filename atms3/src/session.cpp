#include <map>
#include <queue>
#include <string>
#include <string.h>
#include "../h/config.h"
#include "../h/parser.h"
#include "../h/sha1.h"
#include <set>
#include <list>

extern Config* conf_updated;


#define UNKNOWN_COOKIE 1
#define VALID_COOKIE 2
#define INVALID_COOKIE 3
#define RETURN 1
#define CONTINUE 2
using namespace std;

map<string, s_sessioninfo> msessionsinfo;//Map that holds all the session structs
map< string, map<string, s_sessioninfo>::iterator > msessionkeys; //a map that holds pairs of hashed keys and iterators to a session info struct
list<index_to_msessioninfo > sessionsinfo_list; //fifo list that holds pointers to iterators. head = oldest , tail = newest 
set<string> mips;
//set<string> mfps;
//set<string> mcookies;
map<string, string> mfingerprints;

static void make_sid(string& output_sid,string& ip,string& timestamp);
static void make_hash(string & output_hash,string& key1,string& key2);
int getCookie_by_name(	string& cookie_str,
			string& cookie_val,
			string& set_cookie_str,
			string& set_cookie_val,
			int& cookie_status,
			bool& has_cookie,
			string& hostname,
			string& cookie_name,
			bool& has_set_cookie);


/*
 * void getCookie(	string& cookie_str,
		string& cookie_val,
		string& set_cookie_str,
		string& set_cookie_val,
		int& cookie_status,
		bool& has_cookie,
		string& hostname,
		string& cookie_name,
		bool& has_set_cookie	)

 *		Gets the cookie value of the request defined for the hostname, checks if the request has a cookie, has a set-cookie,
 *		
 * 
 * Parameters:
 * 	string& cookie_str, - the full cookie string from the request
 * 	string& cookie_val, - the cookie value to be returned
 * 	string& set_cookie_str, - the full set-cookie string from the request
 * 	string& set_cookie_val, - the set-cookie value to be returned
 * 	int& cookie_status, - the cookie status to be returned,valid if we found the application cookie, invalid else, unkown if it doesnt have a cookie at all
 * 	bool& has_cookie, - changed to true if we found our cookie
 * 	string& hostname, - the hostname string
 * 	string& cookie_name, the application cookie name, will be given in this function
 * 	bool& has_set_cookie - changed to true if we found our set-cookie
 * 		
 * Return value:
 * 		void
 */
//cookie status : 3 = invalid, 2 = valid, 1 = unknown
void getCookie(	string& cookie_str,
		string& cookie_val,
		string& set_cookie_str,
		string& set_cookie_val,
		int& cookie_status,
		bool& has_cookie,
		string& hostname,
		string& cookie_name,
		bool& has_set_cookie	)
{
	set<string>::iterator cookie_names_iter;	
	int getCookie_by_name_val;
	if (conf_updated == NULL)
	{
		syslog(LOG_ERR,"ERROR - Configuration struct is NULL, please report to Hybrid");
                cookie_status = INVALID_COOKIE;
                has_cookie = false;
                has_set_cookie = false;
                return;
	}

	unsigned int i_lowcase;
	AppKey app;
	app.name = ".";
	app.name += hostname;
	app.search_subdomain = true;
	reverse(app.name.begin(), app.name.end());
	map<class AppKey,struct AppData>::iterator itr = conf_updated->mAppData.find(app);

	//-----------------To lower case-----------------
	for(i_lowcase=0;i_lowcase<cookie_str.size();i_lowcase++){
		if(cookie_str[i_lowcase]<91 && cookie_str[i_lowcase]>64){
			cookie_str[i_lowcase] +=32;
		}
	}

	for(i_lowcase=0;i_lowcase<set_cookie_str.size();i_lowcase++){
		if(set_cookie_str[i_lowcase]<91 && set_cookie_str[i_lowcase]>64){
			set_cookie_str[i_lowcase] +=32;
		}
	}
	//-----------------------------------------------

	//if the app is unknown, set invalid and return
	if (itr==conf_updated->mAppData.end()){
		cookie_status = INVALID_COOKIE;
		has_cookie = false;
		has_set_cookie = false;
		return;
	}
	//edge case, empty set cookie string or cookie string, "-" in the cookie value , return false and unknown
	if ((cookie_str.size()==0 && set_cookie_str.size()==0) || (!strcmp(cookie_str.c_str(),"-") && set_cookie_str.size()==0)){
		cookie_status = UNKNOWN_COOKIE;
		has_cookie = false;
		has_set_cookie = false;
		return;				
	} 
	////////////////////////////////////////////////////////////////////
	//////////////loop starts here on cookie name///////////////////////
	cookie_names_iter = itr->second.cookie_names.begin();
	//if no cookies were inserted, and we got here, then return invalid
	if(itr->second.cookie_names.size()==0){
		cookie_status = INVALID_COOKIE;
		//printf("no cookies returning invalid\n");
	}
	else{
		for(;cookie_names_iter!=itr->second.cookie_names.end();cookie_names_iter++)
		{
			cookie_name = *cookie_names_iter;
			getCookie_by_name_val = getCookie_by_name(	cookie_str,
								cookie_val,
								set_cookie_str,
								set_cookie_val,
								cookie_status,
								has_cookie,
								hostname,
								cookie_name,
								has_set_cookie	);
			if (cookie_status==VALID_COOKIE){
				//if its valid, no need to check other cookie names
				return;
			}
			if (getCookie_by_name_val==RETURN){				
				return;
			}
			else if (getCookie_by_name_val==CONTINUE)
				continue;
		}		
	}	
	////////////////////////////////////////////////////////////////////
	//////////////loop starts here on cookie name///////////////////////
}

/*
 * void getCookie_by_name(	string& cookie_str,
		string& cookie_val,
		string& set_cookie_str,
		string& set_cookie_val,
		int& cookie_status,
		bool& has_cookie,
		string& hostname,
		string& cookie_name,
		bool& has_set_cookie	)

 *		Gets the cookie value of the request defined for the hostname, checks if the request has a cookie, has a set-cookie,
 *		
 * 
 * Parameters:
 * 	string& cookie_str, - the full cookie string from the request
 * 	string& cookie_val, - the cookie value to be returned
 * 	string& set_cookie_str, - the full set-cookie string from the request
 * 	string& set_cookie_val, - the set-cookie value to be returned
 * 	int& cookie_status, - the cookie status to be returned,valid if we found the application cookie, invalid else, unkown if it doesnt have a cookie at all
 * 	bool& has_cookie, - changed to true if we found our cookie
 * 	string& hostname, - the hostname string
 * 	string& cookie_name, the application cookie name, will be given in this function
 * 	bool& has_set_cookie - changed to true if we found our set-cookie
 * 		
 * Return value:
 * 		void
 */
//return value 1- return, return value 2, continue
int getCookie_by_name(	string& cookie_str,
			string& cookie_val,
			string& set_cookie_str,
			string& set_cookie_val,
			int& cookie_status,
			bool& has_cookie,
			string& hostname,
			string& cookie_name,
			bool& has_set_cookie)
{
		size_t start_pos=0;
		size_t end_pos=0;
		//if theres no set cookie and no cookie , set unknown and return
		//crucial: if cookie name wasn't inserted in the application info, we need to return INVALID
		if (cookie_name.size()==0){
			cookie_status = INVALID_COOKIE;
			has_cookie = false;
			has_set_cookie = false;
			return CONTINUE;				
		}
		//look for our cookie in the cookie_string	
		if((start_pos = cookie_str.find(cookie_name+"=")) == string::npos){
			start_pos = 0;
			has_cookie = false;
		}
		//found ours
		else{
			cookie_status = VALID_COOKIE;
			has_cookie = false;
			start_pos+= cookie_name.size()+1; //skip the =
			end_pos = cookie_str.find_first_of("; \t\r\n",start_pos);
			//case its the last cookie 
			if (end_pos == string::npos)
				end_pos = cookie_str.size();
			if (end_pos!=start_pos){
				cookie_val.assign(cookie_str.c_str()+start_pos,end_pos-start_pos); 
				has_cookie = true;
			}
		}
		//if we didnt find our cookie in the set_cookie, and we didn't find the cookie as well return invalid,continue to look for next cookie name	
		if(((start_pos = set_cookie_str.find(cookie_name+"=")) == string::npos) && !has_cookie){				
			has_set_cookie = false;
			cookie_status = INVALID_COOKIE;
			return CONTINUE;
		}
		//if we didnt find the cookie in the set cookie and we did find it in the cookie header,return cookie value
		if (start_pos== string::npos && has_cookie)
		{
			has_set_cookie = false;
			cookie_status = VALID_COOKIE;
			return RETURN;
		}
		//we either didnt find cookie or didnt find 
		has_set_cookie = false;
		start_pos+= cookie_name.size()+1; // 1 = "+" //skip the =
		end_pos = set_cookie_str.find_first_of("; \t\r\n",start_pos);
		//case its the last cookie 
		if (end_pos == string::npos)
			end_pos = set_cookie_str.size();
		if (end_pos!=start_pos){
			set_cookie_val.assign(set_cookie_str.c_str()+start_pos,end_pos-start_pos);
			cookie_status = VALID_COOKIE;
			has_set_cookie = true;
			return RETURN;
		}
//		if (cookie_status==INVALID_COOKIE)
//			syslog(LOG_ERR,"GOT INVALID(6): hotname:%s,app_cookie_name:%s,cookie_str:%s,cookie_found_flag:%s,cookie_value_found:%s,set_cookie_found_flag:%s,set_cookie_val:%s,,cookie_status:%d",hostname.c_str(),cookie_name.c_str(),cookie_str.c_str(),(has_cookie?"True":"False"),cookie_val.c_str(),(has_set_cookie?"True":"False"),set_cookie_val.c_str(),cookie_status);
		return CONTINUE;		
}


	

//go over map of sessions
//if found session - update it
//else - create a new session and update it
/*
void getSID(string& sessionId,string& cookie_name,int cookie_status,string& cookie_value,int& req_seq,map<string,string>& mheaders){
	MapStringKeySessionsValue::iterator it;
	string hashed_sid;
	string hashed_cookie_value;
	string fingerprints_list;
	map<string, string>::iterator fingerprints_it;
	//if cookie is invalid, hash finger-print
	if (cookie_status==INVALID_COOKIE || cookie_status==UNKNOWN_COOKIE){
		for(fingerprints_it=mfingerprints.begin() ; fingerprints_it != mfingerprints.end() ; ++fingerprints_it)
		{
			if (mheaders.find(fingerprints_it->second) != mheaders.end()){				
				fingerprints_list += mheaders[fingerprints_it->second];
			}
		}
		SHA1::RunSHA1(fingerprints_list, hashed_sid);
		//if the session already exists in msessioninfo, update it,otherwise insert a new one
		it =  msessionsinfo.find(hashed_sid);
		if(it!= msessionsinfo.end())
		{	
			it->second.count++;
			it->second.last_time = time(NULL);
			//it->second.start_time = time(NULL);
			req_seq = it->second.count; 
		} else {
			s_sessioninfo si;
			si.count = 1;
			req_seq = 1;
			si.last_time = time(NULL);
			si.start_time = time(NULL);
			msessionsinfo[hashed_sid] = si; 				
		}
		sessionId = hashed_sid;
	}
	//if cookie is valid, update the session
	else if (cookie_status==VALID_COOKIE){
		SHA1::RunSHA1(cookie_value, hashed_cookie_value);
		it =  msessionsinfo.find(hashed_cookie_value);
		if(it!= msessionsinfo.end())
		{
			it->second.count++;
			it->second.last_time = time(NULL);
			//it->second.start_time = time(NULL);
			req_seq = it->second.count;
		} else {
			s_sessioninfo si;
			si.count = 1;
			req_seq = 1;
			si.last_time = time(NULL);
			si.start_time = time(NULL);
			msessionsinfo[hashed_cookie_value] = si;
		}
		sessionId = hashed_cookie_value;
	} else {
		req_seq = 0;
		sessionId.clear();
	}
}
*/

//see sheet attached

/*
 * void getSID_new(string& sessionId,
		string& set_cookie_value,
		string& cookie_value,
		int& req_seq,
		map<string,string>& mheaders,
		string& real_ip,
		string& time_stamp,
		bool has_set_cookie,
		bool has_cookie)

 *		Gets the SID of the request, using a flow chart that checks for a unique combination of ip, known cookie value, known set-cookie value,
 *		to identify the user(SEE TWO PICTURES IN THE ATMS FOLDER OF DOC ON ALON'S DESKTOP)
 *		
 * 
 * Parameters:
 * 	string& set_cookie_value - the set-cookie header value of the hostname cookie name
 * 	string& cookie_value - the cookie header value of the hostname cookie name
 * 	int& req_seq -  the index of the request in the session, will be updated in this function
 * 	map<string,string>& mheaders - the headers map
 * 	string& real_ip - the ip of the request
 * 	string& time_stamp - the time stamp of the request(was meant to be used as part of the request)
 * 	bool has_set_cookie - true if we found the application cookie in the set cookie header
 * 	bool has_cookie - true if we found the application cookie in the cookie header
 * 
 * 		
 * Return value:
 * 		void
 */
void getSID_new(string& sessionId,
		string& set_cookie_value,
		string& cookie_value,
		int& req_seq,
		map<string,string>& mheaders,
		string& real_ip,
		string& time_stamp,
		bool has_set_cookie,
		bool has_cookie){	
	//first go over all sessions and remove expired sessions
	list <index_to_msessioninfo >::iterator sessioninfo_list_itr = sessionsinfo_list.begin();	//iterator on the fifo list of the sessions	
	set<string>::iterator ips_itr = mips.begin();

	map<string, index_to_msessioninfo>::iterator msessionkeys_itr;
	list < index_to_msessionkeys>::iterator list_of_sessionkeys_indices_iterator;
	index_to_msessioninfo sessioninfo_index;
	string new_sid,hashed_sid,hashed_finger_prints,finger_prints;
	index_to_msessioninfo new_index_to_msessioninfo;
	//TODO make sure the set cookie flag works on get cookie	
	string entry_hashed,entry_string;
	string cookie_value_hashed,set_cookie_value_hashed;	
	string old_sid ;
	string new_key ;
	string hashed_key ;
	pair<index_to_msessioninfo,bool> p1;
	pair<index_to_msessionkeys,bool> p2;
	//if a new SID is created, it is created from chaining IP+Timestamp and SHA1 it
	new_sid.append(real_ip);
	new_sid.append(time_stamp);
	finger_prints += mheaders["user-agent"] + mheaders["accept"] + mheaders["accept-language"];
	SHA1::RunSHA1(new_sid, hashed_sid);
	static const string empty; 	
	///////////////started cleaning timeout'd sessions///////////////////////////
	while(sessioninfo_list_itr != sessionsinfo_list.end()){		
		sessioninfo_index = (*sessioninfo_list_itr);
		//if we found a node that isnt expired , break the loop
		if ((time(NULL) - sessioninfo_index->second.last_time) < (conf_updated->session_timeout))
			break;
		//else , we need to go keep deleting nodes
		//remove the session from sessions table
		//first, take the struct of s_sessioninfo,take the list of indices from 
		list < index_to_msessionkeys>& list_of_sessionkeys_indices = sessioninfo_index->second.keys_pointed_by;
		list_of_sessionkeys_indices_iterator = list_of_sessionkeys_indices.begin();
		//go on the list of keys from sessionkeys pointing to that struct and delete them 
		while (list_of_sessionkeys_indices_iterator!=list_of_sessionkeys_indices.end()){
			msessionkeys.erase(*list_of_sessionkeys_indices_iterator);//delete from the sessionkeys
			list_of_sessionkeys_indices_iterator++;
			//list_of_sessionkeys_indices_iterator = list_of_sessionkeys_indices.erase(list_of_sessionkeys_indices_iterator);//delete from the session pointers list
		}
		list_of_sessionkeys_indices.clear();
		//this will remove the session from both the fifo list and the session table
		msessionsinfo.erase(sessioninfo_index);
		sessionsinfo_list.erase(sessioninfo_list_itr++);
	}	
	///////////////finished cleaning timeout'd sessions	///////////////////////////	
	//first, look for ip and check if it's known
	if ((ips_itr = mips.find(real_ip))!=mips.end()){
		/////////	CASE 1 Known IP      		/////////
		if(has_cookie && has_set_cookie){
			/////////	CASE 1.1 	/////////
			SHA1::RunSHA1(cookie_value, cookie_value_hashed);
			if ((msessionkeys_itr = msessionkeys.find(cookie_value_hashed))!=msessionkeys.end()){
				/////////			CASE 1.1.1	case the cookie is known 	/////////
				//save the new cookie with the same SID
				old_sid = msessionkeys_itr->second->second.SID;
				SHA1::RunSHA1(set_cookie_value, set_cookie_value_hashed);
				update_msessioninfo_table(set_cookie_value_hashed,empty,true,msessionkeys_itr->second);//true for session already exists

				sessionId = old_sid;
				return;
			}
			else{
				/////////	CASE 1.1.2	old cookie is unknown,has new set-cookie		/////////
				IP_AND_SET_COOKIE:				
				//in that case, check for the fp
				make_hash(new_key,real_ip,finger_prints);
				if ((msessionkeys_itr = msessionkeys.find(new_key))!=msessionkeys.end()){
					////// 			CASE 1.2.1	known ip+fp + new set-cookie		/////////
					//save the new cookie with the same SID
					old_sid = msessionkeys_itr->second->second.SID;
					//update that sid for the entry of the set cookie
					SHA1::RunSHA1(set_cookie_value, set_cookie_value_hashed);
					update_msessioninfo_table(set_cookie_value_hashed,empty,true,msessionkeys_itr->second);//true for session already exists

					sessionId = old_sid;
					return;
				}
				else{
					////// 			CASE 1.2.2	unknown ip+fp + new set-cookie		/////////
					//make new sid
					make_sid(hashed_sid,real_ip,time_stamp);
					//insert the new sid to ip+fp = new sid in the sessions
					new_index_to_msessioninfo = update_msessioninfo_table(new_key,hashed_sid,false,new_index_to_msessioninfo);//false for new session

					//and insert it to the place of set-cookie
					SHA1::RunSHA1(set_cookie_value, set_cookie_value_hashed);
					update_msessioninfo_table(set_cookie_value_hashed,empty,true,new_index_to_msessioninfo);//true for session already exists

					sessionId = hashed_sid;
					return;
				}
			}
		}		
		else if(!has_cookie && has_set_cookie){
			/////////////			CASE 1.2  has cookie but doesnt have set-cookie in the request////////
			goto IP_AND_SET_COOKIE;
		}		
		else if(!has_cookie && !has_set_cookie){
			////////////			CASE 1.3  no cookie and no set-cookie			//////////
			//in that case, check for the fp
			make_hash(new_key,real_ip,finger_prints);
			if ((msessionkeys_itr = msessionkeys.find(new_key))!=msessionkeys.end()){
				////////////		CASE 1.3.1  no cookie and no set-cookie, known ip+fp	//////////
				//return the old sid
				update_msessioninfo_table(new_key,empty,true,msessionkeys_itr->second);//true for session already exists
				old_sid = msessionkeys_itr->second->second.SID;
				sessionId = old_sid;
				return;
			}
			else{
				////////////		CASE 1.3.2  no cookie and no set-cookie, unknown ip+fp	//////////
				//make new sid
				make_sid(hashed_sid,real_ip,time_stamp);
				//insert the new sid to ip+fp = new sid in the sessions

				update_msessioninfo_table(new_key,hashed_sid,false,new_index_to_msessioninfo);//false for new session

				sessionId = hashed_sid;
				return;
			}
		}
		else if (has_cookie){
			////////////			CASE 1.4  has cookie and no set-cookie	 				//////////
			SHA1::RunSHA1(cookie_value, cookie_value_hashed);
			if ((msessionkeys_itr = msessionkeys.find(cookie_value_hashed))!=msessionkeys.end()){
				////////////		CASE 1.4.1  known cookie value						//////////
				update_msessioninfo_table(cookie_value_hashed,empty,true,msessionkeys_itr->second);//true for session already exists
				old_sid = msessionkeys_itr->second->second.SID;
				sessionId = old_sid;
				return;
			}
			else {
				////////////		CASE 1.4.2  unknown cookie value					//////////
				//insert the cookie value to our cookie list			
				make_hash(new_key,real_ip,finger_prints);
				if ((msessionkeys_itr = msessionkeys.find(new_key))!=msessionkeys.end()){//case we had that one before using his fp
					////// 		CASE 1.4.2.1 unknown cookie value, known ip+fp				/////////
					//save the new cookie with the same SID
					new_index_to_msessioninfo = update_msessioninfo_table(new_key,empty,true,msessionkeys_itr->second);
					update_msessioninfo_table(cookie_value_hashed,empty,true,new_index_to_msessioninfo);//true for session already exists
					old_sid = msessionkeys_itr->second->second.SID;
					sessionId = old_sid;
					return;
				} 
				else {
					////// 		CASE 1.4.2.2 unknown cookie value, unknown ip+fp			/////////
					//make new sid
					make_sid(hashed_sid,real_ip,time_stamp);
					//ip+fp-->new sid					
					new_index_to_msessioninfo = update_msessioninfo_table(new_key,hashed_sid,false,new_index_to_msessioninfo);//false for new session
					update_msessioninfo_table(cookie_value_hashed,empty,true,new_index_to_msessioninfo);//true for session already exists
					//cookie-->new_sid
					sessionId = hashed_sid;
					return;					
				}
		
			}
		}
	}
	else{		
		/////////				CASE 2 Unknown IP     /////////
		//insert the ip to the ip set
		mips.insert(real_ip);
		if (has_cookie)
			SHA1::RunSHA1(cookie_value, cookie_value_hashed);
		if (has_cookie && (msessionkeys_itr = msessionkeys.find(cookie_value_hashed))!=msessionkeys.end()){//unknown ip, found cookie
			////// 		CASE 2.1 Unknown IP has cookie		///////////
			if (has_set_cookie){//set_cookie exists
				////// 		CASE 2.1.1 Unknown IP has cookie has set cookie		///////////
				//look for old sid, the cookie was found so it already has a sid
				old_sid = msessionkeys_itr->second->second.SID;
				//insert that sid into the ip+fp entry
				make_hash(new_key,real_ip,finger_prints);
				//sessionkeys[new_key] = pointer to the existing session 
				update_msessioninfo_table(new_key,empty,true,msessionkeys_itr->second);//true for session already exists
				//insert that sid also into the ip-setcookie entry				
				//insert to set cookie alone
				SHA1::RunSHA1(set_cookie_value, set_cookie_value_hashed);
				update_msessioninfo_table(set_cookie_value_hashed,empty,true,msessionkeys_itr->second);//true for session already exists

				//update 
				sessionId = old_sid;
				return;
			}//end of unknown ip, known cookie and a valid set_cookie
			else{//case of unkown ip known cookie and no set cookie
				////// 		CASE 2.1.2 Unknown IP has cookie has set cookie		///////////
				//make new sid
				old_sid = msessionkeys_itr->second->second.SID;
				//insert the new sid to ip+fp = new sid in the sessions
				make_hash(new_key,real_ip,finger_prints);
				update_msessioninfo_table(new_key,empty,true,msessionkeys_itr->second);//true for session already exists
				//also insert the new sid to the entry of ip+cookie
				sessionId = old_sid;
				return;			
			}//end of unkown ip known cookie and no set cookie
		}
		//end of unknown ip, known cookie
		else{//unknown ip, unknown cookie
			////// 		CASE 2.2 Unknown IP has cookie		///////////					
			if (has_set_cookie){//set_cookie exists
				////// 		CASE 2.2.1 Unknown IP has cookie	has set cookie	///////////					
				SHA1::RunSHA1(set_cookie_value, set_cookie_value_hashed);
				//create a new sid								
				make_sid(hashed_sid,real_ip,time_stamp);
				//insert the sid for the new cookie entry
				new_index_to_msessioninfo = update_msessioninfo_table(set_cookie_value_hashed,hashed_sid,false,new_index_to_msessioninfo);//false = create new session using the hashed sid	
				make_hash(new_key,real_ip,finger_prints);
				update_msessioninfo_table(new_key,empty,true,new_index_to_msessioninfo);//true for session already exists
				//insert the sid in the ip+new cookie entry
				sessionId = hashed_sid;
				return;
			}			
			else{//unknown ip, unknown cookie, no set cookie
				make_sid(hashed_sid,real_ip,time_stamp);				
				make_hash(new_key,real_ip,finger_prints);
				new_index_to_msessioninfo = update_msessioninfo_table(new_key,hashed_sid,false,new_index_to_msessioninfo);//true for session already exists
				update_msessioninfo_table(cookie_value_hashed,empty,true,new_index_to_msessioninfo);//true for session already exists
				sessionId = hashed_sid;
				return;
			}
		}//end of unknown ip and unknown cookie
		
	}
}

//the function checks if sid_already_exists
//if it does:
//  it will add a pointer to that session struct in the sessionkeys table
//	it will use session_struct_index as a pointer to the msessioninfo table and update the list of pointers to it
//if it doesnt:
//	it will create a struct with a new SID and initialize all other fields
//	it will add it to the msessioninfo table and save the iterator
//	it will push the iterator to the sessionkeys table in the "key" index and save that entry as an iter
//	it will add iter to the list of indices pointing to that session struct

index_to_msessioninfo update_msessioninfo_table(string& key,const string& sid,bool sid_already_exists,index_to_msessioninfo session_struct_index){
	//if the session exists, use the session_struct given as param
	//iterator to go over the session keys table
	map<string,index_to_msessioninfo >::iterator itr;
	pair<index_to_msessioninfo,bool> p1;
	pair<index_to_msessionkeys,bool> p2;
	if (sid_already_exists){
		s_sessioninfo & session = session_struct_index->second; 
		//update the msessionkeys table to hold the iterator
		//get the iterator to our sessionkeys entry		
		//add the key index in the sessionkeys to point to that iterator
		if((itr = msessionkeys.find(key))==msessionkeys.end()){
			p2 = msessionkeys.insert(pair<string,index_to_msessioninfo>(key,session_struct_index));
			itr = p2.first;
			session.keys_pointed_by.push_back(itr);
		}
		
		//increase count of requests seen for this session
		session.count++;
		//update last time of request
		session.last_time = time(NULL);
		sessionsinfo_list.erase(session.iter_to_fifo);
		sessionsinfo_list.push_back(session_struct_index);
		session.iter_to_fifo = --sessionsinfo_list.end();

		return session_struct_index;
		//}
	}
	//else, we need to add a new entry to the table
	else{
		s_sessioninfo session;
		session.SID = sid;
		session.count = 0;
		session.last_time = time(NULL);
		p1 = msessionsinfo.insert(pair<string,s_sessioninfo>(sid,session));
		//added successfully, save the iterator to the new struct
		session_struct_index = p1.first;
		//add the key index in the sessionkeys to point to that iterator
		if ((itr=msessionkeys.find(key))==msessionkeys.end()){
			p2 = msessionkeys.insert(pair<string,index_to_msessioninfo>(key,session_struct_index));
			itr = p2.first;
			//add it to the list of keys pointing to that s_sessioninfo
			session_struct_index->second.keys_pointed_by.push_back(itr);
		}

		//update the fifo for the session, push to the back as its the newest one
		if (p1.second)
		{
			sessionsinfo_list.push_back(session_struct_index);
			session_struct_index->second.iter_to_fifo = --sessionsinfo_list.end();
		}
		else{//collision in SHA1, only move to the head of the list but do not insert as new one
			session_struct_index->second.last_time = time(NULL);
			sessionsinfo_list.erase(session_struct_index->second.iter_to_fifo);
			sessionsinfo_list.push_back(session_struct_index);
			session_struct_index->second.iter_to_fifo = --sessionsinfo_list.end();
		}
		return session_struct_index;
	}
	
}
	
static void make_sid(string& output_sid,string& ip,string& timestamp){
	string sid_vals = ip;
	sid_vals.append(timestamp);
	output_sid.clear();
	SHA1::RunSHA1(sid_vals, output_sid);
}

static void make_hash(string & output_hash,string& key1,string& key2){
	string to_hash = key1;
	to_hash.append(key2);
	output_hash.clear();
	SHA1::RunSHA1(to_hash, output_hash);
}


