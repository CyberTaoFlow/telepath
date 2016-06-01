#include "Path.h"
#include "Session.h"
#include "Operation.h"
#include "Link.h"
#include <syslog.h>

using namespace std;

extern map <unsigned int, Operation > mOperation;
extern map <long long, OpRID > mOpRIDs;
extern pthread_mutex_t mutexgeoip;

unsigned int compareStatus(vector <unsigned int> nums , unsigned int status){ // Check if the status code is inside the vector.
	switch ( nums.size() ){
		case 1:
			if(status == nums[0])
				return 1;
			return 0;
		
		case 2:
			if(status >= nums[0] && status <= nums[1] )
				return 1;
			return 0;
		default:
			return 0;
	}
}

char checkMapPage(string str,unsigned short status){ // Get a comma separated string and a status code - check matching.
	string diff;
	vector <unsigned int> nums;
	unsigned int i,size=str.size();
	for(i=0;i<size;i++){
		if(str[i]>57 || str[i] < 48 || i==size-1){
			if(i==size-1){ // if it is the last char at the string.
				diff.push_back(str[i]);
				nums.push_back( (unsigned int)atoi( diff.c_str() ) );
				if( compareStatus(nums,status) ){
					vector <unsigned int>().swap(nums);
					return 1;
				}
			}
			nums.push_back( (unsigned int)atoi( diff.c_str() ) );
			if(str[i]==45){	 // sign (-)		

			}
			if(str[i]==44){	 // sign (,)		
				if( compareStatus(nums,status) ){
					vector <unsigned int>().swap(nums);
					return 1;
				}
				nums.clear();
			}
			diff.clear();
			continue;	
		}
		diff.push_back(str[i]);
	}
	vector <unsigned int>().swap(nums);
	return 0;
}

void checkMapPage(char & tainted,unsigned short & status){
	if(status >= 400){
		tainted=1;
	}
}


Path::Path(){}
Path::Path(string & domain){
	this->domain = domain;
	this->behavior=0;
	this->activate=0;
	this->numOfSessions=0;
	this->sampleP.assign(100,0); 
}

Path::Path(string & domain,unsigned int behavior){
	this->domain = domain;
	this->behavior = behavior;
	this->activate=0;
	this->numOfSessions=0;
	this->sampleP.assign(100,0); 
}

Path::Path(string & domain,unsigned short group_id){
	this->domain = domain;
	this->behavior = 0;
	this->activate=0;
	this->group_id = group_id;
	this->numOfSessions=0;
	this->sampleP.assign(100,0); 
}

void Path::tokenize(Session & s,short byHostOrUserOrGroup ){ // Build and update the Path per session.
	double speed;
	short flag;	
	unsigned long i,size = s.vRequest.size();
	
	flag=0;
	//Pushing Pages to the map

	for(i=0 ; i < size ;i++){	

		if(  byHostOrUserOrGroup==0 && s.vRequest[i].parsedHost == 2  ){ // Application
			this->mNodeExtended[s.vRequest[i].compare ].link_sample+=1;
			s.vRequest[i].parsedHost =1 ;
		}
		else if(  byHostOrUserOrGroup==1 && s.vRequest[i].parsedUser == 2  ){ // User
			this->mNodeExtended[s.vRequest[i].compare ].link_sample+=1;
			s.vRequest[i].parsedUser =1 ;
		}

		if( (byHostOrUserOrGroup==0 && s.vRequest[i].parsedHost==1) || (byHostOrUserOrGroup==1 && s.vRequest[i].parsedUser==1)  ){ // Go to the next page.
			continue;
		}

		checkMapPage(s.vRequest[i].tainted,s.vRequest[i].status_code); // Tainted Page or not.

		if(i==size-1){ // Last Page in the session or not.
			flag=1;
		}
		
		this->updatePage(s.vRequest[i],flag,byHostOrUserOrGroup);	
		//Pushing or Updating samples to/into the vector.

		if( s.vRequest[i].tainted==0 ){
			if(this->sampleP.size() > s.vRequest[i].index ){
				this->sampleP[s.vRequest[i].index]++;
			}else{
				this->sampleP.resize(s.vRequest[i].index+100,0);
				this->sampleP[s.vRequest[i].index]++;
			}
			if(s.vRequest[i].index<=350){
				pthread_mutex_lock(&mutexgeoip);
				this->location.insert( getCoordinate(s.vRequest[i].user_ip) );
				pthread_mutex_unlock(&mutexgeoip);
			}
		}

	}	

	for(i=0; i < size-1 ;i++){
		if( ( byHostOrUserOrGroup==0 && s.vRequest[i+1].parsedHost == 0 ) ||  ( byHostOrUserOrGroup==1 && s.vRequest[i+1].parsedUser == 0 )  ){	
			if(s.vRequest[i+1].ts > s.vRequest[i].ts){
				speed = (unsigned short)(s.vRequest[i+1].ts - s.vRequest[i].ts);	
			}else{
				speed=0;
			}				
			Link l(s.vRequest[i+1].compare,s.vRequest[i].compare,speed);

			bool modeStatus=false;
			if(s.vRequest[i].status_code==200 && s.vRequest[i+1].status_code==200){
				modeStatus=true;
			}

			this->updateLink(l,byHostOrUserOrGroup,s.vRequest[i+1].ID,s.vRequest[i].ID,modeStatus);	
		}
	}
	for(i=0 ; i < size-1 ;i++){
		if(byHostOrUserOrGroup == 0)
			s.vRequest[i].parsedHost=1;
		if(byHostOrUserOrGroup == 1)
			s.vRequest[i].parsedUser=1;
	}

	// the current last page in a session parsed with 2. 
	if(byHostOrUserOrGroup == 0)
		s.vRequest[i].parsedHost=2; 
	if(byHostOrUserOrGroup == 1)
		s.vRequest[i].parsedUser=2;

	this->numOfSessions++;

}

