
#include "Case.h"
#include "Range.h"
#include "jsoncpp/json.h"
#include <stdlib.h>
#include <iostream>
#include <syslog.h>

using namespace std;

SubCase::SubCase(){}

void SubCase::clean(){
	this->vals.clear();
	this->ips.clear();
	this->countries.clear();
	this->apps.clear();
	this->rules.clear();
}

void SubCase::init(){
	unsigned int pos=0,i;
	string name,from,to;
	for(i=0;i<this->vals.size();i++){
		if(this->vals[i]==',' || i==this->vals.size()-1 ){
			if( i!=(this->vals.size()-1) ){
				name.assign(this->vals.begin()+pos,this->vals.begin()+i);
				pos=i+1;
			}else{
				name.assign(this->vals.begin()+pos,this->vals.begin()+i+1);
			}

			this->values.push_back(name);
			if( i==(this->vals.size()-1) ){
				break;
			}

			name.clear();
		}
	}

	if(this->type=='I'){// I=IP.
		for (i=0;i<this->values.size();i++){

			if( ipSingleOrRange(this->values[i],from,to) == false ){
				unsigned int IP = ipToNum(this->values[i]);
				Range r( IP,IP );
				this->ips.push_back(r);
			}else{
				Range r( ipToNum(from),ipToNum(to) );
				this->ips.push_back(r);
			}
		}
	}
	else if(this->type=='c'){
		for (i=0;i<this->values.size();i++){
			this->countries.insert(this->values[i]);
		}
	}
	else if(this->type=='a'){
		for (i=0;i<this->values.size();i++){
			this->apps.insert(this->values[i]);
		}
	}

	else if(this->type=='r'){ //rule group
		for (i=0;i<this->values.size();i++){
			this->rules.insert((unsigned int)atoi(this->values[i].c_str()));
		}
	}

	this->vals.clear();
}

bool SubCase::isMatch(unsigned int ip,string & country,string & app/*,unsigned int rule_id*/){
	if(this->type=='I'){
		bool ipFlag=false;
		for(unsigned int i=0;i<this->ips.size();i++){
			if(this->ips[i].inRange(ip) == true){
				ipFlag=true;
				break;
			}
		}

		if(ipFlag == true){
			if(this->neg==false){
				return true;
			}else{
				return false;
			}
		}else{
			if(this->neg==false){
				return false;
			}else{
				return true;
			}
		}
	}
	else if(this->type=='c'){
		if(this->countries.count(country) != 0 ){
			if(this->neg==false){
				return true;
			}else{
				return false;
			}
		}else{
			if(this->neg==false){
				return false;
			}else{
				return true;
			}
		}
	}
	else if(this->type=='a'){
		if(this->apps.count(app) != 0 ){
			if(this->neg==false){
				return true;
			}else{
				return false;
			}
		}else{
			if(this->neg==false){
				return false;
			}else{
				return true;
			}
		}
	}

	/*else if(this->type=='r'){
		if(this->rules.count(rule_id) != 0 ){
			if(this->neg==false){
				return true;
			}else{
				return false;
			}
		}else{
			if(this->neg==false){
				return false;
			}else{
				return true;
			}
		}
	}*/

	return false;
}

void SubCase::print(){
	cout << "Type:"<<this->type <<endl;
	cout << "Neg:"<<this->neg <<endl;

	if(this->type=='I'){// I=IP.
		for (unsigned int j=0;j<this->ips.size();j++){
			cout << "\t";
			this->ips[j].print();
			cout << "\n";
		}
	}
	else if(this->type=='c'){
		set <string>::iterator it;
		for (it = this->countries.begin();it != this->countries.end();it++){
			cout << "\t" << (*it) << endl;
		}
		cout<<endl;
	}
	else if(this->type=='a'){
		set <string>::iterator it;
		for (it = this->apps.begin();it != this->apps.end();it++){
			cout << "\t"<<(*it) << endl;
		}
		cout<<endl;
	}
	else if(this->type=='r'){
		set <unsigned int>::iterator it;
		for (it = this->rules.begin();it != this->rules.end();it++){
			cout << "\t"<<(*it) << endl;
		}
		cout<<endl;
	}

}

void SubCase::print_syslog(){
	syslog(LOG_NOTICE,"Type:%c",this->type);

	if(this->type=='I'){// I=IP.
		for (unsigned int j=0;j<this->ips.size();j++){
			this->ips[j].print_syslog();
		}
	}
	else if(this->type=='c'){
		set <string>::iterator it;
		for (it = this->countries.begin();it != this->countries.end();it++){
			syslog(LOG_NOTICE,"     %s",(*it).c_str() );
		}
	}
	else if(this->type=='a'){
		set <string>::iterator it;
		for (it = this->apps.begin();it != this->apps.end();it++){
			syslog(LOG_NOTICE,"     %s",(*it).c_str() );
 		}
        }
	else if(this->type=='r'){
		set <unsigned int>::iterator it;
		for (it = this->rules.begin();it != this->rules.end();it++){
			syslog(LOG_NOTICE,"     %u",(*it) );
		}
	}

}

Case::Case(){
	this->case_name.clear();
}

void Case::clean(){
	this->case_name.clear();
	this->subcase.clear();
}

void Case::init(){
	for(unsigned int i=0;i<this->subcase.size();i++){
		this->subcase[i].init();
	}
}

void Case::print(){
	cout<<"\n***********************************"<<endl;
	cout<<"case_name:"<<case_name<<endl;
	for(unsigned int i=0;i<this->subcase.size();i++){
		this->subcase[i].print();
	}

	cout<<"***********************************"<<endl;
}

void Case::print_syslog(){
	syslog(LOG_NOTICE,"***********************************");
	syslog(LOG_NOTICE,"case_name:%s",case_name.c_str());
	for(unsigned int i=0;i<this->subcase.size();i++){
		this->subcase[i].print_syslog();
	}

	syslog(LOG_NOTICE,"***********************************");
}

