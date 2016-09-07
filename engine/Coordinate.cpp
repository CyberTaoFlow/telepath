#include "Coordinate.h"
#include <stdio.h>
#include <syslog.h>

using namespace std;

Coordinate::Coordinate(){
	this->noise='0';
	this->visited='0';
}

Coordinate::Coordinate(float x,float y){
	char buffer[50];
	this->x=x;
	this->y=y;
	this->visited='n'; 
	this->counter=1;
	this->noise='0';

	sprintf(buffer,"%.4f,%.4f",this->x,this->y); // Building a unique key to Coordinate.
	this->key = buffer;			     // For example: NYC longitude and latitude = "40.7127,74.0059"
}

void Coordinate::setCoordinate(float x,float y){
	char buffer[50];

	this->x=x;
	this->y=y;	
	this->visited='n';
	this->counter=1;

	sprintf(buffer,"%.4f,%.4f",this->x,this->y); // Building a unique key to Coordinate.
	this->key = buffer;			     // For example: LA longitude and latitude = "34.05,118.25"	
}

void Coordinate::print_syslog(){
	syslog(LOG_NOTICE,"(%f,%f)  -E: %u   key:%s  visited:%c  noise:%c",this->x,this->y,this->counter,this->key.c_str(),this->visited,this->noise);
}

void Coordinate::show_syslog(){
	syslog(LOG_NOTICE,"(%f,%f)",this->x,this->y);
}

