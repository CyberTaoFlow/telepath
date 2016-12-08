#include <iostream>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <unordered_map> 
#include <queue>
#include <set>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <pthread.h>
#include <semaphore.h>
#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/functional/hash.hpp>
#include <pcre.h>
#include <hiredis/hiredis.h>
#include <msgpack.hpp>
#include <stdio.h>

#include <iomanip> 
#include <openssl/sha.h>

#include "teleindex.h"
#include "jsoncpp/json.h"
#include <syslog.h>
#include <time.h>

#include "ElasticSearch.h"
#include "local_time.h"
#include "Syslog.h"
#include "Tree.h"
#include "LogFile.h"
#include "Url.h"
#include "Numeric.h"
#include "Enumeration.h"
#include "Query.h"
#include "Page.h"
#include "Link.h"
#include "Session.h"
#include "Operation.h"
#include "geoip.h"
#include "Path.h"
#include "Sha1.h"
#include "ScoreData.h"
#include "Action.h"
#include "Rule.h"
#include "Reputation.h"
#include "Attribute.h"
#include "Globals.h"
#include "Request.h"
#include "Serialization.h"
#include "Markov.h"
#include "LoadConf.h"
#include "enginetypes.h"
#include "FunctionAndGlobal.h"
#include "EngineGuard.h"
#include "scheduler.h"
#include "Threads.h"

thread_struct_ tstr[NUM_OF_GET_ATT]  = {{0}};
thread_struct_ tstrP[NUM_OF_GET_ATT] = {{0}};

using namespace std;



//initiating all pthread semaphores.
void initSemaphores(){
        sem_init(&sem_tokenize1,0,0);
	sem_init(&sem_tokenize2,0,0);
	sem_init(&sem_markov_att_start,0,0);
	sem_init(&sem_markov_att_end,0,0);
	sem_init(&sem_markov_session_start,0,0);
	sem_init(&sem_markov_session_end,0,0);
	sem_init(&sem_session_alerts,0,0);
	sem_init(&sem_insert_attributes,0,0);
	sem_init(&sem_insert_reqs,0,0);
	sem_init(&sem_insert_logins,0,0);
	sem_init(&sem_command,0,0);
	sem_init(&sem_captcha,0,0);

	#ifdef LOGGER
		sem_init(&sem_log_iis,0,0);
		sem_init(&sem_log_apache,0,0);
		sem_init(&sem_log_json,0,0);
	#endif
}

//destroying all pthread semaphores.
void semaphores_destruction(){
	sem_destroy(&sem_markov_att_start);
	sem_destroy(&sem_markov_att_end);
	sem_destroy(&sem_markov_session_start);
	sem_destroy(&sem_markov_session_end);

        sem_destroy(&sem_tokenize1);
        sem_destroy(&sem_tokenize2);
	sem_destroy(&sem_log_iis);
	sem_destroy(&sem_log_apache);
	sem_destroy(&sem_log_json);

	sem_destroy(&sem_command);
	sem_destroy(&sem_captcha);
}

//void loadMaxSession(unsigned int val){
//	maxSession = val; 
//	if(maxSession==0){
//		maxSession=350;
//	}
//}

void load_config(){ 

	string output;

	es_get_config("/telepath-config/config/move_to_production_id/_source",output);
	moveToProductionAfter = (unsigned int)atoi(output.c_str());
	//es_get_config("/telepath-config/config/max_session_id/_source",output);
	//loadMaxSession((unsigned int)atoi(output.c_str()));
	es_get_config("/telepath-config/config/max_distance_id/_source",output);
	max_dist = ( (double)atof(output.c_str()) ) / 100;
	es_get_config("/telepath-config/config/noise_percent_id/_source",output);
	noisePercent = ( (float)atof(output.c_str())/100 );
	opClusterPercent = ( (float)atof(output.c_str())/200 );
	operationPercent = ( (float)atof(output.c_str())/1000 );
	es_get_config("/telepath-config/config/landing_speed_id/_source",output);
	slowOrFastLanding = (unsigned short)atoi(output.c_str());
	es_get_config("/telepath-config/config/bot_intelligence_id/_source",output);
	bot_intelligence = (unsigned short)atoi(output.c_str());
	es_get_config("/telepath-config/config/license_key_id/_source",output);
	license_key = output;
	es_get_config("/telepath-config/config/write_to_syslog_id/_source",output);
	write_to_syslog = (unsigned short)atoi(output.c_str());
	es_get_config("/telepath-config/config/enable_weight_id/_source",output);
	enable_weight = (unsigned short)atoi(output.c_str());
	es_get_config("/telepath-config/config/password_masking_id/_source",output);
	password_masking = (unsigned short)atoi(output.c_str());
	es_get_config("/telepath-config/config/cc_masking_id/_source",output);
	cc_masking = (unsigned short)atoi(output.c_str());
}

