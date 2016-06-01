
#include "Reputation.h"



Reputation::Reputation(){}
Reputation::Reputation(double sum){
	this->update='y';
	this->counter=1;
	this->sum=sum;
}

Reputation::Reputation(double score,unsigned int counter){
	this->update='n';
	this->sum=score*counter;
	this->counter=counter;
}

UserReputation::UserReputation(){}
UserReputation::UserReputation(double sum){
	this->update='y';
	this->sum=sum;
	this->counter=1;
}

UserReputation::UserReputation(double score,unsigned int counter,unsigned int userid){
	this->update='n';
	this->sum=score*counter;
	this->counter=counter;
	this->userid=userid;
}


