
#include <boost/unordered_map.hpp>

#include "Reputation.h"
#include "Request.h"
#include <stdio.h>

extern pthread_mutex_t mutexRepIPs;
extern boost::unordered_map <unsigned int,Reputation> mReputationIPs;

using namespace std;


RequestValToInsert::RequestValToInsert(){}

void RequestValToInsert::init(string & user_name,unsigned long long RID,unsigned int & Sid,unsigned int & index,string & page_name,string & host_name,double & ts,string & ip_orig,string & ip_resp,double flow_score,double query_score,double geo_normal,double landing_normal,unsigned short & status_code,string & country,string & city,string & SID_status,string & protocol,string & method,Coordinate & c,unsigned int decimalIP,string & shard,string & title)
{ 
	boost::unordered_map <unsigned int,boost::unordered_map <string,UserReputation> >::iterator itReputationUsers;
	boost::unordered_map <string,UserReputation>::iterator itUsers;
	boost::unordered_map <unsigned int,Reputation>::iterator itReputationIPs;


	this->user_name=user_name;
	this->RID=RID;
	this->Sid=Sid;
	this->sequence=index;
	this->query_score=query_score;
	this->page_name=page_name;
	this->host_name=host_name;
	this->ts=ts;
	this->ip_orig=ip_orig;
	this->ip_resp=ip_resp;
	this->flow_score=flow_score;
	this->geo_normal=geo_normal;
	this->landing_normal=landing_normal;
	this->status_code=status_code;
	this->country=country;
	this->city=city;
	this->SID_status=SID_status;
	this->protocol=protocol;
	this->method=method;
	this->shard=shard;
	this->title=title;
	this->c=c;

	this->avg_normal = ( query_score + flow_score + landing_normal ) / 3;
	if(this->avg_normal < 0){
		this->avg_normal = 0;
	}
	else if(geo_normal > 0.6){
		this->avg_normal = sqrt(this->avg_normal);
	}

	pthread_mutex_lock(&mutexRepIPs);
	itReputationIPs = mReputationIPs.find(decimalIP);
	if( itReputationIPs != mReputationIPs.end()){
		itReputationIPs->second.update = 'y';
		itReputationIPs->second.counter++;
		itReputationIPs->second.sum += this->avg_normal;
	}else{
		Reputation r(this->avg_normal);
		mReputationIPs.insert(pair<unsigned int,Reputation>(decimalIP,r));
	}
	pthread_mutex_unlock(&mutexRepIPs);

}

void RequestValToInsert::print(){

	printf("%s|||%llu|||%u|||%u|||%s|||%s|||%s|||%s|||%s|||%s|||%s|||%f|||%f",this->user_name.c_str(),this->RID,this->Sid,this->sequence,page_name.c_str(),host_name.c_str(),this->ip_orig.c_str(),this->country.c_str(),this->city.c_str(),this->protocol.c_str(),this->method.c_str(),this->c.x,this->c.y);
}



