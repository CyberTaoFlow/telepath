#include <iostream>
#include <curl/curl.h>
#include <syslog.h>

#include "teleindex.h"
#include "ElasticSearch.h"
#include "local_time.h"

class Range{
public:
	unsigned int from;
	unsigned int to;

	Range(){}

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

vector <Range> vRanges;

using namespace std;

void demonize()
{
	//to create process as a daemon, we need to untie it from current process session.
	//to do it correctly we need to make fork(), new process will be created, and it's session can be changed.
	int pid;

	pid=fork();

	switch (pid)
	{
		case 0:
			setsid();
			//log((char*)"ATMSd: starting in daemon mode",log_path.c_str());
			syslog (LOG_NOTICE, "%s","starting in daemon mode");
			break;
		case -1:
			cerr<<"ERROR! fork() failed!"<<endl;
			exit(1);
			break;
		default:
			exit(0);
	}
}

void parse_ranges(string & output){
	Range range;
	unsigned int status=0,num1,num2;
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

int main (int argc,char* array[])
{
	#ifdef DEBUG
		syslog(LOG_NOTICE,"Scheduler Debug");
	#endif

	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("Scheduler", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	demonize();
	char buffer[100];
	bool in_range;
	unsigned int current_time,i,hour,op_mode;	
	string day,output;
	while(1){
		sleep(5);
		es_get_config("http://localhost:9200/telepath-config/config/operation_mode_id/_source",output);
		op_mode = (unsigned int)atoi(output.c_str());
		if(op_mode==1){
			continue;
		}
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
		if( in_range == true){
			if(op_mode==2){
				es_insert("http://localhost:9200/telepath-config/config/operation_mode_id","{\"value\":\"3\"}");
			}
		}else{
			if(op_mode==3){
				es_insert("http://localhost:9200/telepath-config/config/operation_mode_id","{\"value\":\"2\"}");
			}
		}
	}
}
