
using namespace std;

//	     Country = Israel
//	     City= Tel-Aviv

class City{
public:
	friend class boost::serialization::access;
	std::map <string,unsigned int> mCity;  // key = city name, value=city emission.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mCity;  
		ar & samples;     
	}
	unsigned int samples;			// samples for specific country.

	City(){
		this->samples=1;
	}

};

class Country{
public:
	friend class boost::serialization::access;
	std::map <string,City> mCountry; 	  // key = country name, value=country emission & all the cities inside it.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mCountry;  
		ar & samples;     
	}
	unsigned int samples;			  // the total samples of all the countries.

	Country(){
		this->samples=0;
	}

	void insert(string & country,string & city){
		if (country.size()==0) // empty country.		
			country+='\0';
		if (city.size()==0){    // empty city.	
			city+='\0';
			return;
		}	

		map <string,City>::iterator itCountry;	
		map <string,unsigned int>::iterator itCity;	
		this->samples++; // Countries samples increment.

		itCountry = this->mCountry.find(country);
		if(itCountry != this->mCountry.end() ){ // Country was found.	
		
			itCountry->second.samples++; // Country emission increment.
			itCity = itCountry->second.mCity.find(city);
			if (itCity != itCountry->second.mCity.end() ){ // City was found.

				itCity->second++; // City emission increment.

			}else{				// City wasn't found.

				itCountry->second.mCity.insert(pair<string,unsigned int>(city,1));
			}

		}else{				         // Country wasn't found.
			City c;
			this->mCountry.insert(pair<string,City> (country,c) );

			this->mCountry[country].mCity.insert(pair<string,unsigned int>(city,1));
		}
	}


	double getProb(string & str1,string & str2){
		map <string,City>::iterator itCountry;	
		map <string,unsigned int>::iterator itCity;	
		double country_score,city_score,score;
		int exp;

		if (str1.size()==0) // empty country.		
			str1+='\0';
		if (str2.size()==0 || str2[0]=='\0'){ // empty city.	
			return MIN_PROB;
		}
	
		itCountry = this->mCountry.find(str1);
		if(itCountry != this->mCountry.end() ){ // Country was found.
			country_score = (double)( (double)itCountry->second.samples / (double)this->samples) ;
			
			itCity = itCountry->second.mCity.find(str2);
			if (itCity != itCountry->second.mCity.end() ){ // City was found.
				city_score = (double)( (double)itCity->second / (double)itCountry->second.samples) ;

				score = country_score * pow(city_score,0.5);

				frexp(score,&exp);	
				if(exp>0){exp=0;}

				return (double)(exp);
								
			}else{							// City wasn't found.
				return MIN_PROB;   // The lowest score.
			}

		}else{				      // Country wasn't found.
			return MIN_PROB;	      // The lowest score.
		}
	}


	void print(){
		map <string,City>::iterator itCountry;	
		map <string,unsigned int>::iterator itCity;		
		cout <<"Sample:"<< this->samples <<endl;
		for(itCountry=mCountry.begin();itCountry!=mCountry.end() ; itCountry++){
			cout << itCountry->first << " - " << itCountry->second.samples <<endl;
			for(itCity=itCountry->second.mCity.begin();itCity!=itCountry->second.mCity.end();itCity++){
				cout <<"\t"<<itCity->first<<" - " <<itCity->second;
			}
			cout <<endl;
		}

	}

};