void checkOperation(){
	syslog (LOG_NOTICE, "Operation Analysis");

	map <unsigned int, Operation >::iterator itOperation;
	map <unsigned long long,map< int,unsigned short> >::iterator itVecShape;
	map <int,unsigned short >::iterator itValues;
	map < int,unsigned int>::iterator itSeen;

	map <unsigned int,myAtt>::iterator itMyAttMap; // iterator of myAttMap.

	for(itMyAttMap = myAttMap.begin(); itMyAttMap!=myAttMap.end();itMyAttMap++){  // itMyAttMap loop - beginning.
		if(itMyAttMap->second.type=='e' && itMyAttMap->second.kind !='H'){
			if(itMyAttMap->second.enumeration.seen.size() <= 20){

				itOperation = mOperation.find(itMyAttMap->second.page_id);
				if(itOperation != mOperation.end() ){ // Page was found. 

					itVecShape = itOperation->second.vecShape.find(itMyAttMap->first);
					if(itVecShape != itOperation->second.vecShape.end()){ // Attribute was found. 

						for(itSeen=itMyAttMap->second.enumeration.seen.begin();itSeen!=itMyAttMap->second.enumeration.seen.end();itSeen++){

							itValues = itVecShape->second.find(itSeen->first);
							if(itValues == itVecShape->second.end() ){ // Value wasn't found.
								itVecShape->second.insert(pair< int,unsigned short>(itSeen->first,itOperation->second.vecSize));
								itOperation->second.vecSize++;
							}

						}

					}else{						 // Attribute wasn't found. 
						
						map<int,unsigned short> mTmp;
						for(itSeen=itMyAttMap->second.enumeration.seen.begin();itSeen!=itMyAttMap->second.enumeration.seen.end();itSeen++){
							mTmp.insert(pair< int,unsigned short>(itSeen->first,itOperation->second.vecSize));
							itOperation->second.vecSize++;
						}
						
						itOperation->second.vecShape.insert(pair<unsigned int,map< int,unsigned short> >(itMyAttMap->first,mTmp));
					}

				}else{					 // Page wasn't found. 
					Operation operation;
					map< int,unsigned short> mTmp;
					for(itSeen=itMyAttMap->second.enumeration.seen.begin();itSeen!=itMyAttMap->second.enumeration.seen.end();itSeen++){
						mTmp.insert(pair<int,unsigned short>(itSeen->first,operation.vecSize));
						operation.vecSize++;
					}

					operation.vecShape.insert(pair<unsigned long long,map< int,unsigned short> >(itMyAttMap->first,mTmp));
					mOperation.insert(pair<unsigned int, Operation>(itMyAttMap->second.page_id,operation));

				}
			}
		}
	}
}

#ifdef REDIS
// This thread pops all the requests from the "Q" redis queue.
void* thread_redis_messages(void* arg)
{
	TeleObject TO;
	TeleCache *pTC=(TeleCache*)arg;
	redisContext *redis;
	redisReply *reply;

	msgpack::object obj;
	std::unordered_map<string,string> mMessage;

	msgpack::sbuffer sbuf;
	msgpack::unpacked msg;


	redis = redisConnect("127.0.0.1", 6379);
	if(redis->err) {
		exit(EXIT_FAILURE);
	}

	while(globalEngine==1){
		reply = (redisReply*)redisCommand(redis, "RPOP %s","Q" );
		if ( reply->type == REDIS_REPLY_ERROR ){
			freeReplyObject(reply);
		}else{
			if(reply->str != NULL){
				getRedis++;
				try{
					msgpack::unpack(&msg, reply->str, reply->len);
					freeReplyObject(reply);

					obj = msg.get();
					obj.convert(&mMessage);
					pTC->addobject(&TO,mMessage);
					TO.clear();
					mMessage.clear();
				}
				catch (msgpack::v1::type_error& bc){syslog(LOG_NOTICE,"Catch: %s",bc.what() );}
				catch (msgpack::v1::insufficient_bytes& bc){syslog(LOG_NOTICE,"Catch: %s",bc.what() );}
			}else{
				usleep(1000);
			}
		}
	}
	sleep(5);
	globalEngine=0;
	sleep(5);

	pthread_exit(NULL);

}
#endif

void findAndDeleteDomain(string del_domain){
	boost::unordered_map <string,AppMode>::iterator itAppMode;
	for(itAppMode = mAppMode.begin() ; itAppMode != mAppMode.end() ; itAppMode++ ){
		string domain = itAppMode->first.c_str();
		size_t found = domain.find(del_domain);
		if(found != string::npos){
			mAppMode.erase(itAppMode);
			syslog(LOG_NOTICE,"Domain: %s - Had Been Deleted From Engine Brain!",domain.c_str());
		}
	}
}

void *thread_Delete_Domain(void *arg){
	
	redisContext *redis;
	redisReply *reply;
	
	redis = redisConnect("127.0.0.1", 6379);
	if(redis->err) {
		exit(EXIT_FAILURE);
	}
	while(globalEngine==1){
		int flag_queue = 1;
		while(flag_queue==1){
			reply = (redisReply*)redisCommand(redis, "RPOP %s","D" );
			if ( reply->type == REDIS_REPLY_ERROR ){
				flag_queue = 0;
				freeReplyObject(reply);
			}else{
				if(reply->str != NULL){
					findAndDeleteDomain((string)reply->str);
				}else{
					flag_queue = 0;
				}
			}
		}
		sleep(60);
	}
	pthread_exit(NULL);
}

bool validMethod(string & method){

	if(methods.count(method) != 0 ){
		return true;
	}else{
		return false;
	}
}

