
#include "DB-SCAN.h"
#include <syslog.h>

using namespace std;

// Density-based spatial clustering of applications with noise (DBSCAN).
// It is a density-based clustering algorithm: 
// given a set of points in some space, it groups together points that are closely
// packed together (points with many nearby neighbors), marking as outliers points that 
// lie alone in low-density regions (whose nearest neighbors are too far away).
//
// The following code is an implementation of a given pseudocode.
// https://en.wikipedia.org/wiki/DBSCAN .

//Initializing.
Db_Scan::Db_Scan(){}

// Inserting new Coordinate into dictionary.
void Db_Scan::insert(Coordinate c){
	map <string,Coordinate>::iterator it;
	if(c.x==0 && c.y==0){
		return;
	}		

	it = this->points.find(c.key);
	if (it!=this->points.end()){ // coorinate was find.
		it->second.counter++;
	}else{
		this->points.insert(pair<string,Coordinate>(c.key,c) );
	}
}

double Db_Scan::getProb(Coordinate & c){
	double min_radius=0,score=1;

	for(unsigned short i=0; i < this->clusters.size() ;i++){
		double dist = Pythagoras(this->clusters[i].centroid,c);
		if(this->clusters[i].Distant >= dist){
			return 0;
		}else{
			double difference = dist - this->clusters[i].Distant;
			if(i==0){
				min_radius=difference;
				score = (min_radius/this->clusters[i].Distant);
			}else{
				if(difference < min_radius){
					min_radius=difference;
					score = (min_radius/this->clusters[i].Distant);
				}
			}	
		}
	}
	if(score > 1){
		score=1;
	}

	return score;
}

void Db_Scan::init(double & eps,unsigned short MinPts){
	unsigned short C = 0;
	map <string,Coordinate>::iterator it;
	unsigned int size;
	for(it=this->points.begin(); it!=this->points.end() ;it++){

		if(it->second.visited=='y'){
			continue;
		}
		it->second.visited='y';
		vector <Coordinate> N;		
		size=0;
		N = regionQuery(it->second,eps,size);

		if(size < MinPts){
			it->second.noise='y';
		}
		else{
			Cluster new_cluster;
			this->clusters.push_back(new_cluster);
			expandCluster(it->second,N,C,eps,MinPts);
			C++;
		}
	}
	this->points.clear();
}

void Db_Scan::expandCluster(Coordinate & p,vector <Coordinate> & N,unsigned short C,double & eps,unsigned short MinPts){

	p.noise='n';
	this->clusters[C].insert(p);
	unsigned int size;

	for(unsigned short i=0; i<N.size() ;i++){
		map <string,Coordinate>::iterator it;
		it = this->points.find(N[i].key);

		if(it->second.visited=='n'){
			it->second.visited='y';
			vector <Coordinate> Nnew;
		
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

vector <Coordinate> Db_Scan::regionQuery(Coordinate & p,double & eps,unsigned int & size){
	vector <Coordinate> N;
	map <string,Coordinate>::iterator it;
	double distance;

	for(it=this->points.begin(); it!=this->points.end() ;it++){

		distance = Pythagoras(p,it->second);

		if(eps>=distance){
			size+=it->second.counter;				

			if(it->second.noise!='n'){
				N.push_back(it->second);
			}
		}		
	}
	return N;
}

// Printing.
void Db_Scan::print(){
	unsigned int i;
	map <string,Coordinate>::iterator it;

	cout<<"---clusters---"<<endl;
	for(i=0;i<clusters.size();i++){
		clusters[i].print(i);
	}
	
	cout<<"---points---"<<endl;
	for( it=points.begin() ; it!=points.end() ; it++ ){
		cout << it->first << " = ";
		it->second.print();
		cout<<endl;
	}
	cout << "------------" << endl;
}

void Db_Scan::print_syslog(){
	unsigned int i;
	map <string,Coordinate>::iterator it;

	syslog(LOG_NOTICE,"---clusters---");
	for(i=0;i<clusters.size();i++){
		clusters[i].print_syslog();
	}

	syslog(LOG_NOTICE,"---points---");
	for( it=points.begin() ; it!=points.end() ; it++ ){
		syslog(LOG_NOTICE,"%s -> ",it->first.c_str());
		it->second.print_syslog();
		syslog(LOG_NOTICE,"------------");
	}

}

