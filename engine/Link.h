#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;

class Link{
public:

	friend class boost::serialization::access;
	std::string from_page_comp;	// Unique key for from_Page.
	std::string compare;		// Unique key for Link.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & from_page_comp;  
		ar & landing;     
		ar & compare;  
	}
	unsigned short landing; 

	Link();
	Link(string &,string &,unsigned short);
	void clean();
	void print();
};

