
#include "../h/sniffer.h"
#include "../h/sendarrayobj.h"
#include "../h/config.h"

//////////////////////////////////////////CONSTANTS///////////////////////////////////////////
#define C_SNIFFER_MESSAGE_SEPARATOR "sniffer_message_separator"
#define C_GULPPATH "gulppath"
#define C_PIPEPATH "pipepath"
#define C_GULP_COMMAND "gulp_command"
#define C_PIPENAME "pipename"
#define CONFIG_FILENAME "config"
#define C_AGENTID "agentid"
#define C_SERVERIP "ip"
#define C_SERVERPORT "port"
#define C_PCAPSIZELIMIT "pcapfilesize"
#define C_COOKIENAME "cookie_name"
#define C_LOGTYPE "printmode"
#define C_NETWORK_INTERFACE "network_interface"
#define C_JSNIFFER_NETWORK_INTERFACE "jsniffer_network_interface"
#define C_SESSION_EXPIRETIME "session_expiretime"
#define C_STATUSINTERVAL "statusinterval"
#define C_CONCURRENTSESSIONS "concurrent_sessions"
#define C_QUEUELIMIT "queuelimit"
#define C_HEADERS_MAXLENGTH "max_headers_length"
#define C_GET_MAXLENGTH "max_getparams_length" 
#define C_POST_MAXLENGTH "max_post_length"
#define C_FilterExpression "serverip_filter"
#define C_MOUNTSIZE "mountsize"
#define C_THREADSNUM "threadsnum"
#define C_XMLOUTPUT "parseXML"
#define GULPNAME "gulp"

extern int open_pcap_pipe(FILE *pipe);
extern int read_pcap_pipe();
extern int close_pcap_pipe();

//extern map<string,string> mConf;
FILE * pipehandler = NULL;							//pipe handler for the pipe that the gulp writes to
string pipeFullName;    					//absolute path for the pipe that the gulp writes to
unsigned long long sentCounter = 0;			//couter for requests sent thus far (sent==written to the std after justniffe parsing)
extern map<string, string> mfingerprints;
extern sendArrayObj * sendqueue;
typedef map<string, s_sessioninfo> MapStringKeySessionsValue;

//pthread_t thread_pipeToStdout;
//int pipeToStdout_thread_handler;

pthread_t thread_requestsRateStats;
int requestsRateStats_thread_handler;
extern Config* conf_current;
extern Config* conf_updated;

void CreateGulpPipe()
{
	return;
        int ret_val;
        pipeFullName.clear();
        pipeFullName.append(conf_updated->mConfig["gulp_to_sniffer_pipepath"]);
        //if(strcmp(sinterface.c_str(), "") != 0)
        //      lpipename.append("_" + sinterface);
        printf("Trying to open pipe %s\n", pipeFullName.c_str());
        syslog(LOG_INFO, "Trying to open pipe %s", pipeFullName.c_str());
        if (pipehandler != NULL)
        {
                fclose(pipehandler);
        }
        /* Create the named - pipe */
        ret_val = mkfifo(pipeFullName.c_str(), 0666);
        if ((ret_val == -1) && (errno != EEXIST)) {
                fprintf(stderr, "Failed to create pipe\n");
                perror("Error creating the pipe");
                syslog(LOG_ERR, "Error creating the pipe");
                //exit(1);
        }
}
//The function opens the pipe for reading.
void OpenPipeRead(/*string sinterface*/)
{
	return;
	//int ret_val;
	pipeFullName.clear();
	pipeFullName.append(conf_updated->mConfig["gulp_to_sniffer_pipepath"]);
	//if(strcmp(sinterface.c_str(), "") != 0)
	//	lpipename.append("_" + sinterface);		
	printf("Trying to open pipe %s\n", pipeFullName.c_str());
	syslog(LOG_INFO, "Trying to open pipe %s", pipeFullName.c_str()); 
	if (pipehandler != NULL)
	{
		fclose(pipehandler);
	}
	if((pipehandler = fopen(pipeFullName.c_str(), "r+")) == NULL)
	{
		fprintf(stderr, "Failed to open pipe %s\n", pipeFullName.c_str() );
		syslog(LOG_ERR, "Failed to open pipe.");
		exit(1);
	}
	printf("Pipe opened: %s\n", pipeFullName.c_str());
	syslog(LOG_INFO, "Pipe opened: %s", pipeFullName.c_str());
}


