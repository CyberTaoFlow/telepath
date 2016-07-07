#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <syslog.h>
#include <sys/statvfs.h>
#include <map>
#include <set>
#include <queue>
#include <ifaddrs.h>
#include <netdb.h>
#include <dirent.h>
#include <hiredis/hiredis.h>

#include "../ElasticSearch.h"
#include "../jsoncpp/json.h"
#include "../Sha1.h"

#define ATMS 1
#define ENGINE 2
#define SCHEDULER 5
#define GULPNAME "gulp"

using namespace std;

struct statvfs vfs;
unsigned int minShard;
int stopflag=0;
unsigned int engine_mode,reverse_proxy_mode,sniffer_mode;
queue <string> files_queue;

vector <string> interface_vec;
vector <string> pcapfilter_vec;


bool es_restart_flag=false;

void *thread_check_disk_space(void*);
void *thread_check_elasticsearch(void*);
void *thread_restart_es_stuck(void*);
void *thread_init_suricata(void*);
void *thread_php_script(void*);
void *thread_php_script2(void*);
void loadInputMode();

pthread_mutex_t mutexDetails         = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_es_check         = PTHREAD_MUTEX_INITIALIZER;

/*
 * void demonize()
 * 		Demonizing process
 * Parameters:
 * 		No
 * Return value:
 * 		Doesn't return value
 */
void demonize(){

	//to create process as a daemon, we need to untie it from current process session.
	//to do it correctly we need to make fork(), new process will be created, and it's session can be changed.
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


void startThreads(){
	pthread_t thread_disk_space,thread_elasticsearch,thread_reset_es,thread_script,thread_script2; 
	unsigned long rc,empty=0;

	rc = pthread_create(&thread_disk_space, NULL,thread_check_disk_space, (void *)empty);// select info from user_groups table.
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}

	rc = pthread_create(&thread_elasticsearch, NULL,thread_check_elasticsearch, (void *)empty);// select info from user_groups table.
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}

	rc = pthread_create(&thread_reset_es, NULL,thread_restart_es_stuck, (void *)empty);// select info from user_groups table.
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}

	rc = pthread_create(&thread_script, NULL,thread_php_script,(void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}

	rc = pthread_create(&thread_script2, NULL,thread_php_script2,(void *)empty);
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}
}

void restart_program()
{
	FILE *fd = popen("telepath restart > /dev/null 2>&1","w");
	pclose(fd);

	sleep(15);
}

