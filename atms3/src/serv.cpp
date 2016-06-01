#include "../h/serv.h"
#include "../h/log.h"
#include "../h/backend.h"
#include "../h/atmstypes.h"
#include "../h/sniffer.h"
#include "../h/sendarrayobj.h"
#include "../h/sqlfeeder.h"
#include "../h/config.h"

#define EXCEPTION_CATCH
#define ATMS_VERSION "08.01.2012"


#define CONNECTIONS 100			//number of socket connections
#define PARSING_THREADS 1		//number of parsing threads, can cause problems with sequence number in addobject() function

//extern map <string, int> utf8_table;

unsigned int  iAddUnknownApp=0;				//add unknown application flag
//different counters for stats
unsigned long long iRecvMes=0;
unsigned long long iPushed=0;				//pushed to engine wait queue/socket 
unsigned long long iPoped=0;
unsigned long long iSentObj=0;
unsigned long long iDropObj=0;				//num of dropped requests
unsigned long long iDropStatusPage=0;
unsigned long long iRecvMesTemp=0;
unsigned long long iExcParamDropped=0;
unsigned long long iExIPDropped=0;			//num of dropped requests due to ip exclusions
unsigned long long iLBDropped=0;			//num of dropped requests due to load balancer
unsigned long long iHostDropped=0;			//num of dropped requests due to bad hostname
u64 ulRIDCount=1;
size_t sleep_time=0;
int timeout=120;
int backend_fd = 0;
bool restart_sniffer = false;				//when received a new configuration that requires restarting the sniffer, like a new pcap filter, flag is raised
bool received_new_config = false;			//upon receiving a new configuartion from the server flag is raised
//extern string log_path;
sendArrayObj * sendqueue = NULL;			//the pointer to the wait queue object. this is where we push requests if the engine is busy
extern unsigned long long sentCounter;			//counts the total sent to engine
extern int close_pcap_pipe();
extern void clean_bro_memory();
//maps to keep Status data for each Agent and Filter
map <string, struct AgentStatus> mAStat;		
map <string, struct AgentStatus>::iterator itAStat;
map <string, struct FilterStatus> mFStat;
map <string, struct FilterStatus>::iterator itFStat;
map<int,string> mSockIDIP;				//save IP for each socketID
unsigned int dontMapPage_lo=0,dontMapPage_hi=0, curStatusPage=0;//dont MapPages
string configuration_file_path;				//configuration file path string
hregex regex;						//regex we'd get from the server to exclude
Backend *ptpipe;					//pointer to telepipe object
SQLFeeder *pF;						//pointer to SQLfeeder object
Config* conf;
//flags to handle
int stopatmsflag =0;
int printtagsflag=0;
int testflag = 0;
int demonizeflag=1;
int stop_parsing_flag=0;
int send_exclude_params_flag=0;
int logmessage=0;
int connsock[CONNECTIONS];				//array to handle socket descriptors
int conn_count=0;		//thread descriptors
pthread_mutex_t mutex;					//mutex to control access to queue - queue<Parser> pqueue;
Config* conf_updated = NULL;				
Config* conf_0 = NULL;
Config* conf_1 = NULL;
clock_t clock_wait;

int iOperationMode = 1;
void free_exclude_params(Config* conf_to_update);
extern int readfile(const char * fname);
int config_id_to_switch_to=1;				//we start with config id 0, hence the first id to switch to will be 1
int current_config_id=0;				//we start with config 0