//Insert new Pages to the Path or update their emission and link_sample.
void Path::updatePage(Page & p,short flag,short & ifUser){
	map <string,NodeExtended>::iterator itNodeExtended;

	map <unsigned int, Operation >::iterator itOperation = mOperation.find(p.ID);
	if( itOperation != mOperation.end() ){
		if(itOperation->second.dbscan.clusters.size() > 0){
			map <long long, OpRID >::iterator itOpRIDs = mOpRIDs.find(p.RID);
			if(itOpRIDs != mOpRIDs.end()){
			
				unsigned short loc = itOperation->second.dbscan.returnClosestCluster(itOpRIDs->second.opts);
				char buffer[8];
				buffer[0] =':';
				itoa3(loc,buffer+1);

				p.compare.append(buffer);
			}
		}
	}

	if(this->numOfSessions == 0){
		NodeExtended ne(p.index);
		if(flag){
			ne.link_sample=0;
			p.link_sample=0;
		}
		this->mNodeExtended.insert( pair<string,NodeExtended>(p.compare,ne) );	
	}
	else{
		itNodeExtended=mNodeExtended.find(p.compare);
		if(itNodeExtended!=this->mNodeExtended.end()){//Page was found
			itNodeExtended->second.emission+=1;	
			if(!flag){
				itNodeExtended->second.link_sample+=1;
			}
			
		}else{			//Page wasn't found
			NodeExtended ne(p.index);
			if(flag){
				ne.link_sample=0;
				p.link_sample=0;
			}
			this->mNodeExtended.insert( pair<string,NodeExtended>(p.compare,ne) );	
		}	
	}
	if(ifUser!=0){
		return;
	}
}

// Insert new Links to the Path or update their emission and diffLanding.
void Path::updateLink(Link & l,short & ifUser,unsigned int & to,unsigned int & from,bool modeStatus){ 
	map <string,LinkExtended>::iterator itLinkExtended;

	map <unsigned int,unsigned int>::iterator itLinkSum;

	if(this->numOfSessions == 0){
		LinkExtended le(l.from_page_comp,l.landing);
		this->mLinkExtended.insert( pair<string,LinkExtended>(l.compare,le) );
	}else{

		itLinkExtended=mLinkExtended.find(l.compare);
		if(itLinkExtended!=mLinkExtended.end()){//Link was found
			itLinkExtended->second.emission+=1;
			itLinkExtended->second.diffLanding.push_back(l.landing);
		}else{			//Link wasn't found
			LinkExtended le(l.from_page_comp,l.landing);
			this->mLinkExtended.insert( pair<string,LinkExtended>(l.compare,le) );
		}
	}
	if(ifUser!=0 || to==from){
		return;
	}
}

void Path::print(){

	cout<<"---mNodeExtended---"<<endl;
	map <string,NodeExtended>::iterator itNodeExtended;
	for (itNodeExtended = this->mNodeExtended.begin() ; itNodeExtended != this->mNodeExtended.end() ; itNodeExtended++){	
		itNodeExtended->second.print();
	}

	cout<<"---mLinkExtended---"<<endl;
	map <string,LinkExtended>::iterator itLinkExtended;
	for (itLinkExtended = this->mLinkExtended.begin() ; itLinkExtended != this->mLinkExtended.end() ; itLinkExtended++){
		itLinkExtended->second.print();
	}

	cout<<"---mLinkExtended---"<<endl;
	for(unsigned int i=0;i<sampleP.size();i++){
		cout << sampleP[i] <<" ";
	}
	cout<<"domain:" <<this->domain<< endl;
	cout<<"behavior:" <<this->behavior<< endl;
	cout<<"activate:" <<this->activate<< endl;
	cout<<"numOfSessions:" <<this->numOfSessions<< endl;
	cout<<"group_id:" <<this->group_id<< endl;
	cout<<"---location---"<<endl;
	location.print();

	cout<<"---End---"<<endl;
}

void Path::printCapacity(){
	cout<<"domain:" <<this->domain<< endl;
	cout<<"activate:" <<this->activate<< endl;
	cout<<"numOfSessions:" <<this->numOfSessions<< endl;
	cout<<"mNodeExtended.size():" <<this->mNodeExtended.size()<< endl;
	cout<<"mLink.size():" <<this->mLinkExtended.size()<< endl;
	cout<<"sampleP.size():" <<this->sampleP.capacity()<< endl<<endl;
}


