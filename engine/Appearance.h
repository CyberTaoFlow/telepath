
using namespace std;

class attAppearance{  // values of attributes score.
public:
	friend class boost::serialization::access;
	std::map <string,unsigned int> att_ids;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & att_ids;
		ar & counter;
	}
	int counter;

	attAppearance(){}
	attAppearance(string & ids){
		this->att_ids.insert(pair<string,unsigned int>(ids,1));
		this->counter=1;
	}

	void print(){
		map <string,unsigned int>::iterator it;
		cout <<"\tC:"<< this->counter <<endl;
		for(it = this->att_ids.begin(); it!=this->att_ids.end() ;it++){
			cout <<"\t"<<it->first<<"->"<< it->second <<endl;
		}
	}
};
