
#include <stdio.h>
#include <syslog.h>
#include "Numeric.h"

extern void es_mapping(string,string);
extern void es_insert(string,string);
using namespace std;

Numeric::Numeric(){
	this->mean = 0;
	this->variance = 0;
	this->stddev=0;
	this->size=0;
	this->valid=false;
	this->min=0;
	this->max=0;
}

void Numeric::init(vector <double> & vec){
	double sum=0.0,sum2=0.0;
	unsigned int i;

	this->size = vec.size();

	if( this->size == 0 ){ //empty vector.
		return;
	}

	for (i=0; i < this->size ; i++){
		if(i == 0){ // First sample.
			this->min = vec[i];
			this->max = vec[i];
			this->valid=true;
		}else{
			if( vec[i] < this->min){
				this->min = vec[i];
			}
			else if( vec[i] > this->max){
				this->max = vec[i]; 
			}
		}
		sum += vec[i]; //Calculating the average. 	
	}

	this->mean = sum/this->size;
	for (i=0; i < this->size ;i++){
		sum2 += pow((vec[i] - this->mean),2); //Calculating the variance. 
	}

	this->variance = sum2/this->size ;
	this->stddev = sqrt(this->variance);
}

void Numeric::init(vector <unsigned short> & vec){
	double sum=0.0,sum2=0.0;
	unsigned int i;

	this->size = vec.size();

	if( this->size == 0 ){ //empty vector.
		return;
	}

	for (i=0; i < this->size ; i++){
		if(i == 0){ // First sample.
			this->min = vec[i];
			this->max = vec[i];
			this->valid=true;
		}else{
			if( vec[i] < this->min){
				this->min = vec[i];
			}
			else if( vec[i] > this->max){
				this->max = vec[i]; 
			}
		}

		sum += vec[i]; //Calculating the average. 
	}

	this->mean = sum/(double)this->size;
	for (i=0; i < this->size ;i++){
		sum2 += pow((vec[i] - this->mean),2); //Calculating the variance.
	}

	this->variance = sum2/(double)vec.size() ;
	this->stddev = sqrt(this->variance);

}

void Numeric::init(vector <int> & vec){
	double sum=0.0,sum2=0.0;
	unsigned int i;

	this->size = vec.size();

	if( this->size == 0 ){ //empty vector.
		return;
	}

	for (i=0; i < this->size ; i++){
		if(i == 0){ // First sample.
			this->min = vec[i];
			this->max = vec[i];
			this->valid=true;
		}else{
			if( vec[i] < this->min){
				this->min = vec[i];
			}
			else if( vec[i] > this->max){
				this->max = vec[i]; 
			}
		}

		sum += vec[i]; //Calculating the average. 
	}

	this->mean = sum/(double)this->size;
	for (i=0; i < this->size ;i++){
		sum2 += pow((vec[i] - this->mean),2); //Calculating the variance.
	}

	this->variance = sum2/(double)this->size ;
	this->stddev = sqrt(this->variance);
}

double Numeric::chebyshev(double x){
	double result;		
	double variance =  this->variance;
	double distance =  pow((x - this->mean), 2 );
	if (distance == 0)
 		return 1.0;
	else{
 		result = variance/distance;
		if (result > 1.0)
			result = 1.0;
		return result;
	}
}

void Numeric::update(double new_num){
	double this_sum,old_mean,A,B;

	if(this->valid == false){
		this->min = new_num;
		this->max = new_num;
		this->valid = true;
	}else{
		if( new_num < this->min){
			this->min = new_num;
		}
		else if( new_num > this->max){
			this->max = new_num; 
		}
	}

	old_mean = this->mean;
	this_sum = this->mean*this->size+new_num;
	this->mean = this_sum/(this->size+1);

	A = this->size * this->variance ;			
	B = (new_num - this->mean) * (new_num - old_mean);

	this->size++;
	this->variance = (A + B) / (this->size);
	this->stddev = sqrt(this->variance);
}

void Numeric::update(double new_num,unsigned int length){
	double this_sum,old_mean,A,B;

	if(this->valid == false){
		this->min = new_num;
		this->max = new_num;
		this->valid = true;
	}else{
		if( new_num < this->min){
			this->min = new_num;
		}
		else if( new_num > this->max){
			this->max = new_num; 
		}
	}

	old_mean = this->mean;
	this_sum = this->mean*this->size+new_num*length;
	this->mean = this_sum/(this->size+length);

	for(unsigned int i=0;i<length;i++){
		A = this->size * this->variance ;			
		B = (new_num - this->mean) * (new_num - old_mean);

		this->size++;
		this->variance = (A + B) / (this->size);
		
	}

	this->stddev = sqrt(this->variance);
}

void Numeric::clean(){
	this->mean = 0;
	this->variance = 0;
	this->stddev=0;
	this->size=0;
	this->valid=false;
	this->min=0;
	this->max=0;
}

void Numeric::print(){
	cout <<endl;
	cout << "\t\tmean:" << this->mean << endl;
	cout << "\t\tstddev:" <<this->stddev << endl;
	cout << "\t\tvariance:" <<this->variance << endl;
	cout << "\t\tsize:" <<this->size << endl;

}

void Numeric::print_syslog(){
	syslog(LOG_NOTICE,"    mean:%f",this->mean);
	syslog(LOG_NOTICE,"    stddev:%f",this->stddev);
	syslog(LOG_NOTICE,"    variance:%f",this->variance);
	syslog(LOG_NOTICE,"    size:%u",this->size);
}

void Numeric::print_elastic(string & att_name){
	char url[1000],postfields[1000];
	unsigned int epoch = (unsigned int)time(NULL);
	sprintf(url,"/telepath-debug/debug/%s/_create",att_name.c_str());
	es_insert(url,"{}");

	sprintf(url,"/telepath-debug/debug/%s/_update",att_name.c_str());
	sprintf(postfields,"{\"doc\":{%u:{\"mean\":%f,\"stddev\":%f,\"size\":%u}}}",epoch,this->mean,this->stddev,this->size);
	es_mapping(url,postfields);
}
