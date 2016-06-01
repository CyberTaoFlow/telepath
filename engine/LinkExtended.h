#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;

class LinkExtended{
public:

	friend class boost::serialization::access;
	std::string from_page_comp; // Unique key for from_Page.
	std::vector <unsigned short> diffLanding; 
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & from_page_comp;  
		ar & diffLanding;     
		ar & emission;  
	}
	unsigned int emission; 

	LinkExtended();
	LinkExtended(string &,unsigned short);
	void clean();
	void print();
};
