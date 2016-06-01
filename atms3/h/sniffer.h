#ifndef SNIFFER_H_
#define SNIFFER_H_
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "netdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <queue>
#include <pthread.h>
#include <sys/timeb.h>
#include <map>
#include <algorithm>
#include <math.h>
#include <syslog.h>
#include <signal.h>
#include <asm-generic/errno-base.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
//Syslog libraries.
#include <sys/wait.h>
#include <ctime>
#include "../h/parser.h"
using namespace std;


#define MAX_BUF_SIZE	1048510
//global vars
#define PROGRAMNAME "Atms"
#define SNIFFER_PROGRAMNAME "justniffer"
#define GULP_PROGRAMNAME "gulp"




void OpenPipeRead(/*string sinterface*/);

void KillProgramByPID(size_t lpid);

void killGulp(size_t lpid);

unsigned int kill_all_program_instances(const char * program_name);

unsigned int get_pidof(const char * program_name);

bool FileExists(string strFilename);

void CheckForConfigFile(string configfile);

void sniffer_demonize();

void CheckLoadedConfigFileFields(MapStringKey lmconfig);

void ReadConfigFile(string gconfigfilename, MapStringKey *lmconfig);

unsigned int lget_pid(const char * program_name);

void CheckForMultiInstances(bool addinterface);

void split(const std::string &s, char delim,int* count,queue<string> *queue);

bool ConvertStringToInt(string in_num, int *out_num);

void makeStats(string xml,int* totalSent,int* totalElapsed );

void startGulp();

void getPostData(string& request,string& post_data);

void getURL(string& request_line,string& ans);



void *pipeToStdout(void* ptr);

void sniffer_sigint_handler(int sig);

void requestsRateStats(void* ptr);

int startSniffer();
int sniffer_restart();

void read_msg_from_sniffer_and_send_to_backend(int backend_fd);



#endif /*SNIFFER_H_*/
