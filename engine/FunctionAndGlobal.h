
using namespace std;

short isTheRuleMatch(unsigned short,Numeric &,double,double &,short);
bool ipABClasses(string &,string &);

struct cookieNames{  // values of attributes score.
	map <string,unsigned int> names;
	int counter;
};

map <string,cookieNames > mCookieStat;

bool excludeIP(Range & range,unsigned int decimalIP,unsigned short ip_neg){
	if(range.to == 0){
		return true;
	}

	bool inRangeFlag = range.inRange(decimalIP);

	if(inRangeFlag == true){
		if(ip_neg==0){
			return true;
		}else{
			return false;
		}
	}else{
		if(ip_neg==0){
			return false;
		}else{
			return true;
		}
	}

}

void checkRules( Rule & c_r , double score , Session & s,unsigned & index,Numeric & numeric,long long RID,string & src_ip,string & resp_ip,string cookie,string & hostname){ // push to SID the id of rule violation.
	double num_score=0;
	string lit_score; 

	score *= 100;

	bool flagIP=false;
	for(unsigned int iii=0;iii<c_r.vecRangeIP.size();iii++){
		if( excludeIP(c_r.vecRangeIP[iii],s.decimalIP,c_r.ip_neg) == true){
			flagIP=true;
			break;
		}
	}

	if( (s.elapsed_ts>=c_r.ts) && /*(s.vRequest[index].index>=c_r.Index)  && ( !c_r.App || c_r.App== s.vRequest[index].hostname ) &&*/ (c_r.user.size()==0 || c_r.user.compare(s.vRequest[0].user) == 0 ) && flagIP==false ){
		if(isTheRuleMatch(c_r.threshold ,numeric,score,num_score,0) == 0){
			insert_alert(c_r,RID,src_ip,num_score,resp_ip,cookie,hostname);
		}
	}
}

void checkRules( Rule & c_r , double score , Session & s,unsigned & index,long long RID,string & src_ip,string & resp_ip,string cookie,string & hostname){ // push to SID the id of rule violation.
	score *= 100;

	bool flagIP=false;
	for(unsigned int iii=0;iii<c_r.vecRangeIP.size();iii++){
		if( excludeIP(c_r.vecRangeIP[iii],s.decimalIP,c_r.ip_neg) == true){
			flagIP=true;
			break;
		}
	}

	if( (s.elapsed_ts>=c_r.ts) && /*(s.vRequest[index].index>=c_r.Index)  && ( !c_r.App || c_r.App== s.vRequest[index].hostname ) &&*/ (c_r.user.size()==0 || c_r.user.compare(s.vRequest[0].user)==0 ) && flagIP==false ){
		if ( c_r.threshold < score) { // Check score matching.
			insert_alert(c_r,RID,src_ip,score,resp_ip,cookie,hostname);
		}

	}
}

short isTheRuleMatch(unsigned short threshold,Numeric & numeric,double score,double & num_score,short constant){
// return 0 or 1  ->   0=Match , 1=No Match . 
	if(constant==1){
		return 0;
	}

	num_score = 100*(1-numeric.chebyshev(score) ) ;
	if(threshold>100){ 			// Literal score.
		short std = threshold-100;	// low=1 std , med=2 std , high=3 std. 

		if( !(score < numeric.mean - std*numeric.stddev ) ){ 
			return 1;		// 			
		}
		return 0;			
	}else{					// Numeric score.

		if ( !(threshold < num_score ) ){
			return 1;
		}
		return 0;
	}	
}

// For levenshteinDistance.
// Get 3 numbers and return the smallest value. 
unsigned short minimum(unsigned short min_num,unsigned short b,unsigned short c){

	if( b < min_num){
		min_num = b;
	}
	if( c < min_num){
		min_num = c;
	}

	return min_num;

}

// For all i and j, mat[i][j] will hold the Levenshtein distance between
// the first i characters of v1 and the first j characters of v2.
// Note that mat has (m+1)(n+1) values.
unsigned short levenshteinDistance(vector <unsigned int> v1,vector <unsigned int> & v2)
{
	unsigned short s1 = (unsigned short)v1.size();
	unsigned short s2 = (unsigned short)v2.size();
	unsigned short * mat = (unsigned short*)malloc( (sizeof(unsigned short)*((s1+1)*(s2+1))) + 2 );

	if (mat==NULL) return 0;;
	memset(mat,'\0', (sizeof(unsigned short)*((s1+1)*(s2+1))) + 2 );

	for(unsigned short i=0 ; i < s2+1 ; i++ ){
		*(mat+i*(s1+1) ) = i;
	}
	
	for(unsigned short j=0; j < s1+1 ; j++ ){		
		*(mat+j) = j;
	}


	for(unsigned short j=1; j < s1+1 ; j++ ){
		for(unsigned short i=1; i < s2+1 ; i++ ){
			if(v1[i-1]==v2[j-1]){
				*(mat+(s1+1)*i+j) = *(mat+(i-1)*(s1+1) + j-1) ;
			}	
			else{
				*(mat+(s1+1)*i+j) = minimum( *(mat+(i-1)*(s1+1)+j) + 1 , *(mat+i*(s1+1)+j-1) + 1 , *(mat+(i-1)*(s1+1) + j-1) + 1 );
			}
		}
	}
	unsigned short result = *(mat+(s1+1)*(s2+1)-1 );
	free(mat);

	return result;

}

void ipSwitching(Session & s,Rule & c_r,unsigned int & i){
	if(i > 1){
		if( ipABClasses(s.vRequest[i].user_ip,s.vRequest[i-1].user_ip) == false ){
			//string rule_name="Network IP Change";
			//insert_alert(s,c_r,IP);
		}
	}
}

void velocityAlert(Session & s,Rule & c_r,unsigned int & i){
	Coordinate coordinate(0,0),coordinate2(0,0);
	if(i > 1){
		pthread_mutex_lock(&mutexgeoip);
		coordinate = getCoordinate(s.vRequest[i].user_ip);
		coordinate2 = getCoordinate(s.vRequest[i-1].user_ip);
		pthread_mutex_unlock(&mutexgeoip);
		if(coordinate.key != coordinate2.key){
			double diff;
			if(s.vRequest[i].ts<=s.vRequest[i-1].ts){
				diff=0;
			}else{
				diff = s.vRequest[i].ts-s.vRequest[i-1].ts;
			}

			if(diff < c_r.ts ){
				if(c_r.radius < Pythagoras( coordinate,coordinate2) ){
					//string rule_name="Geo Velocity";
					//insert_alert(s,c_r,IP);
				}	
			}		
		}
	}
}

