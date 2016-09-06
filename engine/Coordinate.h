#ifndef _Coordinate_h
#define _Coordinate_h


#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class Coordinate{
public:

	friend class boost::serialization::access;
	std::string key;
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Tree object to/from the disk.
	*/
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
	/*! Will be used in the Db-scan algorithm. */
	char visited;

	//! Noise
	/*! The Coordinate does not belong to any Cluster. */
	char noise;

	//! Counter
	/*! The counter of Coordinate with the same Longitude/Latitude.*/
	unsigned int counter;

	//!  Defult Constrator.
	/*!
		Initializing the object fields with zero values.
	*/
	Coordinate();

	//!  Constrator.
	/*!
		Initializing the object fields with the argument values.
		\param x a float argument.
		\param y a float argument.
	*/
	Coordinate(float,float);

	//!  Setting the Coordinate Field Values.
	/*!
                This function sets the object fields with the argument values.
		\param x a float argument.
		\param y a float argument.
	*/
	void setCoordinate(float,float);

	//! Printing the class variables.
	/*!
		Printing to syslog the object fields.
		\param none
		\return void
	*/
	void print_syslog();

	//! Printing the class variables.
	/*!
		Printing to syslog the object fields.
		\param none
		\return void
	*/
	void show_syslog();
};

#endif