int main(int argc, char *argv[])
{	

	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("Atms", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	pthread_t threads[CONNECTIONS],conf_thread,stats_thread;

	fd_set rfds,wfds;
	fd_set * p_wfds;

	int max_fd=1,nofreeconn,i,message_length,num_written,total_written,sniffer_fd,retval,max_to_send,sentLast = 0,listenfd = 0,optval,last_time_stamp=time(0);

	socklen_t len;
	char buff[200],fbuf[200];

	//stuctures to handle address data
	struct sockaddr_in servaddr,claddr;
	struct timeval tv;

	string * message_dequeued;

	bool keep_writing;									//set interval wait usecs

	//args handling
	if (argc==3 && !strcmp(argv[1],"-pcap"))
	{
		readfile(argv[2]);
		exit(0);
	}
	if(argc==3 && !strcmp(argv[2],"-pm"))
		logmessage=1;
	if(argc==3 && !strcmp(argv[2],"-p"))//prints
		printtagsflag=1;
	if(argc==3 && !strcmp(argv[2],"-d"))//demonize
		demonizeflag=1;
	if(argc==3 && !strcmp(argv[2],"-test"))
	{
		testflag = 1;
		demonizeflag=0;
	}
	if(argc==3 && !strcmp(argv[2],"-nd")){//dont demonize
		demonizeflag=0;
	}

	if (argc>=2 && !strcmp(argv[1],"-v"))//for the UI display. shows the svn revision
	{
		printf("ATMS Version: %s\n", SVN_REV);
		exit(0);
	}
	//check for already running atms
	if (lget_pid("atms")!= 0){
		syslog(LOG_INFO,"The atms is already running, to kill it , please run atms -k");
		exit(0);
	}
	//check for configuration file, parameter two
	if (!argv[1])
	{
		printf("Atms config file required\n");
		exit(0);
	}

	//this is the config object we will use on first run
	conf_0 = new Config(0);
	conf_updated = conf_0;
	configuration_file_path = argv[1];

	init_atms(argv[1]);

	//copy initial configuration to the second one
	conf_1 = new Config(*conf_0);
	conf_1->id = 1;
	
	//start running the statistics thread
	pthread_create(&stats_thread,NULL ,thread_stats ,NULL);
	//bracket currently not in use as we dont use sockets
	if (0)
	{
		//initializing sockets handle array, to know if slot is empty or not
		for(int i=0;i<CONNECTIONS;i++){
			connsock[i]=-1;
		}

		listenfd=socket(AF_INET,SOCK_STREAM,0);
		if (listenfd>max_fd)
			max_fd = listenfd+1;

		fcntl(listenfd,F_SETFL,O_NONBLOCK);				//setting socket to non-blocking mode
		//allow addr reuse: can bind, if port is waiting for FIN_ACK
		optval = 1;
		if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))<0)
		{
			perror("setsockopt");
			cout<<"setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)"<<endl;
			exit(1);
		}

		bzero(&servaddr, sizeof(servaddr));

		servaddr.sin_family=AF_INET;

		servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
		servaddr.sin_port=htons(atoi(conf_updated->mConfig["server_port"].c_str()));

		sprintf(fbuf,"Trying to bind to %s, port %d",inet_ntop(AF_INET, &servaddr.sin_addr, buff, sizeof(buff)),ntohs(servaddr.sin_port));

		if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		{
			perror("bind");
			exit(1);
		}

		sprintf(fbuf,"Binded to %s, port %d",inet_ntop(AF_INET, &servaddr.sin_addr, buff, sizeof(buff)),ntohs(servaddr.sin_port));

		syslog (LOG_NOTICE, "%s",fbuf);

		if(listen(listenfd, 0) < 0)
		{
			perror("listen");
			exit(1);
		}
	}
	len=sizeof(claddr);

	//starting configuration thread. the thread checks if the user changed configuration every defined interval
	pthread_create(&conf_thread,NULL,agent_conf_thread,NULL);

	//-------------------------------------------------------------------
	//----------main loop listens and accepts connections----------------
	//-------------------------------------------------------------------
	init_lookuptable();


	nofreeconn=0;
	i=0;
	message_length=0;
	num_written = 0;
	total_written = 0;
	//this is where we write the requests to

	backend_fd = ptpipe->open_write_pipe();
	if (backend_fd == -1)
	{
		printf("Failed to open pipe with engine: %s\n", (char*)conf_updated->mConfig["atms_to_engine_pipepath"].c_str());
	}else{
		if (backend_fd>max_fd){
			max_fd = backend_fd+1;
		}
	}
	//start running the sniffer and return the file descriptor from which we will read
	sniffer_fd = startSniffer();
	if (sniffer_fd>max_fd){
		max_fd = sniffer_fd+1;
	}

	keep_writing = true;									//set interval wait usecs
	max_to_send = 0;
	while(stopatmsflag==0)
	{
		//every 30 seconds print the stats
		if ((time(NULL)-last_time_stamp)>=30){
			syslog(LOG_INFO,"STATS : Requests rate for the 5 minutes is %f Reqs/Sec",(double)(sentCounter - sentLast)/(double)(time(NULL)-last_time_stamp));

			sentLast = sentCounter;
			last_time_stamp = time(NULL);
			// stop ATMS
			if (testflag)
			{
				stopatmsflag = 1;
				break;
			}
		}

		//check if the configuration thread got a new configuration
		if (received_new_config){
			//check which configuration object to switch to
			if (conf_updated->id == 0){
				//update conf_update pointer to the new object
				conf_updated = conf_1;
			}
			else{
				conf_updated = conf_0;
			}
			//reset flag
			received_new_config = false;
			//check if the change of config also needs restarting of sniffer
			if (restart_sniffer){
				sniffer_fd = sniffer_restart();
				restart_sniffer = false;
				if (sniffer_fd>max_fd){
					max_fd = sniffer_fd+1;
				}
			}
			syslog(LOG_INFO,"Changing configuartion");
		}

		if (sendqueue->count()>0)
		{
			FD_ZERO(&wfds);	//select new
			FD_SET(backend_fd, &wfds); //select new
			p_wfds = &wfds;
		} else {
			p_wfds = NULL;
		}
		FD_ZERO(&rfds);

		tv.tv_sec = 0;												//set interval wait secs
		tv.tv_usec = 200;
		retval = select(max_fd, &rfds, p_wfds, NULL, &tv);

		//if(FD_ISSET(sniffer_fd, &rfds))
		{
			//call the reader of the sniffer on the socket, 
			//send the sniffer the fd of the backend, so it can write to it in case it's available			
			read_msg_from_sniffer_and_send_to_backend(backend_fd);
		}
		//else 
		if (p_wfds == NULL){
			continue;
		}
		if (FD_ISSET(backend_fd, &wfds))
		{
			keep_writing = true;
			max_to_send = 100;
			// write to the pipe fd of engine
			//check if there are any messages left in the wait queue
			while (sendqueue->count()>0 && keep_writing && max_to_send){
				message_dequeued = sendqueue->dequeue();
				message_length = sendqueue->getDequeuedSize();
				total_written = 0;
				max_to_send--;
				//start writing them one by one, but handle cut messages 
				while (message_length!=0){
					num_written = write(backend_fd, message_dequeued->c_str()+total_written, message_length);
					if (num_written<=0 && message_length==sendqueue->getDequeuedSize()){ 
						sendqueue->enqueue(message_dequeued);
						keep_writing = false;
						break;
					}
					else if(num_written <= 0) {
						// error writing to socket !
						keep_writing = false;
						if(fcntl(backend_fd, F_SETFL, fcntl(backend_fd, F_GETFL) & ~O_NONBLOCK) < 0){
							// write all data we have in current packet in block mode
							write(backend_fd, message_dequeued->c_str()+total_written, message_length);
							fcntl(backend_fd, F_SETFL, fcntl(backend_fd, F_GETFL) | O_NONBLOCK); 
						}else{
							close(backend_fd);
							backend_fd = ptpipe->open_write_pipe();
							if (backend_fd>max_fd){
								max_fd = backend_fd+1;
							}
						}
						message_length = 0;
						delete message_dequeued;
						break;
					}
					total_written += num_written;
					message_length = message_length - num_written;
					if (message_length == 0){
						delete message_dequeued;
					}
				}
			}
		}
		if(listenfd > 0 && FD_ISSET(listenfd, &rfds))
		{
			nofreeconn=0;
			//looking for the first free descriptor in array.
			//first free == -1
			for(i=0;i<CONNECTIONS;i++)
			{
				if(connsock[i]==-1)
					break;
				if(i==CONNECTIONS-1)
					nofreeconn=1;
			}
			if (nofreeconn){
				continue;
			}

			//accepting connection in non-blocking mode
			if((connsock[i]=accept(listenfd, (struct sockaddr*)&claddr, &len))<0){

				continue;
			}

			//setting opened socket to non-blocking mode
			fcntl(connsock[i],F_SETFL,O_NONBLOCK);

			sprintf(fbuf,"Connection from %s, port %d",inet_ntop(AF_INET, &claddr.sin_addr, buff, sizeof(buff)),ntohs(claddr.sin_port));
			syslog (LOG_NOTICE, "%s",fbuf);

			//saving client IP to the map (not used, but may be useful)
			mSockIDIP[connsock[i]]=string(inet_ntop(AF_INET, &claddr.sin_addr, buff, sizeof(buff)));

			//checking list of agents and filters if one of them has this socketid; need to set it to -1
			for(map<string, struct AgentStatus>::iterator i_map=mAStat.begin();i_map!=mAStat.end();++i_map)
			{
				if(i_map->second.iSockNum==connsock[i])
				{
					i_map->second.iSockNum=-1;
					break;
				}
			}

			for(map<string, struct FilterStatus>::iterator i_map=mFStat.begin();i_map!=mFStat.end();++i_map)
			{
				if(i_map->second.iSockNum==connsock[i])
				{
					i_map->second.iSockNum=-1;
					break;
				}
			}

			//launching new thread for this socket connection
			pthread_create(&threads[i],NULL,thread_sock_recv,(void *)(&connsock[i]));
			pthread_detach(threads[i]); //detaching this thread (because there is now way to join it)

			conn_count++;
		}

	}


	//---------------------------------------------------------------------------------
	//----------main loop listening---------------------------------------------------
	//---------------------------------------------------------------------------------

	close_pcap_pipe();
	close(sniffer_fd);
	clean_bro_memory();
	if (sendqueue)
	{
		delete sendqueue;
		sendqueue = NULL;
	}

	//waiting for socket threads to finish
	mAStat.clear();
	mFStat.clear();

	pthread_join(conf_thread, NULL);

	free_exclude_params(conf_0);
	delete conf_0;
	if (conf_1 != conf_0)
	{
		delete conf_1;
	}

	delete pF;              //deleting SQLFeeder question

	delete ptpipe;          //deleting pipe object
	ptpipe=NULL;
	mysql_library_end();

	if (listenfd != 0)
	{
		if(!close(listenfd))
		{
			syslog (LOG_NOTICE, "%s","Listen socket closed");
		}
	}

	syslog (LOG_NOTICE,"ATMS stopped");
	return 0;
}

