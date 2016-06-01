
#include "Cluster.h"
#include <syslog.h>

using namespace std;

// Distance between two Coordinates in Euclidean space.
//
// For Example: 
//         
//	Input: c1 = (31.5,34.5)   c2 = (28.5,38.5)
//	Return: dist = 5.
double Pythagoras(Coordinate & c1,Coordinate & c2){
	double side1,side2,dist;

	side1=(double)(c1.x-c2.x);
	side2=(double)(c1.y-c2.y);
	side1= pow(side1,2);
	side2= pow(side2,2);

	dist=sqrt(side1+side2);
	return dist;
}

// Initializing Cluster.
Cluster::Cluster(){
	totalX=0;
	totalY=0;
	numOfPoints=0;
}

// Inserting new Coordinate into Cluster.
void Cluster::insert(Coordinate & c){
	this->points[c.key] = c;
	this->totalX += c.x*c.counter;
	this->totalY += c.y*c.counter;
	numOfPoints += c.counter;
}

// Saving the centroid of the Cluster.
//
// Returning a zero value if the Cluster is empty.
// Otherwise returning a one value.
unsigned int Cluster::getCentroid(){
	if(numOfPoints==0){
		this->centroid.x = 0;
		this->centroid.y = 0;
		return 0;
	}

	this->centroid.x = totalX/numOfPoints;
	this->centroid.y = totalY/numOfPoints;

	return 1;
}

// Returning the most distant Coordinate from the Cluster centroid.
void Cluster::getMostDistantPoint(){
	double dist=0.5,dist2;
	map <string,Coordinate>::iterator itPoint;
	for(itPoint = this->points.begin() ; itPoint != this->points.end() ; itPoint++){
		dist2 = Pythagoras(this->centroid,itPoint->second);
		if( dist2 > dist ){
			dist = dist2;
		}
	}
	this->Distant = dist;
}

// Printing
void Cluster::print(unsigned int i){
	cout << "Cluster "<<i<<endl;
	cout << "x "<<totalX<<endl;
	cout << "y "<<totalY<<endl;
	cout << "numOfPoints "<<numOfPoints<<endl;
	cout << "mean:";
	centroid.show();
	cout << "Distant:"<<this->Distant<<endl;
}

void Cluster::print_syslog(){
	syslog(LOG_NOTICE,"x %f",totalX);
	syslog(LOG_NOTICE,"y %f",totalY);
	syslog(LOG_NOTICE,"numOfPoints %u",numOfPoints);
	centroid.show_syslog();

	syslog(LOG_NOTICE,"Distant: %f",this->Distant);

}
