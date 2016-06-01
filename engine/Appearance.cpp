#include "Appearance.h"

using namespace std;

attAppearance::attAppearance(){}
attAppearance::attAppearance(string & ids){
	this->att_ids.insert(pair<string,unsigned int>(ids,1));
	this->counter=1;
}

void attAppearance::print(){
	map <string,unsigned int>::iterator it;
	cout <<"\tC:"<< this->counter <<endl;
	for(it = this->att_ids.begin(); it!=this->att_ids.end() ;it++){
		cout <<"\t"<<it->first<<"->"<< it->second <<endl;
	}
}
