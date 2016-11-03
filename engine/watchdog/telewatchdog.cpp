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
#include <fstream>

#include "../local_time.h"
#include "../ElasticSearch.h"
#include "../jsoncpp/json.h"
#include "../Sha1.h"

using namespace std;

//! Checking the Disk Space.
/*!
	This thread checks every 10 seconds the free space percentage.If that percentage is less than 20, the telewatchdog will delete the oldest elasticsearch index using delete_oldest() function. 
	\param threadid
	\return void*
	\sa statvfs
*/
void *thread_check_disk_space(void*);

//! Checking the Elasticsearch Responsiveness.
/*!
	This thread executes a simple curl query to the Elasticsearch every 30 seconds. If there is no response from Elasticsearch it means it is locked and es_restart_flag=true.  
	\param threadid
	\return void*
*/
void *thread_check_elasticsearch(void*);

//! Restarting Telepath when Elasticsearch not Responding.
/*!
	This thread checks the es_restart_flag value every second. If the es_restart_flag==true for 60 seconds than all process(telewatchdog included) will be restarted itself.
	\param threadid
	\return void*
*/
void *thread_restart_es_stuck(void*);

//! Starting/Stopping Suricata.
/*!
	This thread starts/stops suricata according to its sniffer_mode variable. Suricata can run in two modes(depending on the content of the global 'files_queue' queue):
	\n 1)sniffer mode - suricata receives packet flow from a given interface(suricata.yaml file) with a pcap filter.
	\n 2)tcpdump mode - suricata loads data from tcpdump file/s.
	\param threadid
	\return void*
	\sa http://192.168.1.222/index.php/Telepath_commands
*/
void *thread_init_suricata(void*);

//! Loading Telepath Status.
/*!
	Loading the "engine","sniffer" and "reverse_proxy" status(off/on) from Elasticsearch.
	\param none
	\return void*
*/
void loadInputMode();

//! Running the Process as a Daemon.
/*!
	A standard way to run the process as a deamon. Most of the work is done by the standard C fork() function.
        \param none
        \return void
        \sa fork() 
*/
void demonize();

//! Starting all Threads.
/*!
	Starting all the telewatchdog threads using the pthread_create() GNU function.
	\param none
	\return void
	\sa pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg)
*/
void startThreads();

//! Restarting all Telepath Processes.
/*!
	\param none
	\return void
	\sa popen(const char *command, const char *type)
	\sa pclose(FILE *stream)
*/
void restart_program();

//! Returning Process ID
/*!
	The function gets process name as an argument and returns its Linux process ID(PID).
	\param program_name a C++ string argument.
	\return the process ID as an integer argument.
*/
int lget_pid(string);

//! Checking the Existance of a Process.
/*!
	This function gets process name as an argument and returns an integer variable that indicates if the process is running or not.
	\param program_name a C++ string argument.
	\return 0 or 1 as an integer argument.
	\n	0 - The process is not running.
	\n	1 - The process is running.
*/
unsigned int get_ps(string);

//! Checking the Existance of a Process .
/*!
	This function gets command name as an argument and returns an integer variable that indicates if that command line is running or not.
	\param command_name a C++ string argument.
	\return 0 or 1 as an integer argument.
	\n      0 - The command is not running.
	\n      1 - The command is running.
*/
unsigned int get_full_ps(string);

//! Loading Suricata pcap filter/s.
/*!
	This function loads the Suricata pcap filter/s information from Elasticsearch. The full path is: http://localhost:9200/telepath-config/interfaces/interface_id .
	\param output a C++ string argument. 
	\param pcap a C++ string argument.
	\return void
*/
void get_pcap_filter(string & output,string & pcap);

//! Loading Suricata interface name/s.
/*!
	This function loads the Suricata interface name/s information from Elasticsearch. The full path is: http://localhost:9200/telepath-config/interfaces/interface_id .
	\param output a C++ string argument. 
	\param interface a C++ string argument.
	\return void
*/
void get_interface_name(string & output,string & interface);

//! Finding the oldest Elasticsearch index.
/*!
	This function finds the earliest/oldest Elasticsearch index date. For example the index telepath-20160808 is older than the index telepath-20160811
	\param ptr a pointer to C character.
	\return void
*/
void setMinShard(char *ptr);

//! Deleting the oldest Elasticsearch index.
/*!
	This function executes a DELETE curl query to remove the earliest/oldest Elasticsearch index date.
	\param none
	\return void
*/
void delete_oldest();

//!  Getting the File Names in a Directory.
/*!
	This function gets a directory name as an argument and returns an integer variable that indicates if there are any file/s inside.
	\param dir_name a C++ string argument.
	\param content a C++ string argument.
	\return 0,1 or -1 as an integer argument.
	\n	0 - No files inside the directory.
	\n	1 - There are file/s inside the directory (Those will be written to the content string).
	\n     -1 - An error has occured in the popen command.
*/
unsigned int get_ls_content(string & dir_name,string & content);

