
#include "Node.h"
#include <stdio.h>

using namespace std;

// Defult Constrator.
Node::Node(){
	this->emission=1;
	this->edge_sample=1;
}

// Printing the class variables.
void Node::print(){
	cout << "E:" << this->emission << "  edge_sample:"<<  edge_sample<<endl;
}


