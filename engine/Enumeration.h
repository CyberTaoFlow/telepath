
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>

using namespace std;

class Enumeration{
public:

	// Using the boost serialization-library for serializing and deserializing.
	friend class boost::serialization::access;
	std::map <int ,unsigned int> seen;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & seen;
		ar & last;
		ar & sumY;
		ar & sumXY;

		ar & size;
	}
	long long size;
	long long offset;
	long long last;
	long long sumY;
	long long sumXY;
	
	Enumeration();	
	void clean();
	bool getCorrelation();
	void insertToList(int);
	void insertIntoEnum(int);
	int getProb(int);
	void print();
	void print_syslog();

};
