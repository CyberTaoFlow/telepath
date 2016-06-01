using namespace std;


class Host{
public:

	friend class boost::serialization::access;
  	std::map <string,Path> mPathPerUser;
	std::string name; // host name.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mPathPerUser;  
		ar & name;     
	}

	Host(){}
	Host(string name){
		this->name=name;
	}

};
