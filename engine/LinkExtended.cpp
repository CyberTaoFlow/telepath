#include "LinkExtended.h"

using namespace std;

// Defult Constrator.
LinkExtended::LinkExtended(){}
LinkExtended::LinkExtended(string & from_page_comp,unsigned short landing){
	this->from_page_comp = from_page_comp;
	this->emission=1;
	this->diffLanding.push_back(landing);
}

// Cleaning object.
void LinkExtended::clean(){
	vector <unsigned short> ().swap(this->diffLanding); 
	this->from_page_comp.clear();
}

// Printing.
void LinkExtended::print(){
	cout << "E:" << this->emission ;

	cout << "  Landing:";
	for(unsigned int i=0;i<this->diffLanding.size();i++)
	 	cout << this->diffLanding[i] <<"  ";
	cout <<endl;
}



