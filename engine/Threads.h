
using namespace std;

struct thread_struct_{  // values of attributes score.
	vector<TeleObject>* rids;
	unsigned short index;
};

void process_mem_usage(double& vm_usage, double& resident_set)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}


string sha256(const string str)
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, str.c_str(), str.size());
	SHA256_Final(hash, &sha256);
	stringstream ss;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		ss << hex << setw(2) << setfill('0') << (int)hash[i];
	}
	return ss.str();
}

char parseCC(string & str,string & new_val) { 
	unsigned int i;
	for(i=0;i<str.size();i++){
		if( (str[i] >= 43 && str[i] <= 47) || (str[i] == ' ') ){ /*'+' ',' '-' '.' '/'*/
		}
		else if(str[i]>57 || str[i] < 48){ // Numbers.
			return 'n';
		}else{
			new_val.push_back(str[i]);
		}
	}
	return 'y';
}

void maskPassword(unsigned int hash,string & value){
	if(setHashMasks.count(hash) != 0){
		string tmp = value + "cd#f3r@Dd24eR$~!"; // Random Hash.
		value = sha256(tmp);
	}
}

void maskCreditCard(string & value){
	int rc,offsets[30];
	if(cc_masking==1){
		string new_value;
		if( parseCC(value,new_value) == 'y' ){
			rc = pcre_exec(cc_regex, NULL, (char*)new_value.c_str(),new_value.size(), 0, 0, offsets,30);
			if (rc < 0) {
			}else{
				new_value = value + "cd#f3r@Dd24eR$~!"; // Random Hash.
				value = sha256(new_value);
			}
		}
	}
}

void initParams(TeleObject *to,string & src_IP,unsigned int & PageID,string & DomainID,long long & RID,unsigned int & SID,unsigned short & c_StatusCode,double & c_TimeStamp,string & c_SetCookie,unsigned int & c_Seq,string & c_Title,string & c_Uri,unsigned int & c_Mode,string & resp_IP){
	src_IP = to->mParams['a'/*UserIP*/];
	resp_IP = to->mParams['u'/*RespIP*/];
	PageID = (unsigned int)atoi( to->mParams['k'/*PageID*/].c_str() );
	DomainID = to->mParams['g'/*AppID*/];
	RID = atoll( to->mParams['j'/*RID*/].c_str() );
	SID = (unsigned int)atoi(to->mParams['e'/*SID*/].c_str());
	c_StatusCode = (unsigned short)atoi(to->mParams['d'/*StatusCode*/].c_str() );

	c_TimeStamp = (double)atof(to->mParams['b'/*TimeStamp*/].c_str() );
	c_SetCookie = to->mParams['p'/*SetCookie*/];
	c_Seq = (unsigned int)atoi(to->mParams['s'/*ReqSeq*/].c_str() );
	c_Title = to->mParams['m'/*Title*/];
	c_Uri = to->mParams['c'/*Page*/];

	c_Mode = mAppMode[to->mParams['g'/*AppID*/]].mode;
}

void pageRules(Rule & rule,Session & session,string & c_IP,long long & c_RID,unsigned int c_PageID,string & c_Title,string & c_Uri,string & resp_ip,string & cookie,string & hostname,unsigned short & StatusCode){

	/*if ( rule.final_type == 'B' ){ // B = Bot Intelligence.
		if(itAttr->first==5){  // user-agent.
			itBotIntelligence = mBotIntelligenceUA.find(itAttr->second.value);
			if( itBotIntelligence != mBotIntelligenceUA.end() ){ //User-Agent was found
				if ( rule.str_match[0] == itBotIntelligence->second.name[0] ){
					pthread_mutex_lock(&mutexgetatt5);
						insert_alert(rule,c_RID,c_IP);
					pthread_mutex_unlock(&mutexgetatt5);
				}
			}
		}
	}*/
	string tmp_c_Uri= c_Uri;
	transform(tmp_c_Uri.begin(), tmp_c_Uri.end(), tmp_c_Uri.begin(),::tolower);
	if(rule.method[0] == 'U'){ // U=URI.
		if(rule.final_type == 's'){ // s=stringmatch
			if ( tmp_c_Uri.find(rule.str_match) != string::npos ){
                                if(rule.negate == false){
                                        pthread_mutex_lock(&mutexgetatt5);
                                        insert_alert(rule,c_RID,c_IP,resp_ip,cookie,hostname);
                                        pthread_mutex_unlock(&mutexgetatt5);
                                }
                        }else{
                                if(rule.negate == true){
                                        pthread_mutex_lock(&mutexgetatt5);
                                        insert_alert(rule,c_RID,c_IP,resp_ip,cookie,hostname);
                                        pthread_mutex_unlock(&mutexgetatt5);
                                }

                        }	
			
		}
		else if(rule.final_type == 'g'){// g=regex
			bool null_alert;
			pthread_mutex_lock(&mutexgetatt5);
			regexRules2(rule,c_RID,c_Uri,session,null_alert,c_IP,resp_ip,cookie,hostname);
			pthread_mutex_unlock(&mutexgetatt5);
		}
	}
	else if(rule.method[0] == 'T'){ // Title.
		if (c_Title.size() > 0){
			if(rule.final_type == 's'){// s=stringmatch
				if ( c_Title.find(rule.str_match) != string::npos ){
	                                if(rule.negate == false){
        	                                pthread_mutex_lock(&mutexgetatt5);
                	                        insert_alert(rule,c_RID,c_IP,resp_ip,cookie,hostname);
                        	                pthread_mutex_unlock(&mutexgetatt5);
                        	        }
                        	}else{
                        	        if(rule.negate == true){
                                	        pthread_mutex_lock(&mutexgetatt5);
                                        	insert_alert(rule,c_RID,c_IP,resp_ip,cookie,hostname);
						pthread_mutex_unlock(&mutexgetatt5);
                                	}
				}
			}
			else if(rule.final_type == 'g'){ // g=regex
				bool null_alert;
				pthread_mutex_lock(&mutexgetatt5);
				regexRules2(rule,c_RID,c_Title,session,null_alert,c_IP,resp_ip,cookie,hostname);
				pthread_mutex_unlock(&mutexgetatt5);
			}
		}
	}
	else if(rule.method[0] == 'S'){ // StatusCode.
		if(StatusCode>=rule.str_length_low && rule.str_length_high >= StatusCode){
			insert_alert(rule,c_RID,c_IP,resp_ip,cookie,hostname);
		}
	}

}