/*
void mountPipeDir(){
	printf("Mounting 1024 MB /mnt/ram on RAM\n");		
	syslog(LOG_INFO,"Mounting 1024 MB /mnt/ram on RAM");
	int err = mkdir("/mnt/ram",S_IRWXU|S_IRGRP|S_IXGRP);					//Create the dir to mount - config file
	system("mount -t ramfs -o size=1024m ramfs /mnt/ram");//mount the FS, so we won't read from Disk rather memory	
}
*/


/*
 * void *thread_stats(void *arg)
 * 	Runs every 30 seconds and prints stats to log and screen
 * Parameters:
 * 		arg - not in use
 * Return value:
 * 		No
 */
void *thread_stats(void *arg){
	//unsigned int tmp;
	int counter = 0;
	while (!stopatmsflag)
	{
		while (counter < 30)
		{
			sleep(1);
			counter++;
		}
		counter = 0;
		syslog(LOG_INFO,"Received: %llu; Pushed: %llu; Hostname Dropped: %llu; ExIP Dropped: %llu; Empty Page Dropped: %llu; StatusCode Dropped: %llu; Bad Load Balancer Dropped: %llu; Param Dropped: %llu",iRecvMes,iPushed,iHostDropped,iExIPDropped,iDropObj,iDropStatusPage,iLBDropped,iExcParamDropped);

	}
	pthread_exit(NULL);
}
/*
 * void *thread_stop_atms(void *arg)
 * 		Raises the flag that stops the main loop for the atms
 * 
 * Parameters:
 * 		arg - not in use
 * Return value:
 * 		No
 */
void *thread_stop_atms(void *arg){
	sleep(60);
	stopatmsflag=1;
	pthread_exit(NULL);
}

/*
 *void *thread_sock_recv(void *arg)
 *      Not used !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * 		Thread started for each socket. Receives all messages.
 * Parameters:
 * 		NULL
 * Return value:
 * 		No
 */
