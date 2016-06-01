
#define BIG 2000

unsigned short geoConvergence;

using namespace std;

unsigned int findMinIndex(vector <double> dist,int k);
double findMinInd(double dist[],int k);
double findMinDist(double dist[],int k);
double findMaxDist(vector <double> dist,int k);
double Pythagoras(Coordinate c1,Coordinate c2);
Coordinate updateMean(Cluster c,Coordinate coor);

class K_means{
public:
	vector <Cluster> cluster;
	vector <Coordinate> vecCoordinate;
	vector <Coordinate> meanCoordinate;
	vector <double> distance;
	unsigned int K,cutoff;
	K_means(){}
	K_means(vector <Coordinate> vecCoordinate,unsigned int K,unsigned int cutoff){
		unsigned int x;
		//-----------------------------------------------------------------------------------------------------
		//1) Get a vector of coordinates (x,y) 
		//-----------------------------------------------------------------------------------------------------
		this->vecCoordinate = vecCoordinate;
		this->K=K;
		this->cutoff=cutoff;
		Cluster tmpCluster;
		for(unsigned int j=0 ; j<this->K ; j++)
			cluster.push_back(tmpCluster);// Initialize the Cluster vector with an empty Coordinate vectors.	
		
		//-----------------------------------------------------------------------------------------------------
		//------------------------------------------K-MEANS algorithm------------------------------------------
		//-----------------------------------------------------------------------------------------------------
		//(A) k initial "means" are randomly selected from the vector.			      -----------------
		//(B) k clusters are created by associating every observation with the nearest mean.  ----------------- 
		//(C) The centroid of each of the k clusters becomes the new means.		      -----------------
		//(D) Steps 2 and 3 are repeated until convergence has been reached.		      -----------------
		//-----------------------------------------------------------------------------------------------------

		//(A)
		for(unsigned int i=0 ; i < this->K ;i++){
			if(this->vecCoordinate.size()==1)
				x=0;
			else
				x = rand() % ( this->vecCoordinate.size() - 1)  ;

			this->meanCoordinate.push_back(this->vecCoordinate[x]);
		}
		//End of (A)


		//(B)(C)(D)
		stepsBCD(0);
	}
	
	void stepsBCD(int add){
		vector <double> shift;
		while(1){
			unsigned int x,flag,size = this->vecCoordinate.size();
			Coordinate tmp;
			double biggest_shift=0;
			//(B)
			for(unsigned int i=0 ; i < size ; ++i){
				for(unsigned int j=0; j<this->K+add ; j++){
					this->distance.push_back(Pythagoras(this->meanCoordinate[j],  this->vecCoordinate[i]  ));// Euclidean distance = Pythagoras.
				}
				x=findMinIndex(this->distance,this->K+add);
				this->distance.erase(this->distance.begin(),this->distance.end());

				cluster[x].vec.push_back( this->vecCoordinate[i] );
			}//end of (B)
		
			//(C)
			flag=0;
			for(unsigned int w=0; w<this->K+add; w++){		
				tmp=updateMean(cluster[w],this->meanCoordinate[w]);
				shift.push_back(Pythagoras(this->meanCoordinate[w],tmp));

				this->meanCoordinate[w]=tmp;
			}//end of (C)
			biggest_shift = findMaxDist(shift,this->K+add);
			shift.erase(shift.begin(),shift.end());

			if (biggest_shift < this->cutoff )
				flag=1;

			//(D)
			if (flag)
				break; //until convergence.
			for(unsigned int v=0 ; v<this->K+add ; v++)
				cluster[v].vec.erase(cluster[v].vec.begin(),cluster[v].vec.end());
		}//end of while and step D.
	}


	double checkNewCoordinate(Coordinate newC){
		double dist[this->K], closestDist;
		vector <double> distInCluster;
		unsigned int index;

		this->meanCoordinate.push_back(newC);
		this->vecCoordinate.push_back(newC);
		for(unsigned int i=0; i<this->K ; i++ ){
			if(this->cluster[i].vec.size()!=0)
				dist[i] = Pythagoras(newC,this->meanCoordinate[i]);// Find distance between the new Coordinate to the centroids.
			else
				dist[i] = BIG; // can't be the closest distance, therefore get big value.
		}
		closestDist = findMinDist(dist,this->K); // Find min distance.	

		index = findMinInd(dist,this->K);
		for(unsigned int j=0; j< this->cluster[index].vec.size()  ; j++ )
			distInCluster.push_back(Pythagoras( this->meanCoordinate[index] , this->cluster[index].vec[j] ) ); // Find distances inside the relevant Cluster.

		Numeric n(distInCluster);
		if(closestDist < n.findMin() && geoConvergence==0 ) 
			return 1;	
		if(closestDist > n.findMax() && geoConvergence==1 ) 
			return 1;				
		return  ( n.chebyshev(closestDist) );

	}

	void print(){
		cout <<"K"<<K<<"    cutoff" <<cutoff<< endl;
	}


};


//-----------------------------------------------------------------------------------
//---------------------------External Function---------------------------------------
//-----------------------------------------------------------------------------------


unsigned int findMinIndex(vector <double> dist,int k){  
	int i,index=0;
	double ans=dist[0];
	for (i=1;i<k ;i++){
		if(ans>dist[i]){
			ans=dist[i];
			index=i;
		}
	}
	return index;
}
double findMinInd(double dist[],int k){
	int i,index=0;
	double ans=dist[0];
	for (i=1; i<k ;i++){
		if(ans>dist[i]){
			ans=dist[i];
			index=i;
		}
	}
	return index;
}

double findMinDist(double dist[],int k){
	int i,index=0;
	double ans=dist[0];
	for (i=1; i<k ;i++){
		if(ans>dist[i]){
			ans=dist[i];
			index=i;
		}
	}
	return ans;
}
double findMaxDist(vector <double> dist,int k){
	int i,index=0;
	double ans=dist[0];
	for (i=1; i<k ;i++){
		if(ans<dist[i]){
			ans=dist[i];
			index=i;
		}
	}
	return ans;
}

double Pythagoras(Coordinate c1,Coordinate c2){
	double side1,side2,dist;

	side1=(double)(c1.x-c2.x);
	side2=(double)(c1.y-c2.y);
	side1= pow(side1,2);
	side2= pow(side2,2);

	dist=sqrt(side1+side2);
	return dist;
}

Coordinate updateMean(Cluster c,Coordinate coor){
	unsigned int i;
	double sumX=0,sumY=0;
	Coordinate result;
	if (c.vec.size() == 0)
		return coor;
	for(i=0;i<c.vec.size();++i){
		sumX+=c.vec[i].x;
		sumY+=c.vec[i].y;
	}
	result.x= (sumX/ (double)i);
	result.y= (sumY/ (double)i);
	return result;

}