//The function kill process by pid.
void KillProgramByPID(size_t lpid)
{
	char sys_command[30];
	sprintf(sys_command, "kill -15 %d", (int)lpid);
	printf("%s\n", sys_command);
	system(sys_command);
	sleep(3);
}


void killGulp(size_t lpid){
	//	size_t lpid;
	//lpid = get_pidof(GULP_PROGRAMNAME);
	if (lpid==0) 
		return;
	char sys_command[30];
	memset(sys_command, '\0', sizeof(sys_command));
	sprintf(sys_command, "kill -15 %d", (int)lpid);
//	printf("Killed %s\n",GULP_PROGRAMNAME);
//	syslog(LOG_INFO, "Killed %s", GULP_PROGRAMNAME);
	system(sys_command);
}





//The function check if there another instance of telepath_sniff, if there is the return the pid, else return -1.
//note: The function not support more then 1 instance at the same time.
/*unsigned int kill_all_program_instances(const char * program_name)
{
	int res;
	char buf[1024];
	string first;
	string second;
	int lSize, pid , pos;
	string command="pidof ";
	FILE *fd;
	memset(buf,'\0',sizeof(buf));
	printf("Getting the pid of %s (%s)\n",program_name,(command+program_name).c_str());
	fd=popen((command+program_name).c_str(),"r");
	unsigned int mypid = getpid();
	printf("Running process id: %d\n", mypid);

	if(fd != NULL)
	{
	 	printf ( "FILE: %p\n", fd);
		printf("fd not null\n");
		fseek (fd , 0 , SEEK_END);
		lSize = ftell(fd);
		rewind (fd);
		if (lSize <= 0)
		{
			printf("lsize is %d\n",lSize);
			pclose(fd);
			return 0;
		}
		res = fread(buf, 1, lSize, fd);
		pclose(fd);
		if (res == 0)
		{
			printf("res is zero\n");
			return 0;
		}
		printf("buf is %s\n",buf);
		//Move to string from chars array.
		first = buf;
		//Get the other process and delete the exists one from the list.
		while((pos = first.find(" ")) != string::npos)
		{
			second = first;

			//Delete found pid.
			first.erase(0, pos+1);

			//Remove the end.
			second.erase(pos, second.length() - pos);

			//Checks for the right pid.
			pid = atoi(second.c_str());
			printf("the pid we found before comparison is :%d\n",pid);
			if(pid != mypid)
			{
				//Kill gulp instance.
				if (strcmp(program_name,GULP_PROGRAMNAME)==0){
					printf("Killed %s\n",GULP_PROGRAMNAME);	
					killGulp(pid);
					waitpid(pid, 0, 0);
				}
				else {
					KillProgramByPID(pid);
				}
			}
		}

		if(first.length() > 0)
		{
			//Checks for the right pid.
			pid = atoi(first.c_str());
			if(pid != getpid())
			{
				//Kill telepath instance.
				if (strcmp(program_name,GULP_PROGRAMNAME)==0){
					printf("Killed %s\n",GULP_PROGRAMNAME);	
					killGulp(pid);
					waitpid(pid, 0, 0);
				}
				else
					KillProgramByPID(pid);
			}
		}

		return 0;
	}
	else
	{
		cout << "cannot open pipe" << endl;
		return 0;
	}
}
*/
unsigned int kill_all_program_instances(const char * program_name)
{

	if( strcmp(program_name,GULPNAME) == 0 ){
		return 1;
	}

	FILE *fp;
	int status;
	char pid_to_kill_str[1024];
	unsigned int pid_to_kill_int;
	char* split_pids;
	string command = "pidof ";
	unsigned int mypid = getpid(); 
	fp = popen((command+program_name).c_str(), "r");
	if (fp == NULL){
		/* Handle error */
		printf("Error while trying to get the pid of %s %s\n",program_name ,strerror(errno));
		syslog(LOG_ERR,"Error while trying to get the pid of %s %s\n",program_name ,strerror(errno));
	}
	    
	while (fgets(pid_to_kill_str, 1024, fp) != NULL){
		printf("Pid of %s is %s\n", program_name,pid_to_kill_str);
		//split the string by spaces
		split_pids = strtok (pid_to_kill_str," \n");
	    	while (split_pids != NULL)
	    	{
   	 		pid_to_kill_int = (unsigned int) atoi(split_pids);
		    	if (pid_to_kill_int==mypid){
				split_pids = strtok (NULL, " \n");
		    		continue;
		    	}
		    	printf("Going to kill %s with pid %d\n",program_name,pid_to_kill_int);
		    	syslog(LOG_INFO,"Going to kill %s with pid %d\n",program_name,pid_to_kill_int);
		    	KillProgramByPID(pid_to_kill_int);
	    		split_pids = strtok (NULL, " \n");
	    		if (split_pids==NULL){	    			
	    			continue;
	    		}
    		}   
	}

	status = pclose(fp);
	if (status == -1) {
		printf("Error while trying to close pipe in get the pid of %s %s\n",program_name ,strerror(errno));
		syslog(LOG_ERR,"Error while trying to close pipe in get the pid of %s %s\n",program_name ,strerror(errno));
	} else {

	}
	return 1;
}
//The function check if the process  exists and if it does, return its pid.
unsigned int get_pidof(const char * program_name)
{
	char buf[1024];
	int res;
	int lSize,pid;
	string command="pidof ";

	FILE *fd;
	fd=popen((command+program_name).c_str(),"r");
	if(fd!=NULL)
	{
		fseek (fd , 0 , SEEK_END);
		lSize = ftell (fd);
		if (lSize <= 0)
		{
			pclose(fd);
			return 0;
		}
		rewind (fd);
		res = fread(buf,1,lSize,fd);
		pclose(fd);
		if (res == 0)
		{
			return 0;
		}
		buf[lSize] = '\0';

		pid=atoi(buf);
		return pid;
	}
	else
	{
		cout<<"cannot open pipe"<<endl;
		return 0;
	}
}



