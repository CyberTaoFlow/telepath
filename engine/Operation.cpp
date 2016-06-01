#include "Operation.h"

using namespace std;


unsigned short lenHelper(unsigned short x) { 
	// this is either a fun exercise in optimization 
	// or it's extremely premature optimization.

	if(x>=1000) {
		if(x>=10000){return 5;}
		return 4;
	}else{
		if(x>=100){return 3;}
		if(x>=10){return 2;}
		return 1;
	}
}

unsigned short itoa(unsigned short val,char *buf){

	unsigned short s=lenHelper(val);
 	char *ptr = buf+s;

	*ptr = '\0';

	do
		*--ptr = '0' + (val % 10);
   	while(val/=10);
	return s;
}

void itoa3(unsigned short val,char *buf){

	unsigned short s=lenHelper(val);
 	char *ptr = buf+s;

	*ptr = '\0';

	do
		*--ptr = '0' + (val % 10);
   	while(val/=10);
}


Coordinate2::Coordinate2(){
	this->noise='0';
	this->visited='0';
}

Coordinate2::Coordinate2(map<unsigned short,float> & mOpVec){
	char buffer[200];	
	unsigned short length=0;
	map<unsigned short,float>::iterator it;

	this->visited='n';
	this->val=1;
	this->noise='0';
	this->mOpVec=mOpVec;

	for(it = this->mOpVec.begin(); it != this->mOpVec.end() ; it++ ){
		length += itoa (it->first,buffer+length);
		buffer[length++] ='+';
	}
	buffer[length--] ='\0';
	buffer[length--] ='\0';

	this->key = buffer;
}

void Coordinate2::print(){
	map<unsigned short,float>::iterator itOpRIDs;
	cout << "key:"<<key<<endl;
	for(itOpRIDs = this->mOpVec.begin() ; itOpRIDs != this->mOpVec.end() ; itOpRIDs++ ){
		cout <<itOpRIDs->first<<"->"<< itOpRIDs->second <<";";
	}
	cout<<endl;
}




double Pythagoras(Coordinate2 & c1,Coordinate2 & c2){
	double dist=0;
	map<unsigned short,float>::iterator it;
	map<unsigned short,float>::iterator it2;

	for(it = c1.mOpVec.begin(); it != c1.mOpVec.end() ; it++ ){
		it2 = c2.mOpVec.find(it->first);
		if(it2 == c2.mOpVec.end()){
			dist += (double)(it->second * it->second);
		}
	}

	for(it2 = c2.mOpVec.begin(); it2 != c2.mOpVec.end() ; it2++ ){
		it = c1.mOpVec.find(it2->first);
		if(it == c1.mOpVec.end()){
			dist += (double)(it2->second * it2->second);
		}
	}

	return (sqrt(dist));
}

double Pythagoras(map<unsigned short,float> & mOpVec,Coordinate2 & c2){
	double dist=0;
	map<unsigned short,float>::iterator it;
	map<unsigned short,float>::iterator it2;

	for(it = mOpVec.begin(); it != mOpVec.end() ; it++ ){
		it2 = c2.mOpVec.find(it->first);
		if(it2 == c2.mOpVec.end()){
			dist += (double)(it->second * it->second);
		}
	}

	for(it2 = c2.mOpVec.begin(); it2 != c2.mOpVec.end() ; it2++ ){
		it = mOpVec.find(it2->first);
		if(it == mOpVec.end()){
			dist += (double)(it2->second * it2->second);
		}
	}

	return (sqrt(dist));
}


Cluster2::Cluster2(){
	numOfPoints=0;
}

void Cluster2::insert(Coordinate2 & c){
	map<unsigned short,float>::iterator it;
	map<unsigned short,double>::iterator it2;
	for(it = c.mOpVec.begin();it != c.mOpVec.end();it++){
		it2 = this->total.find(it->first);
		if( it2 != this->total.end() ){
			it2->second += (double)it->second;
		}else{
			this->total.insert(pair<unsigned short,double>(it->first,(double)(it->second) ));
		}
	}

	numOfPoints += c.val;
}

