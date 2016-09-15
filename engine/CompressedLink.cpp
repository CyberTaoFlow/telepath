#include "CompressedLink.h"

using namespace std;

CompressedLink::CompressedLink(){}
CompressedLink::CompressedLink(string & from_page_comp,double & landing){
	this->from_page_comp = from_page_comp;
	this->emission=1;
	this->diffLanding.push_back(landing);
}

void CompressedLink::clean(){
	vector <double> ().swap(this->diffLanding); 
	this->from_page_comp.clear();
}

void CompressedLink::print(){
	cout << "E:" << this->emission ;

	cout << "  Landing:";
	for(unsigned int i=0;i<this->diffLanding.size();i++)
	 	cout << this->diffLanding[i] <<"  ";
	cout <<endl;
}



