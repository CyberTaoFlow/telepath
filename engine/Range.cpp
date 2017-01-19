#include "Range.h"
#include <stdlib.h>
#include <iostream>
#include <syslog.h>

using namespace std;

unsigned int ipToNum(string & ip){
	string tmp;
	unsigned int ipNum=0,num,flag=3,size=0;

	for(unsigned int i=0 ; i<ip.size() ; i++){
		if( (ip[i]=='.') || (i==ip.size()-1) ){
			switch(flag){
				case 3:
					tmp.assign(ip,i-size,size);
					num = (unsigned int)atoi( tmp.c_str() );
					ipNum = num*16777216;
					break;
				
				case 2:
					tmp.assign(ip,i-size,size);
					num = (unsigned int)atoi( tmp.c_str() );
					ipNum += num*65536;
					break;
				case 1:
					tmp.assign(ip,i-size,size);
					num = (unsigned int)atoi( tmp.c_str() );
					ipNum += num*256;
					break;
				default:
					tmp.assign(ip,i-size,size+1);
					num = (unsigned int)atoi( tmp.c_str() );
					ipNum += num;
					return ipNum;
			}
			size=0;
			flag--;
			tmp.clear();
		}else{
			size++;
		}
	}
	return 0;
}

bool ipSingleOrRange(string & IP,string & from,string & to){
	
	for(unsigned int i=0 ; i<IP.size() ; i++){
		if(IP[i] == '-'){
			from.assign(IP,0,i);
			to.assign(IP,i+1,(IP.size()-i) );
			return true;
		}
	}

	return false;
}


Range::Range(){}
Range::Range(unsigned int from,unsigned int to){
	this->from = from;
	this->to = to;
}

void Range::init(unsigned int from,unsigned int to){
	this->from = from;
	this->to = to;
}

bool Range::inRange(unsigned int decimalIP){
	if( (decimalIP >= this->from) && (this->to >= decimalIP) ){
		return true;
	}else{
		return false;
	}
}

void Range::print(){
	cout<<from<<"-"<<to;
}

void Range::print_syslog(){
	syslog(LOG_NOTICE,"%u-%u",from,to);
}
