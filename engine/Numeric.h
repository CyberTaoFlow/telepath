#ifndef _Numeric_h
#define _Numeric_h

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <vector>

#include <math.h>


using namespace std;

class Numeric{
public:

	// Using the boost serialization-library for serializing and deserializing.
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mean;
		ar & stddev;
		ar & variance;
		ar & size;
	}
	double mean; // Average.
	double variance; // Measures how far a set of numbers is spread out.
	double stddev; // Measures the amount of variation or dispersion of a set of data values. The square root of the variance.
	unsigned int size; // Number of samples.
	double min; // The minimum sample.
	double max; // The maximum sample.
	bool valid; // The first sample.

	Numeric();

	void init(vector <double> &);
	void init(vector <unsigned short> &);
	void init(vector <int> &);

	double chebyshev(double);
	void update(double);
	void update(double,unsigned int);
	void print();
	void print_syslog();
	void print_elastic(string&);
};

#endif