void insert_alert(Rule & c_r,long long RID,string & src_ip,string & resp_ip,string & cookie,string & hostname){
	char alertfields[300];
	boost::unordered_map <long long,ElasticData>::iterator itRidAlertAndAction;

	if(c_r.db_save==true){
		pthread_mutex_lock(&mutexRidAlertAndBusiness);
		itRidAlertAndAction = mRidAlertAndAction.find(RID);
		if( itRidAlertAndAction != mRidAlertAndAction.end() ){
			boost::unordered_map<string,boost::unordered_set<unsigned int> >::iterator itRuleName = itRidAlertAndAction->second.rule_names.find(c_r.rule_name);
			if( itRuleName == itRidAlertAndAction->second.rule_names.end() ){
				if(c_r.criteria_count>1){
					boost::unordered_set <unsigned int> tmp_set;
					tmp_set.insert(c_r.criterion_hash);
					itRidAlertAndAction->second.rule_names.insert(pair<string,boost::unordered_set <unsigned int> >(c_r.rule_name,tmp_set));
				}else{
					if(itRidAlertAndAction->second.counter_alerts==0){
						sprintf(alertfields,",\"alerts\":[{\"name\":\"%s\",\"score\":%u}",&c_r.rule_name[0],c_r.threshold);
					}else{
						sprintf(alertfields,",{\"name\":\"%s\",\"score\":%u}",&c_r.rule_name[0],c_r.threshold);
					}
					itRidAlertAndAction->second.counter_alerts++;
					itRidAlertAndAction->second.alerts.append(alertfields);
					itRidAlertAndAction->second.threshold = (double)c_r.threshold/100;
					boost::unordered_set <unsigned int> tmp_set;
					tmp_set.insert(c_r.criterion_hash);
					itRidAlertAndAction->second.rule_names.insert(pair<string,boost::unordered_set <unsigned int> >(c_r.rule_name,tmp_set));
				}
			}else{
				if(c_r.criteria_count>1){
					itRuleName->second.insert(c_r.criterion_hash);
					if(itRuleName->second.size() == c_r.criteria_count){
						if(itRidAlertAndAction->second.counter_alerts==0){
							sprintf(alertfields,",\"alerts\":[{\"name\":\"%s\",\"score\":%u}",&c_r.rule_name[0],c_r.threshold);
						}else{
							sprintf(alertfields,",{\"name\":\"%s\",\"score\":%u}",&c_r.rule_name[0],c_r.threshold);
						}
						itRidAlertAndAction->second.counter_alerts++;
						itRidAlertAndAction->second.alerts.append(alertfields);
						itRidAlertAndAction->second.threshold = (double)c_r.threshold/100;
					}
				}
			}
		}else{
			if(c_r.criteria_count>1){
				ElasticData ed;
				boost::unordered_set <unsigned int> tmp_set;
				tmp_set.insert(c_r.criterion_hash);
				ed.rule_names.insert(pair<string,boost::unordered_set <unsigned int> >(c_r.rule_name,tmp_set));
				mRidAlertAndAction.insert(pair<long long,ElasticData>(RID,ed));
			}else{
				ElasticData ed;
				sprintf(alertfields,",\"alerts\":[{\"name\":\"%s\",\"score\":%u}",&c_r.rule_name[0],c_r.threshold);
				ed.alerts = alertfields;
				ed.counter_alerts++;
				boost::unordered_set <unsigned int> tmp_set;
				tmp_set.insert(c_r.criterion_hash);
				ed.rule_names.insert(pair<string,boost::unordered_set <unsigned int> >(c_r.rule_name,tmp_set));
				ed.threshold = (double)c_r.threshold/100;
				mRidAlertAndAction.insert(pair<long long,ElasticData>(RID,ed));

				pthread_mutex_lock(&mutexAppMode);
					if(mAppMode[hostname].counter>0){
						mAppMode[hostname].counter--;
					}
				pthread_mutex_unlock(&mutexAppMode);
			}
		}
		pthread_mutex_unlock(&mutexRidAlertAndBusiness);
	}

	for(unsigned int i=0 ; i<c_r.cmds.size() ; i++){
		if(c_r.cmds[i] == "captcha"){
			sprintf(alertfields,"CAPTCHA_%s",(char*)src_ip.c_str());

			pthread_mutex_lock(&mutexCaptcha);
			valCaptchaQueue.push(alertfields);
			pthread_mutex_unlock(&mutexCaptcha);
			sem_post(&sem_captcha);
		}
		else if(c_r.cmds[i] == "block"){
			sprintf(alertfields,"BLOCK_%s",(char*)src_ip.c_str());

			pthread_mutex_lock(&mutexCaptcha);
			valCaptchaQueue.push(alertfields);
			pthread_mutex_unlock(&mutexCaptcha);
			sem_post(&sem_captcha);
		}else{
			if(cookie.size()==0){
				cookie="null";
			}
			sprintf(alertfields,"%s %s %s %s %s",(char*)c_r.cmds[i].c_str(),(char*)src_ip.c_str(),(char*)resp_ip.c_str(),(char*)cookie.c_str(),(char*)hostname.c_str());

			pthread_mutex_lock(&mutexCommand);
			valCommandQueue.push(alertfields);
			pthread_mutex_unlock(&mutexCommand);
			sem_post(&sem_command);
		}
	}
}

void insert_alert(Rule & c_r,long long RID,string & src_ip,double score,string & resp_ip,string cookie,string & hostname){
	char alertfields[300];
	boost::unordered_map <long long,ElasticData>::iterator itRidAlertAndAction;

	if(c_r.db_save==true){
		pthread_mutex_lock(&mutexRidAlertAndBusiness);
		itRidAlertAndAction = mRidAlertAndAction.find(RID);
		if( itRidAlertAndAction != mRidAlertAndAction.end() ){
			boost::unordered_map<string,boost::unordered_set<unsigned int> >::iterator itRuleName = itRidAlertAndAction->second.rule_names.find(c_r.rule_name);
			if( itRuleName == itRidAlertAndAction->second.rule_names.end() ){
				if(c_r.criterion_hash>1){
					boost::unordered_set <unsigned int> tmp_set;
					tmp_set.insert(c_r.criterion_hash);
					itRidAlertAndAction->second.rule_names.insert(pair<string,boost::unordered_set <unsigned int> >(c_r.rule_name,tmp_set));
				}else{
					if(itRidAlertAndAction->second.counter_alerts==0){
						sprintf(alertfields,",\"alerts\":[{\"name\":\"%s\",\"score\":%f}",&c_r.rule_name[0],score);
					}else{
						sprintf(alertfields,",{\"name\":\"%s\",\"score\":%f}",&c_r.rule_name[0],score);
					}
					itRidAlertAndAction->second.counter_alerts++;
					itRidAlertAndAction->second.alerts.append(alertfields);

					boost::unordered_set <unsigned int> tmp_set;
					tmp_set.insert(c_r.criterion_hash);
					itRidAlertAndAction->second.rule_names.insert(pair<string,boost::unordered_set <unsigned int> >(c_r.rule_name,tmp_set));
				}
			}else{
				if(c_r.criterion_hash>1){
					itRuleName->second.insert(c_r.criterion_hash);
					if(itRuleName->second.size() == c_r.criteria_count){
						if(itRidAlertAndAction->second.counter_alerts==0){
							sprintf(alertfields,",\"alerts\":[{\"name\":\"%s\",\"score\":%f}",&c_r.rule_name[0],score);
						}else{
							sprintf(alertfields,",{\"name\":\"%s\",\"score\":%f}",&c_r.rule_name[0],score);
						}
						itRidAlertAndAction->second.counter_alerts++;
						itRidAlertAndAction->second.alerts.append(alertfields);
					}
				}
			}
		}else{
			if(c_r.criteria_count>1){
				ElasticData ed;
				boost::unordered_set <unsigned int> tmp_set;
				tmp_set.insert(c_r.criterion_hash);
				ed.rule_names.insert(pair<string,boost::unordered_set <unsigned int> >(c_r.rule_name,tmp_set));
				mRidAlertAndAction.insert(pair<long long,ElasticData>(RID,ed));
			}else{
				ElasticData ed;
				sprintf(alertfields,",\"alerts\":[{\"name\":\"%s\",\"score\":%f}",&c_r.rule_name[0],score);
				ed.alerts = alertfields;
				ed.counter_alerts++;
				boost::unordered_set <unsigned int> tmp_set;
				tmp_set.insert(c_r.criterion_hash);
				ed.rule_names.insert(pair<string,boost::unordered_set <unsigned int> >(c_r.rule_name,tmp_set));;
				mRidAlertAndAction.insert(pair<long long,ElasticData>(RID,ed));

				pthread_mutex_lock(&mutexAppMode);
					if(mAppMode[hostname].counter>0){
						mAppMode[hostname].counter--;
					}
				pthread_mutex_unlock(&mutexAppMode);
			}
		}
		pthread_mutex_unlock(&mutexRidAlertAndBusiness);
	}

	for(unsigned int i=0 ; i<c_r.cmds.size() ; i++){
		if(c_r.cmds[i] == "captcha"){
			sprintf(alertfields,"CAPTCHA_%s",(char*)src_ip.c_str());

			pthread_mutex_lock(&mutexCaptcha);
			valCaptchaQueue.push(alertfields);
			pthread_mutex_unlock(&mutexCaptcha);
			sem_post(&sem_captcha);
		}
		else if(c_r.cmds[i] == "block"){
			sprintf(alertfields,"BLOCK_%s",(char*)src_ip.c_str());

			pthread_mutex_lock(&mutexCaptcha);
			valCaptchaQueue.push(alertfields);
			pthread_mutex_unlock(&mutexCaptcha);
			sem_post(&sem_captcha);
		}else{
			if(cookie.size()==0){
				cookie="null";
			}
			sprintf(alertfields,"%s %s %s %s %s",(char*)c_r.cmds[i].c_str(),(char*)src_ip.c_str(),(char*)resp_ip.c_str(),(char*)cookie.c_str(),(char*)hostname.c_str());

			pthread_mutex_lock(&mutexCommand);
			valCommandQueue.push(alertfields);
			pthread_mutex_unlock(&mutexCommand);
			sem_post(&sem_command);
		}
	}
}