void attributeRules(bool learning_mode,Attribute & tmp_att,boost::unordered_map <unsigned int,Session> ::iterator & itSession,TeleObject *to,vector <unsigned int> & vector_value,string & tmp,char nf,char nfu,map <unsigned int,ScoreNumericAtt>::iterator & itN,map <string,ScoreNumericAtt>::iterator & itNU,int exp_score,double size_of_markov,string & c_IP,string & c_UserID,long long & c_RID,unsigned int & c_SID,string & c_Title,double & alert_score,string & resp_ip,string & cookie,string & hostname,string lower_att_val,unsigned short c_StatusCode){
	unsigned int ruleSize = rules.size();
	int hash_value;
	double s_of_m,num_score;
	boost::unordered_multimap <unsigned int,struct Attribute>::iterator itAttr2; // Attributes iterator
	unsigned int key;

	//Lowercase
	for(unsigned int i_lower=0;i_lower<lower_att_val.size();i_lower++){
		if(lower_att_val[i_lower]>64 && lower_att_val[i_lower]<91){
			lower_att_val[i_lower] += 32;
		}
	}

	for(unsigned int j=0 ; j < ruleSize ; j++){	
		if ( rules[j].method.size()!=0 && tmp_att.attribute_source !=0 ){
			if( rules[j].att_types.count(tmp_att.attribute_source) > 0/*H,G,P,J or X*/ ){ // H=Header,G=Get,P=Post,J=Json,X=XML.
				if ( rules[j].final_type == 'g' ){		  // g = regex match.	
					pthread_mutex_lock(&mutexgetatt5);
					regexRules(rules[j],c_RID,lower_att_val,itSession->second,alert_score,c_IP,resp_ip,cookie,hostname);
					pthread_mutex_unlock(&mutexgetatt5);

					//The Engine doesn't learn MS rules.
					if(alert_score != 0 ){
						/*for( unsigned int i_ms=0 ; i_ms < itSession->second.vRequest.size() ; i_ms++){
							itSession->second.vRequest[i_ms].tainted=1;
						}*/

						pthread_mutex_lock(&mutexMSatt);
						MSatt.insert( c_RID );
						pthread_mutex_unlock(&mutexMSatt);
					}
				}
				else if(rules[j].final_type == 's'){		 // s = substring.
					if ( findSubString( lower_att_val ,rules[j].str_match,rules[j].negate)  ){
						alert_score = (double)rules[j].threshold;
						pthread_mutex_lock(&mutexgetatt5);
							insert_alert(rules[j],c_RID,c_IP,resp_ip,cookie,hostname);
						pthread_mutex_unlock(&mutexgetatt5);
					}
				}
			}
		}

		if( tmp_att.hash != rules[j].att_id ){
			if( rules[j].att_name.size() == 0 ){
				continue;
			}else{
				if( tmp_att.name != rules[j].att_name){
					continue;
				}
			}
		}

		//--------------Attribute rules-----------------
		if (rules[j].anchor.size() == 0 ){
			if( !isAnySpecification(itSession->second.decimalIP , c_UserID , rules[j] ) ){			// App/User/IP violation ?.
				continue;
			}

			switch (rules[j].final_type){
				case 's':			// s = String match.
					if ( findSubString( lower_att_val ,rules[j].str_match,rules[j].negate)  ){
						alert_score = (double)rules[j].threshold;
						pthread_mutex_lock(&mutexgetatt5);
							insert_alert(rules[j],c_RID,c_IP,resp_ip,cookie,hostname);
						pthread_mutex_unlock(&mutexgetatt5);
					}
					break;
				case 'g':			// g = regex match.
					pthread_mutex_lock(&mutexgetatt5);
					regexRules(rules[j],c_RID,lower_att_val,itSession->second,alert_score,c_IP,resp_ip,cookie,hostname);
					pthread_mutex_unlock(&mutexgetatt5);
					break;
				case 'h':			//Heuristic anomaly for a value . h = Heuristic.  
					if(learning_mode==true){
						continue;
					}

					//Production Mode 
					num_score=0;
					if(rules[j].personal==0){// General
						if(nf=='y'){
							if ( isTheRuleMatch(rules[j].threshold ,itN->second.attribute,exp_score,num_score,0) == 0 ){ 
								alert_score = (double)rules[j].threshold;
								pthread_mutex_lock(&mutexgetatt5);
									insert_alert(rules[j],c_RID,c_IP,num_score,resp_ip,cookie,hostname);
								pthread_mutex_unlock(&mutexgetatt5);
							}
						}
					}else{			// For user.
						/*if(nfu=='y'){
							if ( isTheRuleMatch(rules[j].threshold,itNU->second.attribute ,exp_score_user,num_score,0) == 0 ){ 
								alert_score = (double)rules[j].threshold;
								pthread_mutex_lock(&mutexgetatt5);
									insert_alert(rules[j],c_RID,c_IP,num_score);
								pthread_mutex_unlock(&mutexgetatt5);
							}
						}*/
					}
					break;
				case 'e':			// e=ecxact.
				case 'r':			// r=range.
					// Length anomaly for a text value .
					s_of_m = 0;
					pthread_mutex_lock(&mutexgetatt5);
					lengthRules(rules[j],s_of_m,vector_value,0,0,itSession->second,c_RID,alert_score,c_IP,resp_ip,cookie,hostname);
					pthread_mutex_unlock(&mutexgetatt5);
					break;
				/*case 'd':
					pthread_mutex_lock(&mutexgetatt5);
					similarityRules(rules[j],vector_value,itSession->second,c_RID);
					pthread_mutex_unlock(&mutexgetatt5);
					break;*/
			}

			if(alert_score != 0){
				pthread_mutex_lock(&mutexMSatt);
				MSatt.insert( c_RID );
				pthread_mutex_unlock(&mutexMSatt);
			}

		}else{
			//----------------Pattern rules-------------------

			string att_anchor;
			key = (unsigned int)atoi(rules[j].anchor.c_str());
			if( key!=0){
				itAttr2 = to->mAttr.find(key);
				if(itAttr2 != to->mAttr.end() ){
					att_anchor = itAttr2->second.value;
				}else{
					continue;
				}
			}

			hash_value = hashCode(tmp);
				if(c_StatusCode < 400){
					if(rules[j].att_id != 0){
						bool alert_flag=false;
						pthread_mutex_lock(&mutexgetatt5);
						patternRulesParameter(rules[j],c_IP,c_UserID,c_RID,hash_value,itSession->second,att_anchor,tmp,resp_ip,cookie,hostname,alert_flag);
						pthread_mutex_unlock(&mutexgetatt5);
						if(alert_flag==true){
							alert_score = (double)rules[j].threshold;
						}
					}
				}
			//--------------------------------------------

			//--------------End Pattern rules-----------------
		}
	}
}
void *thread_check_memory(void *threadid)
{

	double vm, rss;
	while(globalEngine)
	{
		vm=0,rss=0;
		process_mem_usage(vm, rss);
		syslog(LOG_NOTICE,"Virtual memory: %f[kb], Resident set size: %f[kb]",vm, rss);
		sleep(5);
	}
	pthread_exit(NULL);
}

void *thread_load_config(void *threadid)
{
	string output,output_ext="0";

	while(globalEngine)
	{
		pthread_mutex_lock(&mutexUnknownApp);
			es_get_config("/telepath-config/config/add_unknown_applications_id/_source",output);
			addUnknownApp = (unsigned short)atoi(output.c_str());
		pthread_mutex_unlock(&mutexUnknownApp);
		es_get_config("/telepath-config/config/extension_was_changed_id/_source",output_ext);
		if(output_ext == "1"){
			load_filter_extensions();
			load_loadbalancer_ips();
			load_loadbalancer_headers();
			es_insert("/telepath-config/config/extension_was_changed_id","{\"value\":\"0\"}");
		}
		sleep(3);
	}
	pthread_exit(NULL);
}

void *thread_captcha(void *threadid)
{

	string tmp;
	redisContext *redis;
	redisReply *reply;


	redis = redisConnect("127.0.0.1", 6379);
	if(redis->err) {
		exit(EXIT_FAILURE);
	}

	while(globalEngine)
	{
		sem_wait(&sem_captcha);
		pthread_mutex_lock(&mutexCaptcha);
		tmp = valCaptchaQueue.front();
		valCaptchaQueue.pop();
		pthread_mutex_unlock(&mutexCaptcha);

		reply = (redisReply*)redisCommand(redis, "SETEX %s %d %s",(char*)tmp.c_str(),600,"" );
		freeReplyObject(reply);
	}
	pthread_mutex_destroy(&mutexCaptcha);
	pthread_exit(NULL);
}

void *thread_command(void *threadid)
{

	string tmp;
	FILE* ppipe_cmd;

	while(globalEngine)
	{
		sem_wait(&sem_command);
		pthread_mutex_lock(&mutexCommand);
		tmp = valCommandQueue.front();
		valCommandQueue.pop();
		pthread_mutex_unlock(&mutexCommand);

		ppipe_cmd = popen(tmp.c_str(),"w");
		pclose(ppipe_cmd);
	}
	pthread_mutex_destroy(&mutexCommand);
	pthread_exit(NULL);
}

// insert values from the valReqQueue to request index.
void *thread_insert_req(void *threadid)
{

	CURL *curl;
	RequestValToInsert tmp;
	curl = curl_easy_init();

	unsigned int insert_count=0,explanations_counter=0;
	unsigned int query_insert_size;
	static char query_insert[200000000];
	char tmp_str[100];

	sprintf(query_insert,"\n");
	query_insert_size = strlen(query_insert);
	char * dataPtr = query_insert+query_insert_size;
	string atts,alertandaction,explanations;
	boost::unordered_map <long long,ElasticAtt>::iterator itRidAtts;
	boost::unordered_map <long long,ElasticData>::iterator itRidAlertAndAction;
	boost::unordered_map <unsigned int,Reputation>::iterator itReputationIPs;

	char url[100];
	sprintf(url,"%s/_bulk",es_connect.c_str());

	while(globalEngine)
	{
		explanations.clear();

		sem_wait(&sem_insert_reqs);
		pthread_mutex_lock(&mutexInsertReq);
		tmp = valReqQueue.front();
		valReqQueue.pop();
		pthread_mutex_unlock(&mutexInsertReq);

		if(tmp.op_mode==1){// Zero anomaly scores for learning mode.
			tmp.flow_score=0;tmp.landing_normal=0;tmp.avg_normal=0;tmp.geo_normal=0;
		}

		pthread_mutex_lock(&mutexRidAtts);
		itRidAtts = mRidAtts.find(tmp.RID);
		if( itRidAtts != mRidAtts.end() ){
			atts = itRidAtts->second.attributes;

			if(itRidAtts->second.counter_atts < 5){
				explanations_counter++;
				sprintf(tmp_str,",\"explanations\":[1"); // There are missing headers in this request.
				explanations = tmp_str;
			}
			else if(itRidAtts->second.counter_atts > 15){ 
				explanations_counter++;
				sprintf(tmp_str,",\"explanations\":[2"); // There are unusual parameters in this request.
				explanations = tmp_str;
			}
			mRidAtts.erase(itRidAtts);
		}else{
			atts.clear();
		}
		pthread_mutex_unlock(&mutexRidAtts);

		pthread_mutex_lock(&mutexRidAlertAndBusiness);
		itRidAlertAndAction = mRidAlertAndAction.find(tmp.RID);
		if( itRidAlertAndAction != mRidAlertAndAction.end() ){
			alertandaction = itRidAlertAndAction->second.action;

			if(itRidAlertAndAction->second.counter_alerts != 0){
				alertandaction.append(itRidAlertAndAction->second.alerts);
				alertandaction.push_back(']');
				sprintf(tmp_str,",\"alerts_count\":%u",itRidAlertAndAction->second.counter_alerts);
				alertandaction.append(tmp_str);

				tmp.avg_normal=itRidAlertAndAction->second.threshold;
			}

			mRidAlertAndAction.erase(itRidAlertAndAction);
		}else{
			alertandaction.clear();
		}
		pthread_mutex_unlock(&mutexRidAlertAndBusiness);

		if(explanations_counter>0){
			sprintf(tmp_str,"],\"explanations_count\":%u",explanations_counter);
			explanations.append(tmp_str);
			explanations_counter=0;
		}


		pthread_mutex_lock(&mutexRepIPs);
		itReputationIPs = mReputationIPs.find(tmp.decimalIP);
		if( itReputationIPs != mReputationIPs.end()){
			itReputationIPs->second.update = 'y';
			itReputationIPs->second.counter++;
			itReputationIPs->second.sum += tmp.avg_normal;
			tmp.ip_score = (double)itReputationIPs->second.sum/itReputationIPs->second.counter;
		}else{
			tmp.ip_score = tmp.avg_normal;
			Reputation r(tmp.avg_normal);
			mReputationIPs.insert(pair<unsigned int,Reputation>(tmp.decimalIP,r));
			}
		pthread_mutex_unlock(&mutexRepIPs);

		insert_count++;
		sprintf(dataPtr,"{\"index\":{\"_index\":\"telepath-%s\",\"_type\":\"http\",\"_id\":\"X%lld\"}}\n{\"ts\":%f,\"ip_orig\":\"%s\",\"ip_score\":%f,\"ip_resp\":\"%s\",\"status_code\":%hu,\"country_code\":\"%s\",\"city\":\"%s\",\"method\":\"%s\",\"location\":\"%f,%f\",\"sid\":%u,\"sha256_sid\":\"%s\",\"score_presence\":%f,\"score_query\":%f,\"score_flow\":%f,\"score_landing\":%f,\"score_geo\":%f,\"score_average\":%f,\"index\":%u,\"host\":\"%s\",\"uri\":\"%s\",\"canonical_url\":\"%s\",\"operation_mode\":%hd,\"title\":\"%s\",\"username\":\"%s\",%s\"parameters\":[%s]%s%s}\n",&tmp.shard[0],tmp.RID,tmp.ts,&tmp.ip_orig[0],tmp.ip_score,&tmp.ip_resp[0],tmp.status_code,&tmp.country[0],&tmp.city[0],&tmp.method[0],tmp.c.x,tmp.c.y,tmp.Sid,&tmp.sha256_sid[0],tmp.presence,tmp.query_score,tmp.flow_score,tmp.landing_normal,tmp.geo_normal,tmp.avg_normal,tmp.sequence,&tmp.host_name[0],&tmp.page_name[0],&tmp.canonical_url[0],tmp.op_mode,&tmp.title[0],&tmp.user_name[0],&tmp.user_scores_string[0],&atts[0],&alertandaction[0],&explanations[0]);
		dataPtr += strlen(dataPtr);

		if( ( insert_count == INSERT_REQ_AT_ONCE ) || valReqQueue.empty() ){
			insertElastic += insert_count;
			insert_count=0;
			dataPtr[0] = '\0';

			curl_easy_setopt(curl,CURLOPT_URL,url);
			curl_easy_setopt(curl,CURLOPT_POSTFIELDS,query_insert);
			curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,dropprinting);
			curl_easy_perform(curl);

			//usleep(1000);

			dataPtr = query_insert+query_insert_size;
		}else{
			(dataPtr++)[0] = '\n';
		}
	}
	pthread_mutex_destroy(&mutexInsertReq);
	pthread_exit(NULL);
}

