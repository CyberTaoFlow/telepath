#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class Node{
public:

	// Using the boost serialization-library for serializing and deserializing.
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & emission;
		ar & edge_sample;
	}
	unsigned int emission; 
	unsigned int edge_sample;
	
	Node(); // Defult Constrator.
	void print(); // Printing the class variables.

};