/*
void insert_case(long long RID,unsigned int ip,string & country,string & hostname){
	bool caseFlag;
	bool empty = false;
	char casefields[300];
	string casearray;
	boost::unordered_map <long long,ElasticData>::iterator itRidAlertAndAction;
	unsigned int j,jj,counter_cases = 0;

	for(j=0;j<vCases.size();j++){
		caseFlag=true;
		for(jj=0;jj<vCases[j].subcase.size();jj++){
			if(vCases[j].subcase[jj].isMatch(ip,country,hostname) == false){
				caseFlag=false;
				break;
			}
		}
		if(caseFlag==true){
			if(empty==true){
				if(vCases[j].case_name.size() != 0 ){
					sprintf(casefields,",{\"name\":\"%s\"}",&vCases[j].case_name[0]);
					casearray.append(casefields);
					counter_cases++;
				}
			}else{
				if(vCases[j].case_name.size() != 0 ){
					empty=true;
					sprintf(casefields,",\"cases\":[{\"name\":\"%s\"}",&vCases[j].case_name[0]);
					casearray = casefields;
					counter_cases++;
				}
			}
		}
	}

	if(counter_cases > 0){
		pthread_mutex_lock(&mutexRidAlertAndBusiness);
			char tmp_str[100];
			sprintf(tmp_str,",\"cases_count\":%u",counter_cases);

			itRidAlertAndAction = mRidAlertAndAction.find(RID);
			if( itRidAlertAndAction != mRidAlertAndAction.end() ){
				itRidAlertAndAction->second.cases.append(casearray);
				itRidAlertAndAction->second.cases.push_back(']');
				itRidAlertAndAction->second.cases.append(tmp_str);
			}else{
				ElasticData ed;
				ed.cases = casearray;
				ed.cases.push_back(']');
				ed.cases.append(tmp_str);
				mRidAlertAndAction.insert(pair<long long,ElasticData>(RID,ed));

			}
		pthread_mutex_unlock(&mutexRidAlertAndBusiness);
	}

}
*/

bool ipABClasses(string & ip1,string & ip2){
	unsigned int dotCounter=0; 

	for(unsigned int i=0 ; i<ip1.size() ; i++){
		if(ip1[i]!=ip2[i]){
			return false;
		}

		if(ip1[i]=='.'){
			dotCounter++;
		}

		if(dotCounter==2){
			return true;
		}
	}
	return true;
}

void checkUserSyntax(string & user){
	short size = user.size();

	if(size==0){
		user="0";
		return;
	}
}

// get a string of numbers and put them in vector.
void stringToVec(string & str,vector <unsigned int> & vec_val){ 

	unsigned int i=0;	
	unsigned int sizeS = str.size();

	if(sizeS==0){
		return;
	}

	for(i=0; i < sizeS ; i++){
		if(str[i]==',' ){
			continue;
		}
		vec_val.push_back( (unsigned int)atoi(str.c_str() + i ) );
		i++;
		while ( i < sizeS && str.c_str()[i] != ','){
			i++;
		}
	}	
}

//Another converter.
//Example: \xd7\x90\xd7\x91 --> אב. 
short c_utf8_converter(string data,string & output){
	int a;

	unsigned short size = data.size()/4;
	if( (data.size() % 4) != 0){
		return 1;
	}

	for(int i=0;i<size;i++){
		if(data[0]!='\\' || data[1]!='x'){
			return 1;
		}
		sscanf((char*)data.c_str(),"\\x%x", &a);
		data.erase(data.begin(),data.begin()+4);	
		output.push_back((char)a);
	}

	return 0;
}

//Another converter.
//Example: \%d7%90%d7%91 --> \xd7\x90\xd7\x91. 
short regex_parse_utf8(string & data,string & new_data){
	new_data ="";
	unsigned int size = data.size();
	if( (size % 3) != 0){
		return 1;
	}

	for(unsigned int i=0;i<size;i++){
		if(data[i]=='%'){
			if(data[i+1]=='%'){
				return 1;
			}
			new_data.push_back('\\');
			new_data.push_back('x');
		}else{
			new_data.push_back(data[i]);
		}
	}
	return 0;
}

void findTopCookies(map <string,cookieNames>::iterator & it){
	map <string,unsigned int>::iterator itCookieName;
	multimap <unsigned int,string >::reverse_iterator itTopCookies;
	string strCookieNames;
	char  url[500],postfields[5000];
	unsigned int i=0;

	for(itCookieName=it->second.names.begin() ; itCookieName!=it->second.names.end() ; itCookieName++){
		topCookies.insert( pair<unsigned int,string>(itCookieName->second,itCookieName->first) );
	}

	for (itTopCookies=topCookies.rbegin(); itTopCookies!=topCookies.rend(); ++itTopCookies){
		if( i == 0 ){
 			snprintf(url,sizeof(url)-1,"http://localhost:9200/telepath-domains/domains/%s/_update",it->first.c_str());
                        sprintf(postfields,"{\"doc\":{\"AppCookieName\":\"%s\"}}",itTopCookies->second.c_str());
                        es_mapping(url,postfields);
			//sprintf(buffer,"UPDATE applications SET AppCookieName='%s' WHERE app_id=%u;",(char*)itTopCookies->second.c_str() , it->first );
		}
		else if( i == NUM_OF_COOKIE_SUGGESTIONS ){
			break;
		}

		strCookieNames.push_back('\"');
		strCookieNames += itTopCookies->second;
		strCookieNames.push_back('\"');
		strCookieNames.push_back(',');
		i++;

	}

	if (strCookieNames.size() > 0){
		strCookieNames.erase(strCookieNames.size() - 1);
	}

        sprintf(postfields,"{\"doc\":{\"cookie_suggestion\":[%s]}}",strCookieNames.c_str());
        es_mapping(url,postfields);
	#ifdef DEBUG
		syslog(LOG_NOTICE,"app_name=%s | cookie_suggestion=%s",it->first.c_str(),strCookieNames.c_str());
	#endif
	topCookies.clear();

}

/*double proOrHybridAppearance(unsigned int page_id,string & ids){
	itPreOrAb = mPresenceOrAbsence.find(page_id);
	if(itPreOrAb == mPresenceOrAbsence.end()){	// Page id wasn't found.
		if(operationMode==3){
			attAppearance aa(ids);
			mPresenceOrAbsence.insert(pair<unsigned int,attAppearance>(page_id,aa));
		}
		return 1.0;
	}else{						// Page id was found.

		map<string,unsigned int>::iterator it = itPreOrAb->second.att_ids.find(ids);
		if(it == itPreOrAb->second.att_ids.end() ){	// Attribute ids weren't found.	
			if(operationMode==3){
				itPreOrAb->second.counter++;
				itPreOrAb->second.att_ids.insert(pair<string,unsigned int>(ids,1));
			}
			return (1 - ( (1/(double)(itPreOrAb->second.counter+1)) ) );
		}else{						// Attribute ids were found.	
			if(operationMode==3){
				itPreOrAb->second.counter++;
				it->second++;
			}
			return (1 - ( it->second/(double)itPreOrAb->second.counter) );
		}		
	}
}*/

