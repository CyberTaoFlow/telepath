class Reputation{
public:

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & update;
		ar & counter;
		ar & sum;
	}

	char update;
	unsigned int counter;
	double sum;

	Reputation(){}
	Reputation(double sum){
		this->update='y';
		this->counter=1;
		this->sum=sum;
	}

	Reputation(double score,unsigned int counter){
		this->update='n';
		this->sum=score*counter;
		this->counter=counter;
	}

};

class UserReputation: public Reputation{
public:

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & update;
		ar & counter;
		ar & sum;
		ar & userid;
	}

	unsigned int userid;

	UserReputation(){}
	UserReputation(double sum){
		this->update='y';
		this->sum=sum;
		this->counter=1;
	}

	UserReputation(double score,unsigned int counter,unsigned int userid){
		this->update='n';
		this->sum=score*counter;
		this->counter=counter;
		this->userid=userid;
	}



};
