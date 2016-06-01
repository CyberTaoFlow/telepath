
#include "Action.h"

#define MAX_VALUE 255 

using namespace std;

// Get a vector of numbers and put them in a string sperate by commas.
void unicodeParser(vector <unsigned int> & shortVec,string & str){ 
	size_t sizeV = shortVec.size();
	size_t length = 0;
	unsigned int len;
	size_t i;

	if(sizeV > MAX_VALUE){
		sizeV = MAX_VALUE;
	}

	char buffer[sizeV * 12+1];
	char * tmp = buffer;

	for( i=0 ; i<sizeV ;i++){
		if(i != sizeV-1 ){	
			len = itoa(shortVec[i],tmp);
			tmp[len++] =',';
			tmp[len] ='\0';
		}else{
			len = itoa(shortVec[i],tmp);				
		}
		length+=len;
		tmp+=len;
	}
	
	buffer[length]='\0';
	str = buffer;
}

ActionPages::ActionPages(){}

void ActionPages::clean(){
	this->pageID=0;
	this->params.clear();
}

Action::Action(){}

void Action::clean(){
	this->action_id=0;
	this->action_name.clear();
	this->pages.clear();
}

ActionParams::ActionParams(boost::unordered_map <unsigned int,string> & params,int unsigned action_id,string & action_name,int unsigned numOfPages,bool logout){
	this->params = params;
	this->action_id = action_id;
	this->action_name = action_name;
	this->numOfPages = numOfPages;
	this->logout = logout;
}

ActionState::ActionState(){}
ActionState::ActionState(unsigned int Sid,unsigned int action_id,string & action_name,boost::unordered_set <unsigned long long> & RIDs,string & shard){
	this->Sid = Sid;
	this->action_id = action_id;
	this->action_name = action_name;
	this->RIDs = RIDs;
	this->shard = shard;
	this->counter=0;
}