void *thread_sock_recv(void *arg)
{
	int recv_res=0,agentID=0,stop_socket_flag=0, need_to_send_conf=1,isFilter=0,tmplen;
	int *socket;

	char in_buf[16385];
	char *tmpstr;

	map<string,struct AgentStatus>::iterator i_map;

	time_t start_time, idle_time=time(NULL);
	start_time=time(NULL);

	SQLFeeder F(conf_updated->mConfig);

	//sbuffer - is used to save incoming message.
	string sbuffer,opentag="<Transmission>", closetag="</Transmission>";

	socket= (int *)(arg);

	string::size_type openpos, closepos;
	string sAgentID, sMesType, sCurConfID;

	memset(in_buf,'\0',sizeof(in_buf));

	//main loop, receiving of message
	while(stop_socket_flag==0 && stopatmsflag==0)
	{
		recv_res=recv(*socket, in_buf, sizeof(in_buf)-1,0);
		if(recv_res>0)
		{

			in_buf[recv_res]='\0';

			//save all incoming messages in one buffer, for case if beginning of message(opentag) and end of it(closetag) will be send separatly
			sbuffer.append(in_buf);

			//there is always should be "<Transmisson>" tag in buffer (because it was in first message).
			//if no - drop message.
			if(sbuffer.find(opentag)==string::npos){
				sbuffer.clear();
			}

			while((openpos=sbuffer.find(opentag))!=string::npos)
			{
				//if closetag was found, so we have at least one full message. We store it in tmpstr
				if((closepos=sbuffer.find(closetag))!=string::npos)
				{
					//copying full message from sbuffer to temp string
					tmplen=closepos+closetag.length()-openpos;
					tmpstr=new char[tmplen+1];
					sbuffer.copy(tmpstr,tmplen,openpos);
					tmpstr[tmplen]='\0';

#ifdef EXCEPTION_CATCH
					try{
						if(openpos<sbuffer.size()){
							sbuffer.erase(openpos,tmplen);
						}
					}
					catch(exception &ex)
					{
						printf("\nvoid *thread_sock_recv(void *arg): string exception\n");
						exit(1);
					}
#else

					//erasing part of the buffer
					if(openpos<sbuffer.size()){
						sbuffer.erase(openpos,tmplen);
					}
#endif

					//fetching "AgentType" - Filter or Agent and checking if we have this agentID
					if(get_tag_value(tmpstr,"<AgentType>","</AgentType>")=="Filter")
					{
						isFilter=1;
						sAgentID=get_tag_value(tmpstr,"<AgentID>","</AgentID>");
						agentID=check_filter_id(&sAgentID, &mFStat,*socket);
					}
					else
					{
						sAgentID=get_tag_value(tmpstr,"<AgentID>","</AgentID>");
						//checking if I have this agent, and adding socket num to lookuptable: agent_id-sock_num
						agentID=check_agent_id(&sAgentID, &mAStat,*socket, &need_to_send_conf);
					}


					//if agentID is zero, we have this ID in list
					if(agentID==0)
					{
					}
					else
					{
						string q="Got message from unknown agent! Message was dropped! AgentID: "+ sAgentID+"; IP: "+mSockIDIP[*socket];

						syslog (LOG_NOTICE, "%s",q.c_str());
						stop_socket_flag=1;
						delete []tmpstr;
						continue;
					}
					//checking type of the message
					sMesType=get_tag_value(tmpstr,"<Type>","</Type>");

					/*
					 *  FILTER PART
					 *  FILTER can send "Status" and "Signals" messages
					 *  This filter block was added unexpectedly. So it's better to organize this block in better way.
					 */
					if(isFilter==1)
					{
						//filter sends "Status" message every statusTimeout seconds

						if(sMesType=="Status")
						{
							F.update_filter_stat(sAgentID,"alive");
							delete []tmpstr;
							tmpstr=NULL;
						}
						//"Signals" - is confirmation, that signal was received
						else if (sMesType=="Signals")
						{
							itFStat = mFStat.find(sAgentID);
							string sSignalID;
							sSignalID=get_tag_value(tmpstr,"<SignalID>","</SignalID>");
							//if sSignalID=="0", means that sniffer sends AlertID
							if(sSignalID=="0")
							{
								string AlertID, q;
								AlertID=get_tag_value(tmpstr,"<AlertID>","</AlertID>");
								q="UPDATE alerts SET counter_sig=counter_sig+1 where msg_id="+AlertID+";";
								F.DAL_query(q.c_str());
							}
							//checking if atms sent signal with such id; if so - signal is confirmed
							else if(itFStat->second.iSignalCounter && (itFStat->second.SignalIDs.find(sSignalID)!=itFStat->second.SignalIDs.end()) )
							{
								itFStat->second.SignalIDs.erase(sSignalID);
								--itFStat->second.iSignalCounter;
							}
							else
							{
							}
							delete []tmpstr;
							tmpstr=NULL;
						}
						continue;
					}

					if (sMesType=="Config")
					{
						sCurConfID = get_tag_value(tmpstr,"<ConfigID>","</ConfigID>");			//fetching ConfigID

						itAStat = mAStat.find(sAgentID);
						//checking if ATMS sent such ConfigID
						if(itAStat->second.iConfigCounter && (itAStat->second.ConfigIDs.find(sCurConfID)!=itAStat->second.ConfigIDs.end()) )
						{
							itAStat->second.ConfigIDs.erase(sCurConfID);
							--itAStat->second.iConfigCounter;

							//checking if configuration was successfully uploaded or not
							if(get_tag_value(tmpstr,"<Status>","</Status>")=="1")
							{
								itAStat->second.initialized=1;
							}
							else{}
						}
						else{}

						delete []tmpstr;
						continue;
					}

					//=============================== STATUS =====================================
					//agent sends "Status" message every TimeOut interval
					else if (sMesType=="Status")
					{

						//if this Status Message is first, so agent should be initialized
						if(mAStat[sAgentID].initialized==0){
							string q="update agents set newConfig=1 where agent_id='"+sAgentID+"';";
							F.DAL_query(q.c_str());
							static const char * u_q ="update config set value='34,22' where name='atmsActions';";
							F.DAL_query(u_q);
						}

						F.update_agent_stat(sAgentID,"alive");
						delete []tmpstr;
						continue;
					}


					//=====================EXCLUDE PARAMS=======================
					//agent sends "Excludes" message, when it receives exclude parameters.
					else if (sMesType=="Excludes")
					{
						int state=1;
						//checking if config with such ID was sent
						sCurConfID = get_tag_value(tmpstr,"<ConfigID>","</ConfigID>");

						itAStat = mAStat.find(sAgentID);					
						if(itAStat->second.iConfigCounter && (itAStat->second.ConfigIDs.find(sCurConfID)!=itAStat->second.ConfigIDs.end()) )
						{
						}
						else
						{
							delete []tmpstr;
							break;
						}

						//checking for Status, which was sent in confirmation
						if(get_tag_value(tmpstr,"<Status>","</Status>")=="0"){
							state=0;
						}

						if(state){}

#ifdef EXCEPTION_CATCH
						try{
							itAStat->second.ConfigIDs.erase(itAStat->second.ConfigIDs.find(sCurConfID));
						}
						catch(exception &ex){
							exit(1);
						}
#else
						itAStat->second.ConfigIDs.erase(itAStat->second.ConfigIDs.find(sCurConfID));
#endif
						--itAStat->second.iConfigCounter;
						delete []tmpstr;
						continue;
					}

					//if no type matches message is dropped
					else{
						delete []tmpstr;
						continue;
					}
				}
				else
					break;
			}
		}
		else//sleep, if didn't receive anything
		{
			usleep(10000);
		}

		//id sAgentID is not empty, so we received at least one message.
		//checking timeout time to understand, if agent can be stopped
		if(sAgentID.size() && stop_socket_flag==0){
			int timeout;
			if(isFilter==1){
				timeout=mFStat[sAgentID].iTimeout;
				if((time(NULL)-start_time) > timeout){
					stop_socket_flag=1;
					pF->update_filter_stat(sAgentID,"dead");
				}
			}else{
				timeout=mAStat[sAgentID].iTimeout;
				if((time(NULL)-start_time) > timeout){
					stop_socket_flag=1;
					pF->update_agent_stat(sAgentID,"dead");
				}
			}
		}
		//check to close connection, if no one message has never been received
		else if(sAgentID.size()==0){
			if(time(NULL)-idle_time > 300){
				stop_socket_flag=1;
			}
		}

	}//main loop


	//closing socket
	conn_count--;
	close(*socket);

	//updating structure for current filter
	if(isFilter==1){
		if(sAgentID!=""){
			itFStat = mFStat.find(sAgentID);
			if(itFStat !=mFStat.end()){
				itFStat->second.iSockNum=-1;
			}
		}
	}else{
		//updating structure for current agent
		if(sAgentID!=""){
			itAStat = mAStat.find(sAgentID);
			if(itAStat != mAStat.end()){
				itAStat->second.iSockNum=-1;
				itAStat->second.initialized=0;
			}
		}
	}

#ifdef EXCEPTION_CATCH
	try{
		mSockIDIP.erase(*socket);
	}
	catch(exception &ex){
		printf("\nParser::thread_recv4: string exception\n");
		exit(1);
	}
#else
	mSockIDIP.erase(*socket);
#endif

	//writing -1 to array connsock[CONNECTIONS]
	*socket=-1;
	pthread_exit(NULL);

}


