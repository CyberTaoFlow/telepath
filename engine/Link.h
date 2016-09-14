#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;

class Link{
public:

	friend class boost::serialization::access;

	//!  Unique key for Link.
	/*!*/
	std::string compare;
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Tree object to/from the disk.
	*/
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & landing;     
		ar & compare;  
	}

	//!  Transition Speed Between Pages.
	/*!
		For Example : epoch_page[2] - epoch_page[1] .
	*/
	double landing; 

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	Link();

	//!  Constrator.
	/*!
		Initializing the object fields with their default values.
		\param compare as a C++ string argument.
		\param landing as an unsigned short argument.
	*/
	Link(string &,double&);

	//!  Cleaning all object fields.
        /*!*/
	void clean();

	//! Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\return void
	*/
	void print();
};

