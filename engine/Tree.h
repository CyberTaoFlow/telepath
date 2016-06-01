#include "Node.h"
#include "Edge.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#define MAX_MARKOV_VALUE 31
#define MIN_PROB -500 

unsigned int lenHelper(unsigned int x);
unsigned int itoa(unsigned int val,char *buf); // Fast implementation for integer to string.


using namespace std;

class Tree{
public:

	// Using the boost serialization-library for serializing and deserializing.
	friend class boost::serialization::access;
	std::map <string,Node> mNode;  // Nodes
	std::map <string,Edge> mEdge; // Egdes
	std::vector <unsigned long> sampleNode;  
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mNode;
		ar & mEdge;
		ar & sampleNode;
		ar & word;
	}

	unsigned long word; // Number of words so far.

	Tree();
	void clean();
	void tokenize(vector <unsigned int> &);
	void updateNode(bool,string &);
	void updateEdge(Edge &,string);
	int calculate(vector <unsigned int>&,double);
	void print();
	void printCapacity();
};
