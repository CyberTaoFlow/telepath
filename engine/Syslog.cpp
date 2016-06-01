#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <syslog.h>
#include <time.h>

#include "Syslog.h"

#ifdef REDIS
unsigned int time_now;
char syslog_buffer[1000];
redisContext *redis;
#endif

void print_debug(const char * str){
		syslog(LOG_NOTICE, "%s",str );
}
void print_debug(const char * str,unsigned int num){
		syslog(LOG_NOTICE, "%s%u",str,num );
}

void print_debug(unsigned int num1,unsigned int num2){
		syslog(LOG_NOTICE, "%u - %u",num1,num2 );
}

void init_log_redis(){
	#ifdef REDIS
		redis = redisConnect("127.0.0.1", 6379);
		if(redis->err) {
			print_log("Connection error: ", redis->errstr);
			exit(EXIT_FAILURE);
		}
	#endif
}

void print_log(const char * str1){
		syslog(LOG_NOTICE, "%s",str1 );
		#ifdef REDIS
			time_now = (unsigned int)time(NULL);
			sprintf(syslog_buffer,"%u %s",time_now,str1);
			redisCommand(redis, "LPUSH %s %s","U",syslog_buffer);
		#endif
}

void print_log(const char *str1,const char *str2){
		syslog(LOG_NOTICE, "%s%s",str1,str2);
		#ifdef REDIS
			time_now = (unsigned int)time(NULL);
			sprintf(syslog_buffer,"%u %s%s",time_now,str1,str2);
			redisCommand(redis, "LPUSH %s %s","U",syslog_buffer);
		#endif
}

void print_log(const char *str1,unsigned int num1){
		syslog(LOG_NOTICE, "%s%u",str1,num1);
		#ifdef REDIS
			time_now = (unsigned int)time(NULL);
			sprintf(syslog_buffer,"%u %s%u",time_now,str1,num1);
			redisCommand(redis, "LPUSH %s %s","U",syslog_buffer);
		#endif
}

void print_log(unsigned int num1,const char *str1,const char *str2){
		syslog(LOG_NOTICE, "%u%s%s",num1,str1,str2);
		#ifdef REDIS
			time_now = (unsigned int)time(NULL);
			sprintf(syslog_buffer,"%u %u%s%s",time_now,num1,str1,str2);
			redisCommand(redis, "LPUSH %s %s","U",syslog_buffer);
		#endif
}

void print_status(unsigned int n1,unsigned int n2,unsigned int n3,unsigned int n4,unsigned int n5,unsigned int n6,unsigned int n7,unsigned int n8,unsigned int n9){
		syslog(LOG_NOTICE,"Sessions: %u mMemory: %u |getRedis:%u |insertElastic:%u| dropApp:%u| dropPage:%u| dropMethod:%u| longSessionDrop:%u",n1,n2,n3,n4,n5,n6,n7,n9);
		#ifdef REDIS
			time_now = (unsigned int)time(NULL);
			sprintf(syslog_buffer,"%u Sessions: %u mMemory: %u |getRedis:%u |insertElastic:%u| dropApp:%u| dropPage:%u| dropMethod:%u| longSessionDrop:%u",time_now,n1,n2,n3,n4,n5,n6,n7,n9);
			redisCommand(redis, "LPUSH %s %s","U",syslog_buffer);
		#endif
}