void calCookieNames(string & domainID,string & cookie){
	std::pair<map <string,cookieNames >::iterator,bool> itCookieStat;

	itCookieStat.first = mCookieStat.find(domainID);
	if(itCookieStat.first == mCookieStat.end() ){
		cookieNames cn;
		cn.counter=1;
		itCookieStat = mCookieStat.insert( pair<string,cookieNames >(domainID,cn) );
	}else{
		if(itCookieStat.first->second.counter > COOKIE_ANALYSIS_SAMPLES){
			itCookieStat.first->second.names.clear();
			return;	
		}
		else if(itCookieStat.first->second.counter % 500 == 0){
			findTopCookies(itCookieStat.first);
			return;	
		}

		itCookieStat.first->second.counter++;
	}

	bool flag=true;
	string tmp;
	std::size_t found;
	map <string,unsigned int>::iterator itCookieName;

	for(unsigned int i=0;i<cookie.size();i++){
		if(flag==true){
			if(cookie[i]=='='){
				if ( !(tmp=="path" || tmp=="expires" || tmp=="domain") ){
					found = tmp.find("httponly");
					if (found == std::string::npos){
						found = tmp.find("drop_lang");
						if (found == std::string::npos){
							itCookieName = itCookieStat.first->second.names.find(tmp);
							if( itCookieName != itCookieStat.first->second.names.end() ){
								itCookieName->second++;
							}else{
								itCookieStat.first->second.names.insert(pair<string,unsigned int>(tmp,1));
							}
						}
					}
				}
				tmp.clear();
				flag=false;
				continue;
			}
			else if(cookie[i]!=' '){
				tmp.push_back(cookie[i]);
				if(tmp.size()==2){
					if(tmp[0]=='_' && tmp[1]=='_'){
						tmp.clear();
						flag=false;
					}
				}
			}
		}else{
			if(cookie[i]==';'){
				flag=true;
			}
		}
	}
}

// Check if a unicode string is inside the unsigned int vector. 
short findSubString(string & v1,string & v2,bool negate){ 
	unsigned int counter=0,j=0;
	short result=0;	
	unsigned int size1=v1.size(),size2=v2.size();

	if(size2==0 ){
		return 0;
	}

	if(size1==0 ){
		if(negate==false)
			return 0;
		if(negate==true)
			return 1;
	}

	for(unsigned int i=0; i<size1 ;i++){
		if(v1[i]==v2[j]  ){
			counter++;
			j++;
		
		}else{
			counter=0;
			j=0;
		}
		
		if(counter==size2){
			result=1;
			break;
		}

	}	
	if(negate==true)
		result=result^1;

	return result;

}

int isNum(vector <unsigned int> & str) { 
	unsigned int i;
	for(i=0;i<str.size();i++){
		if( str[i] >= 43/*'+'*/ && str[i] <= 47/*'/'*/){ /*'+' ',' '-' '.' '/'*/
		}
		else if(str[i]>57 || str[i] < 48){ // Numbers.
			return (-1);
		}
	}
	return 1;
}

// check if it is a numeric type or not.
int isNum(vector <unsigned int> & str,string & tmp) { 
	unsigned int i;
	for(i=0;i<str.size();i++){
		if(str[i] == 44/*','*/ || str[i] == 47/*'/'*/ || str[i] == 46/*'.'*/ || str[i] == 43/*'+'*/){
		}
		else if(str[i] == 45/*'-'*/){ 
			if(i==0){
				tmp.push_back(str[i]);
			}
		}
		else if(str[i]>57 || str[i] < 48){ // Numbers.
			return (-1);
		}else{
			tmp.push_back(str[i]);
		}
	}
	return 1;
}


void numericType(myAtt & att,unsigned int mode){
	string tmp;
	unsigned int i;
	if(mode==1){ // Learning Mode.
		for(i=0;i< att.values.size() ;i++){
			tmp.clear(); 
			if(isNum(att.values[i],tmp) == 1 ){ 
				att.numeric.update(atof(tmp.c_str()) );
			}
		}
		att.type='n'; 
	}
}

bool const_att_type(myAtt & att){
	boost::unordered_map <string,char>::iterator it_const_att_types;

	it_const_att_types = const_attribute_types.find(att.name);
	if( it_const_att_types != const_attribute_types.end() ){
		att.type = it_const_att_types->second;
		return true;
	}else{
		return false;
	}

}

void decideType(myAtt & att,short userOrNot,unsigned int mode){ // deciding what is the type of the attribute.
	double percent,sum;
	unsigned int sizeAtt = att.hash_values.size();

	att.ifUpdate='n'; // In the next cycle - 'no' .

	if(const_att_type(att) == true){
		if(att.type == 'n'){
			numericType(att,mode);
		}
		else if(att.type == 'e'){
			for(unsigned int i=0; i < sizeAtt ;i++){
				att.enumeration.insertToList(att.hash_values[i]);
			}
		}

	}else{

		sum = (double)(att.num_of_values - att.num_of_nulls);
		if(sum==0){
			percent = 0;
		}else{
			percent =  100*(double)((double)att.num_of_numeric / sum );
		}

		if(percent > MAX_PERCENT){  // high percentage of these values were numbers in this specific attribute.
			numericType(att,mode);
			return;
		}

		for(unsigned int i=0; i < sizeAtt ;i++){
			att.enumeration.insertToList(att.hash_values[i]);
		}

		if( att.enumeration.size - att.enumeration.offset > 100 || att.enumeration.size > 15){
			att.enumeration.offset = att.enumeration.size;
			if( att.enumeration.getCorrelation() == true ){
				vector <unsigned short> ().swap(att.sizeMarkovVec);
				att.type='e';
				att.tree.word=0;
				att.tree.clean();
				return;
			}else{
				att.type='u';
			}
		}

		if(att.type=='F' || att.type=='U'){
			return; 
		}

		else{
			att.type='t';
		}
	}
}

//  Deciding if to analyze an attribute.
int isIrrelevant(unsigned int aid){
	if(aid==7 || aid==8){ // cookie and host.
 		return 1;
	}else{
		return 0;
	}
}

// Check the rule time limit.
unsigned short isTimeWinPass(double & pattern_timer,double & rule_ts){ 

	unsigned int currentTime = (unsigned int)( time(NULL) );

	if(pattern_timer==0){
		pattern_timer=currentTime;
	}

	unsigned int timeWindow = currentTime - pattern_timer;	
	if(timeWindow > (unsigned int)rule_ts ){
		pattern_timer = currentTime;
		return 1;
	}
	return 0;
}

// Check if there is a violation in a fruad rule. 
void checkRuleViolation(Rule & rule, boost::unordered_map <string,boost::unordered_set<int> > & vars , string & currentVar,Session & s,long long request_id,string & src_ip,string & resp_ip,string & cookie,string & hostname){ 
	boost::unordered_map <string,boost::unordered_set<int> >::iterator itVars;

	itVars = vars.find(currentVar);
	if(itVars == vars.end()){
		return;
	}

	if(rule.count < itVars->second.size() && rule.count>0 ){
		insert_alert(rule,request_id,src_ip,resp_ip,cookie,hostname);
	}

}

void checkRuleViolation2(Rule & rule,unsigned short countOfPid,Session & s,long long request_id,string & src_ip,string & resp_ip,string & cookie,string & hostname){ 

	if(rule.count < countOfPid && rule.count>0 ){
		insert_alert(rule,request_id,src_ip,resp_ip,cookie,hostname);
	}
}