void cutSuff(string & url){
	if(url.size() == 0){
		return;
	}

	for(unsigned int i=url.size();i>=0;i--){
		if(url[i]=='/'){
			url.erase(i+1,url.size());
			break;
		}
	}
}

void parseAction(string & action,string & uri,string & host){
	size_t found;

	size_t pos = action.find('?');
	if(pos != string::npos){
		action.erase(pos,action.size());
	}

	if(action[0] == '/'){
		uri = action;
		return;
	}

	pos = action.find("http://");
	if(pos != string::npos){
		pos += 7;
		found = action.find("/",pos,1);
		if(found != string::npos){
			host.clear();
			host.assign(action.begin()+pos,action.begin()+found);
			uri.clear();
			uri.assign(action.begin()+found,action.end());
		}
		return;
	}

	pos = action.find("https://");
	if(pos != string::npos){
		pos += 8;
		found = action.find("/",pos,1);
		if(found != string::npos){
			host.clear();
			host.assign(action.begin()+pos,action.begin()+found);
			uri.clear();
			uri.assign(action.begin()+found,action.end());
		}
		return;
	}

	cutSuff(uri);
	uri += action;
}

void *thread_insert_logins(void *threadid)
{
	
	Login login;
	unsigned int hash_value,hash_value2;
	string hash_str;
	char postfields[5000],url[300];

	
	while(globalEngine)
	{
		//Lock When The Queue is empty.
		sem_wait(&sem_insert_logins);

		pthread_mutex_lock(&mutexLogin);
		login = loginQueue.front();
		loginQueue.pop();
		pthread_mutex_unlock(&mutexLogin);

		if(login.action.size() != 0){
			parseAction(login.action,login.orig_uri,login.host);
		}

		if(login.password.size()==0 || login.username.size() == 0){
			continue;
		}

		hash_str = "Brute-Force "+login.host+login.orig_uri+login.password+login.username;
		hash_value = hashCode(hash_str);
		hash_str = "Credential-Stuffing "+login.host+login.orig_uri+login.username;
		hash_value2 = hashCode(hash_str);
		pthread_mutex_lock(&mutexHashRules);
		if(setHashRules.count(hash_value) != 0){
			pthread_mutex_unlock(&mutexHashRules);
			continue;
		}else{
			setHashRules.insert(hash_value);
			setHashRules.insert(hash_value2);
			pthread_mutex_unlock(&mutexHashRules);
		}

		pthread_mutex_lock(&mutexAddRules);
		addRulesIDs.push_back(hash_value);
		addRulesIDs.push_back(hash_value2);

		sprintf(url,"/telepath-rules/rules/X%u",hash_value);
		sprintf(postfields,"{\"hash\":%u,\"name\":\"Login Brute-Force\",\"builtin_rule\": true,\"desc\":\"-\",\"category\":\"Brute-Force\",\"score\":100,\"cmd\":[\"captcha\"],\"criteria\":[{\"kind\":\"p\",\"type\":\"Other\",\"Other\":{\"domain\":\"%s\",\"pagename\":\"%s\",\"paramname\":\"%s\"},\"subtype\":\"parameter\",\"domain\":\"%s\",\"pagename\":\"%s\",\"paramname\":\"%s\",\"count\":\"3\",\"time\":\"180\",\"enable\":true,\"aggregate\":\"1\"}]}",hash_value,&login.host[0],&login.orig_uri[0],&login.username[0],&login.host[0],&login.orig_uri[0],&login.password[0]);
		es_insert(url,postfields);

		sprintf(url,"/telepath-rules/rules/X%u",hash_value2);
		sprintf(postfields,"{\"hash\":%u,\"name\":\"Credential-Stuffing\",\"builtin_rule\": true,\"desc\":\"-\",\"category\":\"Credential-Stuffing\",\"score\":100,\"cmd\":[\"captcha\"],\"criteria\":[{\"kind\":\"p\",\"type\":\"IP\",\"subtype\":\"parameter\",\"domain\":\"%s\",\"pagename\":\"%s\",\"paramname\":\"%s\",\"count\":\"3\",\"time\":\"600\",\"enable\":true,\"aggregate\":\"1\"}]}",hash_value2,&login.host[0],&login.orig_uri[0],&login.username[0]);
		es_insert(url,postfields);
		pthread_mutex_unlock(&mutexAddRules);

		pthread_mutex_lock(&mutexAddActions);
		addActionIDs.push_back(hash_value);

		sprintf(url,"/telepath-actions/actions/X%u",hash_value);
		sprintf(postfields,"{\"action_name\":\"Login\",\"application\":\"%s\",\"business\":[{\"pagename\":\"%s\",\"params\":[{\"name\":\"%s\",\"data\":\"*\"},{\"name\":\"%s\",\"data\":\"*\"}]}]}",&login.host[0],&login.orig_uri[0],&login.username[0],&login.password[0]);
		es_insert(url,postfields);
		pthread_mutex_unlock(&mutexAddActions);

		sprintf(url,"/telepath-domains/domains/%s/_update",login.host.c_str());
		sprintf(postfields,"{\"doc\":{\"form_param_name\":\"%s\"}}",login.username.c_str());
		es_mapping(url,postfields);

	}

	pthread_mutex_destroy(&mutexAddRules);
	pthread_mutex_destroy(&mutexAddActions);
	pthread_mutex_destroy(&mutexLogin);
	pthread_exit(NULL);
}

void *markovAtt_thread(void *threadid)
{
	while(globalEngine){
		sem_wait(&sem_markov_att_start);

		double score;
		int exp,exp_score_;
		unsigned int size = markov_Att->values.size();
		for(unsigned int i=0; i < size ; i++ ){
			if(markovAtt_values_flag[i]!=0){
				continue;
			}

			pthread_mutex_lock(&mutexmarkov);      // lock
			if(markovAtt_values_flag[i]==0){       // check if this value already checked by other thread.
				markovAtt_values_flag[i]=1;            // turn on the check flag for this value.
				pthread_mutex_unlock(&mutexmarkov);    // unlock 
			}else{
				pthread_mutex_unlock(&mutexmarkov);
				continue;
			}

			exp_score_ = markov_Att->tree.calculate(markov_Att->values[i],0); // markov calculation.

			score = markov_Length->chebyshev( (double)markov_Att->val_s_f[i].size );
			frexp(score,&exp);
			if(exp>0){exp=0;}
			
			if(exp==1){exp=0;}
			markov_Att->exp_length[i] = (short)exp;

			markov_Att->exp_score[i] = exp_score_;
		}

		sem_post(&sem_markov_att_end);
	}
	pthread_exit(NULL);
}

void *markovSession_thread(void *threadid)
{
	while(globalEngine){
		sem_wait(&sem_markov_session_start);

		unsigned int size = markovSession_.size();

		for(unsigned int i=0; i < size ; i++ ){
			if(markovSession_values_flag[i]=='y'){
				continue;
			}

			pthread_mutex_lock(&mutexmarkovSession);   // lock	
			if(markovSession_values_flag[i]=='n'){       // check if this value already checked by other thread.
				markovSession_values_flag[i]='y';            // turn on the check flag for this value.
				pthread_mutex_unlock(&mutexmarkovSession); // unlock 
			}else{
				pthread_mutex_unlock(&mutexmarkovSession);
				continue;
			}

			Markov markov;
			markov.calculate( *(markovSession_path[i]) , *(markovSession_path_user[i]) ,*(markovSession_[i]),markovSession_mode[i]);
		}

		sem_post(&sem_markov_session_end);
	}
	pthread_exit(NULL);
}

