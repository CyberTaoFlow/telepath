
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class NodeExtended{
public:

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & index;     
		ar & tainted;  
		ar & emission;
		ar & link_sample;        
	}

	//string name; // URL
	unsigned int index; 
	char tainted;    // status_code dependency.
	unsigned int emission;
	unsigned int link_sample;

	NodeExtended();
	NodeExtended(unsigned int);
	void print();
};