void checkRuleViolation3(Rule & rule,Session & s,long long request_id,unsigned short countOfBusiness,string & src_ip,string & resp_ip,string & cookie,string & hostname){ 

	if(rule.count < countOfBusiness && rule.count>0 ){
		insert_alert(rule,request_id,src_ip,resp_ip,cookie,hostname);
	}
}

void checkRuleViolation4(Rule & rule,Session & s,boost::unordered_map <unsigned int,boost::unordered_set<int> > & vars,unsigned int currentVar,long long request_id,string & src_ip,string & resp_ip,string & cookie,string & hostname){ 
	boost::unordered_map <unsigned int,boost::unordered_set<int> >::iterator itVars;

	itVars = vars.find(currentVar);
	if(itVars == vars.end()){
		return;
	}

	if(rule.count < itVars->second.size() && rule.count>0 ){
		insert_alert(rule,request_id,src_ip,resp_ip,cookie,hostname);
	}

}

void checkRuleViolation4(Rule & rule,Session & s,boost::unordered_map <unsigned int,boost::unordered_set<int> > & vars,unsigned int currentVar,long long request_id,string & src_ip,string & resp_ip,string & cookie,string & hostname,bool & alert_flag){
	boost::unordered_map <unsigned int,boost::unordered_set<int> >::iterator itVars;

	itVars = vars.find(currentVar);
	if(itVars == vars.end()){
		return;
	}

	if(rule.count < itVars->second.size() && rule.count>0 ){
		insert_alert(rule,request_id,src_ip,resp_ip,cookie,hostname);
		alert_flag=true;
	}
}

// Compare ip,user and hostname in the rule syntax.
unsigned short isAnySpecification(unsigned int decimalIP,string & user,Rule & rule){
	bool flagIP=false;
	for(unsigned int iii=0;iii<rule.vecRangeIP.size();iii++){
		if( excludeIP(rule.vecRangeIP[iii],decimalIP,rule.ip_neg) == true){
			flagIP=true;
			break;
		}
	}

	if ( (rule.user.size()==0 || rule.user== user) && flagIP==false  ){
		return 1;
	}

	return 0;

}

// check if to analyze value.
// shortLongAll = short , analyze just short values.
// shortLongAll = long , analyze just long values.
// shortLongAll = both , analyze both.
// shortLongAll = range ( 3-5 for example ) 

unsigned int checkRange(double & str_size,unsigned short low,unsigned short high){
	unsigned short size = (unsigned short)str_size;
	if( size >= low && size<=high ){
		return 1;
	}else{
		return 0;
	}
}

unsigned short longOrShort(string & shortLongAll,double & avg,double str_size,short low,short high,short & constant){
	if(shortLongAll.size() == 0){
		return 0;		
	}

	switch (shortLongAll[0]){

		case 'b':		// b=both
			return 1;
		case 's':		// s=short
			if(avg > str_size){
				return 1;
			}
			break;
		case 'l':		// l=long
			if(avg < str_size){
				return 1;
			}
			break;
	}

	if(checkRange(str_size,low,high)==1 ){
		constant=1;
		return 1;
	}

	return 0;
}

//Delete Spaces from the beginning & ending.
void deleteSpaces(vector <unsigned int> & vec){ 
	unsigned int count=0;

	//Delete Spaces from the beginning

	for(unsigned int i=0; i < vec.size() ;i++){

		if(vec[i] == 32 || vec[i] == 43) // 32 = ' ' ,43='+'.
			count++;
		else
			break;
	}

	vec.erase(vec.begin(),vec.begin()+count);
	count=0;

	if(vec.size()==0)
		return;

	//Delete Spaces from the end
	for(unsigned int i=vec.size()-1 ; i >= 0  ;i--){
		if(vec[i] == 32 || vec[i] == 43) // 32 = ' ' ,43='+'.
			count++;
		else
			break;
	}
	vec.erase( vec.end()-count , vec.end() );
}

// Initialization of attribute value.
void attributeValInit(vector <unsigned int> & values){ 
	if(values.size()==0){       	  // if you get an empty value convert it to NULL BYTE.
		values.push_back(0);
	}
}

// Initialization of attribute value.
bool attributeValInit2(vector <unsigned int> & values){ 
	if(values.size()==0){       	  // if you get an empty value convert it to NULL BYTE.
		values.push_back(0);
		return true;
	}else{
		return false;
	}
}

// Find geometric mean for free-text.
int getGeometricMean(vector <int> & vec){
	int exp=0;
	unsigned short sizeV = vec.size();

	if(sizeV == 1){
		return vec[0];
	}
	for(unsigned short i=0;i<sizeV;i++){
		exp+=vec[i];
	}

	exp = (exp/(double)sizeV) ;

	return exp;

}

// Check integrity of session Arithmetic progression.
unsigned int integrityCheck(unsigned int a_0,unsigned long n){ 
	unsigned int a_n= a_0+n-1;
	return ( (unsigned int)((a_0+a_n)*n/2)  );
}

// Check in which seq_index the session was broken.
unsigned long whereSessionInconsistent(Session & s){ 
	unsigned int sizeP = s.vRequest.size();
	for(unsigned int i=1; i < sizeP ; i++ ){
		if ( (s.vRequest[i].index - s.vRequest[i-1].index) !=1)
			return ((unsigned long)(s.vRequest[i-1].index));
	}
	return 0;
}

void print(const char * str){
	if(write_to_syslog){
		syslog (LOG_NOTICE, "%s",str );
	}
}
void print(const char * str,unsigned int num){
	if(write_to_syslog){
		syslog (LOG_NOTICE, "%s%u",str,num );
	}
}

void print(unsigned int num1,unsigned int num2){
	if(write_to_syslog){
		syslog (LOG_NOTICE, "%u - %u",num1,num2 );
	}
}



void getAttributeThatWasFound(myAtt & att,Attribute & Attr,size_and_flag & s_f,long long RID,unsigned int i){
	att.ifUpdate='y';
	att.num_of_values++;
	int is_num_res;
	bool isNull;
	
	//Lowercase
	string lower_val;
	for(unsigned int i=0;i<Attr.value.size();i++){
		if(Attr.value[i]>64 && Attr.value[i]<91){
			lower_val.push_back( Attr.value[i]+32 );
		}else{
			lower_val.push_back( Attr.value[i] );
		}
	}

	att.hash_values.push_back( hashCode(lower_val) );

	isNull = attributeValInit2(Attr.vec_value);
	att.values.push_back( Attr.vec_value);

	s_f.size = Attr.vec_value.size();
	s_f.flag = 'y';
	att.val_s_f.push_back(s_f);
	att.RIDs.push_back( RID );
	if(isNull==true){
		att.num_of_nulls++;
	}else{
		is_num_res = isNum(Attr.vec_value);
		if(is_num_res == 1 ){ // Numeric
			att.num_of_numeric++;
		}
	}
}

void getAttributeThatWasntFound(myAtt & att,Attribute & Attr,size_and_flag & s_f,long long RID,unsigned int PID_ ){
	att.name=Attr.name;
	att.kind=Attr.attribute_source;
	att.ifUpdate='y';
	att.num_of_values++;
	int is_num_res;
	bool isNull;

	//Lowercase
	string lower_val;
	for(unsigned int i=0;i<Attr.value.size();i++){
		if(Attr.value[i]>64 && Attr.value[i]<91){
			lower_val.push_back( Attr.value[i]+32 );
		}else{
			lower_val.push_back( Attr.value[i] );
		}
	}

	att.hash_values.push_back( hashCode(lower_val) );

	isNull = attributeValInit2(Attr.vec_value);
	att.values.push_back(Attr.vec_value);

	s_f.size = Attr.vec_value.size();
	s_f.flag = 'y';
	att.val_s_f.push_back(s_f);
	att.RIDs.push_back( RID );
	if(isNull==true){
		att.num_of_nulls++;
	}else{
		is_num_res = isNum(Attr.vec_value);
		if(is_num_res == 1 ){ // Numeric
			att.num_of_numeric++;
		}
	}

	att.page_id = PID_;
}

