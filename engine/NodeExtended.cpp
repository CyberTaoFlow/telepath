#include "NodeExtended.h"

using namespace std;

// Defult Constrator.
NodeExtended::NodeExtended(){}
NodeExtended::NodeExtended(unsigned int index){
	this->index= index ;
	this->emission=1;
	this->link_sample=1;
}

// Printing
void NodeExtended::print(){
	cout << "index:" << this->index << "   emission:" << this->emission << "   link_sample" << this->link_sample << endl;
}


