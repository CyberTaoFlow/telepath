unsigned int maxRuleGroupID;

using namespace std;

void UpToLow(string & str){
	char ch = '%';
	for (unsigned int i=0;i< str.size() ;i++){
		if (str[i] >= 0x41 && str[i] <= 0x5A){
			str[i] = str[i] + 0x20;
		}
		if(i%3==0){
			str.insert(str.begin()+i,ch);
		}
		
	}
}

unsigned int string_to_utf8_engine(string & data,string & new_data)
{
	vector <unsigned int> result;
	result.clear();
	unsigned int uiTmp=0;
	unsigned int tmp, hi_b, lo_b, mid_b, res_b;
	int i=0, sz;
	string ind="%__";
	unsigned short noisy_res=0;

	sz=data.size(); // saving length of string

	while(i<sz)	//running through all string
	{
		ind="%__";
		tmp=hi_b=lo_b=mid_b=res_b=0;

		//%d7%99 - UTF8; %u05db - UTF16
		//so looking for UTF8 and not for UTF16
		if(data[i]=='%' && data[i+1]!='u')
		{

			ind[1]=data[i+1];
			ind[2]=data[i+2];
			tmp=to_utf8(ind);//saving first found element to tmp

			if(tmp>=0 && tmp <= 127)	//1-byte ascii symbol
			{
				ind="%__";
				result.push_back(tmp);
				i+=3;
				continue;
			}
			else if(tmp>=192 && tmp <= 223)// 2-bytes UTF8
			{					//so looking for second byte
				ind="%__";
				i+=3;			//moving iterator to the next element
				if(i>=sz)		//if iterator more then size of string, so we have only one byte, which is not UTF8, so mark it as noisy
				{
					result.push_back(tmp);
					noisy_res=1;
					return noisy_res;
				}

				else if(data[i]!='%')	//if we cannot find '%' in the next byte - mark as noisy
				{
					result.push_back(tmp);
					noisy_res=1;
					return noisy_res; //or maybe better to return
				}

				//checking every byte
				++i;
				if(i<sz)
					ind[1]=data[i];
				else
				{
					result.push_back(tmp);
					syslog (LOG_NOTICE, "UTF8 WARNING: Unexpected end after: %s in %s\n Letter after '%%' expected.\n",ind.c_str(),data.c_str());
					noisy_res=1;
					return noisy_res;
				}

				++i;
				if(i<sz)
					ind[2]=data[i];
				else
				{
					result.push_back(tmp);
					syslog (LOG_NOTICE, "UTF8 WARNING: Unexpected end after: %s in %s\n Second letter after '%%' expected.\n",ind.c_str(),data.c_str());
					noisy_res=1;
					return noisy_res;
				}


				hi_b=tmp;
				lo_b=to_utf8(ind);

				uiTmp=hi_b*256+lo_b;		//building 2-bytes integer

				ind="%__";

				result.push_back(uiTmp);
				++i;
				continue;
			}
			else if(tmp>=224 && tmp<=239) // 3-bytes UTF8
			{
				if(sz-i>=8)//checking, if after 1st byte there are 2 more
				{

					hi_b=tmp;
					//saving three bytes: hi is in tmp, mid and lo
					string b1="%__",b2="%__";
					b1[1]=data[i+4];
					b1[2]=data[i+5];
					mid_b=to_utf8(b1);

					b2[1]=data[i+7];
					b2[2]=data[i+8];
					lo_b=to_utf8(b2);

					//if sequence of bytes doesn't meet UTF8, i convert each byte to number
					if( !((mid_b>=128 && mid_b <= 191) && (lo_b>=128 && lo_b <= 191)) )
					{


						result.push_back(hi_b);
						result.push_back(mid_b);
						result.push_back(lo_b);
						noisy_res=1;
						return noisy_res; //added 17.01.2012
						i+=9;
						ind="%__";
						continue;
					}

					//else
					uiTmp=hi_b*256*256+mid_b*256+lo_b;
					result.push_back(uiTmp);

					i+=9;
					ind="%__";
					continue;
				}
				else
				{


					result.push_back(tmp);
					i+=3;
					ind="%__";
					noisy_res=1;
					return noisy_res; //added 17.01.2012
					continue;
				}
			}
			else  //should delete this else???
			{
				result.push_back(tmp);
				i+=3;
				noisy_res=1;
				continue;
			}
		}

		else//if it's simply ascii
		{
			result.push_back(data[i]);
			++i;
		}
	}
	unicodeParser(result,new_data);
	vector <unsigned int> ().swap(result);

	return noisy_res;
}

