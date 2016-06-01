#include <iostream>
#include <syslog.h>

class ipRange{
public:
	unsigned int from;
	unsigned int to;

	ipRange(){}

	void init(unsigned int from,unsigned int to){
		this->from = from;
		this->to = to;
	}

	bool inRange(unsigned int num){
		if( (num >= this->from) && (this->to > num) ){
			return true;
		}else{
			return false;
		}
	}

	void print(){
		cout<<this->from<<"-"<<this->to<<endl;
	}

	void print_syslog(){
		syslog(LOG_NOTICE,"%u - %u",this->from,this->to);
	}

};

vector <ipRange> vRanges;

using namespace std;

void parse_ranges(string & output){
	ipRange range;
	unsigned int status=0,num1=0,num2=0;
	string str_num,final;
	for(unsigned int i=0; i < output.size() ; i++){
		switch (status){
			case 0:
				if(output[i]==':'){
					status++;
				}
				break;
			case 1:
				if(output[i]==','){
					status++;
					if(str_num.size()>2){
						for(unsigned int ii=0;ii<str_num.size();ii++){
							if(str_num[ii]>='0' && str_num[ii]<='9'){
								final.push_back(str_num[ii]);
							}
						}
					}else{
						final = str_num;
					}
					num1 = (unsigned int)atoi(final.c_str());
					str_num.clear();
					final.clear();
				}else{
					str_num.push_back(output[i]);
				}
				break;
			case 2:
				if(output[i]==':'){
					status++;
				}
				break;
			case 3:
				if(output[i]=='}'){
					status=0;
					if(str_num.size()>2){
						for(unsigned int ii=0;ii<str_num.size();ii++){
							if(str_num[ii]>='0' && str_num[ii]<='9'){
								final.push_back(str_num[ii]);
							}
						}
					}else{
						final = str_num;
					}
					num2 = (unsigned int)atoi(final.c_str());
					str_num.clear();
					final.clear();

					if(num1>num2){
						range.init(num2,num1);
					}else{
						range.init(num1,num2);
					}
					vRanges.push_back(range);
				}else{
					str_num.push_back(output[i]);
				}
				break;
		}
	}
}

void *thread_scheduler(void* threadid)
{
	#ifdef DEBUG
		syslog(LOG_NOTICE,"Scheduler Debug");
	#endif

	bool in_range;
	unsigned int current_time,i,hour;	
	string day,output;
	char buffer[1000];
	vector <string> tmpAppMode;
	boost::unordered_map <string,AppMode>::iterator itAppMode;

	sleep(60);
	while(globalEngine){
		sleep(60);
		current_time = (unsigned int)(time(NULL) );
		day = getWeekDay2(current_time);
		hour = getHour2(current_time);

                #ifdef DEBUG
                        syslog(LOG_NOTICE,"Day=%s   Hour=%u",day.c_str(),hour);
                #endif

                sprintf(buffer,"http://localhost:9200/telepath-scheduler/times/%s/_source",day.c_str());
                es_get_config(string(buffer),output);

                #ifdef DEBUG
                        syslog(LOG_NOTICE,"Elastic Output=%s",output.c_str());
                #endif

                vRanges.clear();
                parse_ranges(output);
                in_range = false;
                for(i=0;i<vRanges.size();i++){
                        #ifdef DEBUG
                                vRanges[i].print_syslog();
                        #endif
			if( vRanges[i].inRange(hour) ){
				in_range = true;
				break;
			}
		}

		tmpAppMode.clear();
		if( in_range == true){
			pthread_mutex_lock(&mutexAppMode);
			for(itAppMode = mAppMode.begin() ; itAppMode != mAppMode.end() ; itAppMode++ ){
				if(itAppMode->second.mode == 2){
					itAppMode->second.mode = 3;
					tmpAppMode.push_back(itAppMode->first);
					#ifdef DEBUG
						syslog(LOG_NOTICE,"[Memory]: Move To Hybrid");
					#endif
				}
			}
			pthread_mutex_unlock(&mutexAppMode);

			for(unsigned int i=0;i<tmpAppMode.size();i++){
				sprintf(buffer,"http://localhost:9200/telepath-domains/domains/%s/_update",tmpAppMode[i].c_str());
				es_mapping(buffer,"{\"doc\":{\"operation_mode\":\"3\"}}");
				#ifdef DEBUG
					syslog(LOG_NOTICE,"[ElasticSearch]: Move To Hybrid");
				#endif
			}
		}else{
			pthread_mutex_lock(&mutexAppMode);
			for(itAppMode = mAppMode.begin() ; itAppMode != mAppMode.end() ; itAppMode++ ){
				if(itAppMode->second.mode == 3){
					itAppMode->second.mode = 2;
					tmpAppMode.push_back(itAppMode->first);
					#ifdef DEBUG
						syslog(LOG_NOTICE,"[Memory]: Move To Production");
					#endif
				}
			}
			pthread_mutex_unlock(&mutexAppMode);

			for(unsigned int i=0;i<tmpAppMode.size();i++){
				sprintf(buffer,"http://localhost:9200/telepath-domains/domains/%s/_update",tmpAppMode[i].c_str());
				es_mapping(buffer,"{\"doc\":{\"operation_mode\":\"2\"}}");
				#ifdef DEBUG
					syslog(LOG_NOTICE,"[ElasticSearch]: Move To Production");
				#endif
			}

		}

		vector <string>().swap(tmpAppMode);
	}
	pthread_exit(NULL);

}