bool FileExists(string strFilename) {
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;
	intStat = stat(strFilename.c_str(),&stFileInfo);
	if(intStat == 0) {
		blnReturn = true;
	} else {

		blnReturn = false;
	}
	return(blnReturn);
}


//The function check if the config file exists, if not process close himself.
void CheckForConfigFile(string configfile)
{
	printf("Sniffer config file path is %s\n",configfile.c_str());
	if(!FileExists(configfile))
	{
		printf("Sniffer config file is missing, exit process.\n");
		exit(1);
	}
}

//The function demonize proccess.
void sniffer_demonize()
{
	//demonizing process
	int pid;

	pid=fork();

	switch (pid)
	{
	case 0:
		setsid();
		break;
	case -1:
	syslog(LOG_CRIT, "ERROR! fork() failed!."); 
	exit(1);
	break;

	default:
		exit(0);
	}
}


//The function check if all fields exists in the config hashtable after loaded them from config file.
void CheckLoadedConfigFileFields(MapStringKey lmconfig)
{
	bool gsuccess = true;
	if(lmconfig.find(C_AGENTID) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_AGENTID);
		gsuccess = false;
	}

	if(lmconfig.find(C_PIPENAME) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_PIPENAME);
		gsuccess = false;
	}

	if(lmconfig.find(C_SERVERIP) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_SERVERIP);
		gsuccess = false;
	}

	if(lmconfig.find(C_COOKIENAME) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_COOKIENAME);
		gsuccess = false;
	}

	if(lmconfig.find(C_SNIFFER_MESSAGE_SEPARATOR) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_SNIFFER_MESSAGE_SEPARATOR);
		gsuccess = false;
	}

	if(lmconfig.find(C_LOGTYPE) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_LOGTYPE);
		gsuccess = false;
	}


	if(lmconfig.find(C_SESSION_EXPIRETIME) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_SESSION_EXPIRETIME);
		gsuccess = false;
	}

	if(lmconfig.find(C_STATUSINTERVAL) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_STATUSINTERVAL);
		gsuccess = false;
	}

	if(lmconfig.find(C_CONCURRENTSESSIONS) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_CONCURRENTSESSIONS);
		gsuccess = false;
	}

	if(lmconfig.find(C_QUEUELIMIT) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_QUEUELIMIT);
		gsuccess = false;
	}

	if(lmconfig.find(C_HEADERS_MAXLENGTH) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_HEADERS_MAXLENGTH);
		gsuccess = false;
	}

	if(lmconfig.find(C_GET_MAXLENGTH) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_GET_MAXLENGTH);
		gsuccess = false;
	}

	if(lmconfig.find(C_POST_MAXLENGTH) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_POST_MAXLENGTH);
		gsuccess = false;
	}

	if(lmconfig.find(C_PIPEPATH) == lmconfig.end())
	{
		printf("%s field is missing.\n", C_PIPEPATH);
		gsuccess = false;
	}