void getAttributeThatWasFound_Hybrid(myAtt & att,Attribute & Attr){
	att.ifUpdate='y';
	att.num_of_values++;

	int is_num_res;
	bool isNull;

	//Lowercase
	string lower_val;
	for(unsigned int i=0;i<Attr.value.size();i++){
		if(Attr.value[i]>64 && Attr.value[i]<91){
			lower_val.push_back( Attr.value[i]+32 );
		}else{
			lower_val.push_back( Attr.value[i] );
		}
	}

        att.hash_values.push_back( hashCode(lower_val) );

	isNull = attributeValInit2(Attr.vec_value);
	att.values.push_back( Attr.vec_value);

	if(isNull==true){
		att.num_of_nulls++;
	}else{
		is_num_res = isNum(Attr.vec_value);
		if(is_num_res == 1 ){ // Numeric
			att.num_of_numeric++;
		}
	}

}

void getAttributeThatWasntFound_Hybrid(myAtt & att,Attribute & Attr,unsigned int PID_ ){
	att.newInHybrid='y';
	att.newCounterInHybrid++;
	att.name=Attr.name;
	att.kind=Attr.attribute_source;
	att.ifUpdate='y';
	att.num_of_values++;
	int is_num_res;
	bool isNull;

	//Lowercase
	string lower_val;
	for(unsigned int i=0;i<Attr.value.size();i++){
		if(Attr.value[i]>64 && Attr.value[i]<91){
			lower_val.push_back( Attr.value[i]+32 );
		}else{
			lower_val.push_back( Attr.value[i] );
		}
	}

	att.hash_values.push_back( hashCode(lower_val) );

				// Regular type.
	isNull = attributeValInit2(Attr.vec_value);
	att.values.push_back(Attr.vec_value);

	if(isNull==true){
		att.num_of_nulls++;
	}else{
		is_num_res = isNum(Attr.vec_value);
		if(is_num_res == 1 ){ // Numeric
			att.num_of_numeric++;
		}
	}


	att.page_id = PID_;
}

bool isItNullVal(vector <unsigned int> & vec){
	if(vec.size() == 1){
		if(vec[0]==0){
			return true;
		}
	}

	return false;
}

void ifNumeric(myAtt & att,int & exp,vector <unsigned int> & vec , unsigned int & mode,unsigned int aid){
	double new_num,score;
	if( isIrrelevant(aid) ){  // dont analyze header,get or post, if config is zero.
		exp=0;	
		attributeValInit(vec);
	}else{
		attributeValInit(vec);
		if( isItNullVal(vec) ){ // Null value
			if(att.num_of_values != 0){
				if(att.num_of_nulls==0){
					score = ( 1/(double)att.num_of_values);
				}else{
					score = ( att.num_of_nulls/(double)att.num_of_values);
				}
				frexp(score,&exp);
				if(exp>0){exp=0;}
				return;
			}
		}

		string str_num;
		if( isNum( vec,str_num ) == (-1) ){
			exp=MIN_PROB;			
		}else{		
			new_num =(double)atof( str_num.c_str() );

			//------------------------------Hybrid Mode------------------------------
			if(mode==3){
				att.numeric.update(new_num);
			}
			//-----------------------------------------------------------------------

			score = att.numeric.chebyshev( new_num );
			frexp(score,&exp);
			if(exp>0){exp=0;}
		}
	}	
}

void ifEnum(myAtt & att,int & exp,vector <unsigned int> & vec , unsigned int & mode,int hash_val,unsigned int aid){
	if( isIrrelevant(aid) ){// dont analyze header,get or post, if config is zero.
		attributeValInit(vec);
		exp=0;
	}else{
		attributeValInit(vec);
		if( isItNullVal(vec) ){ // Null value
			if(att.num_of_values != 0){
				double s;
				if(att.num_of_nulls==0){
					s = ( 1/(double)att.num_of_values);
				}else{
					s = ( att.num_of_nulls/(double)att.num_of_values);
				}
				frexp(s,&exp);
				if(exp>0){exp=0;}
				return;
			}
		}

		//------------------------------Hybrid Mode------------------------------
		if(mode==3){
			att.enumeration.insertIntoEnum(hash_val);
		}
		//-----------------------------------------------------------------------

		exp = att.enumeration.getProb(hash_val);	
	}
}

/*
void ifUrl(myAtt & att,int & exp_score,Attribute & Attr, unsigned int & mode){
	if(Attr.vec2[0][0] == 0){
		vector <unsigned int> backslash;
		backslash.push_back(47);
		Attr.vec2.push_back(backslash);

		vector <unsigned int>().swap(backslash);
	}

	if( isIrrelevant(att.ID) ){// dont analyze header,get or post, if config is zero.
		attributeValInit(Attr.vec2[1]);
		attributeValInit(Attr.vec2[2]);
		exp_score=0;
	}else{

		attributeValInit(Attr.vec2[1]);
		attributeValInit(Attr.vec2[2]);

		if( isItNullVal(Attr.vec2[1]) ){ // Null value
			if(att.num_of_values != 0){
				double s;
				if(att.num_of_nulls==0){
					s = ( 1/(double)att.num_of_values);
				}else{
					s = ( att.num_of_nulls/(double)att.num_of_values);
				}
				frexp(s,&exp_score);
				if(exp_score>0){exp_score=0;}
				return;
			}
		}
		
		//------------------------------Hybrid Mode------------------------------
		if(mode==3){
			att.url.insert(Attr.vec2[1],Attr.vec2[2]);
		}
		//-----------------------------------------------------------------------

		exp_score = att.url.getProb(Attr.vec2[1],Attr.vec2[2]);
	}
}*/

void ifTextOrFree(myAtt & att,int & exp_score,Attribute & Attr, unsigned int & mode,double & len_score,vector <unsigned int> & vec_val,double & avg_size,unsigned int aid){
	if( isIrrelevant(aid) ){// dont analyze header,get or post, if config is zero.
		attributeValInit(vec_val);
	}else{

		attributeValInit(vec_val);
		if( isItNullVal(vec_val) ){ // Null value
			if( att.num_of_values!=0 ){
				double s;
				if(att.num_of_nulls==0){
					s = ( 1/(double)att.num_of_values);
				}else{
					s = (att.num_of_nulls/(double)att.num_of_values);
				}
				s *= s;
				s *= s;
				frexp(s,&exp_score);
				if(exp_score>0){exp_score=0;}

				len_score = att.sizeMarkov.chebyshev( 0 );
				len_score = 1-len_score;

				return;
			}
		}

		//------------------------------Hybrid Mode------------------------------
		//if(mode==3){
		//	att.tree.tokenize(vec_val);
		//}
		//-----------------------------------------------------------------------
		exp_score = att.tree.calculate(vec_val,avg_size);
	}

	//------------------------------Hybrid Mode------------------------------
	if(mode==3){
		att.sizeMarkov.update( (double)vec_val.size() ) ;
	}
	//-----------------------------------------------------------------------	


	len_score = att.sizeMarkov.chebyshev( (double)vec_val.size() );
	len_score = 1-len_score;

}

