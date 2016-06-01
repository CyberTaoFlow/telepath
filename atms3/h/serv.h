#ifndef _serv_h
#define _serv_h

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
//#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h> // for gethostbyname()
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <map>
#include <vector>
#include <fstream>
#include <net/if.h> 
#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <queue>
#include <string>
#include <iostream>
#include <locale>
#include <set>
#include "../h/config.h"

using namespace std;

class SQLFeeder;

//void mountPipeDir();
void *thread_sock_recv(void *arg);
void *parser_thread(void *arg);
void *agent_conf_thread(void *arg);
void *gui_dump_thread(void *arg);
void *gui_dump_thread(void *arg);
void *thread_stats(void *arg);
void *thread_stop_atms(void *arg);
void update_configuration_exclude_params(Config* conf_to_update);
void update_configuration_load_balancer(Config* conf_to_update,SQLFeeder& f);
void update_configuration_agent_edit(Config* conf_to_update,SQLFeeder& f,bool);
void read_conf(map<string,string> *, const char * filename);
void get_list_of_agents(map<string, struct AgentStatus>&);
void get_loadbalance_settings(size_t &, set<string> &, string &);
void get_atms_config_flag(set<int> &);
int check_agent_id(string *, map<string,struct AgentStatus>*, int , int*);
int check_filter_id(string *, map<string,struct FilterStatus>*, int);
void init_atms(const char* filename);
void update_config_file(Config*);
bool is_interface_online(string interface);


#endif