void *thread_sid_per_min(void *threadid)
{
	boost::unordered_map <unsigned int,boost::unordered_set <unsigned int> >::iterator itSessionPerMinuteHost;
	unsigned int size,errorCheck=10,counter=0,current = getRedis, delta=0;
	while(globalEngine)
	{
		sleep(60);

		/*syslog(LOG_NOTICE,"mSession.size(): %u\n", (unsigned int)mSession.size() );
		syslog(LOG_NOTICE,"mAttAvg.size(): %u\n", (unsigned int)mAttAvg.size() );
		syslog(LOG_NOTICE,"sTorIntelligenceIP.size(): %u\n", (unsigned int)sTorIntelligenceIP.size() );
		syslog(LOG_NOTICE,"mQuery.size(): %u\n", (unsigned int)mQuery.size() );
		syslog(LOG_NOTICE,"mQueryUser.size(): %u\n", (unsigned int)mQueryUser.size() );
		syslog(LOG_NOTICE,"rules.capacity(): %u\n", (unsigned int)rules.capacity() );
		syslog(LOG_NOTICE,"mPathPerGroup.size(): %u\n", (unsigned int)mPathPerGroup.size() );
		syslog(LOG_NOTICE,"score_data.size(): %u\n", (unsigned int)score_data.size() );
		syslog(LOG_NOTICE,"score_numeric.size(): %u\n", (unsigned int)score_numeric.size() );
		syslog(LOG_NOTICE,"mScoreAtt.size(): %u\n", (unsigned int)mScoreAtt.size() );
		syslog(LOG_NOTICE,"mScoreNumericAtt.size(): %u\n", (unsigned int)mScoreNumericAtt.size() );
		syslog(LOG_NOTICE,"noisyExcludeAttSet.size(): %u\n", (unsigned int)noisyExcludeAttSet.size() );
		syslog(LOG_NOTICE,"myAttMap.size(): %u\n", (unsigned int)myAttMap.size() );
		syslog(LOG_NOTICE,"---------------------------------\n");*/
		

		pthread_mutex_lock(&mutexSidPerMin); //lock
		size = (unsigned int)sessionPerMinute.size();
		sessionPerMinute.clear();
		pthread_mutex_unlock(&mutexSidPerMin); //unlock

		pthread_mutex_lock(&mutexSessionsPerHost); //lock
		writeB2(numOfSessionsPerDomain);
		pthread_mutex_unlock(&mutexSessionsPerHost); //unlock

		syslog(LOG_NOTICE,"Sessions:%u mMem:%u |getRedis:%u |insertElastic:%u |dropApp:%u |dropPage:%u |dropMethod:%u |dropLong:%u|",size,(unsigned int)TC->teleObjQueue.size(),getRedis,insertElastic,dropApp,dropPage,dropMethod,longSessionDrop);	
		
		delta = getRedis - current;
		current = getRedis;

		// if(errorCheck==getRedis){
		// 	counter++;
		// }else{
		// 	counter=0;
		// 	errorCheck=getRedis;
		// }
		if(delta == 0){
			counter++;
		}else{
			counter=0;
		}
		if(counter==3){
			counter=0;
			syslog(LOG_NOTICE,"Suricata didn't receive any DATA");
			FILE *fd = popen("telepath suricata > /dev/null 2>&1","w");
			pclose(fd);
			sleep(5);
		}
		//if(counter==15){
		//	counter=0;
		//	syslog(LOG_NOTICE,"Engine has not gotten any requests for 15 minutes ... ");
		//
		//	FILE *fd = popen("telepath restart > /dev/null 2>&1","w");
		//	pclose(fd);
		//	sleep(15);
		//}

	}
	pthread_mutex_destroy(&mutexSidPerMin);
	pthread_exit(NULL);
}

