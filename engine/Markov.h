
using namespace std;

queue <RequestValToInsert> valReqQueue; //queue that insert to request table.

//Convert to fixxed utf8 syntax.
//Example: D7A8 --> %d7%a8 . 

void checkRules(Rule &,double,Session &,unsigned &,Numeric &,long long,string&,string&,string,string&); // push to SID the id of rule violation.
void checkRules(Rule &,double,Session &,unsigned &,long long,string&,string&,string,string&); // push to SID the id of rule violation.
//void insert_case(long long,unsigned int,string &,string&);
void insert_alert(Rule &,long long,string&,string&,string&,string&);
void insert_alert(Rule &,long long,string&,double,string&,string,string&);
void ipSwitching(Rule &, Session &,unsigned &,long long,string &p,string &,string,string &);
class Markov{
public:
	int goodP;

	Markov(){
		this->goodP = 0;
	}
	
	void calculate(Path & path,Path & path_user,Session & s,short learn_or_pro){
		double rule_score,flow_score,query_score,geo_normal,geo_normal_user,landing_normal,landing_score=1,num2,avg_score_user;
		int flow_exp;
		RequestValToInsert reqVal;
		string country,city,user_scores_string;
		char user_scores[2000],tmp[200];
		Coordinate coordinate;
		bool hostFlag;
		unsigned int i_rule;
		map <string,ScoresNumeric>::iterator itScoreNumeric;
		map <unsigned int, Operation >::iterator itOperation;
		map <long long, OpRID >::iterator itOpRIDs;
		map <string,CompressedPage>::iterator itCompressedPage;
		map <string,CompressedLink>::iterator itCompressedLink;
		Numeric numeric;

		for(unsigned int i=0 ;i < s.vRequest.size() ; i++){//2 for
			itScoreNumeric = score_numeric.find(s.vRequest[i].domain_id);
			if(itScoreNumeric != score_numeric.end() ){ // hostname was found.
				hostFlag=true;
			}else{				   	    // hostname wasn't found.
				hostFlag=false;
			}

			pthread_mutex_lock(&mutexgeoip);
			getIpInfo(s.vRequest[i].user_ip,country,city,coordinate);
			pthread_mutex_unlock(&mutexgeoip);

			if( s.vRequest[i].parsedHostPro==1 ){
				continue;
			}

			pthread_mutex_lock(&mutexRidAlertAndBusiness);
			if(sRidSaveDB.count(s.vRequest[i].RID)!=0){
				sRidSaveDB.erase(s.vRequest[i].RID);
				mRidAlertAndAction.erase(s.vRequest[i].RID);
				pthread_mutex_unlock(&mutexRidAlertAndBusiness);
				continue;
			}
			pthread_mutex_unlock(&mutexRidAlertAndBusiness);

			if(learn_or_pro!=1){	// production or hybrid mode.
				//------------------------------Location-------------------------------
				pthread_mutex_lock(&mutexlocation);
				geo_normal = path.location.getProb(coordinate);
				pthread_mutex_unlock(&mutexlocation);

				if (s.vRequest[i].user.size()>0){
					pthread_mutex_lock(&mutexlocation);
					geo_normal_user = path_user.location.getProb(coordinate);
					pthread_mutex_unlock(&mutexlocation);

					sprintf(user_scores,"\"user_scores\":{\"score_geo\":%f,",geo_normal_user);
					avg_score_user = geo_normal_user;
				}
				//---------------------------------------------------------------------
			}else{			// learning mode.
				geo_normal = 0;
				if (s.vRequest[i].user.size()>0){
					geo_normal_user = 0;
					sprintf(user_scores,"\"user_scores\":{\"score_geo\":0,");
				}
			}

			//--------------------------------Query--------------------------------
			pthread_mutex_lock(&mutex_mQuery);
				if(learn_or_pro!=1){ // production mode.
					Query q_page(mQuery[s.vRequest[i].RID],s.vRequest[i].presence);     //query probability
					query_score = q_page.result;
					if (s.vRequest[i].user.size()>0){
						sprintf(tmp,"\"score_query\":%f,",q_page.result);
						strcat(user_scores,tmp);
						avg_score_user += q_page.result;
					}
				}else{		  // learning mode.
					query_score = 0;
					if (s.vRequest[i].user.size()>0){
						sprintf(tmp,"\"score_query\":0,");
						strcat(user_scores,tmp);
					}
				}

			pthread_mutex_unlock(&mutex_mQuery);

			itOperation = mOperation.find(s.vRequest[i].ID);
			if( itOperation != mOperation.end() ){
				if(itOperation->second.dbscan.clusters.size() > 0){
					itOpRIDs = mOpRIDs.find(s.vRequest[i].RID);
					if(itOpRIDs != mOpRIDs.end()){
						unsigned short loc = itOperation->second.dbscan.returnClosestCluster(itOpRIDs->second.opts);
						char buffer[8];
						buffer[0] =':';
						itoa3(loc,buffer+1);
						s.vRequest[i].compare.append(buffer);
					}
				}
			}

			//---------------------------------------------------------------------
			if(s.vRequest[i].index!=0){
				itCompressedLink=path.mCompressedLink.find(s.vRequest[i].compare_link);
				if(itCompressedLink!=path.mCompressedLink.end()){//Link was found
					numeric.clean();
					numeric.init(itCompressedLink->second.diffLanding);

					if( (s.vRequest[i].diff_speed < numeric.mean && slowOrFastLanding==0) || (s.vRequest[i].diff_speed > numeric.mean && slowOrFastLanding==2) ){ // don't analyze fast speed or slow speed. 
						landing_score = 1;
					}else{
						landing_score = numeric.chebyshev(s.vRequest[i].diff_speed);
					}

					if(path.sampleP.size() > (s.vRequest[i].index) || (path.sampleP[s.vRequest[i].index-1]==0) ){
						num2 = (double)(  ((double)(itCompressedLink->second.emission))  /  ((double)( path.sampleP[s.vRequest[i].index] ))  );
						frexp( num2, &(flow_exp) );
						if (flow_exp > 0){flow_exp=0;}
						flow_exp = flow_exp;   //flow_score_exponent
					}else{
						flow_exp=MIN_PROB;
					}
				}else{//Link wasn't found
					landing_score=0;
					flow_exp=MIN_PROB;
				}
			}else{
				flow_exp=0;
			}

			checkMapPage(s.vRequest[i].tainted,s.vRequest[i].status_code); // Tainted Page or not.
			itCompressedPage=path.mCompressedPage.find(s.vRequest[i].compare);
			if(itCompressedPage!=path.mCompressedPage.end() && s.vRequest[i].tainted==0){// The Page was found and the Page isn't tainted.
				if(learn_or_pro!=1){ // production mode.
					if(hostFlag==true){

						if(itScoreNumeric->second.flow.mean > (double)flow_exp){
							flow_score = 1 - itScoreNumeric->second.flow.chebyshev( (double)flow_exp );
						}else{
							flow_score = 0;
						}

						if(itScoreNumeric->second.landing.mean > landing_score  ){
							landing_normal = 1 - itScoreNumeric->second.landing.chebyshev( landing_score );
						}else{
							landing_normal = 0;
						}
						
						if (s.vRequest[i].user.size()>0){
							avg_score_user += flow_score;
							avg_score_user += landing_normal;
							avg_score_user /= 4;
							sprintf(tmp,"\"score_flow\":%f,\"score_landing\":%f,\"score_average\":%f},",flow_score,landing_normal,avg_score_user);
							strcat(user_scores,tmp);
						}
					}else{
						flow_score = 1;
						landing_normal = 1;

						if (s.vRequest[i].user.size()>0){
							avg_score_user += 2;
							avg_score_user /= 4;
							sprintf(tmp,"\"score_flow\":1,\"score_landing\":1,\"score_average\":%f},",avg_score_user);
							strcat(user_scores,tmp);
						}
					}
				}else{		  // learning mode.
					flow_score =  0;
					landing_normal = 0;
					if (s.vRequest[i].user.size()>0){
						avg_score_user /= 4;
						sprintf(tmp,"\"score_flow\":0,\"score_landing\":0,\"score_average\":%f},",avg_score_user);
						strcat(user_scores,tmp);
					}
				}

				//Insert Request.
				user_scores_string = string(user_scores);
				reqVal.init(s.vRequest[i].user,s.vRequest[i].RID,s.sid,s.vRequest[i].sha256_sid,s.vRequest[i].index,s.vRequest[i].page_name,s.vRequest[i].host_name,s.vRequest[i].ts,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,flow_score,query_score,geo_normal,landing_normal,s.vRequest[i].status_code,country,city,s.status,s.vRequest[i].protocol,s.vRequest[i].method,coordinate,s.decimalIP,s.vRequest[i].shard,s.vRequest[i].title,s.vRequest[i].presence,s.vRequest[i].canonical_url,learn_or_pro,user_scores_string);

				pthread_mutex_lock(&mutexScoreData);		
				score_data[s.vRequest[i].domain_id].insert((double)flow_exp,landing_score,geo_normal);
				pthread_mutex_unlock(&mutexScoreData);

				pthread_mutex_lock(&mutexInsertReq);
				valReqQueue.push(reqVal);
				pthread_mutex_unlock(&mutexInsertReq);
				sem_post(&sem_insert_reqs);
			}else{ // The Page wasn't found or the Page is tainted.
				//Insert Request.
				user_scores_string = string(user_scores);
				reqVal.init(s.vRequest[i].user,s.vRequest[i].RID,s.sid,s.vRequest[i].sha256_sid,s.vRequest[i].index,s.vRequest[i].page_name,s.vRequest[i].host_name,s.vRequest[i].ts,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,1,query_score,geo_normal,1,s.vRequest[i].status_code,country,city,s.status,s.vRequest[i].protocol,s.vRequest[i].method,coordinate,s.decimalIP,s.vRequest[i].shard,s.vRequest[i].title,s.vRequest[i].presence,s.vRequest[i].canonical_url,learn_or_pro,user_scores_string);

				pthread_mutex_lock(&mutexInsertReq);
				valReqQueue.push(reqVal);
				pthread_mutex_unlock(&mutexInsertReq);
				sem_post(&sem_insert_reqs);
			}

			for(i_rule=0 ; i_rule<rules.size() ; i_rule++ ){
				if(rules[i_rule].final_type == 'G'){
					if(s.status=='v'){
						ipSwitching(rules[i_rule],s,i,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
					}
					continue;	
				}

				if(learn_or_pro != 1){ // Rules
					if(hostFlag==true){
						switch (rules[i_rule].method[0]) {
							case 'l':							//landing rules.
								rule_score=landing_score;
								checkRules(rules[i_rule],rule_score,s,i,itScoreNumeric->second.landing,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'q':							//query rules.
								checkRules(rules[i_rule],query_score ,s,i,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'g':							//geo rules.
								if(rules[i_rule].personal==0){
									rule_score=geo_normal;
								}else{
									rule_score=geo_normal_user;
								} 
								checkRules(rules[i_rule],rule_score,s,i,itScoreNumeric->second.geo,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'f':							//flow rules.
								rule_score=flow_score;
								checkRules(rules[i_rule],rule_score,s,i,itScoreNumeric->second.flow,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'a':							//average rules. 
								if(rules[i_rule].personal==0){
									rule_score=reqVal.avg_normal;
								}else{
									rule_score=avg_score_user;
								}
								checkRules(rules[i_rule],rule_score ,s,i,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'p':
								checkRules(rules[i_rule],s.vRequest[i].presence ,s,i,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
						}
					}
				}
			}

			s.vRequest[i].parsedHostPro=1;

		}//2 for end

		if(s.vRequest.size() != 0 ){
			vector< Page >().swap(s.vRequest);
		}

	}

};

