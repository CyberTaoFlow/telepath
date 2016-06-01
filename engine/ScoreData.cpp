
#include <syslog.h>
#include "ScoreData.h"
// These 4 Classes operate just once.
// When the engine switch to production.

ScoresData::ScoresData(){}
void ScoresData::insert(double flow,double landing,double geo){
	map <double,unsigned int>::iterator itData;

	itData = this->flow.find(flow);
	if(itData==this->flow.end() ){ //Wasn't found.
		this->flow.insert ( pair<double,unsigned int>(flow,1) );
	}else{ 
		itData->second++;
	}

	itData = this->landing.find(landing);
	if(itData==this->landing.end() ){ //Wasn't found.
		this->landing.insert ( pair<double,unsigned int>(landing,1) );
	}else{ 
		itData->second++;
	}

	itData = this->geo.find(geo);
	if(itData==this->geo.end() ){ //Wasn't found.
		this->geo.insert ( pair<double,unsigned int>(geo,1) );
	}else{ 
		itData->second++;
	}
}

void ScoresData::print(){
	map <double,unsigned int>::iterator itData;
	cout << "\tflow"<<	this->flow.size()<<endl;
	for(itData = this->flow.begin() ;itData != this->flow.end();itData++){
		cout << "\t\t"<<itData->first <<"="<<itData->second<<endl;
	}
	cout << "\tlanding"<<	this->landing.size()<<endl;
	for(itData = this->landing.begin() ;itData != this->landing.end();itData++){
		cout << "\t\t"<<itData->first <<"="<<itData->second<<endl;
	}
	cout << "\tgeo"<<	this->geo.size()<<endl;
	for(itData = this->geo.begin() ;itData != this->geo.end();itData++){
		cout << "\t\t"<<itData->first <<"="<<itData->second<<endl;
	}
}

void ScoresData::clean(){
	this->flow.clear();
	this->landing.clear();
	this->geo.clear();
}


ScoresNumeric::ScoresNumeric(){}	
void ScoresNumeric::insert(ScoresData & data){ // attach the unsortVECs in the Nimeric objects
	map <double,unsigned int>::iterator itData;		

	for(itData=data.flow.begin();itData!=data.flow.end();itData++){
		this->flow.update(itData->first,itData->second);
	}

	for(itData=data.landing.begin();itData!=data.landing.end();itData++){
		this->landing.update(itData->first,itData->second);
	}

	for(itData=data.geo.begin();itData!=data.geo.end();itData++){
		this->geo.update(itData->first,itData->second);
	}

}

void ScoresNumeric::print(){
	this->flow.print();
	this->landing.print();
	this->geo.print();
}

void ScoresNumeric::print_syslog(){
	syslog(LOG_NOTICE,"---------------");
	syslog(LOG_NOTICE,"  Flow");
	this->flow.print_syslog();
	syslog(LOG_NOTICE,"  Landing");
	this->landing.print_syslog();
	syslog(LOG_NOTICE,"  Geo");
	this->geo.print_syslog();
}

ScoreAtt::ScoreAtt(){}
void ScoreAtt::insert(double attribute,double att_size){
	map <double,unsigned int>::iterator itAtt;

	itAtt = this->attribute.find(attribute);
	if( itAtt == this->attribute.end() ){
		this->attribute.insert(pair<double,unsigned int>(attribute,1));
	}else{
		itAtt->second++;
	}


	if(att_size != 1){
		itAtt = this->att_size.find(att_size);
		if( itAtt == this->att_size.end() ){
			this->att_size.insert(pair<double,unsigned int>(att_size,1));
		}else{
			itAtt->second++;
		}
	}
}

void ScoreAtt::print(){
	cout << "\t"<<	this->attribute.size()<<endl;
	cout << "\t"<<	this->att_size.size()<<endl;
}

void ScoreAtt::clean(){
	this->attribute.clear();
	this->att_size.clear();
}

	
ScoreNumericAtt::ScoreNumericAtt(){}
void ScoreNumericAtt::insert(ScoreAtt & att){
	map <double,unsigned int>::iterator itAtt;
	for(itAtt=att.attribute.begin();itAtt!=att.attribute.end();itAtt++){
		this->attribute.update(itAtt->first,itAtt->second);
	}	

	for(itAtt=att.att_size.begin();itAtt!=att.att_size.end();itAtt++){
		this->att_size.update(itAtt->first,itAtt->second);
	}	

}

void ScoreNumericAtt::print(){
	this->attribute.print();
	this->att_size.print();
}

void ScoreNumericAtt::print_syslog(){
	syslog(LOG_NOTICE,"  Attribute Score");
	this->attribute.print_syslog();
	syslog(LOG_NOTICE,"  Attribute Length");
	this->att_size.print_syslog();
}

void ScoreNumericAtt::print_elastic(string att_name){
	this->attribute.print_elastic(att_name);
}