int lget_pid(string program_name){

	string first,second,command="pidof ";
	int pid,pos;

	FILE *fd;
	fd=popen((command+program_name).c_str(),"r");
	if(fd!=NULL){
		//fseek (fd , 0 , SEEK_END);
		//rewind (fd);

		char buf[256];
		fgets(buf, sizeof(buf), fd);

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

unsigned int get_ps(string program_name){
	char buf[1000],buffer[1000];
	int lSize;

	sprintf(buffer,"ps -A | grep %s",(char*)program_name.c_str());

	FILE *fd;
	memset(buf,'\0',sizeof(buf));
	fd=popen(buffer,"r");	//opening pipe to run command
	if(fd!=NULL)
	{
		fseek (fd , 0 , SEEK_END);		//going to the end of file
		lSize = ftell (fd);				//get current position of end (t.e. size)
		rewind (fd);					//going back to beginning
		fread(buf,1,lSize,fd);			//reading result

		pclose(fd);

		if(strlen(buf) < 2){
			return 0;
		}else{
			return 1;
		}
	}
	else
	{
		cout<<"cannot open pipe"<<endl;
		return -1;
	}
}

unsigned int get_full_ps(string program_name){
	char buf[1000],buffer[1000];
	int lSize;

	sprintf(buffer,"ps -aux | grep %s",(char*)program_name.c_str());

	FILE *fd;
	memset(buf,'\0',sizeof(buf));
	fd=popen(buffer,"r");	//opening pipe to run command
	if(fd!=NULL)
	{
		fseek (fd , 0 , SEEK_END);		//going to the end of file
		lSize = ftell (fd);				//get current position of end (t.e. size)
		rewind (fd);					//going back to beginning
		fread(buf,1,lSize,fd);			//reading result

		pclose(fd);

		if(strlen(buf) < 2){
			return 0;
		}else{
			return 1;
		}
	}
	else
	{
		cout<<"cannot open pipe"<<endl;
		return -1;
	}
}

void get_pcap_filter(string & pcap){
	string tmp;
	size_t find_pos = pcap.find("_filter\"");
	if(find_pos != string::npos ){
		find_pos += 10;

		tmp.clear();
		for(unsigned int i=find_pos;i<pcap.size();i++){
			if(pcap[i]=='\"'){
				break;
			}
			tmp.push_back(pcap[i]);
		}
		pcap=tmp;
	}
}

void *thread_init_suricata(void *threadid){
	char suricata_cmd[5000];
	FILE* ppipe_suricata;
	string output;

	redisContext *redis;
	redisReply *reply;

	redis = redisConnect("127.0.0.1", 6379);
	if(redis->err) {
		exit(EXIT_FAILURE);
	}

	while(1){
		sleep(1);
		loadInputMode();
		if(sniffer_mode==1){
			if( (get_ps("Suricata-Main")==0) ){
				if( files_queue.empty() ){
					es_get_config("http://localhost:9200/telepath-config/interfaces/interface_id/_source",output);
					get_pcap_filter(output);

					syslog(LOG_NOTICE, "No more files to upload ... Reload Suricata");
					sprintf( suricata_cmd,"/opt/telepath/suricata/suricata -D -c /opt/telepath/suricata/suricata.yaml --af-packet %s > /dev/null 2>&1",output.c_str());
					es_insert("http://localhost:9200/telepath-config/config/file_loader_mode_id","{\"value\":\"0\"}");
				}else{
					string pcap_file = files_queue.front();
					syslog(LOG_NOTICE, "Next file to upload: %s", (char*)pcap_file.c_str() );
					files_queue.pop();		
					sprintf( suricata_cmd,"/opt/telepath/suricata/suricata -r %s -c /opt/telepath/suricata/suricata.yaml > /dev/null 2>&1",(char*)pcap_file.c_str() );
					es_insert("http://localhost:9200/telepath-config/config/file_loader_mode_id","{\"value\":\"1\"}");
				}

				while(1){
					reply = (redisReply*)redisCommand(redis, "LPOP %s","Q" );
					if ( reply->type == REDIS_REPLY_ERROR ){
						freeReplyObject(reply);
						syslog(LOG_NOTICE,"REDIS ERROR");
						sprintf(suricata_cmd,"killall -9 redis-server");
						ppipe_suricata = popen(suricata_cmd,"w");
						pclose(ppipe_suricata);
						sleep(2);
						restart_program();
					}else{
						if(reply->str != NULL){
							syslog(LOG_NOTICE, "Redis is full");
							sleep(20);
						}else{
							syslog(LOG_NOTICE, "Redis is empty");
							sleep(10);
							break;
						}
					}
				}

				syslog (LOG_NOTICE,"%s",suricata_cmd);
				ppipe_suricata = popen(suricata_cmd,"w");
				pclose(ppipe_suricata);
				sleep(2);		
			}
		}else{
			ppipe_suricata = popen("killall -9 Suricata-Main > /dev/null 2>&1 || true","w");
			pclose(ppipe_suricata);
 		}
		sleep(5);
	}

}

void setMinShard(char *ptr)
{
	static const char * telepath = "telepath-";
	char tmp[20];
	unsigned int tmp_min,min_shard=0;

	while(1){
		ptr = strcasestr(ptr,telepath);
		if(ptr>0){

			ptr += 9;
			strncpy(tmp,ptr,8);
			tmp[8] = '\0';
			tmp_min = (unsigned int)atoi(tmp);
			if(tmp_min != 0){
				if(min_shard == 0){
					min_shard = tmp_min;
				}else{
					if(min_shard > tmp_min){
						min_shard = tmp_min;
					}
				}
			}
		}else{
			break;
		}
	}

	minShard = min_shard;
}

void delete_oldest()
{

	CURL *curl;
	char url[500];
	curl = curl_easy_init();

        syslog(LOG_NOTICE,"No Disk Space, Clean Old data" );
        // we need to delete the oldest shard

	curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9200/_settings");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, setMinShard);
	curl_easy_perform(curl);

	if(minShard == 0){
		return;
	}

	sprintf(url,"http://localhost:9200/telepath-%u",minShard);

	curl_easy_setopt(curl,CURLOPT_CUSTOMREQUEST,"DELETE"); 
	curl_easy_setopt(curl, CURLOPT_URL,url);
	curl_easy_perform(curl);

        syslog(LOG_NOTICE,"curl -XDELETE %s", url);

	FILE* ppipe_logs_delete = popen("rm /opt/telepath/db/elasticsearch/logs/* > /dev/null 2>&1", "w");
	pclose(ppipe_delete);
	FILE* ppipe_redis_delete = popen("rm /var/lib/redis/dump.rdb > /dev/null 2>&1", "w");
	pclose(ppipe_redis_delete);

}

unsigned int get_ls_content(string & dir_name,string & content){
	char buf[1000],buffer[1000];
	int lSize;

	sprintf(buffer,"ls %s",(char*)dir_name.c_str());

	FILE *fd;
	memset(buf,'\0',sizeof(buf));
	fd=popen(buffer,"r");	//opening pipe to run command
	if(fd!=NULL)
	{
		fseek (fd , 0 , SEEK_END);		//going to the end of file
		lSize = ftell (fd);				//get current position of end (t.e. size)
		rewind (fd);					//going back to beginning
		fread(buf,1,lSize,fd);			//reading result

		content = buf;
		pclose(fd);

		if(strlen(buf) < 2){
			return 0;
		}else{
			return 1;
		}
	}
	else
	{
		cout<<"cannot open pipe"<<endl;
		return -1;
	}
}

void push_files_to_queue(string & dir_name,string & content){
	string tmp_str;
	for(unsigned int i=0;i<content.size();i++){
		if(content[i]=='\n'){
			files_queue.push(dir_name+"/"+tmp_str);
			tmp_str.clear();
		}else{
			tmp_str.push_back(content[i]);
		}
	}
}

/*
 * void sighandler(int sig)
 * 		Signal handler
 * Parameters:
 * 		int sig - signal ID
 * Return value:
 * 		Doesn't return value
 */

void sighandler(int sig){

	//the thing is: watchdog runs another processes. If child process stopped, watchdog must receive exit status of it's child process.
	//Or child process will be zombie.

	//If wait() or waitpid() return because the status of a child process is available, these functions shall return a value equal to the process ID
	//of the child process. In this case, if the value of the argument stat_loc is not a null pointer, information shall be stored in the location
	//pointed to by stat_loc. The value stored at the location pointed to by stat_loc shall be 0 if and only if the status returned is from a terminated child
	//process that terminated by one of the following means:

	//	1.	The process returned 0 from main().
	//	2.	The process called _exit() or exit() with a status argument of 0.
	//	3.	The process was terminated because the last thread in the process terminated.

	//So watchdog must handle  	SIGCHLD signal

	int st=0;//status for wait() function

	if(sig == SIGCHLD)
	{
		wait(&st);		//reads exit status of child process
		if(st==256)		//if its 0 ??? just print status
		{
		}
	}
	else if(sig == SIGINT)
	{

	}

	else if(sig == SIGUSR1)	//receives signal, sent by "telepath" script
	{
		stopflag=1;
	}
}

/*
 * string getprocessinfo()
 * 		Function runs once during timer interval
 * Parameters:
 * 		No
 * Return value:
 * 		Doesn't return value
 */
void getprocessinfo(){

	char buf[2000];

	string s="", delimeter;
	FILE *fd;
	vector<string> commands, results;
	delimeter[0]='\n';
	//creates vector of commands to run. It checks, if process is running
	commands.push_back("ps aux | grep '[e]ngine'");
	commands.push_back("ps aux | grep '[a]tmsd'");
	commands.push_back("ps aux | grep '[m]ysqld'");
	commands.push_back("head -n2 /proc/meminfo;");
	commands.push_back("head -n15 /proc/meminfo | tail -n2;");

	//runs every command and logs result to syslog
	for(vector<string>::iterator i=commands.begin(); i!=commands.end(); ++i)
	{
		memset(buf,'\0',sizeof(buf));
		fd=popen((*i).c_str(),"r");	//opens pipe and runs command
		if(fd!=NULL)
		{
			fread(buf,1,sizeof(buf),fd);	//reads result from pipe
			syslog (LOG_NOTICE, "%s",buf);
			pclose(fd);
		}
		else
		{
			syslog (LOG_NOTICE, "%s","Can't open pipe for:");
			syslog (LOG_NOTICE, "%s",(char*)(*i).c_str());
		}
	}
}

// Execute a simple curl query to check ElasticSearch responsiveness.
void *thread_check_elasticsearch(void *threadid){
	CURL *curl;
	curl = curl_easy_init();
	static const char * check_es = "http://localhost:9200/telepath-rules/_search";
	sleep(60);
	while(1){
		sleep(30);

		pthread_mutex_lock(&mutex_es_check);
		es_restart_flag = true;
		pthread_mutex_unlock(&mutex_es_check);

		curl_easy_setopt(curl, CURLOPT_URL,check_es);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dropprinting);
		curl_easy_perform(curl);

		pthread_mutex_lock(&mutex_es_check);
		es_restart_flag = false;
		pthread_mutex_unlock(&mutex_es_check);
	}
	return NULL;
}

// When ElasticSearch doesn't respond, the system restarting itself.
void *thread_restart_es_stuck(void *threadid){
	unsigned int counter=0;

	while(1){
		sleep(5);

		while(1){
			pthread_mutex_lock(&mutex_es_check);
			if(es_restart_flag == false){
				pthread_mutex_unlock(&mutex_es_check);
				counter=0;
				break;
			}
			pthread_mutex_unlock(&mutex_es_check);
			counter++;

			syslog (LOG_NOTICE,"Trying to reconnect to ElasticSearch ... %u.",counter);
			if(counter == 20){
				syslog (LOG_NOTICE,"Restarting ElasticSearch");
				restart_program();
			}
			sleep(1);
		}
	}
	return NULL;
}

void *thread_check_disk_space(void *threadid)
{
	double long free_space_ratio;

	while(1){
		sleep(10);

		if (statvfs("/", &vfs) < 0)
			exit(-1);
		free_space_ratio = 100 * (double long)vfs.f_bavail/ (double long)vfs.f_blocks;
		if(free_space_ratio < 10){
			delete_oldest();
		}
	}
	return NULL;
}

void *thread_php_script(void *threadid)
{

	while(1){
		sleep(60);

		FILE* ppipe_php = popen("php /opt/telepath/ui/html/index.php cases flag_requests_by_cases  >> /var/log/flag_requests_by_cases.log 2>&1 || true","w");
		pclose(ppipe_php);		
	}
	return NULL;
}

void *thread_php_script2(void *threadid)
{

        while(1){
                sleep(3600);

                FILE* ppipe_php = popen("php /opt/telepath/ui/html/index.php cases store_similar_case_sessions  >> /var/log/store_similar_case_sessions.log 2>&1 || true","w");
                pclose(ppipe_php);
        }
        return NULL;
}

void loadInputMode(){
	string output;
	es_get_config("http://localhost:9200/telepath-config/config/engine_mode_id/_source",output);
	engine_mode = (unsigned int)atoi(output.c_str());
	es_get_config("http://localhost:9200/telepath-config/config/reverse_proxy_mode_id/_source",output);
	reverse_proxy_mode = (unsigned int)atoi(output.c_str());
	es_get_config("http://localhost:9200/telepath-config/config/sniffer_mode_id/_source",output);
	sniffer_mode = (unsigned int)atoi(output.c_str());
}

bool validKey(string & key,unsigned int & epoch){
	vector <string> vecStr;
	unsigned int i,len = key.size();
	string tmp;
	if(len != 36){
		return false;
	}

	for(i=0; i<len ;i++){
		if(key[i]=='-'){
			vecStr.push_back(tmp);
			tmp.clear();

			if(vecStr.size()==1){
				if(vecStr[0].size() != 8 ){
					return false;
				}
			}
			else if( vecStr.size()>1 && vecStr.size()<5){
				if(vecStr[vecStr.size()-1].size() != 4 ){
					return false;
				}
			}else{
				return false;
			}
		}
		else if(i==len-1){
			tmp.push_back(key[i]);
			vecStr.push_back(tmp);
			tmp.clear();

			if(vecStr.size()==5){
				if(vecStr[4].size() != 12 ){
					return false;
				}
			}else{
					return false;
			}

		}else{
			tmp.push_back(key[i]);
		}
	}

	string v1 = vecStr[0] + "HybridSeed~!";
	string v2 = vecStr[1] + "HybridTime~!";

	string sign_seed,sign_time;
	SHA1::RunSHA1(v1,& sign_seed);
	SHA1::RunSHA1(v2,& sign_time);
	


	if(vecStr[4][0]=='0'){
		sign_seed.erase(sign_seed.begin()+11,sign_seed.end());
		vecStr[4].erase(vecStr[4].begin());
	}else{
		sign_seed.erase(sign_seed.begin()+12,sign_seed.end());	
	}


	if(vecStr[2][0]=='0'){
		sign_time.erase(sign_time.begin()+3,sign_time.end());
		vecStr[2].erase(vecStr[2].begin());
	}else{
		sign_time.erase(sign_time.begin()+4,sign_time.end());	
	}


	if(sign_seed != vecStr[4]){
		return false;
	}

	if(vecStr[3] != "TPTH"){
		return false;
	}

	if(sign_time != vecStr[2]){
		return false;
	}

	vecStr[1].erase( vecStr[1].begin()+3,vecStr[1].end() );
	epoch = strtoul(vecStr[1].c_str(),NULL,36) * 3600 + 1388534400;

	return true;

}

bool checkLicenseKey(){
	string license_key;
	unsigned int check_time,epoch;
	es_get_config("http://localhost:9200/telepath-config/config/license_key_id/_source",license_key);

	check_time = (unsigned int)time(NULL);
	if( validKey(license_key,epoch) == true ){
		if(check_time > 2678400 + epoch){
			es_insert("http://localhost:9200/telepath-config/config/license_mode_id","{\"value\":\"EXPIRED\"}");

			char open_elastic[300];
			sprintf(open_elastic,"/opt/telepath/db/elasticsearch/bin/elasticsearch -d -Des.insecure.allow.root=true");
			FILE* ppipe_elastic = popen(open_elastic, "w");
			pclose(ppipe_elastic);

			sleep(2);
			syslog(LOG_NOTICE,"***Trial Version Expired***");
			cout <<"***Trial Version Expired***"<<endl;
			cout <<"[!] Trial Version Expired ... Telepath Cannot Start."<<endl;
			exit(1);
		}else{
			es_insert("http://localhost:9200/telepath-config/config/license_mode_id","{\"value\":\"VALID\"}");
			return true;
		}
	}else{
		es_insert("http://localhost:9200/telepath-config/config/license_mode_id","{\"value\":\"INVALID\"}");

		syslog(LOG_NOTICE,"***Invalid License***");
		syslog(LOG_NOTICE,"Waiting 10 seconds before checking the license key again ...");
		cout <<"[!] Invalid License ... Telepath Cannot Start."<<endl;
		sleep(10);
		
		return false;
	}

}

int main(int argc, char *argv[])
{
	
        time_t timer=0;

	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("Telewatchdog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	if( argc > 1 &&  strcmp("-v",argv[1]) == 0 ){
		cout <<"Version:"<< GIT_REV <<endl;
		exit(1);
	}

	if (lget_pid("telewatchdog")!=-1){
		syslog(LOG_NOTICE,"The telewatchdog is already running");
		exit(0);
	}

	else if(argc > 3){
		syslog (LOG_NOTICE, "Too Many Arguments");
		exit(1);
	}

	syslog (LOG_NOTICE, "The telewatchdog has started...");

	demonize();
	signal(SIGCHLD,sighandler);
	signal(SIGUSR1,sighandler);

	// Suricata is reading from a tcpdump file.
	if( (argc == 3) ){
		if(strcmp("--pcap-file",argv[1]) == 0){
			string pcap_file = argv[2];
			files_queue.push(pcap_file);
		}
		else if(strcmp("--pcap-dir",argv[1]) == 0){
			string content;
			string dir_name = argv[2];
			get_ls_content(dir_name,content);
			if(content.size() > 0){
				push_files_to_queue(dir_name,content);
			}
		}
	}

	sleep(3);
	#ifdef REDIS

	if(get_full_ps("[/opt/]telepath/db/elasticsearch")==0){
		char open_elastic[300];
		//unsigned int half_of_mem = ( unsigned int )( (sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE)) / (1000*1000*2000) );
		//if(half_of_mem==0){
			sprintf(open_elastic,"/opt/telepath/db/elasticsearch/bin/elasticsearch -d -Des.insecure.allow.root=true");
		//}else{
		//	sprintf(open_elastic,"/opt/telepath/db/elasticsearch/bin/elasticsearch -d -Xmx%ug -Xms%ug",half_of_mem,half_of_mem);
		//}

		syslog (LOG_NOTICE,"Starting Elasticsearch[!!!]");
		syslog (LOG_NOTICE, "%s",open_elastic);

		FILE* ppipe_elastic = popen(open_elastic, "w");
		pclose(ppipe_elastic);
		sleep(15);
	}

	initElasticSearchData();
	while( checkLicenseKey() == false );
	startThreads();
	loadInputMode();

	if(reverse_proxy_mode==1){
		if(get_ps("nginx")==0){
			syslog (LOG_NOTICE, "httpd stop[!!!]");
			FILE* ppipe_service = popen("service httpd stop", "w");
			pclose(ppipe_service);

			syslog (LOG_NOTICE, "Starting nginx[!!!]");
			FILE* ppipe_nginx = popen("/opt/telepath/openresty/nginx/sbin/nginx &", "w");
			pclose(ppipe_nginx);
			sleep(1);
		}
	}

	if(get_ps("redis-server")==0){
		syslog (LOG_NOTICE, "Starting Redis[!!!]");
		FILE* ppipe_redis = popen("nohup redis-server &", "w");
		pclose(ppipe_redis);
		sleep(1);
	}

	if(engine_mode==1){
		syslog (LOG_NOTICE, "Starting Engine[!!!]");
		FILE* ppipe_eng = popen("/opt/telepath/bin/engine", "w");
		pclose(ppipe_eng);
	}

	FILE* af_packet = popen("/opt/telepath/suricata/af-packet.sh > /dev/null 2>&1 || true", "w");
	pclose(af_packet);

	//----- Initiating Suricata thread -----

	pthread_t thread_suricata;
	unsigned long rc,empty=0;
	syslog (LOG_NOTICE, "Starting Suricata[!!!]");

	rc = pthread_create(&thread_suricata, NULL,thread_init_suricata, (void *)empty);// select info from user_groups table.
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc);}

	sleep(5);
	#endif

	while(stopflag==0)
	{
		sleep(5);
		loadInputMode();

		//running function to print info to syslog
		if(time(NULL)-timer>900){
			timer=time(NULL);
			getprocessinfo();
		}

		#ifdef REDIS

			if(reverse_proxy_mode==1){
				if(get_ps("nginx")==0){
					syslog (LOG_NOTICE, "nginx is not running - Restart");
					restart_program();				
				}
			}

			if(get_ps("redis-server")==0){
				syslog (LOG_NOTICE, "Starting Redis[!!!]");
				FILE* ppipe_redis = popen("nohup redis-server &", "w");
				pclose(ppipe_redis);
				sleep(1);
			}

			if(get_full_ps("[/opt/]telepath/db/elasticsearch")==0){
				syslog (LOG_NOTICE, "elasticsearch is not running - Restart");
				restart_program();
			}

			if( engine_mode==1 ){
				if(get_ps("engine")==0){
					syslog (LOG_NOTICE, "Engine Restart");
					FILE* ppipe_engine = popen("/opt/telepath/bin/engine", "w");
					pclose(ppipe_engine);
				}
			}else{
				FILE* ppipe_engine = popen("killall -9 engine > /dev/null 2>&1 || true","w");
				pclose(ppipe_engine);
			}

		#endif
	}
}


