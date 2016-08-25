#include "Node.h"
#include "Edge.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#define MAX_MARKOV_VALUE 31
#define MIN_PROB -500 

//!  Optimization Method.
/*!
	\param x an unsigned integer argument.
*/
unsigned int lenHelper(unsigned int x);

//!  Optimization Method.
/*!
	The tokenize() function uses that function instead of sprintf() which is slower.
	\param val an unsigned integer argument.
	\param buf a pointer to character argument.
*/
unsigned int itoa(unsigned int val,char *buf);


using namespace std;

class Tree{
public:

	friend class boost::serialization::access;

	//!  A C++ STL map of Nodes sorted by C++ string keys.
	/*!
		For example : the word "dad" gets these node keys - "0+100","1+97","2+100".
		\n "0+100" is a possible key with the character 'd'(100='d') in the first level/index(0=the index/level).
	*/
	std::map <string,Node> mNode;

	//!  A C++ STL map of Edges sorted by C++ string keys.
	/*!
		For example : the word "dad" gets these edge keys - "0+100|97","1+97|100".
	*/
	std::map <string,Edge> mEdge;

	//!  A C++ STL vector of unsigned longs(8 bytes) that stores the total emissions for each index.
	/*!
		For example : "doctor","cat","lord". 
		\n Following the words/strings listing above the vector looks like that:
		\n [3,3,3,2,1,1]
	*/
	std::vector <unsigned long> sampleNode;  
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Tree object to/from the disk.
	*/
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mNode;
		ar & mEdge;
		ar & sampleNode;
		ar & word;
	}

	//!  The number of analyzed words.
	/*!*/	
	unsigned long word;

	//!  Defult Constrator.
	/*!
		Initializing the object fields (only the word fields) with their default values.
	*/
	Tree();

	//!  Cleaning all object fields.
	/*!
		\sa http://www.cplusplus.com/forum/beginner/32265/
	*/
	void clean();

	//!  Comprassing Technique.
	/*!
		Replacing data with unique identification symbols that retain all the essential information. The comprassed modulelooks like a python dictionary or a C++ STL map.
		\param attr_value a C++ STL vector of unsigned integers.
	*/
	void tokenize(vector <unsigned int> &);

	//! Creating/Increasing a Node.
	/*!
		Creating a new Node or increasing the emission of an existing one.
		\param last_letter a boolean argument.
		\param comp a C++ string agrument.
		\return void
	*/
	void updateNode(bool,string &);

	//! Creating/Increasing an Edge.
	/*!
		Creating a new Edge or increasing the emission of an existing one.
		\param edge a Edge object.
		\param comp a C++ string agrument.
		\return void
	*/
	void updateEdge(Edge &,string);

	//!  Calculating a Score for a given word.
	/*!
		This function gets 'value'(a vector of utf8 values represnting a word) as an argument and returns a statistical score.
		\param value a C++ STL vector of unsigned integers.
		\param avg_size a double agrument.
		\return an exponent(less than zero) number as a score.
	*/
	int calculate(vector <unsigned int>&,double);

	//! Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\return void
	*/
	void print();

	//! Printing the class memory consumption.
	/*!
		Printing to stdout the object field capacities.
		\param none
		\return void
	*/
	void printCapacity();
};
