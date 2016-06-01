

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include "Coordinate.h"

using namespace std;

double Pythagoras(Coordinate &,Coordinate &);

class Cluster{
public:

	friend class boost::serialization::access;
	std::map <string,Coordinate> points;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & points;    
		ar & totalX;     
		ar & totalY;     
		ar & numOfPoints;  
		ar & centroid;        
		ar & Distant; 
	}
	double totalX;
	double totalY;
	unsigned int numOfPoints;
	Coordinate centroid;
	double Distant;

	Cluster();
	void insert(Coordinate &);
	unsigned int getCentroid();
	void getMostDistantPoint();
	void print(unsigned int);
	void print_syslog();


};
