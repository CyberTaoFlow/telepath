#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;

unsigned short lenHelper(unsigned short);
unsigned short itoa(unsigned short,char *);
void itoa3(unsigned short,char *);

class Coordinate2{
public:

	friend class boost::serialization::access;
	std::string key;
	std::map<unsigned short,float> mOpVec;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mOpVec;
		ar & key;    
		ar & visited;  
		ar & noise;        
		ar & val; 
	}

	char visited;
	char noise;
	unsigned int val;

	Coordinate2();
	Coordinate2(map<unsigned short,float> &);
	void print();
};

double Pythagoras(Coordinate2 &,Coordinate2 &);
double Pythagoras(map<unsigned short,float> &,Coordinate2 &);

class Cluster2{
public:

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & numOfPoints;  
		ar & centroid;        
	}

	std::map<unsigned short,double> total;
	unsigned int numOfPoints;
	Coordinate2 centroid;

	Cluster2();
	void insert(Coordinate2 &);
	unsigned int getCentroid();
	void print();
};


class Db_Scan2{
public:

	friend class boost::serialization::access;
	std::vector <Cluster2> clusters; 
	std::map <string,Coordinate2> points;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & clusters;    
		ar & points;     
	}

	Db_Scan2();
	void insert(Coordinate2 &);
	void buildCentroids();
	unsigned short returnClosestCluster(map<unsigned short,float> &);
	void init(double,unsigned short);
	void expandCluster(Coordinate2 &,vector <Coordinate2> &,unsigned short,double &,unsigned short);
	vector <Coordinate2> regionQuery(Coordinate2 &,double &,unsigned int &);
	void print();
};


class Operation{
public:


	friend class boost::serialization::access;
	std::map <unsigned long long,map< int,unsigned short> > vecShape;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & vecShape;
		ar & vecSize;
		ar & dbscan;     
	}
	unsigned short vecSize;
	Db_Scan2 dbscan;

	Operation();
	void print();

};

class OpRID{
public:

	unsigned int page_id;
	map<unsigned short,float> opts;

	OpRID(unsigned int page_id);
};

