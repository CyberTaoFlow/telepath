

using namespace std;

queue <RequestValToInsert> valReqQueue; //queue that insert to request table.

//Convert to fixxed utf8 syntax.
//Example: D7A8 --> %d7%a8 . 

void checkRules(Rule &,double,Session &,unsigned &,Numeric &,long long,string&,string&,string,string&); // push to SID the id of rule violation.
void checkRules(Rule &,double,Session &,unsigned &,long long,string&,string&,string,string&); // push to SID the id of rule violation.
//void insert_case(long long,unsigned int,string &,string&);
void insert_alert(Rule &,long long,string&,string&,string&,string&);
void insert_alert(Rule &,long long,string&,double,string&,string,string&);

class Markov{
public:
	int goodP;

	Markov(){
		this->goodP = 0;
	}
	
	void calculate(Path & path,Session & s,short host_user_or_group,short learn_or_pro,unsigned int I){
		double flow_score,query_score,geo_normal,landing_normal,landing_score=1,num2,diff;
		int totalExp=0,exp_2;
		RequestValToInsert reqVal;
		string country,city,compareLink;
		Coordinate coordinate;
		bool hostFlag;
		unsigned int i_rule;
		map <string,ScoresNumeric>::iterator itScoreNumeric;
		map <unsigned int, Operation >::iterator itOperation;
		map <long long, OpRID >::iterator itOpRIDs;

		map <string,NodeExtended>::iterator itNodeExtended;
		map <string,LinkExtended>::iterator itLinkExtended;
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

			if( (host_user_or_group==0 && s.vRequest[i].parsedHostPro==1) || (host_user_or_group==1 && s.vRequest[i].parsedUserPro==1) ){
				totalExp = s.totalExp;
				goto label_m;
			}
			
			if(sRidSaveDB.count(s.vRequest[i].RID)!=0){
				totalExp = s.totalExp;
				sRidSaveDB.erase(s.vRequest[i].RID);
				goto label_m;
			}

			if(learn_or_pro!=1){	// production or hybrid mode.
				//------------------------------Location-------------------------------
				pthread_mutex_lock(&mutexlocation);
				geo_normal = path.location.getProb(coordinate);
				pthread_mutex_unlock(&mutexlocation);
				//---------------------------------------------------------------------
			}else{			// learning mode.
				geo_normal = 0;
			}

			//--------------------------------Query--------------------------------
			pthread_mutex_lock(&mutex_mQuery);
			if(host_user_or_group==0){
				if(learn_or_pro!=1){ // production mode.
					Query q_page(mQuery[s.vRequest[i].RID],s.vRequest[i].presence);     //query probability
					query_score = q_page.result;
				}else{		  // learning mode.
					query_score = 0;
				}
			}else{
				if(learn_or_pro!=1){ // production mode.
					Query q_page(mQueryUser[s.vRequest[i].RID],s.vRequest[i].presence); //user query probability
					query_score = q_page.result;
				}else{		  // learning mode.
					query_score = 0;
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
			checkMapPage(s.vRequest[i].tainted,s.vRequest[i].status_code); // Tainted Page or not.
			itNodeExtended=path.mNodeExtended.find(s.vRequest[i].compare);
			if(itNodeExtended!=path.mNodeExtended.end() && s.vRequest[i].tainted==0){// The Page was found and the Page isn't tainted.

				if(learn_or_pro!=1){ // production mode.
					if(hostFlag==true){

						if(itScoreNumeric->second.flow.mean > (double)totalExp){
							flow_score = 1 - itScoreNumeric->second.flow.chebyshev( (double)totalExp );
						}else{
							flow_score = 0;
						}

						if(itScoreNumeric->second.landing.mean > landing_score  ){
							landing_normal = 1 - itScoreNumeric->second.landing.chebyshev( landing_score );
						}else{
							landing_normal = 0;
						}
					}else{
						flow_score = 1;
						landing_normal = 1;
					}
				}else{		  // learning mode.
					flow_score =  0;
					landing_normal = 0;
				}

//---------------------------------------------------------------Insert Requests to scores--------------------------------------------------------------------
				if(host_user_or_group==0){ // Per application.
					reqVal.init(s.vRequest[i].user,s.vRequest[i].RID,s.sid,s.vRequest[i].index,s.vRequest[i].page_name,s.vRequest[i].host_name,s.vRequest[i].ts,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,flow_score,query_score,geo_normal,landing_normal,s.vRequest[i].status_code,country,city,s.status,s.vRequest[i].protocol,s.vRequest[i].method,coordinate,s.decimalIP,s.vRequest[i].shard,s.vRequest[i].title,s.vRequest[i].presence,s.vRequest[i].canonical_url,learn_or_pro);

					pthread_mutex_lock(&mutexScoreData);		
					score_data[s.vRequest[i].domain_id].insert((double)totalExp,landing_score,geo_normal);
					pthread_mutex_unlock(&mutexScoreData);

					pthread_mutex_lock(&mutexInsertReq);
					valReqQueue.push(reqVal);
					pthread_mutex_unlock(&mutexInsertReq);
					sem_post(&sem_insert_reqs);
				}
//-------------------------------------------------------------Insert Requests to scores_user-----------------------------------------------------------------
				else if(host_user_or_group==1){ // Per User.
					//reqVal.init(s.vRequest[i].user,s.vRequest[i].RID,s.sid,s.vRequest[i].index,s.vRequest[i].page_name,s.vRequest[i].host_name,s.vRequest[i].ts,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,0,0,0,0,s.vRequest[i].status_code,country,city,s.status,s.vRequest[i].protocol,s.vRequest[i].method,coordinate,s.decimalIP,s.vRequest[i].shard,s.vRequest[i].title);
				}

			}else{ // The Page wasn't found or the Page is tainted.
//---------------------------------------------------------------Insert Requests to scores--------------------------------------------------------------------
				if(host_user_or_group==0){
					reqVal.init(s.vRequest[i].user,s.vRequest[i].RID,s.sid,s.vRequest[i].index,s.vRequest[i].page_name,s.vRequest[i].host_name,s.vRequest[i].ts,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,1,query_score,geo_normal,1,s.vRequest[i].status_code,country,city,s.status,s.vRequest[i].protocol,s.vRequest[i].method,coordinate,s.decimalIP,s.vRequest[i].shard,s.vRequest[i].title,s.vRequest[i].presence,s.vRequest[i].canonical_url,learn_or_pro);

					pthread_mutex_lock(&mutexInsertReq);
					valReqQueue.push(reqVal);
					pthread_mutex_unlock(&mutexInsertReq);
					sem_post(&sem_insert_reqs);
				}
//-------------------------------------------------------------Insert Requests to scores_user-----------------------------------------------------------------
				else if(host_user_or_group==1){
					//reqVal.init(s.vRequest[i].user,s.vRequest[i].RID,s.sid,s.vRequest[i].index,s.vRequest[i].page_name,s.vRequest[i].host_name,s.vRequest[i].ts,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,0,0,0,0,s.vRequest[i].status_code,country,city,s.status,s.vRequest[i].protocol,s.vRequest[i].method,coordinate,s.decimalIP,s.vRequest[i].shard,s.vRequest[i].title);
				}
			}

			if(learn_or_pro!=1){ // Rules
				for(i_rule=0 ; i_rule<rules.size() ; i_rule++ ){

					/*if(rules[i_rule].radius != 0){
						if(s.status=='v'){
							velocityAlert(s,rules[i_rule],i);
						}
						continue;	
					}
					if(rules[i_rule].final_type == 'N'){ // N=Network IP Change.  
						if(s.status=='v'){
							ipSwitching(s,rules[i_rule],i);
						}
						continue;
					}*/

					// CHECK THE BUG IN THIS FLAG FOR SEPERATED APPS
					if(hostFlag==true){
						switch (rules[i_rule].method[0]) {
							case 'l':							//landing rules.
								checkRules(rules[i_rule],landing_score,s,i,itScoreNumeric->second.landing,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'q':							//query rules. 
								checkRules(rules[i_rule],query_score ,s,i,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'g':							//geo rules. 
								checkRules(rules[i_rule],geo_normal,s,i,itScoreNumeric->second.geo,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'f':							//flow rules.
								checkRules(rules[i_rule],totalExp ,s,i,itScoreNumeric->second.flow,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'a':							//average rules. 
								checkRules(rules[i_rule],reqVal.avg_normal ,s,i,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
							case 'p':
								checkRules(rules[i_rule],s.vRequest[i].presence ,s,i,s.vRequest[i].RID,s.vRequest[i].user_ip,s.vRequest[i].resp_ip,"phpsessid",s.vRequest[i].host_name);
								break;
						}
					}
				}
			}

			if(host_user_or_group==0){ // per application.
				s.vRequest[i].parsedHostPro=1;
			}
			else if(host_user_or_group==1){ // per user in application.
				s.vRequest[i].parsedUserPro=1;
			}
			label_m:

			if(i == s.vRequest.size()-1){ // last page in session.
				s.totalExp = totalExp;				
				continue;
			}

			compareLink.clear();
			compareLink.append(s.vRequest[i].compare);
                        compareLink.push_back('|');
                        compareLink.append(s.vRequest[i+1].compare);

			itLinkExtended=path.mLinkExtended.find(compareLink);
			if(itLinkExtended!=path.mLinkExtended.end()){//Link was found
				Numeric num_1;
				num_1.init(itLinkExtended->second.diffLanding);

				diff = (double)(s.vRequest[i+1].ts - s.vRequest[i].ts);
				if( (diff < num_1.mean && slowOrFastLanding==0) || (diff > num_1.mean && slowOrFastLanding==2) ){ // don't analyze fast speed or slow speed. 
					landing_score = 1;
				}else{
					landing_score = num_1.chebyshev(diff)	; //landing_score	
				}

				num2 = (double)(  ((double)(itLinkExtended->second.emission))  /  ((double)( path.mNodeExtended[itLinkExtended->second.from_page_comp].link_sample ))  );
				frexp( num2, &(exp_2) );
				if (exp_2 > 0){exp_2=0;}
				totalExp = exp_2;   //flow_score_exponent

			}else{//Link wasn't found
				landing_score=0;
				totalExp=MIN_PROB;
			}
		}//2 for end

		if(s.vRequest.size() != 0 ){
			vector< Page >().swap(s.vRequest);
		}

	}

};

