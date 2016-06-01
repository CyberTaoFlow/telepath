using namespace std;


// Represent samll number by using mantissa.

class Probability{
public:
	
	friend class boost::serialization::access;	
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & prob;
		ar & exp;
	}
	double prob;
	int exp; 



	Probability(){}
	Probability(double prob,int exp){
		this->prob=prob;
		this->exp=exp;
		this->betterRepresentation();
	}

	void setProbability(double prob,int exp){
		this->prob = prob;
		this->exp = exp;
		this->betterRepresentation();
	}
	
	double normalized(){
		return (this->prob * pow(2,this->exp) );
	}

	unsigned int operator==(Probability p){
		if(this->prob == p.prob && this->exp == p.exp)
			return 1;
		return 0;
	}

	void print(){
		cout<<"("<<this->prob<<","<<this->exp<<")";
	}

	void betterRepresentation(){
		while(this->prob < 0.01 || this->exp>0 ){ // better Representation for probabilities.
			if(this->prob==0)
				break;
			this->exp--;
			this->prob*=2;

		}
	}
	
	/*Probability mulProb(Probability p){ // multiplication of two Probabilities.
		Probability result(this->prob*p.prob,this->exp+p.exp);
		return result;
	}	

	void geometricMean(Probability p){ // Geometric mean of two Probabilities.
		double probability,exponent,result;

		this->prob*=p.prob;
		this->exp+=p.exp;
		
		probability = pow(this->prob,0.5);
		exponent = (double)this->exp * 0.5 ;		
		result = probability * pow(2,exponent);

		this->prob = frexp( result, &(this->exp) );	
		this->betterRepresentation();
	}*/	

};