//	if(lmconfig.find(C_GULPPATH) == lmconfig.end())
//	{
//		printf("%s field is missing.\n", C_GULPPATH);
//		gsuccess = false;
//	}
//	if(lmconfig.find(C_PCAPSIZELIMIT) == lmconfig.end())
//	{
//		printf("%s field is missing.\n", C_PCAPSIZELIMIT);
//		gsuccess = false;
//	}
//	if(lmconfig.find(C_MOUNTSIZE) == lmconfig.end())
//	{
//		printf("%s field is missing.\n", C_MOUNTSIZE);
//		gsuccess = false;
//	}


	if(!gsuccess){
		printf("The Sniffer onfig file has missing fields,exiting.\n");
		exit(1);
	}
}



void InitHeadersFingerPrintList()
{
	for (MapStringKey::iterator itr = mfingerprints.begin() ; itr != mfingerprints.end(); ++itr)
	{
		mfingerprints[itr->first] = itr->second;
	}	
}


//The function read the configuration fields from the config file.
void ReadConfigFile(string _gconfigfilename, MapStringKey *lmconfig)
{
	std::string str="";
	std::ifstream input(_gconfigfilename.c_str());
	char tempkey[512];
	size_t pos=0;

	syslog (LOG_INFO, "Loading config file."); 	
	printf("Loading config file: %s\n", _gconfigfilename.c_str());

	//Parse every line in the config file, and push the info into the mconfig map object.
	while(std::getline(input,str)){
		pos = str.find(":");

		if(pos == string::npos){
			printf("Config file corrupted.\n"); 
			syslog (LOG_ERR, "Config file corrupted."); 
			exit(2);
		}

		if(pos > 512)
		{
			printf("Config key bigger then the limit (512).\n");
			syslog (LOG_ERR, "Config key bigger then the limit (512)."); 
			exit(2);
		}

		memset(&tempkey, '\0', 512);	
		int length = str.copy(tempkey, pos, 0);
		if(length == 0)
		{
			printf("Config file corrupted.\n");
			syslog (LOG_ERR, "Config file corrupted."); 
			exit(2);
		}

		str.erase(0, pos+1);
		tempkey[length]='\0';
		(*lmconfig)[tempkey] = str;

		str.clear();
	}
	syslog (LOG_INFO, "Check for valid Config file fields."); 	
	printf("Check for valid Config file fields.\n");
	CheckLoadedConfigFileFields(*lmconfig);
	syslog (LOG_INFO, "Config file loaded successfully."); 
	printf("Config file loaded successfully.\n");	
}


