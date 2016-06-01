#define MAX_VALUE 512 

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

class ActionPages{
public:
	unsigned int pageID;
	boost::unordered_map <unsigned int,string> params;

	ActionPages(){}

	void clean(){
		this->pageID=0;
		this->params.clear();
	}

};

class Action{
public:
	unsigned int action_id;
	string action_name;
	string domain;
	bool logout;
	vector <ActionPages> pages;

	Action(){}

	void clean(){
		this->action_id=0;
		this->action_name.clear();
		this->pages.clear();
	}

};


class ActionParams{
public:

	unsigned int numOfPages;
	unsigned int action_id;
	string action_name;
	bool logout;
	boost::unordered_map <unsigned int,string> params;

	ActionParams(boost::unordered_map <unsigned int,string> & params,int unsigned action_id,string & action_name,int unsigned numOfPages,bool logout){
		this->params = params;
		this->action_id = action_id;
		this->action_name = action_name;
		this->numOfPages = numOfPages;
		this->logout = logout;
	}
};

class ActionState{
public:
	unsigned int Sid;
	unsigned int action_id;
	string action_name;

	ActionState(){}
	ActionState(unsigned int Sid,unsigned int action_id,string & action_name){
		this->Sid = Sid;
		this->action_id = action_id;
		this->action_name = action_name;
	}
};