void *getAtt_thread(void *threadarg)
{
	thread_struct_* v = static_cast< thread_struct_* >(threadarg);
	boost::unordered_multimap <unsigned int,struct Attribute>::iterator itAttr; // Attributes iterator
	map<unsigned int,myAtt> ::iterator itMyAttMap; // iterator of myAttMap.
	pair< boost::unordered_map <unsigned int,Session> ::iterator,bool > itSession;// iterator of mSession
	map <unsigned int,ScoreNumericAtt>::iterator itN;
	map <string,ScoreNumericAtt>::iterator itNU;
	vector<TeleObject> * rid_;
	vector <struct Attribute> atts;
	vector <struct Attribute>::iterator it_atts;
	struct Attribute tmp_att;

	unsigned short c_StatusCode;
	unsigned int size = 0,i,start_i,c_PageID,c_Seq,c_SID,c_Mode;
	string c_Domain;
	long long c_RID;
	string tmp,user_key,c_IP,c_UserID,c_SetCookie,c_Title,c_Uri,sha_string,c_Resp_IP;
	int exp_score=0;
	double alert_score,c_TimeStamp;
	boost::unordered_map<unsigned int,alert >::iterator itRIDsPerFlow;
	pair< boost::unordered_map<unsigned int,alert >::iterator,bool > itRIDsPerFlow2;
	alert a;

	boost::unordered_map <long long,ElasticData>::iterator itRidAlertAndAction;
	boost::unordered_map <unsigned int,string>::iterator itTmpAtts;
	unsigned int i_flow,k;

	vector <ActionParams> tmpAttributesVec;
	char attField[15000];
	boost::unordered_map <long long,ElasticAtt>::iterator itRidAtts;

	while (globalEngine)
	{
		start_i = (unsigned int)-1;

		pthread_mutex_lock(&mutexgetatt);   // lock
		pthread_cond_wait(&getatt_cond, &mutexgetatt);

		if(v->rids == NULL){
			pthread_mutex_unlock(&mutexgetatt);
			continue;
		}
		size = v->rids->size();
		for(i=0; i < size ; i++ ){
			if(learning_values_flag[i]==0){       // check if this value already checked by other thread.
				learning_values_flag[i]=1;            // turn on the check flag for this value.
				start_i = i;
				break;
			}
		}

		pthread_mutex_unlock(&mutexgetatt);
		if (!globalEngine)
		{
			pthread_exit(NULL);
		}
		if (start_i == (unsigned int)-1)
		{
			continue;
		}

		for(i=start_i; i < size ; i++ ){
			if (i != start_i)
			{
				if(learning_values_flag[i]==1){
					continue;
				}
				pthread_mutex_lock(&mutexgetatt);   // lock
				if(learning_values_flag[i]==0){       // check if this value already checked by other thread.
					learning_values_flag[i]=1;            // turn on the check flag for this v	alue.
					pthread_mutex_unlock(&mutexgetatt); // unlock 	
				}else{
					pthread_mutex_unlock(&mutexgetatt);
					continue;
				}
			}

			rid_ = v->rids;
			TeleObject *to = &(*rid_)[i];
			initParams(to,c_IP,c_PageID,c_Domain,c_RID,c_SID,c_StatusCode,c_TimeStamp,c_SetCookie,c_Seq,c_Title,c_Uri,c_Mode,c_Resp_IP);

			pthread_mutex_lock(&mutexSidPerMin); //lock
				sessionPerMinute.insert(c_SID);
			pthread_mutex_unlock(&mutexSidPerMin); //unlock

			pthread_mutex_lock(&mutexSessions);
			itSession.first=mSession.find(c_SID);		//Check if the session is already in the Cache.
	  		if(itSession.first != mSession.end()){		//Session was found
				//if( itSession.first->second.sessionSize > maxSession ){ // long session!!!
				//	itSession.first->second.erase = 'y';
				//	longSessionDrop++;
				//}else{
					c_UserID.clear();
					if(itSession.first->second.validUser==false){
						if( ( to->mParams['v'/*LoginMsg*/].size() > 0 )  &&  ( itSession.first->second.sUsername.size() != 0 ) ){
							itSession.first->second.validUser = true;
						}
						else if(to->mParams['w'/*BasicDigestAuth*/].size() > 0 ){
							itSession.first->second.validUser = true;
						}
					}

					if(itSession.first->second.validUser==true){
						c_UserID = itSession.first->second.sUsername;
					}

					if(itSession.first->second.IP.compare(c_IP) != 0 ){
						itSession.first->second.IP = c_IP;
						itSession.first->second.decimalIP = ipToNum(c_IP);
					}

					if(c_SetCookie.size() != 0 ){
						calCookieNames(c_Domain,c_SetCookie);
					}

					if( c_UserID.size()>0 ){	
						session_user_search(c_UserID,itSession.first->second.sid,mSession,*to);
					}

					if(c_Seq == 0){
						pthread_mutex_lock(&mutexSessionsPerHost); //lock
						itSessionsPerDomain = numOfSessionsPerDomain.find(c_Domain);
						if(itSessionsPerDomain != numOfSessionsPerDomain.end() ){
							itSessionsPerDomain->second++;
						}else{
							numOfSessionsPerDomain.insert(pair<string,unsigned int>(c_Domain,1) );
						}
						pthread_mutex_unlock(&mutexSessionsPerHost); //unlock
					}

					Page page( to->mParams,c_Seq,c_PageID,c_RID,itSession.first->second.IP,c_UserID);
					page.presence = (double)atof( to->mParams['l'/*PresenceOrAbsence*/].c_str() );

					itSession.first->second.sessionSize++;
					itSession.first->second.vRequest.push_back(page);
					itSession.first->second.update=1; // need to update path.
					unsigned int source_IP = ipToNum(c_IP);
					unsigned int resp_IP = ipToNum(c_Resp_IP);
					//--------------Page rules,Bot Intelligence & Country Rule-----------------
					for(k=0; k<rules.size() ;k++){
						if(rules[k].domain_block.empty() && rules[k].vecRangeIP.size()==0){
							generalRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_UserID,c_Resp_IP,c_SetCookie,c_Domain);
							pageRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_Title,c_Uri,c_Resp_IP,c_SetCookie,c_Domain,c_StatusCode);
						}
						else if(!rules[k].domain_block.empty() && rules[k].domain_block != c_Domain){
							generalRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_UserID,c_Resp_IP,c_SetCookie,c_Domain);
							pageRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_Title,c_Uri,c_Resp_IP,c_SetCookie,c_Domain,c_StatusCode);
						}else if(rules[k].vecRangeIP.size() != 0){
							if(!rules[k].vecRangeIP[0].inRange(resp_IP) && !rules[k].vecRangeIP[0].inRange(source_IP)){	
								generalRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_UserID,c_Resp_IP,c_SetCookie,c_Domain);
								pageRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_Title,c_Uri,c_Resp_IP,c_SetCookie,c_Domain,c_StatusCode);
							}
						}
					}
					//--------------------------------End--------------------------------------

					if(itSession.first->second.erase == 'n'){
						itSession.first->second.elapsed_ts = itSession.first->second.vRequest[itSession.first->second.vRequest.size()-1].ts - itSession.first->second.vRequest[0].ts; //session time.
					}
				//}
			}

			pthread_mutex_unlock(&mutexSessions);

			atts.clear();
			//if( itSession.first->second.sessionSize <= maxSession ){
				for(itAttr= to->mAttr.begin() ;itAttr!= to->mAttr.end();itAttr++ ){ // insert all the attribute of a page to the myAttMap.
					atts.push_back(itAttr->second);
				}
				it_atts = atts.begin();
			//}

			pthread_mutex_lock(&mutexBusinessLogic);
			if( itSession.first != mSession.end() ){
				for(i_flow=0;i_flow<businessFlowVec.size();i_flow++){
					itRIDsPerFlow = itSession.first->second.RIDsPerFlow.find(businessFlowVec[i_flow].action_id);
					if( itRIDsPerFlow != itSession.first->second.RIDsPerFlow.end() ){
						if(c_PageID == businessFlowVec[i_flow].pages[itRIDsPerFlow->second.rids.size()].pageID ){
							ActionParams aa(businessFlowVec[i_flow].pages[itRIDsPerFlow->second.rids.size()].params,businessFlowVec[i_flow].action_id,businessFlowVec[i_flow].action_name,businessFlowVec[i_flow].pages.size(),businessFlowVec[i_flow].logout);
							tmpAttributesVec.push_back(aa);
						}
					}else{
						if(businessFlowVec[i_flow].pages.size()>0 ){
							if(c_PageID == businessFlowVec[i_flow].pages[0].pageID){
								ActionParams aa(businessFlowVec[i_flow].pages[0].params,businessFlowVec[i_flow].action_id,businessFlowVec[i_flow].action_name,businessFlowVec[i_flow].pages.size(),businessFlowVec[i_flow].logout);
								tmpAttributesVec.push_back(aa);
							}
						}
					}
				}
			}
			pthread_mutex_unlock(&mutexBusinessLogic);


			while(atts.size() != 0){
				tmp_att = *it_atts ;

				size_and_flag s_f;			

				for(i_flow=0; i_flow<tmpAttributesVec.size() ;i_flow++) {
					itTmpAtts = tmpAttributesVec[i_flow].params.find(tmp_att.hash);
					if(itTmpAtts != tmpAttributesVec[i_flow].params.end() ){
						if(itTmpAtts->second == "*" || itTmpAtts->second.compare(tmp_att.value)==0  ){
							tmpAttributesVec[i_flow].params.erase(tmp_att.hash);
						}
					}

					if(tmpAttributesVec[i_flow].params.size()==0){

						if(tmpAttributesVec[i_flow].logout == true && itSession.first->second.logout == false){
							itSession.first->second.logout = true;
							itSession.first->second.logout_counter++;
						}
						pthread_mutex_lock(&mutexBusinessLogic);
						itRIDsPerFlow2.first = itSession.first->second.RIDsPerFlow.find(tmpAttributesVec[i_flow].action_id);
						if( itRIDsPerFlow2.first == itSession.first->second.RIDsPerFlow.end() ){
							itRIDsPerFlow2 = itSession.first->second.RIDsPerFlow.insert(pair<unsigned int,alert >(tmpAttributesVec[i_flow].action_id,a));
						}

						itRIDsPerFlow2.first->second.rids.insert(c_RID);
						if(itRIDsPerFlow2.first->second.rids.size() == 1){
							itRIDsPerFlow2.first->second.start_ts = c_TimeStamp;
							itRIDsPerFlow2.first->second.end_ts = c_TimeStamp;
						}else{
							itRIDsPerFlow2.first->second.end_ts = c_TimeStamp;
						}

						if(itRIDsPerFlow2.first->second.rids.size() == tmpAttributesVec[i_flow].numOfPages){
							ActionState fs(itSession.first->second.sid,tmpAttributesVec[i_flow].action_id,tmpAttributesVec[i_flow].action_name);
							itRIDsPerFlow2.first->second.rids.clear();

							for(k=0; k<rules.size() ;k++){
								if(rules[k].action_id == fs.action_id){
									patternRulesBusiness(rules[k],itSession.first->second.IP,c_UserID,c_PageID,itSession.first->second,c_RID,c_Resp_IP,c_SetCookie,c_Domain);
								}
							}

							pthread_mutex_unlock(&mutexBusinessLogic);

							if(fs.action_name.size() != 0){
								sprintf(attField,",\"business_actions_count\":1,\"business_actions\":[{\"name\":\"%s\",\"status\":2}]",&fs.action_name[0]);
								pthread_mutex_lock(&mutexRidAlertAndBusiness);
									itRidAlertAndAction = mRidAlertAndAction.find(c_RID);
									if( itRidAlertAndAction != mRidAlertAndAction.end() ){
										itRidAlertAndAction->second.action = attField;
									}else{
										ElasticData ed;
										ed.action = attField;
										mRidAlertAndAction.insert(pair<long long,ElasticData>(c_RID,ed));

									}
								pthread_mutex_unlock(&mutexRidAlertAndBusiness);
							}
						}else{
							pthread_mutex_unlock(&mutexBusinessLogic);
						}
					}
				}

				pthread_mutex_lock(&mutexMyAttMap);
				itMyAttMap=myAttMap.find(tmp_att.hash);
				if(itMyAttMap == myAttMap.end()){
					myAtt att;  //lock=0
					getAttributeThatWasntFound(att, tmp_att, s_f, c_RID, c_PageID);
					myAttMap.insert( pair<unsigned int,myAtt>(tmp_att.hash,att) );
					pthread_mutex_unlock(&mutexMyAttMap);

					atts.erase(it_atts);
					it_atts = atts.begin();
				}else{
					if(itMyAttMap->second.lock==1){
						pthread_mutex_unlock(&mutexMyAttMap);

						if( it_atts == (atts.end()-1) ){
							it_atts=atts.begin();
						}else{
							it_atts++;
						}
	
						continue;			
					}else{
						itMyAttMap->second.lock=1;
						pthread_mutex_unlock(&mutexMyAttMap);
						atts.erase(it_atts);
						it_atts = atts.begin();

					}

					getAttributeThatWasFound(itMyAttMap->second,tmp_att, s_f, c_RID,i);

					if(tmp_att.attribute_source!='H' && itMyAttMap->second.type=='e'){
						if(itMyAttMap->second.enumeration.seen.size() <= 20){

							map <unsigned int, Operation >::iterator itOperation = mOperation.find(c_PageID);
							if(itOperation != mOperation.end()){ // Page was found.
								map <unsigned long long,map<int,unsigned short> >::iterator itVecShape;
								itVecShape = itOperation->second.vecShape.find(tmp_att.hash);
								if( itVecShape != itOperation->second.vecShape.end() ){ // Attribute was found.
									map <int,unsigned short >::iterator itValues = itVecShape->second.find(hashCode(tmp_att.value) );
									if( itValues != itVecShape->second.end() ){ // value was found.
										map <long long, OpRID >::iterator itOpRIDs;

										pthread_mutex_lock(&mutexOperation);
										itOpRIDs = mOpRIDs.find(c_RID);
										if(itOpRIDs != mOpRIDs.end() ){
											if(tmp_att.attribute_source=='J' || tmp_att.attribute_source=='X'){
												itOpRIDs->second.opts[itValues->second] = (float)2/itVecShape->second.size();
											}
											else if(tmp_att.attribute_source=='P' || tmp_att.attribute_source=='G'){
												itOpRIDs->second.opts[itValues->second] = (float)1/itVecShape->second.size();
											}
											pthread_mutex_unlock(&mutexOperation);
										}else{
											pthread_mutex_unlock(&mutexOperation);
											OpRID optTmp(c_PageID);
											if(tmp_att.attribute_source=='J' || tmp_att.attribute_source=='X'){
												optTmp.opts[itValues->second] = (float)2/itVecShape->second.size();
											}
											else if(tmp_att.attribute_source=='P' || tmp_att.attribute_source=='G'){
												optTmp.opts[itValues->second] = (float)1/itVecShape->second.size();
											}

											pthread_mutex_lock(&mutexOperation);
											mOpRIDs.insert(pair<long long,OpRID >(c_RID,optTmp));
											pthread_mutex_unlock(&mutexOperation);

										}
									}
								}
							}

						}
					}

					itMyAttMap->second.lock=0;
				}

				unicodeParser(tmp_att.vec_value,tmp);


				alert_score=0;
				if(itSession.first!=mSession.end()){	//Session was found.
					//-----------Rules---------
					attributeRules(true,tmp_att,itSession.first,to,tmp_att.vec_value,tmp,'n','n',itN,itNU,exp_score,0,c_IP,c_UserID,c_RID,itSession.first->second.sid,c_Title,alert_score,c_Resp_IP,c_SetCookie,c_Domain,tmp_att.value,c_StatusCode);
					//---------End Rules-------
				}

				pthread_mutex_lock(&mutexRidAtts);
				itRidAtts = mRidAtts.find(c_RID);
				if( itRidAtts != mRidAtts.end() ){
					maskPassword(tmp_att.hash,tmp_att.value);
					maskCreditCard(tmp_att.value);
					sprintf(attField,",{\"name\":\"%s\",\"value\":\"%s\",\"type\":\"%c\",\"score_data\":%f}",&tmp_att.name[0],&tmp_att.value[0],tmp_att.attribute_source,alert_score);
					itRidAtts->second.attributes.append(attField);
					itRidAtts->second.counter_atts++;
				}else{
					maskPassword(tmp_att.hash,tmp_att.value);
					maskCreditCard(tmp_att.value);
					sprintf(attField,"{\"name\":\"%s\",\"value\":\"%s\",\"type\":\"%c\",\"score_data\":%f}",&tmp_att.name[0],&tmp_att.value[0],tmp_att.attribute_source,alert_score);
					ElasticAtt ea;
					ea.attributes = attField;
					mRidAtts.insert(pair<long long,ElasticAtt>(c_RID,ea));
				}

				pthread_mutex_unlock(&mutexRidAtts);

				vector <unsigned int>().swap(tmp_att.vec_value);
			}

			vector <ActionParams>().swap(tmpAttributesVec);

			learning_values_flag[i]=2;
		}
	}
	vector <struct Attribute>().swap(atts);
	pthread_exit(NULL);
}

