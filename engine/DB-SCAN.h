#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include "Cluster.h"

using namespace std;

class Db_Scan{
public:
	friend class boost::serialization::access;
	std::vector <Cluster> clusters; 
	std::map <string,Coordinate> points;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & clusters;    
		ar & points;     
	}

	Db_Scan();
	void insert(Coordinate);
	double getProb(Coordinate &);
	void init(double &,unsigned short);
	void expandCluster(Coordinate &,vector <Coordinate> &,unsigned short,double &,unsigned short);
	vector <Coordinate> regionQuery(Coordinate &,double &,unsigned int &);
	void print();
	void print_syslog();
};



