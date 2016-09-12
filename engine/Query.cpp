#include "Query.h"
#include "syslog.h"

using namespace std;

Query::Query(){}

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


