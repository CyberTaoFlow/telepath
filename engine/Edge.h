#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;

class Edge{
public:

	friend class boost::serialization::access;

	//!  Unique key for the exit node( the index and the utf8 value of the character ).
	/*!*/
	std::string from_node_comp;
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Edge object to/from the disk.
	*/
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & emission;
		ar & from_node_comp;
	}

	//!  The number of times this transition(between characters) exists in that level/index.
	/*!*/
	unsigned int emission;

	//! Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	Edge();

	//! Defult Constrator.
	/*!
		Initializing the object fields with their default values.
		\param from_node_comp as a C++ string argument. 
	*/
	Edge(string &);

	//! Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\param none
		\return void
	*/
	void print(); // Printing the class variables.
};

