#ifndef _Coordinate_h
#define _Coordinate_h


#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class Coordinate{
public:

	friend class boost::serialization::access;
	std::string key;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & key;    
		ar & x;     
		ar & y;     
		ar & visited;  
		ar & noise;        
		ar & counter; 
	}

	//!  Longitude, Latitude.
	/*!*/
	float x,y;

	//!  
	/*!*/
	char visited;

	//!
	/*!*/
	char noise;

	//!
	/*!*/
	unsigned int counter;

	Coordinate();
	Coordinate(float,float);
	void setCoordinate(float,float);
	void print();
	void show();
	void print_syslog();
	void show_syslog();
};

#endif