void findAppID(string & appName,string & appid){
	char url[500],buffer[2000];
	unsigned int i_lowcase,colon=0;
	size_t find_pos;
	bool find_flag=false;
	appid.clear();

	boost::unordered_map <string,AppMode>::iterator itAppMode;
	for(i_lowcase=0;i_lowcase<appName.size();i_lowcase++){
		if(appName[i_lowcase]==':' || appName[i_lowcase]=='/'){
			colon = i_lowcase;
			break;
		}
	}

	if(colon > 0 && colon < 200){
		appName.erase(colon);
	}
	pthread_mutex_lock(&mutexAppMode);
	for(itAppMode = mAppMode.begin() ; itAppMode != mAppMode.end() ; itAppMode++ ){
		if(itAppMode->first==appName){
			appid = itAppMode->first;
			find_flag=true;
			break;
		}
		if(itAppMode->second.top_level_domain==0){
			continue;
		}
		find_pos = appName.find( itAppMode->first );
		if(find_pos != string::npos ){
			appid = itAppMode->first;
			find_flag=true;

			if(itAppMode->second.subdomains.size() == 0){
				snprintf(url,sizeof(url)-1,"/telepath-domains/domains/%s/_update",itAppMode->first.c_str());
				snprintf(buffer,sizeof(buffer)-1,"{\"doc\":{\"subdomains\":[\"%s\"]}}",appName.c_str());
				es_mapping(string(url),string(buffer) );
				itAppMode->second.subdomains.insert(appName);
			}
			else if(itAppMode->second.subdomains.size() <= 300){
				if(itAppMode->second.subdomains.count(appName) == 0){ // Attribute name was not found.
					snprintf(url,sizeof(url)-1,"/telepath-domains/domains/%s/_update",itAppMode->first.c_str());
					snprintf(buffer,sizeof(buffer)-1,"{\"script\":\"ctx._source.subdomains+=sub\",\"params\":{\"sub\":\"%s\"}}",appName.c_str());
					es_mapping(string(url),string(buffer) );
					itAppMode->second.subdomains.insert(appName);
				}
			}

			break;
		}
	}
	pthread_mutex_unlock(&mutexAppMode);

	if(find_flag==false){
		pthread_mutex_lock(&mutexUnknownApp);
			unsigned short tmp_stat = addUnknownApp;
		pthread_mutex_unlock(&mutexUnknownApp);
		if( tmp_stat == 1 ){
			if(appName.size() > 100){
				appName.erase(appName.begin()+100,appName.end());
			}

			AppMode am;
			pthread_mutex_lock(&mutexAppMode);
				mAppMode.insert(pair<string,AppMode>(appName,am));
			pthread_mutex_unlock(&mutexAppMode);
			appid = appName;

			snprintf(url,sizeof(url)-1,"/telepath-domains/domains/%s/_create",appName.c_str());
			snprintf(buffer,sizeof(buffer)-1,"{\"host\":\"%s\",\"operation_mode\":\"1\",\"learning_so_far\":1,\"move_to_production\":1000000,\"eta\":\"1d 0h 0m\",\"redirect_mode\":\"0\",\"redirect_page\":\"\",\"body_value_mode\":\"0\",\"basic_mode\":\"0\",\"basic_mode\":\"0\",\"digest_mode\":\"0\",\"ntlm_mode\":\"0\",\"body_value_html\":\"\",\"form_param_name\":\"\",\"cookie_mode\":\"0\",\"cookie_name\":\"\",\"cookie_value\":\"\",\"cookie_value_appearance\":\"0\",\"top_level_domain\":\"0\",\"subdomains\":[]}",appName.c_str() );
			es_insert(url,buffer );
		}
	}
}

/*void load_log(){ 
	MYSQL_RES *res;
	MYSQL_ROW row;

	static const char * query = "SELECT file,type,app_domain FROM logmode_files_que,applications WHERE applications.app_id=logmode_files_que.app_id;";
	while (mysql_query(, query )) {sqlError(,query);}
	if (!(res = mysql_store_result()))
	       exit(4);

	string logfile,type,domain;
	while((row = mysql_fetch_row(res))) {
		logfile = row[0];
		type = row[1];
		domain = row[2];
		LogFile tmplog(logfile,domain);

		if(type.size()==0){
			continue;
		}

		if(type[0]=='a'){ // a=apache
			apache_logs.push_back(tmplog);
		}
		else if(type[0]=='i'){ // i=iis
			iis_logs.push_back(tmplog);
		}
	}
	if(res)
		mysql_free_result(res);//free result memory
}*/

// The function returns the process id of a given program name.
int lget_pid(string program_name){

	string first,second,command="pidof ";
	int pid,pos;

	FILE *fd;
	fd=popen((command+program_name).c_str(),"r");
	if(fd!=NULL){

		char buf[256];
		if ( fgets(buf, sizeof(buf), fd) == NULL){
			//fgets failed.
			syslog(LOG_NOTICE,"ERROR[!!!] failed operation %s",command.c_str());
			exit(1);
		}

		first = buf;

		//Get the other process and delete the exists one from the list.
		if((pos = first.find(" ")) != string::npos)
		{
			second = first;

			first.erase(pos, first.length() - (pos+1));
			second.erase(0, pos+1);

			//checks for the right pid.
			pid = atoi(first.c_str());
			if(pid == getpid())
			{
				pid = atoi(second.c_str());
			}
		}else{  //exist only 1 instance.
			return -1;
		}

		pclose(fd);
		return pid;
	}else{	
		return -1;
	}
}

// The engine will be terminated if there is another instance of it.
void isEngineRunning(){
	#ifdef LOGGER
	if ( lget_pid("engine_logger") != -1){
		syslog(LOG_NOTICE,"The log_engine is already running...");
		exit(0);
	}
	#elif DEBUG
	#elif REDIS
	if ( lget_pid("engine") != -1 ){
		syslog(LOG_NOTICE,"The engine is already running...");
		exit(0);
	}
	#endif
}

// Demonizing process
void demonize(){
	int pid;

	pid=fork();

	switch (pid)
	{
		case 0:
			setsid();
			break;
		case -1:
			cerr<<"ERROR! fork() failed!"<<endl;
			exit(1);
			break;

		default:
			exit(0);
	}
}

// The function disables stderr messages.
void disableErrorMessages(){
	FILE *in;
	if(!(in = freopen("/dev/null", "w", stderr))){}
}

