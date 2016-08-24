#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;

class CompressedLink{
public:

	friend class boost::serialization::access;

	//!  Unique key for the exit page( the index and the hash page value ).
	/*!*/
	std::string from_page_comp;

	//!  A C++ STL vector that contains all the transition speeds from index to index. 
	/*!
		For Example :
		\n
		\n From key1(/index.html in the first index) to key2(/clients.html in the second index) the vector looks like that:
		\n [5,23,6,1,4,2,7,4,8,1]
	*/
	std::vector <unsigned short> diffLanding; 
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Tree object to/from the disk.
	*/
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & from_page_comp;  
		ar & diffLanding;     
		ar & emission;  
	}

	//!  The number of times this key(Page_ID_from -> Page_ID_to) exists in that level/index.
	/*!*/
	unsigned int emission; 

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	CompressedLink();

	//!  Constrator.
	/*!
		Initializing the object fields with their default values.
		\param from_page_comp as a C++ string argument.
		\param landing as an unsigned short argument.
	*/
	CompressedLink(string &,unsigned short);

	//!  Cleaning all object fields.
	/*!
		\sa http://www.cplusplus.com/forum/beginner/32265/
	*/
	void clean();

	//! Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\param none
		\return void
	*/
	void print();
};
