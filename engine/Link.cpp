#include "Tree.h"
#include "Link.h"

using namespace std;

Link::Link(){}
Link::Link(string & compare,double & landing){
	this->compare = compare;
	this->landing = landing;
}

void Link::clean(){
	this->compare.clear();
}

void Link::print(){
	cout << "compare:"<< this->compare;
	cout << "  Landing:" << landing << endl;
}