void initSyslog(){
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("Engine", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
}

void startThreads(){

	unsigned long rc,empty=0;
	pthread_t thread_sch,thread_s_p_m,thread_logins,thread_load,thread_delete;
	pthread_t thread_req,thread_tokenize,thread_cmd,thread_cap,thread_app_updated;
	pthread_t thread_m[NUM_OF_MARKOV_THREADS],thread_mS[NUM_OF_MARKOV_THREADS],thread_gA[NUM_OF_GET_ATT],thread_gAP[NUM_OF_GET_ATT];

	#ifdef TRIAL
		pthread_t expiration;
		rc = pthread_create(&expiration, NULL, thread_expiration_date, (void *)empty);
		if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
		pthread_detach(expiration);
	#endif

	rc = pthread_create(&thread_load, NULL,thread_load_config, (void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_load);

	rc = pthread_create(&thread_logins, NULL,thread_insert_logins, (void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_logins);

	rc = pthread_create(&thread_cmd, NULL,thread_command, (void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_cmd);

	rc = pthread_create(&thread_cap, NULL,thread_captcha, (void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_cap);

	rc = pthread_create(&thread_s_p_m, NULL, thread_sid_per_min, (void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_s_p_m);

        rc = pthread_create(&thread_req, NULL, thread_insert_req, (void *)empty);
        if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_req);

        rc = pthread_create(&thread_tokenize, NULL, tokenize_thread, (void *)empty);
        if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_tokenize);

	rc = pthread_create(&thread_sch, NULL, thread_scheduler, (void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_sch);

	rc = pthread_create(&thread_app_updated, NULL, thread_app_was_updated, (void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_sch);

	rc = pthread_create(&thread_delete, NULL, thread_Delete_Domain, (void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
	pthread_detach(thread_sch);


	// Create NUM_OF_MARKOV_THREADS threads which calculate the attribute scores.
	for(markov_i=0 ; markov_i < NUM_OF_MARKOV_THREADS;markov_i++){
		rc = pthread_create(&thread_m[markov_i], NULL, markovAtt_thread , (void *)empty);
		if (rc){printf("ERROR; return code from pthread_create() is %lu\n", rc);	exit(1);}
		pthread_detach(thread_m[markov_i]);
	}

	// Create NUM_OF_MARKOV_THREADS threads which calculate the request scores.
	for(markov_i=0 ; markov_i < NUM_OF_MARKOV_THREADS ; markov_i++){
		rc = pthread_create(&thread_mS[markov_i], NULL, markovSession_thread , (void *)empty);
		if (rc){printf("ERROR; return code from pthread_create() is %lu\n", rc);	exit(1);}
		pthread_detach(thread_mS[markov_i]);
	}

	// Create NUM_OF_GET_ATT threads which handle the incoming attributes.

	unsigned int gA_i;
	for(gA_i=0 ; gA_i < NUM_OF_GET_ATT ; gA_i++){
		tstr[gA_i].rids = NULL;
		tstr[gA_i].index = gA_i;
		rc = pthread_create( &thread_gA[gA_i], NULL, getAtt_thread, &(tstr[gA_i]) );
		if (rc){syslog(LOG_NOTICE,"ERROR; return code from pthread_create() is %lu    index:%u \n", rc,gA_i );	exit(1);}
		pthread_detach(thread_gA[gA_i]);
	}

	for(gA_i=0 ; gA_i < NUM_OF_GET_ATT ; gA_i++){
		tstrP[gA_i].rids = NULL;
		tstrP[gA_i].index = gA_i;
		rc = pthread_create( &thread_gAP[gA_i], NULL, getAtt_thread_pro , &(tstrP[gA_i]) );
		if (rc){syslog(LOG_NOTICE,"ERROR; return code from pthread_create() is %lu    index:%u \n", rc,gA_i );	exit(1);}
		pthread_detach(thread_gAP[gA_i]);
	}

	//---------------------------------------logger----------------------------------------
	#ifdef LOGGER
	//	syslog(LOG_NOTICE,"Logger");
	//	load_log();
	//	pthread_t iis_log_thread; 
	//	pthread_t apache_log_thread; 

	//	pthread_create(&iis_log_thread,NULL,thread_iis_log,(void*)TC);
	//	pthread_detach(iis_log_thread);
	//	pthread_create(&apache_log_thread,NULL,thread_apache_log,(void*)TC);
	//	pthread_detach(apache_log_thread);

	//-------------------------------------------------------------------------------------
	//----------------------------------------pipe-----------------------------------------
	#elif REDIS
		syslog(LOG_NOTICE,"REDIS");

		pthread_t redis_messages_thread; 
		rc = pthread_create(&redis_messages_thread,NULL,thread_redis_messages,(void*)TC);
		if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); exit(1);}
		pthread_detach(redis_messages_thread);
	#endif
	//-------------------------------------------------------------------------------------

}

void showSvnVersion(char * mode){
	if( strcmp("-v",mode) == 0 ){
		cout <<"Engine Version:  "<< GIT_REV;
		print_time(COMPILATION_TIME);
		cout <<"Serial Number:   "<< COUNT_REV << endl;
		exit(1);
	}
}

void createLicenseKey(int argc,char* array[]){
	if(argc > 1){
		if( strcmp("-c",array[1]) == 0 ){
			es_get_config("/telepath-config/config/license_key_id/_source",license_key);

			unsigned int epoch,check_time = (unsigned int)time(NULL);

			if( validKey(license_key,epoch) == true ){
				if(check_time > 2678400 + epoch){
					es_insert("/telepath-config/config/license_mode_id","{\"value\":\"EXPIRED\"}");
				}else{
					es_insert("/telepath-config/config/license_mode_id","{\"value\":\"VALID\"}");
				}
			}else{
				es_insert("/telepath-config/config/license_mode_id","{\"value\":\"INVALID\"}");
			}

			exit(1);
		}
	}
}

// Session initialization. 
// Opening a new session or classifing an existing session to an old one.
void insertOrUpdateSessionMap(TeleObject & teleobj){
	unsigned int sid = (unsigned int)atoi(teleobj.mParams['e'/*SID*/].c_str());

	boost::unordered_map <unsigned int,Session> ::iterator itSession = mSession.find(sid);
	if (itSession != mSession.end()) {
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK LOGOUT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//if(itSession->second.logout == true){ 
			//itSession->second.reset();
			//teleobj.mParams['s'/*ReqSeq*/] = "0";
			//itSession->second.sid += itSession->second.logout_counter;
		//}else{
			itSession->second.sequence++;
			teleobj.mParams['s'/*ReqSeq*/] = int_to_string(itSession->second.sequence);
			if(itSession->second.validUser == false){
				if(teleobj.mParams['A'/*Username*/].size() > 0){
					itSession->second.sUsername = teleobj.mParams['A'/*Username*/];
				}
			}

			teleobj.mParams['Y'/*ComapredPage*/] = teleobj.mParams['s'/*ReqSeq*/] + "+" + teleobj.mParams['k'/*PageID*/];
			double tmp_speed = (double)atof(teleobj.mParams['b'/*TimeStamp*/].c_str() ) ;
			itSession->second.last_speed = tmp_speed - itSession->second.last_speed;
			teleobj.mParams['W'/*DiffSpeed*/]=int_to_string(itSession->second.last_speed);
			itSession->second.last_speed = tmp_speed;
			itSession->second.compared_link.push_back('|');
			itSession->second.compared_link.append(teleobj.mParams['Y'/*ComapredPage*/]);
			teleobj.mParams['Z'/*ComapredLink*/] = itSession->second.compared_link;
			itSession->second.compared_link = teleobj.mParams['Y'/*ComapredPage*/];
			if(itSession->second.sequence >= 10000){
                        	itSession->second.reset();
			}

		//}
	}else{
		teleobj.mParams['s'/*ReqSeq*/] = "0";
		Session session;//
		session.sid = sid;
		if(session.validUser == false){
				if(teleobj.mParams['A'/*Username*/].size() > 0){
					session.sUsername = teleobj.mParams['A'/*Username*/];
				}
		}

		session.last_speed=(double)atof(teleobj.mParams['b'/*TimeStamp*/].c_str() ) ;
		teleobj.mParams['W'/*DiffSpeed*/]="0";
		session.compared_link = "0+" + teleobj.mParams['k'/*PageID*/];
		teleobj.mParams['Y'/*ComparePage*/]=session.compared_link;
		teleobj.mParams['Z'/*ComapredLink*/] = "0";
		mSession.insert( pair<unsigned int,Session>(session.sid,session) );
	}

}

void clearMyAttIrrelevantParams(){
	map <unsigned int,myAtt> ::iterator itMyAttMap;
	for(itMyAttMap = myAttMap.begin(); itMyAttMap!=myAttMap.end();itMyAttMap++){
		vector <int> ().swap(itMyAttMap->second.exp_score); // clear old exponent scores.
		vector <short> ().swap(itMyAttMap->second.exp_length); // clear old exponent length scores.
		vector < vector <unsigned int> > ().swap(itMyAttMap->second.values);
		vector < int > ().swap(itMyAttMap->second.hash_values);
		vector <long long> ().swap(itMyAttMap->second.RIDs);
		vector < size_and_flag > ().swap(itMyAttMap->second.val_s_f);
		vector <unsigned short> ().swap(itMyAttMap->second.sizeMarkovVec);
	}
}

int main (int argc,char* array[])
{

if(argc > 1){
	showSvnVersion(array[1]);
}
createLicenseKey(argc,array);
isEngineRunning();
demonize();
disableErrorMessages();
initRegex();			// This function is defined at enginetypes.h .
initAttType();			// This function is defined at enginetypes.h .
initCookieBlackList();		// This function is defined at enginetypes.h .
initSyslog();
open_geoIP_database();		// This function is defined at geoip.h .

read_connect_conf_file();	// This function is defined at Elasticsearch.h .
TC = new TeleCache;

load_config();
rules_table_was_changed=1;
business_flow_was_changed=1;
bot_intelligence=1;

sleep(3);
loadActions();			/*This function is defined at LoadConfig.h .*/ syslog(LOG_NOTICE,"%u Actions Were Loaded",(unsigned)businessFlowVec.size());
loadRules();			/*This function is defined at LoadConfig.h .*/ syslog(LOG_NOTICE,"%u Rules Were Loaded",(unsigned)rules.size());
loadBotIntelligence();		/*This function is defined at LoadConfig.h .*/ 
	syslog(LOG_NOTICE,"%u Tor IPs Were Loaded",(unsigned)sTorIntelligenceIP.size());
	syslog(LOG_NOTICE,"%u Bot IPs Were Loaded",(unsigned)sBotIntelligenceIP.size());

initMonths();			// This function is defined at enginetypes.h .
initMethod();			// This function is defined at enginetypes.h .
loadApps();

initSemaphores();
startThreads();

map <unsigned int,myAtt> ::iterator itMyAttMap; // iterator of myAttMap.
boost::unordered_map <unsigned int,Session> ::iterator itSession;// iterator of mSession
map<string,Path> ::iterator itPath;// iterator of mPath.
map<string,Path> ::iterator itPathUser;// iterator of mPathUser.
map <unsigned int,myAtt>::iterator itdecideType;
boost::unordered_map <string,AppMode>::iterator itAppMode;

sleep(1);

unsigned int sizeVal,i_flags,cycleLearning=0,gA_i;
short exp_length;
string str_num;
double att_score;
int att_exp;
char url_mode[500],postfields_mode[1000];
try{
	loader();
}catch(...){syslog(LOG_NOTICE,"Error!!! While trying to load old engine brain");}

#ifdef DEBUG
	syslog(LOG_NOTICE,"Debug Mode");
	unsigned int graph_counter=1;
#endif

while(globalEngine){ // while engine learning - run the engine every time we get data from the pipe.
	usleep(5);

	//Loading Rules.
	loadRules();
	//Loading Actions.
	loadActions();

	//TODO - Check a Redis queue to determine if the configuration was changed.
	if(cycleLearning % 5 == 0){
		//------------Load config----------- 
		load_config();
	}
	cycleLearning++;
		
	//Building TeleObject Vectors.
	vector<TeleObject> learning_vec;
	vector<TeleObject> production_vec;

	while( !TC->teleObjQueue.empty() ){
		if(globalEngine==0){
			break;
		}

		//Poping TeleObjects.
		pthread_mutex_lock(&mutexTeleObjQueue);
		TeleObject teleobj = TC->teleObjQueue.front() ;
		TC->teleObjQueue.pop();
		pthread_mutex_unlock(&mutexTeleObjQueue);

		
		//Inserting TeleObjects.
		insertOrUpdateSessionMap(teleobj);

		pthread_mutex_lock(&mutexAppMode);
		itAppMode = mAppMode.find( teleobj.mParams['g'/*AppID*/] );
		if( itAppMode != mAppMode.end() ){
			itAppMode->second.counter++;
			if(itAppMode->second.mode == 1){
				if(itAppMode->second.counter > itAppMode->second.move_to_production ){
					itAppMode->second.mode=2;

					// Normalizing scores for each aspect -> landing,flow and geo.
					score_numeric[teleobj.mParams['g'/*AppID*/]].insert(score_data[teleobj.mParams['g'/*AppID*/]]);
					// Cleaning unnormalized data.
					score_data[teleobj.mParams['g'/*AppID*/]].clean();

					#ifdef DEBUG
						score_numeric[teleobj.mParams['g'/*AppID*/]].print_syslog();
					#endif

					production_vec.push_back(teleobj);
					snprintf(url_mode,sizeof(url_mode)-1,"/telepath-domains/domains/%s/_update",teleobj.mParams['g'/*AppID*/].c_str());
					sprintf(postfields_mode,"{\"doc\":{\"operation_mode\":\"2\",\"learning_so_far\":%u,\"eta\":\"0d 0h 0m\"}}",itAppMode->second.counter);
					es_mapping(url_mode,postfields_mode);
					#ifdef DEBUG
						syslog(LOG_NOTICE,"Production mode : %s   counter=%u",teleobj.mParams['g'/*AppID*/].c_str(),itAppMode->second.counter);
					#endif
				}else{
					learning_vec.push_back(teleobj);

					if( (itAppMode->second.counter % 1000) == 0 ){
						unsigned int mins,hours,days;
						char eta[150];
						unsigned int current_time = (unsigned int)time(NULL);
						unsigned int diff_time = current_time - itAppMode->second.timer;
						unsigned int diff_reqs = ( itAppMode->second.move_to_production - itAppMode->second.counter)/1000;
						diff_time *= diff_reqs;

						if(diff_time==0){
							sprintf(eta ,"0d 0h 1m");
						}else{
							mins = diff_time / 60;
							hours = mins / 60;
							mins = mins % 60;
							days = hours / 24;
							hours = hours % 24;
							sprintf(eta ,"%ud %uh %um",days,hours,mins );
						}

						snprintf(url_mode,sizeof(url_mode)-1,"/telepath-domains/domains/%s/_update",teleobj.mParams['g'/*AppID*/].c_str());
						sprintf(postfields_mode,"{\"doc\":{\"learning_so_far\":%u,\"eta\":\"%s\"}}",itAppMode->second.counter,eta);
						es_mapping(url_mode,postfields_mode);
						#ifdef DEBUG
							syslog(LOG_NOTICE,"Learning mode : %s   counter=%u  eta=%s",teleobj.mParams['g'/*AppID*/].c_str(),itAppMode->second.counter,eta);
						#endif

						itAppMode->second.timer = current_time;
					}
					else if(itAppMode->second.counter<=5){
						snprintf(url_mode,sizeof(url_mode)-1,"/telepath-domains/domains/%s/_update",teleobj.mParams['g'/*AppID*/].c_str());
						sprintf(postfields_mode,"{\"doc\":{\"learning_so_far\":%u}}",itAppMode->second.counter);
						es_mapping(url_mode,postfields_mode);
					}

				}
			}else{

				if( (itAppMode->second.counter % 1000) == 0 ){
					snprintf(url_mode,sizeof(url_mode)-1,"/telepath-domains/domains/%s/_update",teleobj.mParams['g'/*AppID*/].c_str());
					sprintf(postfields_mode,"{\"doc\":{\"learning_so_far\":%u}}",itAppMode->second.counter);
					es_mapping(url_mode,postfields_mode);
					#ifdef DEBUG
						syslog(LOG_NOTICE,"Production mode : %s   counter=%u",teleobj.mParams['g'/*AppID*/].c_str(),itAppMode->second.counter);
					#endif
				}
				production_vec.push_back(teleobj);
			}
		}else{
			//AppMode am;
			//mAppMode.insert(pair<string,AppMode>(teleobj.mParams['g'/*AppID*/],am));
			//learning_vec.push_back(teleobj);
		}
		pthread_mutex_unlock(&mutexAppMode);

		if(learning_vec.size() > 5000 || production_vec.size() > 5000){
			break;
		}

	}

	learning_values_flag.assign(learning_vec.size(),0);
	production_values_flag.assign(production_vec.size(),0);

	// Create NUM_OF_GET_ATT threads which handle the incoming attributes.
	for(gA_i=0 ; gA_i < NUM_OF_GET_ATT ; gA_i++){
		tstr[gA_i].rids = &learning_vec;
		tstrP[gA_i].rids = &production_vec;

		tstr[gA_i].index = gA_i;
		tstrP[gA_i].index = gA_i;
	}

	pthread_mutex_lock(&mutexgetatt);
	pthread_cond_broadcast(&getatt_cond);
	pthread_mutex_unlock(&mutexgetatt);

	pthread_mutex_lock(&mutexgetattPro);
	pthread_cond_broadcast(&getatt_condPro);
	pthread_mutex_unlock(&mutexgetattPro);

	usleep(10);
	for (i_flags=0; i_flags <learning_vec.size(); i_flags++)
	{
		while (learning_values_flag[i_flags] != 2)
		{
			usleep(10);
		}
	}

	for (i_flags=0; i_flags <production_vec.size(); i_flags++)
	{
		while (production_values_flag[i_flags] != 2)
		{
			usleep(10);
		}
	}


	pthread_mutex_lock(&mutexgetatt);
	for(gA_i=0 ; gA_i < NUM_OF_GET_ATT ; gA_i++){
		tstr[gA_i].rids = NULL;
		tstrP[gA_i].rids = NULL;
	}
	pthread_mutex_unlock(&mutexgetatt);
	vector <TeleObject> ().swap(learning_vec);
	vector <TeleObject> ().swap(production_vec);

	// Attributes - Types decision.
	for(itdecideType=myAttMap.begin(); itdecideType !=myAttMap.end() ; itdecideType++ ){
		if(itdecideType->second.ifUpdate=='y'){
			decideType(itdecideType->second,0,1);
		}
	}

	//Attributes Calculation.
	unsigned int j;
	Numeric numeric_markov;

	for(itMyAttMap = myAttMap.begin(); itMyAttMap!=myAttMap.end();itMyAttMap++){  // itMyAttMap loop - beginning.
		sizeVal = itMyAttMap->second.values.size();
		if(sizeVal == 0){
			continue;
		}

		itMyAttMap->second.exp_score.clear();
		itMyAttMap->second.exp_length.clear();

		switch (itMyAttMap->second.type){
			case 't':					//t=text analyzing by Markov. F=Free text
			case 'F':
				for(j=0;j < sizeVal  ; j++ ){
					if(itMyAttMap->second.val_s_f[j].tokenOrNot=='n' ){
						itMyAttMap->second.val_s_f[j].tokenOrNot='y';

						pthread_mutex_lock(&mutexMSatt);
						if(MSatt.count(itMyAttMap->second.RIDs[j]) != 0){
							pthread_mutex_unlock(&mutexMSatt);
							continue;
						}
						pthread_mutex_unlock(&mutexMSatt);

						itMyAttMap->second.tree.tokenize(itMyAttMap->second.values[j]);
						if(itMyAttMap->second.val_s_f[j].flag == 'y'){
							itMyAttMap->second.sizeMarkovVec.push_back( itMyAttMap->second.val_s_f[j].size );
						}
					}
				}

				numeric_markov.init(itMyAttMap->second.sizeMarkovVec);// Numeric of length.
				markovAtt_values_flag.assign(sizeVal,0);          // flages assignment.
				itMyAttMap->second.exp_score.assign(sizeVal,0); // probability assignment.
				itMyAttMap->second.exp_length.assign(sizeVal,0);    // size exponent assignment.
				markov_Att = &(itMyAttMap->second);
				markov_Length = &(numeric_markov);

				// Activate NUM_OF_MARKOV_THREADS threads which calculate the attribute scores.
				for(markov_i=0 ; markov_i < NUM_OF_MARKOV_THREADS;markov_i++){
					sem_post(&sem_markov_att_start);
				}

				// Wait until the all threads(markovAtt_thread) stop.
				for(markov_i=0 ; markov_i < NUM_OF_MARKOV_THREADS;markov_i++){
					sem_wait(&sem_markov_att_end);
				}

				markovAtt_values_flag.clear();
				break;
			case 'n':					//n=numeric analyzing by Chebyshev.
				for(j=0; j< sizeVal ; j++ ){
					str_num.clear();
					if( isNum( itMyAttMap->second.values[j],str_num ) == (-1) ){
						att_exp=MIN_PROB;
					}else{	
						att_score = itMyAttMap->second.numeric.chebyshev( atof(str_num.c_str() ) );
						frexp(att_score,&att_exp);
						if(att_exp>0){att_exp=0;}
					}
					itMyAttMap->second.exp_score.push_back(att_exp);
				}
				break;
			case 'e':			 		//e=enumeration analyzing by Covariance and Chebyshev.
			case 'u':
				for(j=0; j < sizeVal ; j++){
					att_exp = itMyAttMap->second.enumeration.getProb(itMyAttMap->second.hash_values[j]);						
					itMyAttMap->second.exp_score.push_back(att_exp);
				}
				break;
			//case 'U':					//U=URL analyzing by binaric search.
				//for(j=0; j < sizeVal ; j++){
				//	att_exp = itMyAttMap->second.url.getProb( itMyAttMap->second.values[j] );
				//	itMyAttMap->second.exp_score.push_back(att_exp);	
				//}
				//break;
		}

		/*if(operationMode!=1){
			itMyAttMap->second.sizeMarkov.init(itMyAttMap->second.sizeMarkovVec) ;
			vector <unsigned short> ().swap(itMyAttMap->second.sizeMarkovVec); 
		}*/

		for(j=0;j<sizeVal;j++){
			if(itMyAttMap->second.exp_length.size() != sizeVal){
				exp_length =1;
			}else{
				exp_length = itMyAttMap->second.exp_length[j];
			}

			if(itMyAttMap->second.exp_score[j]!=MIN_PROB){
				mScoreAtt[itMyAttMap->first].insert((double)itMyAttMap->second.exp_score[j] , (double)exp_length );
			}

			itMyAttMap->second.sumOfScores += itMyAttMap->second.exp_score[j];
			itMyAttMap->second.countOfScores++;
		}
		
		#ifdef DEBUG
			if(graph_counter % 500 == 0){
				// Calculate normal score for each attribute -> value and length.
				mScoreNumericAtt[itMyAttMap->first].insert(mScoreAtt[itMyAttMap->first]);
				//clean each object
				mScoreNumericAtt[itMyAttMap->first].print_elastic(itMyAttMap->second.name);
				mScoreNumericAtt.clear();
				
			}
		#endif

	}	// itMyAttMap loop - ending.

	#ifdef DEBUG
		graph_counter++;
	#endif

	clearMyAttIrrelevantParams();
	sem_post(&sem_tokenize1);

	//***********************************************************************END Attributes******************************************************************************

	itSession = mSession.begin();
	while( itSession!=mSession.end() ){
		if(itSession->second.update == 0){ // if session wasn't update then go to the next. 
			itSession++;
			continue;
		}

		if(itSession->second.vRequest.size() == 0){ // if session was erased earlier then go to the next. 
			itSession++;
			continue;
		}

		//--------------------Build path to each hostname and users------------------------

		if(itSession->second.user_flag==0){
			itPath=mPath.find(itSession->second.vRequest[0].domain_id);
			if(itPath!=mPath.end()){//Hostname was found
				itPath->second.tokenize(itSession->second,0);
			}else{//Hostname wasn't found
				Path path(itSession->second.vRequest[0].domain_id);
				path.tokenize(itSession->second,0);
				mPath.insert( pair<string,Path>(itSession->second.vRequest[0].domain_id,path) );
			}
		}else{
			string hostUser=itSession->second.vRequest[0].domain_id;
			hostUser.push_back('+');
			hostUser.append(itSession->second.vRequest[0].user);
			itPathUser=mPathUser.find(hostUser);
			if(itPathUser!=mPathUser.end()){//Hostname was found
				itPathUser->second.tokenize(itSession->second,1);
			}else{//Hostname wasn't found
				Path path(itSession->second.vRequest[0].domain_id);
				path.tokenize(itSession->second,1);
				mPathUser.insert( pair<string,Path>(hostUser,path) );
			}
		}

		itSession->second.update=0; // after checking.
		itSession++;
	}

	usleep(10);

	for(itSession = mSession.begin(); itSession!=mSession.end();itSession++){
		if(itSession->second.vRequest.size() == 0){
			continue;
		}

		if(itSession->second.user_flag==0){
			itPath=mPath.find(itSession->second.vRequest[0].domain_id);
			if(itPath!=mPath.end()){//Hostname was found
				markovSession_path.push_back( &(itPath->second) );
				markovSession_.push_back( &(itSession->second) );
				markovSession_values_flag.push_back('n');
				markovSession_mode.push_back(mAppMode[itPath->first].mode);
				if(itSession->second.vRequest[0].user.size()>0){
					string hostUser=itSession->second.vRequest[0].domain_id;
					hostUser.push_back('+');
					hostUser.append(itSession->second.vRequest[0].user);
					itPathUser=mPathUser.find(hostUser);
					if(itPathUser!=mPathUser.end()){//Hostname was found
						markovSession_path_user.push_back( &(itPathUser->second) );
					}else{
						markovSession_path_user.push_back( &(itPath->second) );
					}
				}else{
					markovSession_path_user.push_back( &(itPath->second) );
				}
			}else{ 		//Hostname wasn't found
				#ifdef DEBUG
					syslog(LOG_NOTICE,"Unknown hostname");
				#endif
			}
		}
	}

	for(markov_i=0 ; markov_i < NUM_OF_MARKOV_THREADS ; markov_i++){
		sem_post(&sem_markov_session_start);
	}

	for(markov_i=0 ; markov_i < NUM_OF_MARKOV_THREADS ; markov_i++){
		sem_wait(&sem_markov_session_end);
	}

	//TODO saving according to redis
	if(cycleLearning > 500){
		/*if(RIDs_counter > (moveToProductionAfter * operationPercent) ){
			checkOperation();
		}*/

		cycleLearning = 0; 
		saver();
	}

	vector <char> ().swap(markovAtt_values_flag);
	vector <Path*> ().swap(markovSession_path);
	vector <Path*> ().swap(markovSession_path_user);
	vector <Session*> ().swap(markovSession_);
	vector <char> ().swap(markovSession_values_flag);
	vector <short> ().swap(markovSession_mode);

	pthread_mutex_lock(&mutexMSatt);
	MSatt.clear();
	pthread_mutex_unlock(&mutexMSatt);

	//===================LEARNING & HYBRID=============================

	map <long long, OpRID >::iterator itOpRIDs2;
	map <unsigned int, Operation >::iterator itOperation3;
	for(itOpRIDs2 = mOpRIDs.begin() ; itOpRIDs2 != mOpRIDs.end() ; itOpRIDs2++){
		itOperation3 = mOperation.find(itOpRIDs2->second.page_id);
		if(itOperation3 != mOperation.end() ){
			if(itOpRIDs2->second.opts.size() < 20){
				Coordinate2 c2(itOpRIDs2->second.opts);
				itOperation3->second.dbscan.insert(c2);
			}
		}

	}
	//=================================================================
	mOpRIDs.clear();


	/*if(RIDs_counter > (moveToProductionAfter * opClusterPercent) ){
		for( itOperation3 = mOperation.begin();itOperation3 != mOperation.end();itOperation3++){
			itOperation3->second.dbscan.init(0.3,10);
			itOperation3->second.dbscan.buildCentroids();
		}
		opClusterPercent = 1;
	}*/

//*****************************************************************************END SIDs******************************************************************************************
	unsigned int counter=0;

	while( TC->teleObjQueue.size()==0 ){
		if(globalEngine==0){
			break;
		}
		usleep(200000); // while the engine don't get new Object from pipe it waits. 
	}

	sem_wait(&sem_tokenize2);


}// end while engine learning

globalEngine=0;
pthread_mutex_lock(&mutexgetatt);
pthread_cond_broadcast(&getatt_cond);
pthread_mutex_unlock(&mutexgetatt);

pthread_mutex_lock(&mutexgetattPro);
pthread_cond_broadcast(&getatt_condPro);
pthread_mutex_unlock(&mutexgetattPro);


semaphores_destruction();

delete TC;
TC = NULL;

cout <<"[!!!]TURNING OFF ENGINE"<<endl;
syslog (LOG_NOTICE, "[!!!]TURNING OFF ENGINE" );

//Free all regex rules.
for(unsigned int i_rules=0;i_rules < rules.size();i_rules++){
	if ( rules[i_rules].final_type == 'g' ){ // g = regex match.
		 pcre_free(rules[i_rules].pcreRegex);
	}
}
sleep(5);

delete_geoIP_obj();

global_header.clear();
domain_header.clear();
const_method.clear();

mPath.clear();
mPathUser.clear();
myAttMap.clear();
mScoreAtt.clear();

#ifdef LOGGER
#elif REDIS
#endif

return 0;

}// end main.


