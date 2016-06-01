#include "Query.h"
#include "syslog.h"

using namespace std;

Query::Query(){}

// Culculating a total query score for a given vector of parameters.
//
// For Example: 
//	Input:
//		params = [
//			{score=0.5,length=0,weight=1},              /*User-Agent*/
//			{score=0.75,length=0.5,weight=0.25},        /*Host*/
//			{score=0,length=0,weight=1},                /*Content-length*/
//			{score=0.1,length=0,weight=1}               /*Accept*/
//		] 
//      	presence = 0.3
//
//	Return: result = ( ( 0.2423/*average_score*/ + 0.0384/*average_length*/ + 0.3/*presence*/ )/3 ) = 0.1935
Query::Query(vector <Parameter> & params,double presence){
	double sum_score=0,sum_weight=0,sum_length=0,average_score,average_length;	

	for (unsigned int i=0; i<params.size();i++){
		sum_weight +=  params[i].weight;
		sum_score +=  (params[i].score*params[i].weight);
		sum_length += (params[i].length*params[i].weight);
	}

	if(sum_weight!=0){
		average_score = (sum_score/sum_weight) ; // Weighted mean.
		average_length = (sum_length/sum_weight) ; // Weighted mean.
		this->result = (average_score + average_length + presence)/3;
	}else{
		this->result = 0;
	}
}