void *getAtt_thread_pro(void *threadarg)
{
	thread_struct_* v = static_cast< thread_struct_* >(threadarg);
	boost::unordered_multimap <unsigned int,struct Attribute>::iterator itAttr; // Attributes iterator
	map <unsigned int,myAtt> ::iterator itMyAttMap; // iterator of myAttMap.
	pair< boost::unordered_map <unsigned int,Session> ::iterator,bool > itSession;// iterator of mSession
	vector<TeleObject> * rid_;
	vector <struct Attribute> atts;
	vector <struct Attribute>::iterator it_atts;
	struct Attribute tmp_att;

	unsigned short c_StatusCode;
	unsigned int size = 0,i,start_i,c_PageID,c_Seq,c_SID,c_Mode;
	string c_Domain;
	long long c_RID;
	map <unsigned int,ScoreNumericAtt>::iterator it_NumericAtt;
	map <string,ScoreNumericAtt>::iterator it_NumericAttUser;
	string tmp,c_IP,c_UserID,c_SetCookie,c_Title,c_Uri,user_,user_key,sha_string,c_Resp_IP;
	double size_of_markov,normalize_score,tmpWeight=1,alert_score,c_TimeStamp,len_score;
	int exp_score;
	char normal_flag,normal_flag_user;
	bool ifSessionBlock;
	boost::unordered_map<unsigned int,alert >::iterator itRIDsPerFlow;
	pair< boost::unordered_map<unsigned int,alert >::iterator,bool > itRIDsPerFlow2;
	alert a;
	boost::unordered_map <unsigned int,string>::iterator itTmpAtts;
	unsigned int i_flow,k;

	vector <ActionParams> tmpAttributesVec;
	char attField[15000];
	boost::unordered_map <long long,ElasticAtt>::iterator itRidAtts;
	boost::unordered_map <long long,ElasticData>::iterator itRidAlertAndAction;

	while (globalEngine)
	{
		start_i = (unsigned int)-1;

		pthread_mutex_lock(&mutexgetattPro);   // lock
		pthread_cond_wait(&getatt_condPro, &mutexgetattPro);

		if(v->rids == NULL){
			pthread_mutex_unlock(&mutexgetattPro);
			continue;
		}
		size = v->rids->size();
		for(i=0; i < size ; i++ ){
			if(production_values_flag[i]==0){       // check if this value already checked by other thread.
				production_values_flag[i]=1;            // turn on the check flag for this value.
				start_i = i;
				break;
			}
		}

		pthread_mutex_unlock(&mutexgetattPro);
		if (!globalEngine)
		{
			pthread_exit(NULL);
		}

		if (start_i == (unsigned int)-1)
		{
			continue;
		}

		for(i=start_i; i < size ; i++ ){
			if(start_i != i){
				if(production_values_flag[i]==1){
					continue;
				}
				pthread_mutex_lock(&mutexgetattPro);   // lock
				if(production_values_flag[i]==0){       // check if this value already checked by other thread.
					production_values_flag[i]=1;            // turn on the check flag for this value.
					pthread_mutex_unlock(&mutexgetattPro); // unlock 	
				}else{
					pthread_mutex_unlock(&mutexgetattPro);	
					continue;		
				}
			}

			rid_ = v->rids;
			TeleObject *to = &(*rid_)[i];

			initParams(to,c_IP,c_PageID,c_Domain,c_RID,c_SID,c_StatusCode,c_TimeStamp,c_SetCookie,c_Seq,c_Title,c_Uri,c_Mode,c_Resp_IP);

			pthread_mutex_lock(&mutexSidPerMin); //lock
				sessionPerMinute.insert(c_SID);
			pthread_mutex_unlock(&mutexSidPerMin); //unlock

			pthread_mutex_lock(&mutexSessions);
			itSession.first = mSession.find(c_SID);
			if(itSession.first != mSession.end()){	//Session was found.
				//if( itSession.first->second.sessionSize > maxSession || itSession.first->second.block == 'y' ){ // long session!!!
				//	ifSessionBlock = true;
				//	longSessionDrop++;
				//}else{
					c_UserID.clear();
					if(itSession.first->second.validUser==false){
						if( ( to->mParams['v'/*LoginMsg*/].size() > 0 )  &&  ( itSession.first->second.sUsername.size() != 0 ) ){
							itSession.first->second.validUser = true;
						}
						else if(to->mParams['w'/*BasicDigestAuth*/].size() > 0 ){
							itSession.first->second.validUser = true;
						}
					}

					//Don't change it to else.
					if(itSession.first->second.validUser==true){
						c_UserID = itSession.first->second.sUsername;
						if(itSession.first->second.sUsername.size() == 0){
							itSession.first->second.validUser = false;
						}
					}

					ifSessionBlock = false;
					if(itSession.first->second.IP.compare(c_IP) != 0 ){
						itSession.first->second.IP = c_IP;
						itSession.first->second.decimalIP = ipToNum(c_IP);
					}

					if(c_SetCookie.size() != 0 ){
						calCookieNames(c_Domain,c_SetCookie);
					}

					if( c_UserID.size()>0 ){
						session_user_search(c_UserID, itSession.first->second.sid,mSession,*to);
					}

					if(c_Seq == 0){
						pthread_mutex_lock(&mutexSessionsPerHost); //lock
						itSessionsPerDomain = numOfSessionsPerDomain.find(c_Domain);
						if(itSessionsPerDomain != numOfSessionsPerDomain.end() ){
							itSessionsPerDomain->second++;
						}else{
							numOfSessionsPerDomain.insert(pair<string,unsigned int>(c_Domain,1) );
						}
						pthread_mutex_unlock(&mutexSessionsPerHost); //unlock
					}

					Page page( to->mParams,c_Seq,c_PageID,c_RID,itSession.first->second.IP,c_UserID);
					page.presence = (double)atof( to->mParams['l'/*PresenceOrAbsence*/].c_str() );

					itSession.first->second.sessionSize++;
					itSession.first->second.update=1;
					itSession.first->second.vRequest.push_back(page);

					unsigned int source_IP = ipToNum(c_IP);
					unsigned int resp_IP = ipToNum(c_Resp_IP);
					//--------------Page rules,Bot Intelligence & Country Rule-----------------
					for(k=0; k<rules.size() ;k++){
						if(rules[k].domain_block.empty() && rules[k].vecRangeIP.size()==0){
							generalRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_UserID,c_Resp_IP,c_SetCookie,c_Domain);
							pageRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_Title,c_Uri,c_Resp_IP,c_SetCookie,c_Domain,c_StatusCode);
						}
						else if(!rules[k].domain_block.empty() && rules[k].domain_block != c_Domain){
							generalRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_UserID,c_Resp_IP,c_SetCookie,c_Domain);
							pageRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_Title,c_Uri,c_Resp_IP,c_SetCookie,c_Domain,c_StatusCode);
						}else if(rules[k].vecRangeIP.size() != 0){
							if(!rules[k].vecRangeIP[0].inRange(resp_IP) && !rules[k].vecRangeIP[0].inRange(source_IP)){	
								generalRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_UserID,c_Resp_IP,c_SetCookie,c_Domain);
								pageRules(rules[k],itSession.first->second,itSession.first->second.IP,c_RID,c_PageID,c_Title,c_Uri,c_Resp_IP,c_SetCookie,c_Domain,c_StatusCode);
							}
						}
					}
					//--------------------------------End--------------------------------------

					if(ifSessionBlock == false){
						if(itSession.first->second.vRequest.size() > 0){
							itSession.first->second.elapsed_ts = itSession.first->second.vRequest[itSession.first->second.vRequest.size()-1].ts - itSession.first->second.begin_ts; //session time.
						}
					}
				//}	
			}
			pthread_mutex_unlock(&mutexSessions);

			atts.clear();
			//if( itSession.first->second.sessionSize <= maxSession ){
				for(itAttr= to->mAttr.begin() ;itAttr!= to->mAttr.end();itAttr++ ){ // insert all the attribute of a page to the myAttMap.
					atts.push_back(itAttr->second);
				}
				it_atts = atts.begin();
			//}

			pthread_mutex_lock(&mutexBusinessLogic);
			if( itSession.first != mSession.end() ){	//Session was found.
				for(i_flow=0;i_flow<businessFlowVec.size();i_flow++){
					itRIDsPerFlow = itSession.first->second.RIDsPerFlow.find(businessFlowVec[i_flow].action_id);
					if( itRIDsPerFlow != itSession.first->second.RIDsPerFlow.end() ){
						if(c_PageID == businessFlowVec[i_flow].pages[itRIDsPerFlow->second.rids.size()].pageID){
							ActionParams aa(businessFlowVec[i_flow].pages[itRIDsPerFlow->second.rids.size()].params,businessFlowVec[i_flow].action_id,businessFlowVec[i_flow].action_name,businessFlowVec[i_flow].pages.size(),businessFlowVec[i_flow].logout);

							tmpAttributesVec.push_back(aa);
						}
					}else{
						if(businessFlowVec[i_flow].pages.size()>0 ){
							if(c_PageID == businessFlowVec[i_flow].pages[0].pageID ){
								ActionParams aa(businessFlowVec[i_flow].pages[0].params,businessFlowVec[i_flow].action_id,businessFlowVec[i_flow].action_name,businessFlowVec[i_flow].pages.size(),businessFlowVec[i_flow].logout);

								tmpAttributesVec.push_back(aa);
							}
						}
					}
				}
			}
			pthread_mutex_unlock(&mutexBusinessLogic);

			while(atts.size() != 0){
				tmp_att = *it_atts ;
			
				size_of_markov=0;
				len_score=0;	
				normal_flag='n';
				normal_flag_user='n';

				for(i_flow=0; i_flow<tmpAttributesVec.size() ;i_flow++) {
					itTmpAtts = tmpAttributesVec[i_flow].params.find(tmp_att.hash);
					if(itTmpAtts != tmpAttributesVec[i_flow].params.end() ){
						if(itTmpAtts->second == "*" || itTmpAtts->second.compare(tmp_att.value) == 0 ){
							tmpAttributesVec[i_flow].params.erase(tmp_att.hash);
						}
					}

					if(tmpAttributesVec[i_flow].params.size()==0){
						if(tmpAttributesVec[i_flow].logout == true && itSession.first->second.logout == false){
							itSession.first->second.logout = true;
							itSession.first->second.logout_counter++;
						}

						pthread_mutex_lock(&mutexBusinessLogic);
						itRIDsPerFlow2.first = itSession.first->second.RIDsPerFlow.find(tmpAttributesVec[i_flow].action_id);
						if( itRIDsPerFlow2.first == itSession.first->second.RIDsPerFlow.end() ){
							itRIDsPerFlow2 = itSession.first->second.RIDsPerFlow.insert(pair<unsigned int,alert>(tmpAttributesVec[i_flow].action_id,a));
						}

						itRIDsPerFlow2.first->second.rids.insert(c_RID);
						if(itRIDsPerFlow2.first->second.rids.size() == tmpAttributesVec[i_flow].numOfPages){

							ActionState fs(itSession.first->second.sid,tmpAttributesVec[i_flow].action_id,tmpAttributesVec[i_flow].action_name);

							itRIDsPerFlow2.first->second.rids.clear();
							itRIDsPerFlow2.first->second.ridAlert = c_RID;

							if(itRIDsPerFlow2.first->second.ridAlert != c_RID){
								for(k=0; k<rules.size() ;k++){
									if(rules[k].action_id == fs.action_id){
										itRIDsPerFlow2.first->second.ridAlert = c_RID;
										patternRulesBusiness(rules[k],itSession.first->second.IP,c_UserID,c_PageID,itSession.first->second,c_RID,c_Resp_IP,c_SetCookie,c_Domain);
									}
								}
							}

							pthread_mutex_unlock(&mutexBusinessLogic);

							if(fs.action_name.size() != 0){
								sprintf(attField,",\"business_actions_count\":1,\"business_actions\":[{\"name\":\"%s\",\"status\":2}]",&fs.action_name[0]);
								pthread_mutex_lock(&mutexRidAlertAndBusiness);
									itRidAlertAndAction = mRidAlertAndAction.find(c_RID);
									if( itRidAlertAndAction != mRidAlertAndAction.end() ){
										itRidAlertAndAction->second.action = attField;
									}else{
										ElasticData ed;
										ed.action = attField;
										mRidAlertAndAction.insert(pair<long long,ElasticData>(c_RID,ed));

									}
								pthread_mutex_unlock(&mutexRidAlertAndBusiness);
							}
						}else{
							pthread_mutex_unlock(&mutexBusinessLogic);
						}
					}

				}

				//Applications!!!!!!!!!!!!!!!!!!!!!!!!!!!
				pthread_mutex_lock(&mutexMyAttMap);
				itMyAttMap=myAttMap.find(tmp_att.hash);
				if(itMyAttMap == myAttMap.end()){
					pthread_mutex_unlock(&mutexMyAttMap);
					if(c_Mode==3){ //Hybrid mode
						myAtt att;//lock=1

						getAttributeThatWasntFound_Hybrid(att,tmp_att,c_PageID);
						pthread_mutex_lock(&mutexMyAttMap);
						myAttMap.insert ( pair<unsigned long long,myAtt>(tmp_att.hash,att) );
						pthread_mutex_unlock(&mutexMyAttMap);
					}

					atts.erase(it_atts);
					it_atts = atts.begin();
					tmp_att.analysis = 'u';
				}else{

					if(itMyAttMap->second.lock==1){
						pthread_mutex_unlock(&mutexMyAttMap);

						if( it_atts == (atts.end()-1) ){
							it_atts=atts.begin();
						}else{
							it_atts++;
						}
	
						continue;			
					}else{
						itMyAttMap->second.lock=1;
						pthread_mutex_unlock(&mutexMyAttMap);

						atts.erase(it_atts);
						it_atts = atts.begin();

					}


					size_of_markov = itMyAttMap->second.sizeMarkov.mean;

					pthread_mutex_lock(&mutexscorenumericatt);
					it_NumericAtt = mScoreNumericAtt.find(tmp_att.hash);
					if( it_NumericAtt!=mScoreNumericAtt.end() ){
						normal_flag='y';
					}else{
						// Calculate normal score for each attribute -> value and length.
						mScoreNumericAtt[tmp_att.hash].insert(mScoreAtt[tmp_att.hash]);
						//clean each object
						mScoreAtt[tmp_att.hash].clean();

						#ifdef DEBUG
							syslog(LOG_NOTICE,"  ---------------");
							syslog(LOG_NOTICE,"  %s",tmp_att.name.c_str());
							mScoreNumericAtt[tmp_att.hash].print_syslog();
						#endif

						if(itMyAttMap->second.countOfScores>10){
							itMyAttMap->second.weight = (-1)/( (itMyAttMap->second.sumOfScores/(double)itMyAttMap->second.countOfScores)-1 );
							if(itMyAttMap->second.type=='e' || itMyAttMap->second.type=='n'){
								itMyAttMap->second.weight = sqrt(itMyAttMap->second.weight);
							}

							if(itMyAttMap->second.kind=='H'){
								itMyAttMap->second.weight = sqrt(itMyAttMap->second.weight);
							}
						}else{
							itMyAttMap->second.weight = 0;
						}

						switch (itMyAttMap->second.type){
							case 'n':					//n=numeric.
								itMyAttMap->second.enumeration.seen.clear();
								itMyAttMap->second.tree.clean();
								break;
							case 't':					//t=text. F=Free text.
							case 'F':
								itMyAttMap->second.enumeration.seen.clear();
								break;
							case 'e':					//e=enumeration.
								itMyAttMap->second.tree.clean();
								break;
						}
					}
					pthread_mutex_unlock(&mutexscorenumericatt);
					if(c_Mode==3){
						getAttributeThatWasFound_Hybrid(itMyAttMap->second,tmp_att);
						if(itMyAttMap->second.newInHybrid=='y'){
							itMyAttMap->second.newCounterInHybrid++;
							if( itMyAttMap->second.newCounterInHybrid < MIN_VALUES){
								decideType(itMyAttMap->second,0,3);
							}
							else if( (itMyAttMap->second.newCounterInHybrid % MIN_VALUES) == 0 ){
								decideType(itMyAttMap->second,0,3);
							}
						}
					}

					tmp_att.analysis = itMyAttMap->second.type;
					switch (itMyAttMap->second.type) {
						case 'n':					//n=numeric analyzing by Chebyshev.
							ifNumeric(itMyAttMap->second,exp_score,tmp_att.vec_value,c_Mode,tmp_att.hash);
							break;
						case 't':					//t=text analyzing by Markov. F=Free text
						case 'F':
							ifTextOrFree(itMyAttMap->second,exp_score,tmp_att,c_Mode,len_score,tmp_att.vec_value,size_of_markov,tmp_att.hash );
							break;
						case 'e':					//e=enumeration analyzing by Covariance and Chebyshev.
						case 'u':
							//Lowercase
							string lower_val;
							for(unsigned int i=0;i<tmp_att.value.size();i++){
								if(tmp_att.value[i]>64 && tmp_att.value[i]<91){
									lower_val.push_back( tmp_att.value[i]+32 );
								}else{
									lower_val.push_back( tmp_att.value[i] );
								}
							}
							ifEnum(itMyAttMap->second,exp_score,tmp_att.vec_value,c_Mode,hashCode(lower_val),tmp_att.hash );
							break;
					}

					if(tmp_att.attribute_source!='H' && itMyAttMap->second.type=='e'){
						if(itMyAttMap->second.enumeration.seen.size() <= 20){

							map <unsigned int, Operation >::iterator itOperation = mOperation.find(c_PageID);
							if(itOperation != mOperation.end()){ // Page was found.
								map <unsigned long long,map<int,unsigned short> >::iterator itVecShape;
								itVecShape = itOperation->second.vecShape.find(tmp_att.hash);
								if( itVecShape != itOperation->second.vecShape.end() ){ // Attribute was found.
									map <int,unsigned short >::iterator itValues = itVecShape->second.find(hashCode(tmp_att.value) );
									if( itValues != itVecShape->second.end() ){ // value was found.
										map <long long, OpRID >::iterator itOpRIDs;

										pthread_mutex_lock(&mutexOperation);
										itOpRIDs = mOpRIDs.find(c_RID);
										if(itOpRIDs != mOpRIDs.end() ){
											switch (tmp_att.attribute_source){
												case 'J':	//JSON
												case 'X':	//XML
													itOpRIDs->second.opts[itValues->second] = (float)2/itVecShape->second.size();
													break;
												case 'P':	//POST , PUT
												case 'G':	//GET
												case 'R':	//REST
												case 'D':	//DELETE
													itOpRIDs->second.opts[itValues->second] = (float)1/itVecShape->second.size();
													break;
											}
											pthread_mutex_unlock(&mutexOperation);
										}else{
											pthread_mutex_unlock(&mutexOperation);
											OpRID optTmp(c_PageID);

											switch (tmp_att.attribute_source){
												case 'J':	//JSON
												case 'X':	//XML
													optTmp.opts[itValues->second] = (float)2/itVecShape->second.size();
													break;
												case 'P':	//POST , PUT
												case 'G':	//GET
												case 'R':	//REST
												case 'D':	//DELETE
													optTmp.opts[itValues->second] = (float)1/itVecShape->second.size();
													break;
											}
											pthread_mutex_lock(&mutexOperation);
											mOpRIDs.insert(pair<long long,OpRID >(c_RID,optTmp));
											pthread_mutex_unlock(&mutexOperation);
										}
									}
								}
							}

						}
					}

					itMyAttMap->second.lock=0;

					if(c_Mode==3){
						itMyAttMap->second.sumOfScores += exp_score;
						itMyAttMap->second.countOfScores++;
						if(itMyAttMap->second.countOfScores>10){
							itMyAttMap->second.weight = (-1)/( (itMyAttMap->second.sumOfScores/(double)itMyAttMap->second.countOfScores)-1 );
						}else{
							itMyAttMap->second.weight=0;
						}
					}
					tmpWeight = itMyAttMap->second.weight;
				}	

				unicodeParser(tmp_att.vec_value,tmp);
                                if(tmp_att.analysis=='u'){
                                        normalize_score=0;
                                }

				alert_score=0;
				if(itSession.first != mSession.end() ){	//Session was found.
					//-----------Rules---------
					attributeRules(false,tmp_att,itSession.first,to,tmp_att.vec_value,tmp,normal_flag,normal_flag_user,it_NumericAtt,it_NumericAttUser,exp_score,size_of_markov,c_IP,c_UserID,c_RID,itSession.first->second.sid,c_Title,alert_score,c_Resp_IP,c_SetCookie,c_Domain,tmp_att.value,c_StatusCode);
					//---------End Rules-------
				}

				normalize_score = alert_score/100 ;
				if(normal_flag=='y'){
					if(alert_score == 0){
						if(exp_score < it_NumericAtt->second.attribute.mean ){
							normalize_score = 1-it_NumericAtt->second.attribute.chebyshev(exp_score) ;
						}else{
							normalize_score=0;
						}
					}else{
						tmpWeight=1; // Maximum weight.
					}
				}else{
					if(tmp_att.analysis=='u'){
						normalize_score=0;
					}else{
						normalize_score=1;
						tmpWeight=1; // 
					}
				}

				Parameter h(normalize_score,len_score,tmpWeight);
				pthread_mutex_lock(&mutex_mQuery);
				mQuery[c_RID].push_back(h); //RID1 -> Attribute_Value_1, Attribute_Value_2, Attribute_Value_3 ... Attribute_Value_n
				pthread_mutex_unlock(&mutex_mQuery);

				if(tmp.size()==0){
					tmp="0";
				}

				pthread_mutex_lock(&mutexRidAtts);
				itRidAtts = mRidAtts.find(c_RID);
				if( itRidAtts != mRidAtts.end() ){			// The request wasn't found.
					maskPassword(tmp_att.hash,tmp_att.value);
					maskCreditCard(tmp_att.value);
					sprintf(attField,",{\"name\":\"%s\",\"value\":\"%s\",\"type\":\"%c\",\"score_data\":%f,\"score_length\":%f,\"weight\":%f,\"analysis\":\"%c\"}",&tmp_att.name[0],&tmp_att.value[0],tmp_att.attribute_source,100*normalize_score*tmpWeight,100*len_score*tmpWeight,tmpWeight,tmp_att.analysis);
					itRidAtts->second.attributes.append(attField);
					itRidAtts->second.counter_atts++;
				}else{							// The request was found.
					maskPassword(tmp_att.hash,tmp_att.value);
					maskCreditCard(tmp_att.value);
					sprintf(attField,"{\"name\":\"%s\",\"value\":\"%s\",\"type\":\"%c\",\"score_data\":%f,\"score_length\":%f,\"weight\":%f,\"analysis\":\"%c\"}",&tmp_att.name[0],&tmp_att.value[0],tmp_att.attribute_source,100*normalize_score*tmpWeight,100*len_score*tmpWeight,tmpWeight,tmp_att.analysis);
					ElasticAtt ea;
					ea.attributes = attField;
					mRidAtts.insert(pair<long long,ElasticAtt>(c_RID,ea));
				}

				pthread_mutex_unlock(&mutexRidAtts);

				//------------------------------Hybrid Mode------------------------------
				if(c_Mode==3){	
					if(exp_score!=MIN_PROB){
						if(normal_flag=='y'){
							pthread_mutex_lock(&mutexscorenumericatt);
							it_NumericAtt->second.attribute.update( (double)exp_score );
							pthread_mutex_unlock(&mutexscorenumericatt);
						}else{
							pthread_mutex_lock(&mutexscorenumericatt);
							mScoreNumericAtt[tmp_att.hash].attribute.update( (double)exp_score );
							pthread_mutex_unlock(&mutexscorenumericatt);
						}
						
					}
				}
				//-----------------------------------------------------------------------

				//********************************************Users INSERT******************************************
				vector <unsigned int>().swap(tmp_att.vec_value);
			}
			vector <ActionParams>().swap(tmpAttributesVec);

			production_values_flag[i]=2;
		}
	}
	vector <struct Attribute>().swap(atts);

	pthread_exit(NULL);
}


