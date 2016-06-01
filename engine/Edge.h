#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;

class Edge{
public:

	// Using the boost serialization-library for serializing and deserializing.
	friend class boost::serialization::access;	
	std::string from_node_comp;	 // Unique key for the source node.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & emission;
		ar & from_node_comp;
	}

	unsigned int emission;	 // The emission of this specific Edge.    

	Edge();
	Edge(string &);
	void print(); // Printing the class variables.

};

