using namespace std;

// For exmple ,URL  = www.google.com/index.php .
//	     Domain = google.com
//	   Extension= /index.php

class Extension{
public:

	friend class boost::serialization::access;
	std::map <vector <unsigned int>,unsigned int> mExtension;  // key = page name, value=page emission.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mExtension;  
		ar & samples;                              
	}
	unsigned int samples;					// samples for specific domain. 	

	Extension(){
		this->samples=1;
	}

};

class Domain{
public:
	friend class boost::serialization::access;
	std::map <vector <unsigned int>,Extension> mDomain; // key = domain name, value=domain emission & all the pages inside it.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		ar & mDomain;  
		ar & samples;  
	}
	unsigned int samples;				 // the total samples of all the domains.

	Domain(){
		this->samples=0;
	}

	void insert(vector <unsigned int> & domain,vector <unsigned int> & extension){
		map <vector <unsigned int>,Extension>::iterator itDomain;	
		map <vector <unsigned int>,unsigned int>::iterator itExtension;	
		this->samples++; // Domains samples increment.

		itDomain = this->mDomain.find(domain);
		if(itDomain != this->mDomain.end() ){ // Domain was found.	
		
			itDomain->second.samples++; // Domain emission increment.
			itExtension = itDomain->second.mExtension.find(extension);
			if (itExtension != itDomain->second.mExtension.end() ){ // Extension was found.

				itExtension->second++; // Extension emission increment.

			}else{							// Extension wasn't found.

				itDomain->second.mExtension.insert(pair<vector <unsigned int>,unsigned int>(extension,1));
			}

		}else{				      // Domain wasn't found.
			Extension e;
			this->mDomain.insert(pair<vector <unsigned int>,Extension> (domain,e) );

			this->mDomain[domain].mExtension.insert(pair<vector <unsigned int>,unsigned int>(extension,1));
		}
	}

	int getProb(vector <unsigned int> & vec){
		map <vector <unsigned int>,Extension>::iterator itDomain;	
		map <vector <unsigned int>,unsigned int>::iterator itExtension;	

		vector <unsigned int> vec1; // Domain.
		vector <unsigned int> vec2; // Extension.
		unsigned int delimiter = 0; 
		double domaine_score,ext_score,score;
		int exp;

		//---------split URL to Domain & Extension---------
		for(unsigned short i=0; i<vec.size() ;i++){
			if(vec[i]==47){ // sign '/'.
				delimiter=i;
				break;
			}
		}
		vec1.insert(vec1.begin(),vec.begin(),vec.begin()+delimiter );
		vec2.insert(vec2.begin(),vec.begin()+delimiter,vec.end() );

		//if(vec2.size()==1) // Just '/' char.
		//	vec.pop_back();
		//-------------------------------------------------

		itDomain = this->mDomain.find(vec1);
		if(itDomain != this->mDomain.end() ){ // Domain was found.
			domaine_score = (double)( (double)itDomain->second.samples / (double)this->samples) ;
			
			itExtension = itDomain->second.mExtension.find(vec2);
			if (itExtension != itDomain->second.mExtension.end() ){ // Extension was found.
				ext_score = (double)( (double)itExtension->second / (double)itDomain->second.samples) ;
			}else{							// Extension wasn't found.
				ext_score = (double)( 1/((double)itDomain->second.samples+1) ) ;		
			}

			score = domaine_score * ext_score;	
			frexp(score,&exp);
			if (exp>0){exp=0;}

			vector <unsigned int>().swap(vec1);
			vector <unsigned int>().swap(vec2);
			return exp;

		}else{				      // Domain wasn't found.
			domaine_score = (double)( 1/((double)this->samples+1) ) ;
			frexp(domaine_score,&exp);
			if(exp>0){exp=0;}

			vector <unsigned int>().swap(vec1);
			vector <unsigned int>().swap(vec2);
			return exp; // The lowest score.
		}
	}

	int getProb(vector <unsigned int> & vec1,vector <unsigned int> & vec2){
		map <vector <unsigned int>,Extension>::iterator itDomain;	
		map <vector <unsigned int>,unsigned int>::iterator itExtension;	
		double domaine_score,ext_score,score;
		int exp;

		itDomain = this->mDomain.find(vec1);
		if(itDomain != this->mDomain.end() ){ // Domain was found.
			domaine_score = (double)( (double)itDomain->second.samples / (double)this->samples) ;
			
			itExtension = itDomain->second.mExtension.find(vec2);
			if (itExtension != itDomain->second.mExtension.end() ){ // Extension was found.
				ext_score = (double)( (double)itExtension->second / (double)itDomain->second.samples) ;
			}else{							// Extension wasn't found.
				ext_score = (double)( 1/((double)itDomain->second.samples+1) ) ;		
			}

			score = domaine_score * ext_score;	
			frexp(score,&exp);
			if (exp>0){exp=0;}
			return exp;
								
		}else{				      // Domain wasn't found.
			domaine_score = (double)( 1/((double)this->samples+1) ) ;
			frexp(domaine_score,&exp);
			if (exp>0){exp=0;}
			return exp; // The lowest score.
		}
	}


	void print(){
		map <vector <unsigned int>,Extension>::iterator itDomain;	
		map <vector <unsigned int>,unsigned int>::iterator itExtension;		
		cout <<"Sample:"<< this->samples <<endl;
		for(itDomain=mDomain.begin();itDomain!=mDomain.end() ; itDomain++){
			cout <<"DOMAIN:"<< itDomain->first[0] << " - " << itDomain->second.samples <<endl;
			for(itExtension=itDomain->second.mExtension.begin();itExtension!=itDomain->second.mExtension.end();itExtension++){
				cout <<"\t"<<itExtension->first[1]<<" - " <<itExtension->second;
			}
			cout <<endl;
		}

	}

	void clean(){
		map <vector <unsigned int>,Extension>().swap(this->mDomain); 
	}

};


