#ifndef _Numeric_h
#define _Numeric_h

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <vector>

#include <math.h>


using namespace std;

class Numeric{
public:

	friend class boost::serialization::access;
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Numeric object to/from the disk.
	*/
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mean;
		ar & stddev;
		ar & variance;
		ar & size;
	}

	//!  Average.
	/*!
		The average of all samples.
	*/
	double mean;

	//!  Variance.
	/*!
		Measuring how far a set of numbers is spread out.
	*/
	double variance;

	//!  Standard Deviation.
	/*!
		Measuring the amount of variation or dispersion of a set of data values. The square root of the variance.
	*/
	double stddev;

	//!  The Number of Samples.
	/*!*/
	unsigned int size;

	//!  The Minimum Sample.
	/*!*/
	double min;

	//!  The Maximum Sample.
	/*!*/
	double max;

	//!  The First Sample.
	/*!*/
	bool valid;

        //!  Defult Constrator.
        /*!
		Initializing the object fields with their default values.
	*/
	Numeric();

	//!  Initializing Object Fields.
	/*!
		Calculating average, variance, standard-deviation,minimun value and maximum value.
		\param vec a C++ STL vector of double numbers. 
	*/
	void init(vector <double> &);

	//!  Initializing Object Fields.
	/*!
		Calculating average, variance, standard-deviation,minimun value and maximum value.
		\param vec a C++ STL vector of unsigned shorts. 
        */
	void init(vector <unsigned short> &);

	//!  Initializing Object Fields.
	/*!
		Calculating average, variance, standard-deviation,minimun value and maximum value.
		\param vec a C++ STL vector of unsigned integers. 
	*/
	void init(vector <int> &);

	//!  Chebyshev's Inequality.
	/*!
		This Functioin get a double number as an argument and returns a probability score according to the average and variance.
		\param x as a double argument. 
		\return A probability score between 0 to 1.
		\sa https://en.wikipedia.org/wiki/Chebyshev%27s_inequality
	*/
	double chebyshev(double);

	//!  Updating fields.
	/*!
		This function updates all the relevant fields (average, variance ,standard-deviation ...) when one value is added to the vector. For Example:
		\n
		\n size=4 , avg=3.5 , var=1.5 , std=1.118033989 , min=2 , max=5
		\n New Number=10.
		\n
		\n Then:
		\n size=5 avg=4.8 , var=7.76 , std=2.785677655 , min=2 , max=10.
		\n 
		\n
		\param new_num as a double argument.
		\return void
	*/
	void update(double);

	//! Updating fields.
	/*!
		This function updates all the relevant fields (average, variance ,standard-deviation ...) when multiple idenical values are added to the vector(no reiteration is needed). For Example:
		\n
		\n size=4 avg=3.5 , var=1.5 , std=1.118033989 , min=2 , max=5.
		\n New Numbers=[10,10,10]
		\n
		\n Then:
		\n size=7 avg=6.285714 , var=11.06122 , std=3.32584 , min=2 , max=10
		\n
		\n
		\param new_num as a double argument.
		\param length as an unsigned integer argument.
		\return void
	*/
	void update(double,unsigned int);

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

	//! Printing the class variables.
	/*!
		Printing to ElasticSearch the object fields.
		\param none
		\return void
	*/
	void print_elastic(string&);
};

#endif