//!  Pushing file names to a global queue.
/*!
	This function pushes filename/s(absolute path/s) to the global 'files_queue' queue.
	\param dir_name a C++ string argument.
	\param content a C++ string argument.
	\return void
*/
void push_files_to_queue(string & dir_name,string & content);

//!  Checking the Telepath License Validity.
/*!
	This function gets a license key as an argument and returns a boolean variable that indicates if the license is Valid/Invalid.
	\param key a C++ string argument.
	\param epoch an unsigned integer argument.
	\return true or false as a boolean argument.
	\n      true - The license is Valid.
	\n      false - The license is Invalid.
*/
bool validKey(string & key,unsigned int & epoch);

//!  Allowing/Forbidding to use Telepath.
/*!
	This function blockes the user/client access to the Telepath when the license is Invalid or Expired.
	\param none
	\return true or false as a boolean argument.
	\n      true - The user can leagaly use Telepath (The license is Valid).
	\n      false - The user cannot use Telepath (The license is Invalid/Expired).
*/
bool checkLicenseKey();

//!  TODO - move to cron.
/*!
*/
void *thread_php_script(void*);

//!  TODO - move to cron.
/*!
*/
void *thread_php_script2(void*);

unsigned int countShard;
unsigned int minShard;
string minShardString;
int stopflag=0;
unsigned int engine_mode,reverse_proxy_mode,sniffer_mode;
queue <string> files_queue;
vector <string> interface_vec;
vector <string> pcapfilter_vec;
bool es_restart_flag=false;
pthread_mutex_t mutexDetails         = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_es_check         = PTHREAD_MUTEX_INITIALIZER;

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


void startThreads(){
	pthread_t thread_disk_space,thread_elasticsearch,thread_reset_es,thread_script,thread_script2; 
	unsigned long rc,empty=0;

	rc = pthread_create(&thread_disk_space, NULL,thread_check_disk_space, (void *)empty);// select info from user_groups table.
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}

	rc = pthread_create(&thread_elasticsearch, NULL,thread_check_elasticsearch, (void *)empty);// select info from user_groups table.
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}

	rc = pthread_create(&thread_reset_es, NULL,thread_restart_es_stuck, (void *)empty);// select info from user_groups table.
	if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}

	//rc = pthread_create(&thread_script, NULL,thread_php_script,(void *)empty);
	//if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}

	//rc = pthread_create(&thread_script2, NULL,thread_php_script2,(void *)empty);
	//if (rc){ printf("ERROR; return code from pthread_create() is %lu\n", rc); return;}
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

