#include <syslog.h>
#include "Enumeration.h"

using namespace std;

Enumeration::Enumeration(){
	this->size = 0;
	this->last = 0;
	this->sumY = 0;
	this->sumXY = 0;
	this->offset = 0;
}	

// Clear the seen vector.
void Enumeration::clean(){
	this->seen.clear(); 
}

// Classifying as Enumeration a list of strings by finding a convergence.
bool Enumeration::getCorrelation(){
	if(this->last > 8000){
		return 0;
	}

	// Using Covariance.
	double Pvalue/*Covar[XY]*/ = (this->sumXY/(double)this->size)/*E[XY]*/ - ( (this->sumY/(double)this->size)/*E[Y]*/ * ( (this->size+1) /(double)2 )/*E[X]*/ );

	if (Pvalue < -0.5){ // if Pvalue is less than 0, the list is classified as Enumeration.
		return true;
	}else{
		return false;
	}
}

// Adding a sample into the enum dictionary.
void Enumeration::insertIntoEnum( int hash_val){
	map <int,unsigned int>::iterator itSeen;

	this->size++;

	itSeen = this->seen.find(hash_val);
	if(itSeen != this->seen.end() ){// value was found
		itSeen->second++;
	}
	else{				// value wasn't found
		this->seen.insert ( pair< int ,unsigned int>(hash_val,1) );// inserting a new value.
	}
}

// Updating the relevant variables(sumY and sumXY) when one sample is added.
void Enumeration::insertToList( int hash_val){
	map <int,unsigned int>::iterator itSeen;

	if(this->last > 8000){
		return;
	}

	if(this->seen.size()==0){
		this->seen.insert ( pair<int,unsigned int>(hash_val,1) );// inserting a new value.
		this->size++;
		return;
	}

	itSeen = this->seen.find(hash_val);
	if(itSeen != this->seen.end() ){// value was found
		this->last--;
		itSeen->second++;
	}
	else{				// value wasn't found
		this->last++;
		this->seen.insert ( pair<int,unsigned int>(hash_val,1) );// inserting a new value.
	}

	this->size++;

	this->sumY += this->last;
	this->sumXY += ( this->size*this->last );

}


// Getting score for a given value inside the enum dictionary.
//
// For Example: 
//	seen = [73613=>35,323435=>30,3453=>15] , size=80 .
//         
//	Input: hash_val = 323435.
//	Return: score = 0.375 (30/80);
//
//	OR	
//
//	Input: hash_val = 1111123 (the value is not in the enum dictionary).
//	Return: score = 0.012345679 (1/81);
double Enumeration::getScore(int hash_val){	// Calculate the Probability of a new string in the Enumeration list.
	map <int,unsigned int>::iterator it;		
	double score;
	
	it = this->seen.find(hash_val);
	if(it == this->seen.end() ){ // value wasn't found
		score =  (1/((double)this->size+1))  ;
	}else{			     // value was found
		score = ( ( (double) it->second ) / ( (double) this->size ) ); //Laplace transform
	}			

	return score;
}

// Getting exponent score for a given value inside the enum dictionary.
//
// For Example: 
//	seen = [73613=>35,323435=>30,3453=>15] , size=80 .
//         
//	Input: hash_val = 323435.
//	Return: score = -1;
//
//	OR	
//
//	Input: hash_val = 1111123 (the value is not in the enum dictionary).
//	Return: score = -6;
int Enumeration::getProb(int hash_val){	// Calculate the Probability of a new string in the Enumeration list.
	map <int,unsigned int>::iterator it;		
	double score;
	int exp;	

	it = this->seen.find(hash_val);
	if(it == this->seen.end() ){ // value wasn't found
		score = (1/((double)this->size+1)) ;
	}else{			     // value was found
		score = ( ( (double) it->second ) / ( (double) this->size ) ); //Laplace transform
	}
	
	frexp(score,&exp);
	if(exp>0){exp=0;}

	return exp;
}

void Enumeration::print(){
	map <int,unsigned int>::iterator it;	

	cout<< "Size:" << this->size <<endl;
	cout<< "sumY:" << this->sumY <<endl;
	cout<< "sumXY:" << this->sumXY <<endl;
	cout<< "last:" << this->last <<endl;
	cout<< "offset:" << this->offset <<endl;
	for(it = this->seen.begin() ; it != this->seen.end() ; it++ ){
		cout << it->first << " <-> "<<it->second<<endl;			
	}
}

void Enumeration::print_syslog(){
	map <int,unsigned int>::iterator it;
	syslog(LOG_NOTICE,"------------------------------------1");
	syslog(LOG_NOTICE,"Size:%lld",this->size);
	syslog(LOG_NOTICE,"sumY:%lld", this->sumY);
	syslog(LOG_NOTICE,"sumXY:%lld", this->sumXY);
	syslog(LOG_NOTICE,"last:%lld" ,this->last);
	syslog(LOG_NOTICE,"offset:%lld" ,this->offset);
	for(it = this->seen.begin() ; it != this->seen.end() ; it++ ){
		syslog(LOG_NOTICE,"%d <-> %u",it->first, it->second);
	}

}