void loader(){
	boost::unordered_map <string,AppMode>::iterator itAppMode;
	short success;
	string domain;

	syslog(LOG_NOTICE,"LOADING CHANGES[!!!]");
	syslog(LOG_NOTICE,"LOADING Attributes[!!!]");
	readB(myAttMap);

	syslog(LOG_NOTICE,"LOADING User Attributes[!!!]");
	readB(myAttUserMap);

	//------------------------------------------------------------------
	syslog(LOG_NOTICE,"LOADING Paths[!!!]");
	//------------------------------Paths-------------------------------

	pthread_mutex_lock(&mutexAppMode);
	for(itAppMode = mAppMode.begin() ; itAppMode != mAppMode.end() ; itAppMode++ ){
		domain = itAppMode->first;
		Path path(domain);
		success = readB4(path,domain);
		if(success!=0){
			mPath.insert( pair<string,Path>(domain,path) );
		}
	}
	pthread_mutex_unlock(&mutexAppMode);

	//------------------------------------------------------------------

	syslog(LOG_NOTICE,"LOADING User Paths[!!!]");
	readB(mPathUser);

	//---------globals---------
	syslog (LOG_NOTICE, "LOADING Globals[!!!]" );
	pthread_mutex_lock(&mutexSessionsPerHost); //lock
	readB2(numOfSessionsPerDomain);
	pthread_mutex_unlock(&mutexSessionsPerHost); //unlock

	// Production or Hybrid mode.
	readB(score_numeric); // map <string,ScoresNumeric> score_numeric;
	readB6(mScoreNumericAtt);
	
	// Learning mode
	readB(score_data);
	readB7(mScoreAtt);

	/*map <unsigned long long,ScoreNumericAtt>::iterator itScoreNumericAtt;
	for(itScoreNumericAtt=mScoreNumericAtt.begin() ;itScoreNumericAtt!=mScoreNumericAtt.end() ;itScoreNumericAtt++){
		cout<<"AttID:"<<itScoreNumericAtt->first << endl;
		itScoreNumericAtt->second.print();
		cout<<"--------------------------------" << endl;
	}*/


	//-------------------------

	syslog (LOG_NOTICE, "LOADING Operations[!!!]" );
	readB(mOperation);

	syslog (LOG_NOTICE, "LOADING END[!!!]");

}

void saver(){

	syslog(LOG_NOTICE,"SAVING CHANGES[!!!]");
	char tmp_cluster[100],url[500],postfields[2000];
	string str_cluster;

	map<string,Path> ::iterator itPath; // iterator of mPath.
	vector<Cluster>::iterator itClusters; 

	#ifdef DEBUG
		syslog(LOG_NOTICE,"SAVING attributes[!!!]");
	#endif
	writeB(myAttMap);

	#ifdef DEBUG
		syslog(LOG_NOTICE,"SAVING User Attributes[!!!]");
	#endif
	writeB(myAttUserMap);

	#ifdef DEBUG
		syslog(LOG_NOTICE,"SAVING globals[!!!]");
	#endif

	// Production or Hybrid mode.
	writeB(score_numeric);
	writeB6(mScoreNumericAtt);
	// Learning mode
	writeB(score_data);
	writeB7(mScoreAtt);

	#ifdef DEBUG
		syslog(LOG_NOTICE,"SAVING Operations[!!!]" );
	#endif
	writeB(mOperation);

	#ifdef DEBUG
		syslog(LOG_NOTICE,"SAVING paths[!!!]");
	#endif

	for(itPath=mPath.begin();itPath!=mPath.end();itPath++){
		pthread_mutex_lock(&mutexAppMode);
		if(mAppMode[itPath->first].mode==2){
			#ifdef DEBUG
				syslog(LOG_NOTICE,"The application %s is in production mode.",itPath->first.c_str());
			#endif
			pthread_mutex_unlock(&mutexAppMode);
			continue;
		}
		pthread_mutex_unlock(&mutexAppMode);

		pthread_mutex_lock(&mutexSessionsPerHost); //lock
		itSessionsPerDomain = numOfSessionsPerDomain.find(itPath->first);
		if(itSessionsPerDomain==numOfSessionsPerDomain.end()){
			#ifdef DEBUG
				syslog(LOG_NOTICE,"Cannot Save %s - Not Enough Data",itPath->first.c_str());
			#endif
			pthread_mutex_unlock(&mutexSessionsPerHost); //unlock
			continue;
		}
		#ifdef DEBUG
		else{
			syslog(LOG_NOTICE,"SAVING %s DATA",itPath->first.c_str());
		}
		#endif

		unsigned short MinPts = (unsigned short)(itSessionsPerDomain->second*COEFFICIENT);
		pthread_mutex_unlock(&mutexSessionsPerHost); //unlock

		MinPts+=5;

		itPath->second.location.init(max_dist,MinPts);

		#ifdef DEBUG
			syslog(LOG_NOTICE,"----Clusters----");
		#endif

		str_cluster.clear();;
		itClusters = itPath->second.location.clusters.begin() ;
		while( itClusters != itPath->second.location.clusters.end() ){
			if( itClusters->getCentroid() == 0 ){
				itPath->second.location.clusters.erase(itClusters);
				continue;
			}

			itClusters->getMostDistantPoint();

			if(itClusters->Distant>4){
				itPath->second.location.clusters.erase(itClusters);
				continue;
			}
			#ifdef DEBUG
				itClusters->print_syslog();
			#endif

			sprintf(tmp_cluster,"\"(%f,%f)  R=%f\"",itClusters->centroid.x,itClusters->centroid.y,itClusters->Distant);
			if(str_cluster.size()==0){
				str_cluster = tmp_cluster;
			}else{
				str_cluster.push_back(',');
				str_cluster.append(tmp_cluster);
			}

			itClusters++;
		}

		#ifdef DEBUG
			syslog(LOG_NOTICE,"----------------");
		#endif
		
		snprintf(url,sizeof(url)-1,"http://localhost:9200/telepath-domains/domains/%s/_update",itPath->first.c_str());
		sprintf(postfields,"{\"doc\":{\"clusters\":[%s]}}",str_cluster.c_str());
		es_mapping(url,postfields);
		writeB4(itPath->second,itPath->first);
	}

	#ifdef DEBUG
		syslog(LOG_NOTICE,"SAVING User Paths[!!!]");
	#endif
	writeB(mPathUser);
	syslog(LOG_NOTICE,"SAVING END[!!!]");

}