//The function check if there is another instance of telepath_sniff, if there is the return the pid, else return -1.
//note: The function not support more then 1 instance at the same time.
unsigned int lget_pid(const char * program_name)
{
	char buf[1024];
	string first;
	string second;
	int lSize, pid, pos;
	int res;
	string command="pidof ";
	FILE *fd;	
	fd=popen((command+program_name).c_str(),"r");
	if(fd!=NULL)
	{
		fseek (fd , 0 , SEEK_END);
		lSize = ftell (fd);
		if (lSize <= 0)
		{
			pclose(fd);
			return 0;
		}
		rewind (fd);

		res = fread(buf, 1, lSize, fd);
		pclose(fd);
		if (res == 0)
		{
			return 0;
		}
		buf[lSize] = '\0';

		//Move to string from chars array.
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
		}
		else//exist only 1 instance.
		{
			return 0;
		}

		//pid=atoi(buf);

		return pid;
	}
	else
	{		
		return 0;
	}
}
void CheckForMultiInstances(bool addinterface)
{
	if(!addinterface) {
		//Check for another instance of telepath_sniff.
		if(lget_pid(PROGRAMNAME) != 0)
		{
			syslog(LOG_ERR, "%s instance exists already, Closing!.", PROGRAMNAME);
			printf("%s sniffer instance exists already, Closing!.\n", PROGRAMNAME);
			exit(1);
		}

		//Check for another instance of telepath_sniff.
		if(lget_pid(SNIFFER_PROGRAMNAME) != 0)
		{
			syslog(LOG_ERR, "%s instance exists already, Closing!.", SNIFFER_PROGRAMNAME);
			printf("%s instance exists already, Closing!.\n", SNIFFER_PROGRAMNAME);
			exit(1);
		}

	}
}

/*
void split(const std::string &s, char delim,int* count,queue<string> *queue) {
	std::stringstream ss(s);
	std::string item;
	count = 0 ;
	//queue<string> ans;
	while(std::getline(ss, item, delim)) {
		//elems.push_back(item);
		if (strcmp(item.c_str(),"")!=0){ 
			queue->push(item);
			*count++;
		}
	}    
}
*/


void makeStats(string xml,int* totalSent,int* totalElapsed ){
	long int t = time(0);
	int xmlTime=0;	
	size_t pos1=0,pos2=0;
	string startTag = string("<TimeStamp>");
	string endTag = string("</TimeStamp>");
	string xmlTimeStr = "";
	//printf("Total Sent is now %d\n",*(totalSent));
	if((pos1=xml.find(startTag)) != string::npos){
		pos2 = xml.find(endTag);
		pos1 = pos1+startTag.length();
		xmlTimeStr = xml.substr((int)(pos1),(int)pos2-pos1);
		if(strcmp(xmlTimeStr.c_str(),"-")==0){
			*totalSent=*totalSent-1;
			return;			
		}
		else {
			xmlTime = atoi(xmlTimeStr.c_str());
			*totalElapsed+= t-xmlTime;
			//			printf("totalElapsed int is %d milliseconds\n",*(totalElapsed));
		}
	}	
	if ((*(totalSent)%10000==0) && (*(totalSent)>0)){
		printf("\nAverage time for the last 10000 requests is %f milliseconds\n" , (double)(*totalSent)/(double)(*totalElapsed));
		syslog(LOG_INFO,"STAT : Average time for the last 10000 requests is %f milliseconds" , (double)(*totalSent)/(double)(*totalElapsed));
		*(totalSent)=0;
		*(totalElapsed)=0;
	}
}

void startGulp(){
	//queue<string> argsQ;
	//string messages_separator;
	//string sinterface;
	string gulp_command;	
	gulp_command.append(conf_updated->mConfig["gulp_program_binary"].c_str());
	gulp_command.append(" -i ");
	gulp_command.append(conf_updated->mConfig["gulp_network_interface"]);
	gulp_command.append(" -r ");
	gulp_command.append(conf_updated->mConfig["gulp_ring_buffer_size"]);
	gulp_command.append(" -f ");
	gulp_command.append("'"+conf_updated->mConfig["gulp_pcap_filter"]+"'");
	//gulp_command.append(" > "+conf_updated->mConfig["gulp_to_sniffer_pipepath"]);
        //gulp_command.append(" 2> /dev/null ");
	//gulp_command.append(" & ");

	//messages_separator.append("<_HYBRID_SEP_>");
	printf("echo 4194304 > /proc/sys/net/core/rmem_default\n");	//increase two caps for configuration
	printf("echo 4194304 > /proc/sys/net/core/rmem_max\n");
	syslog(LOG_INFO,"echo 4194304 > /proc/sys/net/core/rmem_default");
	syslog(LOG_INFO,"echo 4194304 > /proc/sys/net/core/rmem_max");
	system("echo 4194304 > /proc/sys/net/core/rmem_default");
	system("echo 4194304 > /proc/sys/net/core/rmem_max");
	kill_all_program_instances(GULP_PROGRAMNAME);	  //first kill all gulp instances,this will also kill the child process in case the child was stuck on execv()
	printf("===================================================================================");
	printf("\nRunning Gulp with command:\n");	
	printf("%s\n",gulp_command.c_str());
	syslog(LOG_INFO,"Running Gulp with command: %s",gulp_command.c_str());	
	//system(gulp_command.c_str());
	pipehandler = popen(gulp_command.c_str(), "r");
	return;
}