/*
 * void read_conf(map<string,string> *c , const char *filename)
 *		Reads the configuration file and maps it
 * 
 * Parameters:
 * 		map<string,string> *c - the map to insert the entries to
 * 		const char *filename - the file to read
 * Return value:
 * 		No
 */
void read_conf(map<string,string> *c , const char *filename)
{

	FILE *pFile;
	int lSize;//, tmplen;
	char *buffer;
	size_t result;

	pFile = fopen ( filename , "rb" );
	if (pFile==NULL)
	{
		fprintf(stderr, "Failed to open configuration file: %s\n",filename); 
		exit (1);
	}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
	rewind (pFile);

	// allocate memory to contain the whole file:
	buffer = (char*) malloc (sizeof(char)*lSize+1);
	if (buffer == NULL) 
	{
		fputs ("Memory error",stderr);
		exit (2);
	}

	// copy the file into the buffer:
	result = fread (buffer,1,lSize,pFile);
	buffer[lSize] = '\0';
	if (result != static_cast<unsigned int>(lSize))
	{
		fputs ("Reading error",stderr); exit (3);
	}

	string b;
	b = buffer;
	free(buffer);

	string::size_type pos1;
	string tag_name,tag_value;

	string::iterator i=b.begin();
	while(i!=b.end())
	{
		if(*i<20)
		{
#ifdef EXCEPTION_CATCH
			try
			{
				b.erase(i);
			}
			catch(exception &ex)
			{
				printf("\nserv.cpp:read_conf1: string exception\n");
				exit(1);
			}
#else
			b.erase(i);
#endif
		}

		else
			++i;
	}

	while(b.size()>2)
	{
		pos1=b.find('=');
		if(pos1==string::npos)
			break;
		b[pos1]='\0';
		tag_name=b.c_str();
		b.erase(0,pos1+1);

		pos1=b.find('\"');
		b.erase(0,pos1+1);
		pos1=b.find('\"');
		b[pos1]='\0';
		tag_value=b.c_str();
		b.erase(0,pos1+1);

		(*c)[tag_name]=tag_value;
	}
	fclose(pFile);
}



/*
 * void demonize()
 * 		Demonizing process
 * Parameters:
 * 		No
 * Return value:
 * 		Doesn't return value
 */
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
		syslog (LOG_NOTICE, "%s","ATMSd: starting in daemon mode");
		break;
	case -1:
	cerr<<"ERROR! fork() failed!"<<endl;
	exit(1);
	break;
	default:
		exit(0);
	}
}


/*
 * void sighandler(int sig)
 * 		Handler for signals.
 * Parameters:
 * 		sig - signal number
 * Return value:
 * 		Doesn't return value
 */
void sighandler(int sig)
{
	printf("Signal: %d\n",sig);
	stopatmsflag=1;
}

// Not used
int check_agent_id(string *id, map<string,struct AgentStatus> *list , int sock_n, int *need_send_conf)
{
	map<string,struct AgentStatus>::iterator i_map;
	struct AgentStatus AStat;

	i_map=list->find(*id);
	if(i_map!=list->end())
	{
		//this agent id found in list, so we just need to check if socket numbers are equel
		//cout<<"i_map->second.iSockNum: "<<i_map->second.iSockNum<<"; sock_n: "<<sock_n<<endl;

		if(i_map->second.iSockNum!=sock_n)
		{
			i_map->second.iSockNum=sock_n;
			i_map->second.initialized=0;
			send_exclude_params_flag=1;
		}

		//even if socket number is the same, maybe agent lost connection between statuses. So if socket thread was started from beginning,
		//I need to send configuration to this agent
		if(*need_send_conf==1)
		{
			i_map->second.initialized=0;
			*need_send_conf=0;
		}
		return 0;
	}
	else
	{
		pF->get_list_of_agents(mAStat);
		i_map=list->find(*id);
		if(i_map!=list->end())
		{
			(*list)[*id].iSockNum=sock_n;
			send_exclude_params_flag=1;
			i_map->second.initialized=0;
			cout<<"Agent added: "<<*id<<endl;
			return 0;
		}
		else
			return 1;
	}
}

/*
 * Function to check if there is such filter name in a list. If we have it in a list, we save it's socket number
 * string *id - filter_id
 * map<string,struct FilterStatus> - list of all filters
 * int sock_n - socket id
 */
