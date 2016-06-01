
#include "Alert.h"
#include <stdio.h>

using namespace std;



AlertValToInsert::AlertValToInsert(){}
AlertValToInsert::AlertValToInsert(unsigned long long RID,string rule_name,unsigned int score,string & shard){
	this->RID=RID;
	this->rule_name=rule_name;
	this->score = (float)score/100;
	this->shard=shard;
	this->counter=0;
}

void AlertValToInsert::print(){
	printf("%llu####%s###%f",this->RID,&this->rule_name[0],this->score);
}



SessionAlertValToInsert::SessionAlertValToInsert(){}

SessionAlertValToInsert::SessionAlertValToInsert(unsigned long long RID,string & rule_name,unsigned int score,string & shard){
	this->RID=RID;
	this->rule_name=rule_name;
	this->score = (float)score/100;
	this->shard=shard;
	this->counter=0;
}

void SessionAlertValToInsert::print(){
	printf("%llu####%s###%f",this->RID,&this->rule_name[0],this->score);
}


