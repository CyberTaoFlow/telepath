
#include "Node.h"
#include <stdio.h>

using namespace std;

Node::Node(){
	this->emission=1;
	this->edge_sample=1;
}

void Node::print(){
	cout << "E:" << this->emission << "  edge_sample:"<<  edge_sample<<endl;
}