int check_filter_id(string *id, map<string,struct FilterStatus> *list , int sock_n)
{
	map<string,struct FilterStatus>::iterator i_map;
	struct FilterStatus FStat;

	i_map=list->find(*id);
	if(i_map!=list->end()) //this filter id found in list
	{
		//this filter id found in list, so we just need to save it's socket id.
		i_map->second.iSockNum=sock_n;
		return 0;
	}
	else //this wasn't found in list. Reread table, find this filter and save it's socket id
	{
		pF->get_list_of_filters(mFStat);
		i_map=list->find(*id);
		if(i_map!=list->end())
		{
			(*list)[*id].iSockNum=sock_n;
			return 0;
		}
		else // if this filter wasn't found - returning 1
			return 1;
	}
}


/*
 * void *agent_conf_thread(void *arg)
 * 		The thread conf. Runs every 1 second and checks for configuration changes made on the UI by the user.
 * Parameters:
 * 		sig - signal number
 * Return value:
 * 		Doesn't return value
 */
void *agent_conf_thread(void *arg)
{
	sleep(20);
	Config* conf_to_update;
	SQLFeeder f(conf_updated->mConfig);

	set<int> sConfCodes;

	bool changed_conf = false;
	while (stopatmsflag==0)
	{

		sleep(1);
		f.get_atms_config_flag(sConfCodes);
		set<int>::iterator i;
		if (sConfCodes.size()>0){
			//if current is 0, switch to 1
			if (conf_updated->id == 0){
				conf_1->Merge( conf_0 );
				conf_1->id = 1;
				conf_to_update = conf_1;
			}
			//if current is 1, switch to 0
			else{
				conf_0->Merge( conf_1 );
				conf_0->id = 0;
				conf_to_update = conf_0;
			}
			//raise flag for change
			changed_conf = true;
		}
		while(sConfCodes.size())
		{
			i=sConfCodes.begin();
			//rereading loadbalancer settings
			if((*i) == LOAD_BALANCER){
				//Get load balancer config
				update_configuration_load_balancer(conf_to_update,f);				
				syslog (LOG_NOTICE, "Load Balancer Configuration Changed");
				sConfCodes.erase(i);			//deleting code from the set
				continue;
			}

			//Sending exclude parameters flag
			if(((*i) == EXCLUDE_PARAMS) /*send_exclude_params_flag==1 ||*/)
			{				
				syslog (LOG_NOTICE, "EXCLUDE PARAMS Changed");
				sConfCodes.erase(i);
				update_configuration_exclude_params(conf_to_update);
				continue;
			}
			
			if((*i) == AGENTADD){
				//SQLFeeder
				f.get_list_of_agents(mAStat);
				sConfCodes.erase(i);
				continue;
			}

			//Sending configuration from database to agent
			//this one requires restarting the gulp, as the pcap filter has been changed
			if((*i) == AGENT_EDIT){
				sConfCodes.erase(i);
				restart_sniffer = true;
				continue;
			}		

			if((*i) == APPADD){
				f.get_apps_list(&iAddUnknownApp, conf_to_update->mAppData, conf_to_update->mAppIPData, conf_to_update->mSslPorts);
				sConfCodes.erase(i);				
				continue;
			}
			sConfCodes.erase(i);
		}//end while

		//if changed, re-write the configuration file on the server
		if (changed_conf){
			update_config_file(conf_to_update);
			received_new_config = true;
			changed_conf = false;
		}
	}
	pthread_exit(NULL);
}


/*
 * void update_configuration_load_balancer(Config* conf_to_update,SQLFeeder& f)
 * 		Updates the load balancer from the database to the lb info on the configuration object
 * Parameters:
 * 		Config* conf_to_update - the fongi object in of which the lb settings need to be changed
 * 		SQLFeeder& f - the sql feeder object
 * Return value:
 * 		Doesn't return value
 */
void update_configuration_load_balancer(Config* conf_to_update,SQLFeeder& f){
	f.get_loadbalance_settings(conf_to_update->lb_flag, conf_to_update->sLB_IPaddr, conf_to_update->lb_header);
}

/*
 * void free_exclude_params(Config* conf_to_update)
 * 		Free's all the exclude param structs of the agents
 * Parameters:
 * 		Config* conf_to_update - the fongi object in of which the lb settings need to be changed
 * 	
 * Return value:
 * 		Doesn't return value
 */
void free_exclude_params(Config* conf_to_update)
{
	//first go over all agents vectors and clear them
	map<string, vector<struct excluded_param> >::iterator exc_par_itr = conf_to_update->excluded_params.begin();
	for( ; exc_par_itr!= conf_to_update->excluded_params.end() ; exc_par_itr++)
	{
		vector<struct excluded_param>::iterator par_itr = exc_par_itr->second.begin();
		for (;par_itr != exc_par_itr->second.end(); par_itr++)
		{
			if ((*par_itr).regex.size())
			{
				regfree(&(*par_itr).compiled_regex);
			}
		}
		(exc_par_itr->second).clear();
	}
}
/*
 * void update_configuration_exclude_params(Config* conf_to_update)
 * 		Updates the exclude params information from the database
 * Parameters:
 * 		Config* conf_to_update - the fongi object in of which the lb settings need to be changed
 * 	
 * Return value:
 * 		Doesn't return value
 */