unsigned int Cluster2::getCentroid(){
	if(numOfPoints==0){
		return 0;
	}

	map<unsigned short,double>::iterator it;
	for(it = this->total.begin();it != this->total.end();it++){
		this->centroid.mOpVec[it->first] = (float)(it->second/numOfPoints);
	}

	return 1;
}

void Cluster2::print(){
	cout<<"---centroid---"<<endl;
	this->centroid.print();
}


Db_Scan2::Db_Scan2(){}
void Db_Scan2::insert(Coordinate2 & c){
	map <string,Coordinate2>::iterator it;		

	it = this->points.find(c.key);
	if (it!=this->points.end()){ // coorinate was find.
		it->second.val++;
	}else{
		this->points.insert(pair<string,Coordinate2>(c.key,c) );
	}
}

void Db_Scan2::buildCentroids(){
	vector<Cluster2>::iterator itClusters;
	itClusters = this->clusters.begin() ;

	while( itClusters != this->clusters.end() ){
		if( itClusters->getCentroid() == 0 ){
			this->clusters.erase(itClusters);
			continue;
		}

		itClusters++;
	}
}

unsigned short Db_Scan2::returnClosestCluster(map<unsigned short,float> & mOpVec){
	double min,tmp;
	unsigned short location=0;
	min = Pythagoras(mOpVec,clusters[0].centroid);

	for(unsigned short i=1;i<this->clusters.size();i++){
		tmp = Pythagoras(mOpVec,clusters[i].centroid);
		if(tmp < min){
			min = tmp;
			location = i;
		}
	}

	return location;
}

void Db_Scan2::init(double eps,unsigned short MinPts){
	unsigned short C = 0;
	map <string,Coordinate2>::iterator it;
	unsigned int size;

	for(it=this->points.begin(); it!=this->points.end() ;it++){
		if(it->second.visited=='y'){
			continue;
		}

		it->second.visited='y';
		vector <Coordinate2> N;		
		size=0;
		N = regionQuery(it->second,eps,size);

		if(size < MinPts){
			it->second.noise='y';
		}
		else{
			Cluster2 new_cluster;
			this->clusters.push_back(new_cluster);
			expandCluster(it->second,N,C,eps,MinPts);
			C++;
		}
	}
	points.clear();
}

void Db_Scan2::expandCluster(Coordinate2 & p,vector <Coordinate2> & N,unsigned short C,double & eps,unsigned short MinPts){
	p.noise='n';
	this->clusters[C].insert(p);
	unsigned int size;

	for(unsigned int i=0; i<N.size() ;i++){
		map <string,Coordinate2>::iterator it;
		it = this->points.find(N[i].key);
		if(it->second.visited=='n'){
			it->second.visited='y';
			vector <Coordinate2> Nnew;
		
			size=0;
			Nnew = regionQuery(N[i],eps,size);
			if(size >= MinPts ){
				N.insert(N.end(),Nnew.begin(),Nnew.end());
			}
		}

		if(it->second.noise != 'n'){	
			it->second.noise='n';				
			this->clusters[C].insert(N[i]);				
		}			
			
	}
}

vector <Coordinate2> Db_Scan2::regionQuery(Coordinate2 & p,double & eps,unsigned int & size){
	vector <Coordinate2> N;
	map <string,Coordinate2>::iterator it;
	double distance;

	for(it=this->points.begin(); it!=this->points.end() ;it++){

		distance = Pythagoras(p,it->second);

		if(eps>=distance){
			size+=it->second.val;				

			if(it->second.noise!='n'){
				N.push_back(it->second);
			}
		}		
	}
	return N;
}

void Db_Scan2::print(){
	map <string,Coordinate2>::iterator it;
	for(unsigned int i=0;i<this->clusters.size();i++){
		clusters[i].print();
	}
}

Operation::Operation(){
	this->vecSize = 0;
}

void Operation::print(){
	map <unsigned long long,map< int,unsigned short> >::iterator itVecShape;
	map <int,unsigned short >::iterator itValues;

	cout << "**dbscan**" << endl;;
	this->dbscan.print();

	cout << "@@@@@@@@@@@@@@" << endl;;
}

OpRID::OpRID(unsigned int page_id){
	this->page_id = page_id;
}

