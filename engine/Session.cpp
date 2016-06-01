#include "Session.h"

using namespace std;

// Session is a string of pages.
// Session is the argument for the tokenize function  
// which create a path.

alert::alert(){
	this->end_ts = 0;
	this->start_ts = 0;
	this->ridAlert = 1;
}

Session::Session(){
	this->sequence=0;
	this->update=1;
	this->totalExp = 0;
	this->block = 'n';
	this->status ='v';
	this->user_flag=0;
	this->sessionSize=0;
	this->erase='n';
	this->validUser = false;
	this->logout = false;
	this->logout_counter = 0;
}

void Session::reset(){
	vector< Page >().swap(this->vRequest);
	this->sequence=0;
	this->update=1;
	this->totalExp = 0;
	this->block = 'n';
	this->user_flag=0;
	this->sessionSize=0;
	this->erase='n';
	this->validUser = false;
	this->logout = false;
}