/*
void *pipeToStdout(int backend_fd){
	int numread = 0;
	int left;
	static char buf[MAX_BUF_SIZE+21 ];
	static string msg;
	char * pos;
	char * prev = buf+20;
	static char * start = buf+20;
	buf[MAX_BUF_SIZE+20] = '\0';
	
	numread = read(fileno(pipe_fd), buf+20, MAX_BUF_SIZE-1);
	if (numread>=0)
		buf[numread+20] = '\0';
	while (numread>0){
		pos = strstr(start, "<_HYBRID_SEP_>\n");
		if (pos != NULL)
		{
			msg.append(start, pos-start);			
			sentCounter++;
			//makeStats(xmlToSend,&totalSent,&totalElapsed);								//print stats
			prepareMessage(msg,backend_fd);
			msg.clear();
			pos+=15;
			start = pos;
			numread -= pos - prev;
			prev = pos;
			if (numread == 0)
			{
				prev = buf+20;
				start = buf+20;
				break;
			}
		} else  if (numread> 0 ){
			left = numread;
			if (left > 20)
			{
				msg.append(start, left-20);
				memcpy(buf, start+left-20, 20);
				prev = buf+20;
				start = buf;
			} else {
				memcpy(buf+20-left, start+20-left, left);
				prev = buf+20;
				start = buf+20-left;
			}
			break;
		}

	}
	
}
*/

void sniffer_sigint_handler(int sig){
	printf("Killing...\n");
	syslog(LOG_INFO,"Killing...");
	//kill_all_program_instances(GULP_PROGRAMNAME);
	//kill_all_program_instances("justniffer");
	printf("Killed\n");
	syslog(LOG_INFO,"Killed");		
	exit(1);
}


void requestsRateStats(void* ptr){
	unsigned long long sentLast = 0;
	while (1){
		sleep(5);
		printf("STATS : Requests rate for the past 5 seconds is %f\n",(double)(sentCounter - sentLast)/(double)5);
		syslog(LOG_INFO,"STATS : Requests rate for the past 5 seconds is %f",(double)(sentCounter - sentLast)/(double)5);
		sentLast = sentCounter;
	}
}



int startSniffer(){
	signal(SIGPIPE, SIG_IGN);
	conf_updated->mfingerprints["hybrid_real_ip"] = "hybrid_real_ip";
	sendqueue = new sendArrayObj(atoi(conf_updated->mConfig["wait_queue_size_limit"].c_str()));			//start the queue to send the requests to in case the engine is not available
	//CreateGulpPipe();
	startGulp();
	//sleep(1);
	//OpenPipeRead();
	open_pcap_pipe(pipehandler);
	//return fileno(pipehandler);
	return fileno(pipehandler);
}

int sniffer_restart(){
	printf("Restarting the Sniffer\n");
	syslog(LOG_INFO,"Restarting the Sniffer");
	close_pcap_pipe();
	pipehandler = NULL;
	//if (pipehandler)
	//{
	//	fclose(pipehandler);
	//}
	kill_all_program_instances(GULP_PROGRAMNAME);
	//OpenPipeRead();
	//delete sendqueue;
	//sendqueue = new sendArrayObj(atoi(conf_updated->mConfig["wait_queue_size_limit"].c_str()));			//start the queue to send the requests to in case the engine is not available
	startGulp();									//start running the gulp---write to pipe
	open_pcap_pipe(pipehandler);
	printf("The Sniffer has restarted\n");
	syslog(LOG_INFO,"The Sniffer has restarted");
	return fileno(pipehandler);
}


void read_msg_from_sniffer_and_send_to_backend(int backend_fd){
	read_pcap_pipe();	
}
