#include "Tree.h"
#include "Link.h"

using namespace std;

// Defult Constrator.
Link::Link(){}
Link::Link(string & to_page_comp,string & from_page_comp,unsigned short landing){
	this->from_page_comp = from_page_comp;
	this->compare = from_page_comp;
	this->compare.push_back('|');
	this->compare.append(to_page_comp); // node_key_a|node_key_b . For example : "0+53|1+71"

	this->landing = landing;
}

// Clear the object.
void Link::clean(){
	this->from_page_comp.clear();
	this->compare.clear();
}

// Printing
void Link::print(){
	cout << "compare:"<< this->compare ;
	cout << "  Landing:" << landing << endl;;
}