void *tokenize_thread(void *threadid)
{
	map <unsigned int,myAtt> ::iterator itMyAttMap; // iterator of myAttMap.
	unsigned long size,y;

	while(globalEngine)
	{
		sem_wait(&sem_tokenize1);
		for(itMyAttMap = myAttMap.begin(); itMyAttMap!=myAttMap.end();itMyAttMap++){  // itMyAttMap loop - beginning.
			if(itMyAttMap->second.type=='t' || itMyAttMap->second.type=='F'){ //text analyzing by Markov.
				size = itMyAttMap->second.values.size();
				for(y=0;y < size ; y++ ){
					if(itMyAttMap->second.val_s_f[y].tokenOrNot=='n' ){
						itMyAttMap->second.val_s_f[y].tokenOrNot='y';

						pthread_mutex_lock(&mutexMSatt);
						if(MSatt.count(itMyAttMap->second.RIDs[y]) != 0){
							pthread_mutex_unlock(&mutexMSatt);
							continue;
						}
						pthread_mutex_unlock(&mutexMSatt);

						itMyAttMap->second.tree.tokenize(itMyAttMap->second.values[y]);
						if(itMyAttMap->second.val_s_f[y].flag == 'y'){
							itMyAttMap->second.sizeMarkovVec.push_back( itMyAttMap->second.val_s_f[y].size );
						}
					}
				}
			}
		}
		pthread_mutex_lock(&mutexMSatt);
		MSatt.clear();
		pthread_mutex_unlock(&mutexMSatt);
		sem_post(&sem_tokenize2);
	}
	pthread_exit(NULL);
}