void update_configuration_exclude_params(Config* conf_to_update){
	string sExParams;
	string sExParams2;

	free_exclude_params( conf_to_update );

	for(map<string, struct AgentStatus>::iterator i=mAStat.begin();i!=mAStat.end();++i)
	{
		sExParams="", sExParams2="";

		sExParams=pF->get_regex(i->first);		//reading exclude parameters from agent_regex table

		int excl_params_num = atoi(get_tag_value(sExParams,"<Field_Quantity>","</Field_Quantity>").c_str());
		unsigned int start_pos = 0,end_pos = 0;				
		string tmp,tmp_type,tmp_header;
		printf("=========================================================\n");
		if (excl_params_num>0)
			printf("Excluded params :\n");
		else
			printf("No excluded params defined by user\n");
		for(int i = 0;i<excl_params_num;i++){
			start_pos = sExParams.find("<Field"+int_to_string(i)+">",start_pos);						
			excluded_param ex_par;
			end_pos = start_pos;
			end_pos = sExParams.find("</Field"+int_to_string(i)+">",end_pos);
			end_pos +=  string("</Field"+int_to_string(i)+">").size();
			tmp = sExParams.substr(start_pos,end_pos-start_pos);
			tmp_type = get_tag_value(tmp,"<Type>","</Type>");
			(!strcmp(tmp_type.c_str(),"URL")? ex_par.param_type = URL:ex_par.param_type = HEADERS);
			ex_par.regex = get_tag_value(tmp,"<RegexExpression>","</RegexExpression>");
			printf("%s\n",ex_par.regex.c_str());
			ex_par.condition = atoi(get_tag_value(tmp,"<Condition>","</Condition>").c_str());
			//if its a headers regex of the type (header_name:)(....value...)
			//we need to lower case the header name
			if (ex_par.param_type==HEADERS){
				start_pos = 0;
				end_pos = 0;
				start_pos = ex_par.regex.find("(");
				if (start_pos==string::npos){
					printf("Corrupted Regex %s\n",ex_par.regex.c_str());
					syslog(LOG_INFO,"Corrupted Regex %s",ex_par.regex.c_str());
					continue;
					//skip to next regex and drop the current
				}
				start_pos++;
				end_pos = start_pos;
				end_pos = ex_par.regex.find(":",end_pos);
				if (start_pos==string::npos){
					printf("Corrupted Regex %s\n",ex_par.regex.c_str());
					syslog(LOG_INFO,"Corrupted Regex %s",ex_par.regex.c_str());
					continue;
					//skip to next regex and drop the current
				}
				end_pos = ex_par.regex.find(")",end_pos);
				if (start_pos==string::npos){
					printf("Corrupted Regex %s\n",ex_par.regex.c_str());
					syslog(LOG_INFO,"Corrupted Regex %s",ex_par.regex.c_str());
					continue;
					//skip to next regex and drop the current
				}
				//tmp_header = ex_par.regex.substr(start_pos,end_pos-start_pos);
				transform(ex_par.regex.begin()+start_pos,ex_par.regex.begin()+end_pos,ex_par.regex.begin()+start_pos,::tolower);
			}
			if(regcomp(&ex_par.compiled_regex,ex_par.regex.c_str(),REG_EXTENDED) != 0 )
			{
				cout << "Failed to compile regex of type" << ex_par.param_type << endl;
				syslog(LOG_INFO,"Failed to compile regex of type %d",ex_par.param_type);							
			}
			//each agent has a vector, insert to this agent's vector
			conf_to_update->excluded_params[conf_to_update->mConfig["agent_id"]].push_back(ex_par);
		}
		printf("=========================================================\n");
//					}
//					}
	}		
}



/*
 * void update_configuration_agent_edit(Config* conf_to_update,SQLFeeder& f,bool get_only_edited)
 * 		Updates the agent fields from the database, namely the wait queue size for the engine, the pcap filter, etc
 * 		Called when the user edits an agent entry on the agent table on the config tab on UI
 * Parameters:
 * 		Config* conf_to_update - the fongi object in of which the lb settings need to be changed
 * 		SQLFeeder& f - the sql feeeder object
 * 		bool get_only_edited - true to get only "dirty" agents, else for all
 * 	
 * Return value:
 * 		Doesn't return value
 */

/*void update_configuration_agent_edit(Config* conf_to_update,SQLFeeder& f,bool get_only_edited){
	set<string> sConfigs;
	string config;
	string temp_fp_header;
	string agent_id;
	string info;
	bool network_interface_up;

	f.get_all_agent_conf(sConfigs,get_only_edited);
	if(sConfigs.size()){
		for(set<string>::iterator s_i=sConfigs.begin(); s_i!=sConfigs.end(); ++s_i){                         
			config=*s_i;
			agent_id = get_tag_value(config,"<AgentID>","</AgentID>");
			conf_to_update->mConfig["agent_id"] = agent_id;
			conf_to_update->mConfig["gulp_pcap_filter"] = get_tag_value(config,"<FilterExpression>","</FilterExpression>");
			conf_to_update->mConfig["max_headers_length"] = get_tag_value(config,"<MaxHeadersLength>","</MaxHeadersLength>");
			conf_to_update->mConfig["max_get_length"] = get_tag_value(config,"<MaxGetParamsLength>","</MaxGetParamsLength>");
			conf_to_update->mConfig["max_post_length"] = get_tag_value(config,"<MaxPostLength>","</MaxPostLength>");                            	
			conf_to_update->mConfig["status_interval"] = get_tag_value(config,"<StatusInterval>","</StatusInterval>");            	
			conf_to_update->mConfig["cookie_expiration_time"] = get_tag_value(config,"<CookieExpireTime>","</CookieExpireTime>");
			conf_to_update->mConfig["concurrent_sessions"] = get_tag_value(config,"<ConcurrentSessions>","</ConcurrentSessions>");                            	
			conf_to_update->mConfig["wait_queue_size_limit"] = get_tag_value(config,"<MessagesQueueSize>","</MessagesQueueSize>");
			conf_to_update->mConfig["gulp_network_interface"] = get_tag_value(config,"<NetworkInterface>","</NetworkInterface>");

			conf_to_update->finger_prints_num = atoi(get_tag_value(config,"<HeadersQuantity>","</HeadersQuantity>").c_str());
			conf_to_update->mfingerprints.clear();
			for (int i = 1;i<=conf_to_update->finger_prints_num;i++){
				temp_fp_header = get_tag_value(config,string("<Header"+int_to_string(i)+">").c_str(),string("</Header"+int_to_string(i)+">").c_str());
				transform(temp_fp_header.begin(),temp_fp_header.end(),temp_fp_header.begin(),::tolower);
				conf_to_update->mfingerprints[temp_fp_header] = temp_fp_header;                            		
			}

			mAStat[agent_id].ConfigIDs.insert(get_tag_value(config,"<ConfigID>","</ConfigID>"));
			++mAStat[agent_id].iConfigCounter;
			mAStat[agent_id].iTimeout=atoi(get_tag_value(config,"<StatusInterval>","</StatusInterval>").c_str())+5;                
			network_interface_up = is_interface_online(conf_to_update->mConfig["gulp_network_interface"]);
			if(!network_interface_up){
				syslog(LOG_ERR,"Network Interface %s is invalid, exiting",(conf_to_update->mConfig["gulp_network_interface"]).c_str());
				printf("Network Interface %s is invalid, exiting\n",(conf_to_update->mConfig["gulp_network_interface"]).c_str());
				kill_all_program_instances("atms");
				kill_all_program_instances("gulp");
				//sniffer_sigint_handler(0);
				exit(0);
			}
		}
		sConfigs.clear();
	}
}*/


