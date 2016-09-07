

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

	//!
	/*! A C++ STL map of all the Coordinate objects in the application */
	std::map <string,Coordinate> points;
	template<class Archive>

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Tree object to/from the disk.
	*/
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & points;    
		ar & totalX;     
		ar & totalY;     
		ar & numOfPoints;  
		ar & centroid;        
		ar & Distant; 
	}

	//!
	/*! An aggregation number of all the latitudes.*/
	double totalX;

	//!
	/*! An aggregation number of all the longitudes.*/
	double totalY;

	//!
        /*! The nubmer of points in the Cluster.*/
	unsigned int numOfPoints;

	//!
	/*! The Cluster Centroid.*/
	Coordinate centroid;

	//!
	/*! The most distant Coordinate from the Cluster Centroid.*/
	double Distant;

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	Cluster();

	//!  Inserting Coordinates into map/dictionary.
	/*!
		This function inserts/updates a given Coordinate to the 'points' STL map. If the given Coordinate already exists in the map the function will increament its 'counter' by one. If the given Coordinate does not exist in the map the function will create a new Coordinate in the 'points' STL map(The 'counter' value is one). \n

		\param c as a Coordinate object.
	*/
	void insert(Coordinate &);

	//!  Getting the Centroid of the Cluster.
	/*!
		\return 0 or 1 as a boolean argument.
		\n      0 - The Coordinate is from an unknown location.
		\n      1 - The Coordinate is from a known location.
	*/
	unsigned int getCentroid();

        //!   Getting the most distant Coordinate from the Cluster Centroid.
        /*!
        */
	void getMostDistantPoint();

	//! Printing the class variables.
	/*!
		Printing to syslog the object fields.
		\param none
		\return void
	*/
	void print_syslog();


};