void *thread_app_was_updated(void *threadid)
{
	string app_name,output;
	CURL *curl;
	char url[200];
	boost::unordered_set <string> sName;
	boost::unordered_set <string>::iterator itnames;
	size_t pos = 0;
	
	while(1)
	{
		sleep(1);

		es_get_config("/telepath-config/config/app_list_was_changed_id/_source",output);
		if(output=="0"){
			continue;
		}
		
		pos=output.find("\"");
		output.erase(0,pos+1);
		while((pos = output.find("\",\"")) != string::npos){
			sName.insert(output.substr(0,pos));
			output.erase(0,pos+3);
		}
		pos = output.find("\"");
		sName.insert(output.substr(0,pos));
		es_insert("/telepath-config/config/app_list_was_changed_id","{\"value\":\"0\"}");	
		
		for (itnames=sName.begin(); itnames != sName.end(); itnames++){
			//syslog(LOG_NOTICE,"app_name: %s",(*itnames).c_str());
			app_name = (*itnames);
			snprintf(url,sizeof(url)-1,"%s/telepath-domains/domains/%s/_source",es_connect.c_str(),app_name.c_str());

			curl = curl_easy_init();
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"GET");
			curl_easy_setopt(curl, CURLOPT_URL,url);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, updateApp);
			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
		}
		sName.clear();

	}
	pthread_exit(NULL);
}


void *thread_expiration_date(void *threadid)
{
	unsigned int check_time,epoch;
	while (globalEngine){
		check_time = (unsigned int)time(NULL);
		if( validKey(license_key,epoch) == true ){
			if(check_time > 2678400 + epoch){
				syslog(LOG_NOTICE,"***Trial Version Expired***");
				es_insert("/telepath-config/config/license_mode_id","{\"value\":\"EXPIRED\"}");
				es_insert("/telepath-config/config/license_key_id","{\"value\":\"0\"}");
				check_license();
				globalEngine=0;
				sleep(30);
				exit(1);
			}else{
				es_insert("/telepath-config/config/license_mode_id","{\"value\":\"VALID\"}");
			}
		}else{
			syslog(LOG_NOTICE,"***Invalid License***");
			es_insert("/telepath-config/config/license_mode_id","{\"value\":\"INVALID\"}");

			globalEngine=0;
			sleep(30);
			exit(1);

		}

		sleep(5400); // 1.5 hours.
	}
	pthread_exit(NULL);
}


