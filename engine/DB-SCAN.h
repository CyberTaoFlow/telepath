#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include "Cluster.h"

using namespace std;

//! Db_Scan class
/*!
	Density-based spatial clustering of applications with noise (DBSCAN).
	This is a density-based clustering algorithm: \n
	\n with a given set of points in some space, it groups together points that are closely packed together (points with many nearby neighbors), marking as outliers points that lie alone in low-density regions (whose nearest neighbors are too far away).\n
	\n
	\n The following code is an implementation of a given pseudocode. \n
	\n https://en.wikipedia.org/wiki/DBSCAN 
*/
class Db_Scan{
public:

	friend class boost::serialization::access;

	//!
	/*! A C++ STL vector of all the Cluster objects in the application */
	std::vector <Cluster> clusters; 

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
		ar & clusters;    
		ar & points;     
	}

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	Db_Scan();

        //!  Inserting Coordinates into map/dictionary.
        /*!
                This function inserts/updates a given Coordinate to the 'points' STL map. If the given Coordinate already exists in the map the function will increament its 'counter' by one. If the given Coordinate does not exist in the map the function will create a new Coordinate in the 'points' STL map(The 'counter' value is one). \n

		\param c as a Coordinate object.
        */
	void insert(Coordinate);

        //!  Calculating Probability.
        /*!
                This function gets a Coordinate as an argument and returns a probability score accroding to the distance from the closest Cluster.

		\param c as a Coordinate object.
        */
	double getProb(Coordinate &);

	//!  Initializing Db_Scan with its Core Values.
	/*!
		This function initializes the class/algorithm with its two core values. The first value is epsilon which means the minimal distance between two Coordinates in the same Cluster. The second value is the minimal number of Coordinates that are needed to create a new Cluster.
		\param eps as a double argument.
		\param MinPts as a unsigned short argument.
		\return A probability score between 0 to 1.
	*/
	void init(double &,unsigned short);

	//!  Expanding the Cluster when needed.
	/*!
		This function expands the cluster or/and decides about noisy Coordinates.
		\param p as a Coordinate object.
		\param N as a C++ STL vector of Coordinate objects.
		\param C as a unsigned short argument.
		\param eps as a double argument.
		\param MinPts as a unsigned short argument.
		\sa https://en.wikipedia.org/wiki/DBSCAN
	*/
	void expandCluster(Coordinate &,vector <Coordinate> &,unsigned short,double &,unsigned short);

	//!  Checking the Region of a Coordinate.
	/*!
		\param p as a Coordinate object.
		\param eps as a double argument.
		\param size as a unsigned integer argument.
		\return a C++ STL vector of Coordinate objects.
		\sa https://en.wikipedia.org/wiki/DBSCAN
		
	*/
	vector <Coordinate> regionQuery(Coordinate &,double &,unsigned int &);

	//! Printing the class variables.
	/*!
		Printing to syslog the object fields.
		\param none
		\return void
	*/
	void print_syslog();
};



