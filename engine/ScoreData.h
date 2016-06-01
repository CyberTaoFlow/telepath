#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>

#include "Numeric.h"

class ScoresData{

// This class build vectors for request scores. 
public:
	friend class boost::serialization::access;
	std::map <double,unsigned int> flow;
	std::map <double,unsigned int> landing;
	std::map <double,unsigned int> geo;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & flow;
		ar & landing;
		ar & geo;
	}

	ScoresData();
	void insert(double,double,double);
	void print();
	void clean();
};

class ScoresNumeric{

// This class make Numeric objects from ScoresData.
// Save the Average,Std and Variance.
// Clear/Delete the unsorted vector.  
public:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & flow;  
		ar & landing;        
		ar & geo;  
	}
	Numeric flow;
	Numeric landing;
	Numeric geo;
	
	ScoresNumeric();	
	void insert(ScoresData &);
	void print();
	void print_syslog();
};

class ScoreAtt{
// This class build vectors for attribute scores. 
public:

	friend class boost::serialization::access;
	std::map <double,unsigned int> attribute;
	std::map <double,unsigned int> att_size;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & attribute;
		ar & att_size;
	}

	ScoreAtt();
	void insert(double,double);
	void print();
	void clean();

};

class ScoreNumericAtt{

// This class make Numeric objects from ScoreAtt.
// Save the Average,Std and Variance.
// Clear/Delete the unsorted vector. 
public:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & attribute;  
		ar & att_size;     
	}
	Numeric attribute;
	Numeric att_size;
	
	ScoreNumericAtt();
	void insert(ScoreAtt &);
	void print();
	void print_syslog();
	void print_elastic(string);
};




