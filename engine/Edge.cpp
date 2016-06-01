#include "Edge.h"

using namespace std;

Edge::Edge(){}
Edge::Edge(string & from_node_comp){
	this->from_node_comp = from_node_comp;
	this->emission=1;
}

// Printing the class variables.
void Edge::print(){
	cout << "E:" << this->emission << "  node_compare:" << from_node_comp << endl;
}