unsigned int get_full_ps(string command_name){
	char buf[1000],buffer[1000];
	int lSize;

	sprintf(buffer,"ps -aux | grep %s",(char*)command_name.c_str());

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

void get_pcap_filter(string & output,string & pcap){
	size_t find_pos = output.find("_filter\"");
	if(find_pos != string::npos ){
		find_pos += 10;

		pcap.clear();
		for(unsigned int i=find_pos;i<output.size();i++){
			if(output[i]=='\"'){
				break;
			}
			pcap.push_back(output[i]);
		}
	}
}

void get_interface_name(string & output,string & interface){
	size_t find_pos = output.find("interface_name\"");
	if(find_pos != string::npos ){
		find_pos += 17;

		interface.clear();
		for(unsigned int i=find_pos;i<output.size();i++){
			if(output[i]=='\"'){
				break;
			}
			interface.push_back(output[i]);
		}
	}
}

void *thread_init_suricata(void *threadid){
	char suricata_cmd[5000];
	FILE* ppipe_suricata;
	string output,pcap,interface;

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
					es_get_config("/telepath-config/interfaces/interface_id/_source",output);
					get_pcap_filter(output,pcap);
					get_interface_name(output,interface);

					sprintf( suricata_cmd,"ifconfig %s up; ifconfig %s promisc;",interface.c_str(),interface.c_str());
					syslog(LOG_NOTICE,"%s",suricata_cmd);
					ppipe_suricata = popen(suricata_cmd,"w");
					pclose(ppipe_suricata);

					syslog(LOG_NOTICE, "No more files to upload ... Reload Suricata");
					sprintf( suricata_cmd,"/opt/telepath/suricata/suricata -D -c /opt/telepath/suricata/suricata.yaml --af-packet %s > /dev/null 2>&1",pcap.c_str());
					es_insert("/telepath-config/config/file_loader_mode_id","{\"value\":\"0\"}");
				}else{
					string pcap_file = files_queue.front();
					syslog(LOG_NOTICE, "Next file to upload: %s", (char*)pcap_file.c_str() );
					files_queue.pop();
					sprintf( suricata_cmd,"/opt/telepath/suricata/suricata -r %s -c /opt/telepath/suricata/suricata.yaml > /dev/null 2>&1",(char*)pcap_file.c_str() );
					es_insert("/telepath-config/config/file_loader_mode_id","{\"value\":\"1\"}");
				}

				while(1){
					reply = (redisReply*)redisCommand(redis, "LLEN %s","Q" );
					if ( reply->type == REDIS_REPLY_ERROR ){
						freeReplyObject(reply);
						syslog(LOG_NOTICE,"REDIS ERROR");
						sprintf(suricata_cmd,"killall -9 redis-server");
						ppipe_suricata = popen(suricata_cmd,"w");
						pclose(ppipe_suricata);
						sleep(2);
						restart_program();
					}else{
						if(reply->integer > 1000000){
							syslog(LOG_NOTICE, "Redis is full: %lld",reply->integer);
							sleep(5);	
							break;
						}else if(reply->integer == 0){
							syslog(LOG_NOTICE, "Redis is empty");
							sleep(5);
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
	countShard=0;

	while(1){
		ptr = strcasestr(ptr,telepath);
		if(ptr>0){

			ptr += 9;
			strncpy(tmp,ptr,8);
			tmp[8] = '\0';
			minShardString = string(tmp);
			minShardString.insert(4,1,'-');
			minShardString.insert(7,1,'-');
			tmp_min = (unsigned int)atoi(tmp);
			if(tmp_min != 0){
				countShard++;
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
	sprintf(url,"%s/_settings",es_connect.c_str());
	curl_easy_setopt(curl, CURLOPT_URL,url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, setMinShard);
	curl_easy_perform(curl);

	if(minShard == 0){
		return;
	}

	if(countShard==1){
		syslog(LOG_NOTICE,"Engine is restarting because there is no enough disk space and the current elasticsearch index is going to be deleted.");
		FILE* ppipe_engine = popen("killall -9 engine > /dev/null 2>&1 || true","w");
		pclose(ppipe_engine);
	}

	sprintf(url,"%s/telepath-%u",es_connect.c_str(),minShard);
	curl_easy_setopt(curl,CURLOPT_CUSTOMREQUEST,"DELETE"); 
	curl_easy_setopt(curl, CURLOPT_URL,url);
	curl_easy_perform(curl);
        syslog(LOG_NOTICE,"curl -XDELETE %s", url);
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

void *thread_check_elasticsearch(void *threadid){
	CURL *curl;
	curl = curl_easy_init();
	char check_es[200];
	sprintf(check_es,"%s/telepath-config/_search",es_connect.c_str());
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
			if(counter == 60){
				syslog (LOG_NOTICE,"Restarting ElasticSearch");
				es_restart_flag = false;
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
	struct statvfs vfs;

	while(1){
		sleep(10);

		if (statvfs("/", &vfs) < 0)
			exit(-1);
		free_space_ratio = 100 * (double long)vfs.f_bavail/ (double long)vfs.f_blocks;
		if(free_space_ratio < 20){
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
	es_get_config("/telepath-config/config/engine_mode_id/_source",output);
	engine_mode = (unsigned int)atoi(output.c_str());
	es_get_config("/telepath-config/config/reverse_proxy_mode_id/_source",output);
	reverse_proxy_mode = (unsigned int)atoi(output.c_str());
	es_get_config("/telepath-config/config/sniffer_mode_id/_source",output);
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
	read_connect_conf_file();
	es_get_config("/telepath-config/config/license_key_id/_source",license_key);

	check_time = (unsigned int)time(NULL);
	if( validKey(license_key,epoch) == true ){
		if(check_time > 2678400 + epoch){
			es_insert("/telepath-config/config/license_mode_id","{\"value\":\"EXPIRED\"}");

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
			es_insert("/telepath-config/config/license_mode_id","{\"value\":\"VALID\"}");
			return true;
		}
	}else{
		es_insert("/telepath-config/config/license_mode_id","{\"value\":\"INVALID\"}");

		syslog(LOG_NOTICE,"The license of your product may be invalid ... please check your license");
		syslog(LOG_NOTICE,"In a case that you are sure that you have a valid license, please check the content of the /opt/telepath/db/elasticsearch/config/connect.conf file which indicates about the ip and the port of the elasticsearch. In a case that this file is not exist the defaulf value is http://localhost:9200 .");
		syslog(LOG_NOTICE,"Waiting 10 seconds before checking the license key and the connection again ...");
		sleep(10);
		
		return false;
	}

}

int main(int argc, char *argv[])
{
	

	//Giving a syslog name to the process.
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("Telewatchdog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	//Showing the git revision and the compilation time.
	if( argc > 1 &&  strcmp("-v",argv[1]) == 0 ){
		cout <<"Watchdog Version:"<< GIT_REV;
		print_time(COMPILATION_TIME);
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

	read_connect_conf_file();
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

		syslog (LOG_NOTICE,"Starting Elasticsearch[!!!] --file=%s",es_connect.c_str());
		syslog (LOG_NOTICE, "%s",open_elastic);

		FILE* ppipe_elastic = popen(open_elastic, "w");
		pclose(ppipe_elastic);
		sleep(15);
	}

	initElasticSearchData(); //Initializing all the elasticsearch data.
	while( checkLicenseKey() == false ); // Checking license keygen (Valid/Invalid/Expired).
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