/*
 * void update_config_file(Config* conf_to_update)
 * 		Rewrites the configuration file on disk from the new config object
 * 
 * Parameters:
 * 		Config* conf_to_update - the fongi object in of which the lb settings need to be changed
 * 	
 * Return value:
 * 		Doesn't return value
 */
void update_config_file(Config* conf_to_update){
	FILE *pFile;
	map<string,string> tmp_fields;
	//first take all the fields from the file to a map
	read_conf(&tmp_fields,configuration_file_path.c_str());
/*
	{
		int lSize;//, tmplen;
		char *buffer;
		pFile = fopen ( configuration_file_path.c_str() , "rb" );
		if (pFile==NULL) {
			fputs ("File error\n",stderr); 
			exit(1);
		}
		// obtain file size:
		fseek (pFile , 0 , SEEK_END);
		lSize = ftell (pFile);
		rewind (pFile);
		// allocate memory to contain the whole file:
		buffer = (char*) malloc (sizeof(char)*lSize+1);
		if (buffer == NULL) 
		{
			fputs ("Memory error",stderr);
			exit (2);
		}
		// copy the file into the buffer:
		result = fread (buffer,1,lSize,pFile);
		buffer[lSize] = '\0';
		if (result != static_cast<unsigned int>(lSize)) {
			fputs ("Reading error",stderr); 
			exit (3);
		}
		fclose(pFile);
	}
*/
	//clear file
	//fclose(fopen(configuration_file_path.c_str(),"w"));
	pFile = fopen(configuration_file_path.c_str(),"wb");
	if (pFile == NULL){
		fputs ("Cannot re-write the config file\n",stderr);	
		return;
	}
	map<string,string>::iterator itr;
	itr = conf_to_update->mConfig.begin();
	syslog(LOG_INFO,"=============Configuration File Fields===========");
	for(;itr!=conf_to_update->mConfig.end();itr++){
		fprintf(pFile ,"%s=\"%s\"\n",itr->first.c_str(), itr->second.c_str());
		syslog(LOG_INFO,"%s=%s",itr->first.c_str(), itr->second.c_str());
	}
	fclose(pFile);
}


/*
 * bool is_interface_online(string interface) 
 * 		When receving a configuration from the agent, we need to check that the interface we got is up.
 * 		If not, to prevent a crash, we need to alert the interface is down, and exit
 * 
 * Parameters:
 * 		string interface - i.e "eth0"
 * 	
 * Return value:
 * 		bool- is up or not
 */
bool is_interface_online(string interface) {
    struct ifreq ifr;
    int sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_IP);
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, interface.c_str());
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
            perror("SIOCGIFFLAGS");
    }
    close(sock);
    return !!(ifr.ifr_flags & IFF_UP);
}


#include <vector>
using namespace std;
/*
 * void init_atms(const char *filename)
 * 		Runs all initialization functions. re-writes the configuration file on the disk, gets all the data from the configuration table on database.
 * 
 * Parameters:
 * 		const char *filename - the path of the config file
 * 	
 * Return value:
 * 		Does not return a value
 */
void init_atms(const char *filename)
{
	if(demonizeflag)
		demonize();

	signal(SIGUSR1,sighandler); 		//setting signal handler for catching SIGUSR1
	signal(SIGTERM,sighandler);
	signal(SIGQUIT,sighandler);

	read_conf(&(conf_updated->mConfig), filename);		//reading configuration from file	

	syslog (LOG_NOTICE, "Telepath ATMS starting");

	ptpipe = new Backend((char*)conf_updated->mConfig["atms_to_engine_pipepath"].c_str());

	pF=new SQLFeeder(conf_updated->mConfig);	//creating SQLFeeder object

	pF->get_list_of_agents(mAStat); 		//reading agents to lookuptable

	pF->update_revision(SVN_REV);

	pF->get_list_of_filters(mFStat);		//reading filters to lookuptable

	pF->get_loadbalance_settings(conf_updated->lb_flag, conf_updated->sLB_IPaddr, conf_updated->lb_header); 				//reading Load Balancer data feom database
	//init operation mode from mysql
	pF->get_op_mod_logtodb(iOperationMode, ulRIDCount);

	string res;
	res.clear();

	//reading excludedIP parameter from database
	conf_updated->excludedIP.clear();
	pF->get_excluded_ip(conf_updated->excludedIP);

	cout<<"--------- Excluded IP ------------"<<endl;
	for(set<string>::iterator i=conf_updated->excludedIP.begin();i!=conf_updated->excludedIP.end();++i)
	{
		cout<<"IP: "<<*i<<endl;
	}

	//reading dontmappage parameter from database
	pF->get_dontmappage(res);
	vector<string> sdontMapPage;
	sdontMapPage.clear();
	while(res.size()){
		sdontMapPage.push_back(parse_str_delim(res,'-'));
	}

	dontMapPage_lo=atoi(sdontMapPage[0].c_str());
	dontMapPage_hi=atoi(sdontMapPage[1].c_str());

	//===========================================================================

	//reading applications list
	pF->get_apps_list(&iAddUnknownApp, conf_updated->mAppData, conf_updated->mAppIPData, conf_updated->mSslPorts);
	cout<<"--------- Application list -------- "<<endl;
	for(map<AppKey, struct AppData>::iterator i=conf_updated->mAppData.begin();i!=conf_updated->mAppData.end();++i)
	{
		cout<<"App: "<<i->first.name<<" : "<<i->second.id<<endl;
	}
	//====================================================================================
	//take all data from database and override the config file
	//update_configuration_agent_edit(conf_updated,*pF,false);
	update_configuration_exclude_params(conf_updated);
	update_configuration_load_balancer(conf_updated,*pF);
	update_config_file(conf_updated);
}




