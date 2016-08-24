
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class CompressedPage{
public:

	friend class boost::serialization::access;
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the CompressedPage object to/from the disk.
	*/
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & index;     
		ar & tainted;  
		ar & emission;
		ar & link_sample;        
	}

        //!  The Sequence of the Request
        /*!
                For example:
                \n  0 - is the first request in the session.
        */
	unsigned int index;

	//!  Tainted Request (HTTP Status Code is greater than 400).
	/*!
		"0" or "1".
	*/
	char tainted;

	//!  The number of times this key(Page ID) exists in that level/index.
	/*!*/
	unsigned int emission;

        //!  The number of times this key(Page ID) in that level/index is not the last request in the session(so far).
        /*!*/
	unsigned int link_sample;

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	CompressedPage();

	//!  Constrator.
	/*!
		Initializing the object fields with their default values.
		\param index as an unsigned integer argument.
	*/
	CompressedPage(unsigned int);

	//! Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\param none
		\return void
	*/
	void print();
};