void session_user_search(string & UserID_,unsigned int sid,boost::unordered_map <unsigned int,Session> & mSession,TeleObject & v){
	
	boost::unordered_map <unsigned int,Session> ::iterator itUserSession;// iterator of mSession	
	sid += (unsigned int)hashCode(UserID_);

	itUserSession=mSession.find(sid);
	if(itUserSession==mSession.end()){				//Session user wasn't found
		Session sessionUser;//
		sessionUser.sid = sid;	

		// constructor for Page
		Page pageUser( v.mParams , sessionUser.sequence , (unsigned int)atoi(v.mParams['k'/*PageID*/].c_str() ), atoll(v.mParams['j'/*RID*/].c_str() ),v.mParams['a'/*UserIP*/],UserID_)  ;

		sessionUser.vRequest.push_back(pageUser);
		
		sessionUser.elapsed_ts = sessionUser.vRequest[sessionUser.vRequest.size()-1].ts - sessionUser.vRequest[0].ts; // session time.
		sessionUser.user_flag=1;			
		mSession.insert( pair<unsigned int,Session>(sid,sessionUser) );

	}else{ 								//Session user was found

		// constructor for Page		

		Page pageUser( v.mParams ,itUserSession->second.sequence, (unsigned int)atoi(v.mParams['k'/*PageID*/].c_str() ), atoll(v.mParams['j'/*RID*/].c_str() ),v.mParams['a'/*UserIP*/],UserID_)  ;

		itUserSession->second.vRequest.push_back(pageUser);
		itUserSession->second.update=1; // need to update path.

		itUserSession->second.elapsed_ts = itUserSession->second.vRequest[itUserSession->second.vRequest.size()-1].ts - itUserSession->second.vRequest[0].ts; //session time.		
	}

}

void session_user_search2(string & UserID_,unsigned int sid,boost::unordered_map<unsigned int,Session> & mSession,TeleObject & v){

	boost::unordered_map<unsigned int,Session> ::iterator itUserSession;// iterator of mSession
	sid += (unsigned int)hashCode(UserID_);

	itUserSession=mSession.find(sid);
	if(itUserSession==mSession.end()){				//Session user wasn't found
		Session sessionUser;//
		sessionUser.sid = sid;	

		// constructor for Page
		Page pageUser(v.mParams,sessionUser.sequence,(unsigned int)atoi(v.mParams['k'/*PageID*/].c_str() ),atoll(v.mParams['j'/*RID*/].c_str() ),v.mParams['a'/*UserIP*/],UserID_);

		sessionUser.vRequest.push_back(pageUser);
		sessionUser.user_flag=1;
		mSession.insert( pair<unsigned int,Session>(sid,sessionUser) );

	}else{ 								//Session user was found
		// constructor for Page	
		Page pageUser( v.mParams,itUserSession->second.sequence,(unsigned int)atoi(v.mParams['k'/*PageID*/].c_str() ),atoll(v.mParams['j'/*RID*/].c_str() ),v.mParams['a'/*UserIP*/],UserID_);

		itUserSession->second.vRequest.push_back(pageUser);
		itUserSession->second.update=1; // need to update path.
	}
}

void countryRules(Rule & rule,string & src_ip,string & UserID,long long RID,unsigned int PageID,string & tmp_country,Session & s,string & resp_ip,string & cookie,string & hostname){
	if( isAnySpecification(s.decimalIP,UserID,rule) ){ // App/User/IP violation ?.
		short flag;
		size_t found = rule.str_match.find(tmp_country);
		if (found!=string::npos){ //Country was found .
			if(rule.negate==false){  // Give alerts to countries in the list(block countries like China or Russia).
				flag=1;
			}else{
				flag=0;
			}
		}else{			  //Country wasn't found .
			if(rule.negate==false){
				flag=0;
			}else{			  // Give alerts to countries which aren't in the list(all countries except Israel & USA).
				flag=1;
			}
		}
		if(flag==1){
			insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
		}
	}
}

void regexRules(Rule & rule,long long RID,string & att_value,Session & s,double & alert_score,string & src_ip,string & resp_ip,string & cookie,string & hostname){
	string str_regex,regex_output;

	if( regex_parse_utf8(att_value,str_regex) ){
		regex_output = att_value;
	}else{
		if( c_utf8_converter(str_regex,regex_output) ){
			regex_output = att_value;
		}
	}


	if(rule.validReg == true){
		int rc,offsets[30];
		rc = pcre_exec(rule.pcreRegex, NULL, (char*)regex_output.c_str(), regex_output.size(), 0, 0, offsets,30);
		if (rc < 0) {	// regex wasn't found.
			if(rule.negate==true){
				alert_score = (double)rule.threshold;
				insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
			}
		}else{		// regex was found.
			if(rule.negate==false){
				alert_score = (double)rule.threshold;
				insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
			}
		}
	}
}

void regexRules2(Rule & rule,long long RID,string & att_value,Session & s,bool & alert_flag,string & src_ip,string & resp_ip,string & cookie,string & hostname){
	string str_regex,regex_output;

	if( regex_parse_utf8(att_value,str_regex) ){
		regex_output = att_value;
	}else{
		if( c_utf8_converter(str_regex,regex_output) ){
			regex_output = att_value;
		}
	}


	if(rule.validReg == true){
		int rc,offsets[30];
		rc = pcre_exec(rule.pcreRegex, NULL, (char*)regex_output.c_str(), regex_output.size(), 0, 0, offsets,30);
		if (rc < 0) {	// regex wasn't found.
			if(rule.negate==true){
				alert_flag = true;
				insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
			}
		}else{		// regex was found.
			if(rule.negate==false){
				alert_flag = true;
				insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
			}
		}
	}
}

void patternRulesParameter(Rule & rule,string & src_ip,string & UserID,long long RID,int hash_value,Session & s,string & att_anchor,string & str_val,string & resp_ip,string & cookie,string & hostname,bool & alert_flag){
	//--------IP---------
	if (rule.anchor[0] == 'I'){ // I = IP.
		if(isTimeWinPass(rule.pattern_ts,rule.ts)){ //check time window.
			rule.mCardinal_IP_att.clear();
		}

		if( rule.mCardinal_IP_att[ src_ip ].count(hash_value) != 0 ){	// hash_value was found.
			return;
		}else{								// hash_value wasn't found.
			rule.mCardinal_IP_att[ src_ip ].insert(hash_value);
		}

		if( isAnySpecification(s.decimalIP , UserID ,rule ) ){
			checkRuleViolation(rule,rule.mCardinal_IP_att,src_ip,s,RID,src_ip,resp_ip,cookie,hostname);
		}

	}
	//--------SID---------
	else if (rule.anchor[0] == 'S'){	// S = SID			 
		if(isTimeWinPass(rule.pattern_ts,rule.ts)){ //check time window.
			rule.mCardinal_SID_att.clear();
		}

		if( rule.mCardinal_SID_att[ s.sid ].count(hash_value) != 0 ){	// hash_value was found.
			return;
		}else{									// hash_value wasn't found.
			rule.mCardinal_SID_att[ s.sid ].insert(hash_value);
		}

		if( isAnySpecification(s.decimalIP , UserID ,rule ) ){
			checkRuleViolation4(rule,s,rule.mCardinal_SID_att,s.sid,RID,src_ip,resp_ip,cookie,hostname);
		}
	}
	//--------Att---------

	if((unsigned int)atoi(rule.anchor.c_str() ) != 0){ // another att id.
		if(isTimeWinPass(rule.pattern_ts,rule.ts)){ //check time window.
			rule.mCardinal_att_att.clear();
		}	

		att_anchor += src_ip;
		unsigned int hash_anchor = (unsigned int)hashCode(att_anchor);
		if( rule.mCardinal_att_att[ hash_anchor ].count(hash_value) != 0 ){	// hash_value was found.
			return;
		}else{									// hash_value wasn't found.
			rule.mCardinal_att_att[ hash_anchor ].insert(hash_value);
		}

		if( isAnySpecification(s.decimalIP , UserID,rule ) ){
			checkRuleViolation4(rule,s,rule.mCardinal_att_att ,hash_anchor,RID,src_ip,resp_ip,cookie,hostname,alert_flag);
		}

	}
}

