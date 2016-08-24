#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class Node{
public:

	friend class boost::serialization::access;
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Node object to/from the disk.
	*/	
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & emission;
		ar & edge_sample;
	}

	//!The number of times this character exists in that level/index.
	/*!*/
	unsigned int emission;

	//!The number of times this character in that level/index is not the last letter in the word.
        /*!*/
	unsigned int edge_sample;

	//! Defult Constrator.
	/*!
		Initializing the object fields with its default values.
	*/
	Node();

	//! Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\param none
		\return void
	*/	
	void print();

};
