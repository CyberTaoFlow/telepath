
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>

using namespace std;

class Enumeration{
public:

	friend class boost::serialization::access;

	//!  A C++ STL map of unsigned integers sorted by integer keys.
	/*!
		\n For example:
		\n [43214353=>35,-322133435=>30,933453=>15]
		\n
		\n The key is a hash number and value is the emission(how many times this hash has been seen) of that hash. 
	*/
	std::map <int ,unsigned int> seen;
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Numeric object to/from the disk.
	*/
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & seen;
		ar & last;
		ar & sumY;
		ar & sumXY;
		ar & size;
	}

	//!  The Number of Samples.
	/*!*/
	long long size;

	long long offset;
	long long last;

	//!  The second parameter for the covariance calculation.
	/*!
		\sa http://math.info/Statistics/Covariance/
	*/
	long long sumY;

	//!  The first parameter for the covariance calculation. 
	/*!
		\sa http://math.info/Statistics/Covariance/
	*/
	long long sumXY;

	//!  Defult Constrator.
	/*!
		Initializing the object fields (only the word fields) with their default values.
	*/
	Enumeration();

	//!  Enumeration Classification.
	/*!
		This function classifies as Enumeration a list of values by finding a convergence between two vector.
		\return true or false as a boolean argument.
		\n      true - The list is classified as Enumeration.
		\n      false - The list is not classified as Enumeration.
	*/
	bool getCorrelation();

	//!  Adding Samples.
	/*!
		This function adds a sample into the 'seen' C++ STL map. It creates a new key or increasing the emission of an existing one.
		\param hash_val as an integer argument.
	*/
	void insertToList(int);

	//!  Adding Samples and Updating Fields.
	/*!
		This function adds a sample into the 'seen' C++ STL map. It creates a new key or increasing the emission of an existing one. It also updates the relevant fields(sumY and sumXY) when one sample is added.
		\param hash_val as an integer argument.
	*/
	void insertIntoEnum(int);

	//!  Calculating Probability.
	/*!
		This function gets a hash value as an argument and returns a probability score accroding to the Enumeration list.
		\n For Example:
		\n 
		\n seen = [73613=>35,323435=>30,3453=>15] , size=80
		\n
		\n Input: hash_val = 323435.
		\n Return: score = -1;
		\n
		\n OR
		\n
		\n Input: hash_val = 1111123 (the value is not in the Enumeration list).
		\n Return: score = -6;
		\param hash_val as an integer argument.
		\return an exponent(less than zero) number as a score.
	*/
	int getProb(int);

	//! Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\param none
		\return void
	*/
	void print();

	//! Printing the class variables.
	/*!
		Printing to syslog the object fields.
		\param none
		\return void
	*/
	void print_syslog();

};
