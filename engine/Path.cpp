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

void Path::tokenize(Session & s,short byHostOrUser ){ // Build and update the Path per session.
	short flag;	
	unsigned long i,size = s.vRequest.size();
	
	flag=0;
	//Pushing Pages to the map

	for(i=0 ; i < size ;i++){	

		if(  byHostOrUser==0 && s.vRequest[i].parsedHost == 2  ){ // Application
			this->mCompressedPage[s.vRequest[i].compare ].link_sample+=1;
			s.vRequest[i].parsedHost =1 ;
		}
		else if(  byHostOrUser==1 && s.vRequest[i].parsedUser == 2  ){ // User
			this->mCompressedPage[s.vRequest[i].compare ].link_sample+=1;
			s.vRequest[i].parsedUser =1 ;
		}

		if( (byHostOrUser==0 && s.vRequest[i].parsedHost==1) || (byHostOrUser==1 && s.vRequest[i].parsedUser==1)  ){ // Go to the next page.
			continue;
		}

		checkMapPage(s.vRequest[i].tainted,s.vRequest[i].status_code); // Tainted Page or not.

		if(i==size-1){ // Last Page in the session or not.
			flag=1;
		}
		
		this->updatePage(s.vRequest[i],flag,byHostOrUser);	
		//Pushing or Updating samples to/into the vector.

		if( s.vRequest[i].tainted==0 ){
			if(this->sampleP.size() > s.vRequest[i].index ){
				this->sampleP[s.vRequest[i].index]++;
			}else{
				this->sampleP.resize(s.vRequest[i].index+100,0);
				this->sampleP[s.vRequest[i].index]++;
			}
			if(s.vRequest[i].index==0){ //Insert IP coordinates only in the first request. 
				pthread_mutex_lock(&mutexgeoip);
				this->location.insert( getCoordinate(s.vRequest[i].user_ip) );
				pthread_mutex_unlock(&mutexgeoip);
			}
		}

		if(s.vRequest[i].index!=0){
			Link l(s.vRequest[i].compare_link,s.vRequest[i].diff_speed);
			this->updateLink(l,byHostOrUser);
		}
	}

	for(i=0 ; i < size-1 ;i++){
		if(byHostOrUser == 0)
			s.vRequest[i].parsedHost=1;
		if(byHostOrUser == 1)
			s.vRequest[i].parsedUser=1;
	}

	// the current last page in a session parsed with 2. 
	if(byHostOrUser == 0)
		s.vRequest[i].parsedHost=2; 
	if(byHostOrUser == 1)
		s.vRequest[i].parsedUser=2;

	this->numOfSessions++;

}

//Insert new Pages to the Path or update their emission and link_sample.
void Path::updatePage(Page & p,short flag,short & ifUser){
	map <string,CompressedPage>::iterator itCompressedPage;

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
		CompressedPage ne(p.index);
		if(flag){
			ne.link_sample=0;
			p.link_sample=0;
		}
		this->mCompressedPage.insert( pair<string,CompressedPage>(p.compare,ne) );	
	}
	else{
		itCompressedPage=mCompressedPage.find(p.compare);
		if(itCompressedPage!=this->mCompressedPage.end()){//Page was found
			itCompressedPage->second.emission+=1;	
			if(!flag){
				itCompressedPage->second.link_sample+=1;
			}
			
		}else{			//Page wasn't found
			CompressedPage ne(p.index);
			if(flag){
				ne.link_sample=0;
				p.link_sample=0;
			}
			this->mCompressedPage.insert( pair<string,CompressedPage>(p.compare,ne) );	
		}	
	}
	if(ifUser!=0){
		return;
	}
}

// Insert new Links to the Path or update their emission and diffLanding.
void Path::updateLink(Link & l,short & ifUser){ 
	map <string,CompressedLink>::iterator itCompressedLink;

	map <unsigned int,unsigned int>::iterator itLinkSum;

	if(this->numOfSessions == 0){
		CompressedLink le(l.compare,l.landing);
		this->mCompressedLink.insert( pair<string,CompressedLink>(l.compare,le) );
	}else{
		itCompressedLink=mCompressedLink.find(l.compare);
		if(itCompressedLink!=mCompressedLink.end()){//Link was found
			itCompressedLink->second.emission+=1;
			itCompressedLink->second.diffLanding.push_back(l.landing);
		}else{			//Link wasn't found
			CompressedLink le(l.compare,l.landing);
			this->mCompressedLink.insert( pair<string,CompressedLink>(l.compare,le) );
		}
	}
	if(ifUser!=0){
		return;
	}
}

void Path::print(){

	cout<<"---mCompressedPage---"<<endl;
	map <string,CompressedPage>::iterator itCompressedPage;
	for (itCompressedPage = this->mCompressedPage.begin() ; itCompressedPage != this->mCompressedPage.end() ; itCompressedPage++){
		itCompressedPage->second.print();
	}

	cout<<"---mCompressedLink---"<<endl;
	map <string,CompressedLink>::iterator itCompressedLink;
	for (itCompressedLink = this->mCompressedLink.begin() ; itCompressedLink != this->mCompressedLink.end() ; itCompressedLink++){
		itCompressedLink->second.print();
	}

	cout<<"---mCompressedLink---"<<endl;
	for(unsigned int i=0;i<sampleP.size();i++){
		cout << sampleP[i] <<" ";
	}
	cout<<"domain:" <<this->domain<< endl;
	cout<<"behavior:" <<this->behavior<< endl;
	cout<<"activate:" <<this->activate<< endl;
	cout<<"numOfSessions:" <<this->numOfSessions<< endl;
	cout<<"group_id:" <<this->group_id<< endl;
	cout<<"---location---"<<endl;

	cout<<"---End---"<<endl;
}

void Path::printCapacity(){
	cout<<"domain:" <<this->domain<< endl;
	cout<<"activate:" <<this->activate<< endl;
	cout<<"numOfSessions:" <<this->numOfSessions<< endl;
	cout<<"mCompressedPage.size():" <<this->mCompressedPage.size()<< endl;
	cout<<"mCompressedLink.size():" <<this->mCompressedLink.size()<< endl;
	cout<<"sampleP.size():" <<this->sampleP.capacity()<< endl<<endl;
}