void patternRulesUser(Rule & rule,string & src_ip,string & UserID,long long RID,Session & s,string & resp_ip,string & cookie,string & hostname){

	int hash_value = hashCode(UserID);
	//--------IP---------
	if (rule.anchor[0] == 'I'){ // I = IP.
		if(isTimeWinPass(rule.pattern_ts,rule.ts)){ //check time window.
			rule.mCardinal_IP_User.clear();
		}

		if( rule.mCardinal_IP_User[ src_ip ].count(hash_value) != 0 ){	// hash_value was found.
			return;
		}else{								// hash_value wasn't found.
			rule.mCardinal_IP_User[ src_ip ].insert(hash_value);
		}

		if( isAnySpecification(s.decimalIP , UserID , rule ) ){
			checkRuleViolation(rule,rule.mCardinal_IP_User,src_ip,s,RID,src_ip,resp_ip,cookie,hostname);
		}

	}
	//--------SID---------
	else if (rule.anchor[0] == 'S'){	// S = SID			 

		if(isTimeWinPass(rule.pattern_ts,rule.ts)){ //check time window.
			rule.mCardinal_SID_User.clear();
		}

		if( rule.mCardinal_SID_User[ s.sid ].count(hash_value) != 0 ){	// hash_value was found.
			return;
		}else{									// hash_value wasn't found.
			rule.mCardinal_SID_User[ s.sid ].insert(hash_value);
		}

		if( isAnySpecification(s.decimalIP , UserID , rule ) ){
			checkRuleViolation4(rule,s,rule.mCardinal_SID_User,s.sid,RID,src_ip,resp_ip,cookie,hostname);
		}
	}
}

void patternRulesPage(Rule & rule,string & src_ip,string & UserID,unsigned int pid,long long RID,Session & s,string & resp_ip,string & cookie,string & hostname){
	if(pid != rule.page_id){
		return;
	}

	boost::unordered_map <string,unsigned short>::iterator itIP; 

	if (rule.anchor[0] == 'I'){ // I = IP.
		if(isTimeWinPass(rule.pattern_ts,rule.ts)){ //check time window.
			rule.mCardinal_IP_pid.clear();
		}

		itIP = rule.mCardinal_IP_pid.find(src_ip) ;
		if(itIP != rule.mCardinal_IP_pid.end() ){ // IP was found;
			itIP->second++;
		}else{				   // IP wasn't found;
			rule.mCardinal_IP_pid.insert(pair<string,unsigned short>(src_ip,1));
			return;
		}

		if( isAnySpecification(s.decimalIP , UserID , rule ) ){
			checkRuleViolation2( rule, itIP->second,s,RID,src_ip,resp_ip,cookie,hostname);
		}

	}
}

void patternRulesBusiness(Rule & rule,string & src_ip,string & UserID,unsigned int pid,Session & s,long long RID,string & resp_ip,string & cookie,string & hostname){
	boost::unordered_map<unsigned int,alert >::iterator itRIDsPerFlow = s.RIDsPerFlow.find(rule.action_id);

	if(itRIDsPerFlow == s.RIDsPerFlow.end()){
		return;
	}

	if (rule.anchor[0] == 'I'){ // I = IP.
		pair< boost::unordered_map <string,alert>::iterator, bool > itCardinal_IP_flow;
		itCardinal_IP_flow.first = rule.mCardinal_IP_flow.find(src_ip);

		if( itCardinal_IP_flow.first != rule.mCardinal_IP_flow.end() ){
			if(itCardinal_IP_flow.first->second.start_ts == 0){
				itCardinal_IP_flow.first->second.start_ts = itRIDsPerFlow->second.start_ts;
			}

			itCardinal_IP_flow.first->second.end_ts = itRIDsPerFlow->second.end_ts;
			itCardinal_IP_flow.first->second.ridAlert++;
		}else{
			alert a;
			a.start_ts = itRIDsPerFlow->second.start_ts;
			a.end_ts = itRIDsPerFlow->second.end_ts;
			itCardinal_IP_flow = rule.mCardinal_IP_flow.insert( pair<string,alert>(src_ip,a) );
		}

		if( (itCardinal_IP_flow.first->second.end_ts - itCardinal_IP_flow.first->second.start_ts ) < rule.ts){
			if( isAnySpecification(s.decimalIP , UserID , rule ) ){
				checkRuleViolation3(rule,s,RID,itCardinal_IP_flow.first->second.ridAlert,src_ip,resp_ip,cookie,hostname);
			}
		}else{
			itCardinal_IP_flow.first->second.start_ts = 0;
		}
	}
}

void lengthRules(Rule & rule,double & s_of_m,vector <unsigned int> & vec_val,double exp_score,double exp_user_score,Session & s,long long RID,double & alert_score,string & src_ip,string & resp_ip,string & cookie,string & hostname){
	Numeric N_S,NU_S;
	short constant=0;

	double val_size = (double)vec_val.size();
	if(vec_val.size()==1){
		if(vec_val[0]==0){
			val_size=0;
		}
	}
	if( longOrShort(rule.str_length,s_of_m,val_size,rule.str_length_low,rule.str_length_high,constant) ){
		if(rule.personal==0){// General
			insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
		}else{
			insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
		}
	}
}

void similarityRules(Rule & rule,vector <unsigned int> & vec_val,Session & s,long long RID,string & src_ip,string & resp_ip,string & cookie,string & hostname){

	if(rule.personal==0){// General
		insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
	}else{
		insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
	}
}

void generalRules(Rule & rule,Session & s,string & src_ip,long long RID,unsigned int PageID,string & UserID,string & resp_ip,string & cookie,string & hostname){
	string country;

	//--------------Page rules-----------------
	if( PageID!=0 && PageID == rule.page_id && rule.att_id == 0 && rule.final_type != 'P' ){
		if( isAnySpecification(s.decimalIP , UserID , rule ) ){			// App/User/IP violation ?.
			insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
		}
	}
	//------------Page rules End---------------
	switch ( rule.final_type ){
		//-----Bot Intelligence rule for IP--------
		case 'B':					// B = Bot Intelligence.
			if(rule.str_match[0]=='t'){
				if(sTorIntelligenceIP.count(src_ip) != 0){//IP was found.
					insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
				}
			}

			if(rule.str_match[0]=='k'){
				if(sBotIntelligenceIP.count(src_ip) != 0){//IP was found.
					insert_alert(rule,RID,src_ip,resp_ip,cookie,hostname);
				}
			}

			break;
		//----Bot Intelligence rule for IP End-----

		//-----------Block Country Rule------------
		case 'G':					// G = Geo Limitation.
			country = getCountry(src_ip);
			countryRules(rule,src_ip,UserID,RID,PageID,country,s,resp_ip,cookie,hostname);
			break;
		//---------Block Country Rule End----------

		//-----------Page Cardinality------------
		case 'P':					// P = Page Cardinality.
			patternRulesPage(rule,src_ip,UserID,PageID,RID,s,resp_ip,cookie,hostname);
			break;
		//---------Page Cardinality End----------
	}

	//---------User Pattern----------
	if(rule.anchor.size() != 0){
		if(rule.dynamic[0] == 'U' && UserID != "0"){
			patternRulesUser(rule,src_ip,UserID,RID,s,resp_ip,cookie,hostname);
		}
	}
	
}

void spitJson(string & name,vector <string> & json_array){
	string tmp;	
	unsigned short flag=0;
	unsigned short sizeN = name.size() ;
	for(unsigned short i=0;i < sizeN ;i++){
		if(i==0 || i==sizeN-1){
			tmp.push_back(name[i]);
			if(i==sizeN-1){
				json_array.push_back(tmp);
			}
			continue;
		}

		if(name[i]==':'){
			if(flag==0){
				flag=1;
			}else{
				json_array.push_back(tmp);
				flag=0;
				tmp.clear();
			}
		}else{
			if(flag==0){
				tmp.push_back(name[i]);
			}else{
				tmp.push_back(':');
				tmp.push_back(name[i]);
				flag=0;
			}
		}

	}
}


