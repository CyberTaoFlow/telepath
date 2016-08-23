#include "Tree.h"

using namespace std;

unsigned int lenHelper(unsigned int x) { 
	if(x>=100000) {
		if(x>=10000000) {
			if(x>=1000000000) return 10;
			if(x>=100000000) return 9;
			return 8;
		}
		if(x>=1000000){return 7;}
	        return 6;
	}else{
		if(x>=1000) {
			if(x>=10000){return 5;}
			return 4;
		}else{
			if(x>=100){return 3;}
			if(x>=10){return 2;}
			return 1;
		}
	}
}

// Fast implementation for integer to string.
unsigned int itoa(unsigned int val,char *buf){

	unsigned int s=lenHelper(val);
 	char *ptr = buf+s;

	*ptr = '\0';

	do
		*--ptr = '0' + (val % 10);
   	while(val/=10);
	return s;
}

Tree::Tree(){this->word=0;}

void Tree::clean(){
	this->mNode.clear(); 
	this->mEdge.clear(); 
	vector <unsigned long> ().swap(this->sampleNode); 
}

void Tree::tokenize(vector <unsigned int> & attr_value){
	bool last_letter = false;
	unsigned long i,length;
	string tmpComp;
	char buffer[55];
	
	unsigned int size = attr_value.size();

	//Saving Memory Consumption.
	if(size > MAX_MARKOV_VALUE){
		size = MAX_MARKOV_VALUE;
	}

	//Pushing Nodes to the map
	for(i=0; i < size ;i++){						
		//Lowercase
		if(attr_value[i]>64 && attr_value[i]<91){
			attr_value[i] += 32;
		}

		length = itoa (i,buffer);
		buffer[length++] ='+';
		length += itoa(attr_value[i],buffer+length); 

		tmpComp = buffer; // index+unicode_value of the character. For example : "0+53"
		if(i==size-1){ // The last letter of the string.
			last_letter = true;
		}
		this->updateNode(last_letter,tmpComp);	
		
		//Pushing or Updating samples into the vector
		if(i >= sampleNode.size() ){
			sampleNode.push_back(1);
		}else{
			sampleNode[i]++;
		}

		if(last_letter==false){ // If it isn't the last letter of the string than build another Edge.
			buffer[length++] ='|';		
			length += itoa(attr_value[i+1],buffer+length); // node_key_a|node_key_b . For example : "0+53|71"

			Edge e(tmpComp);
			this->updateEdge(e,buffer);
		}

	}	
	this->word++;
}

void Tree::updateNode(bool last_letter,string & comp){
	map <string,Node>::iterator itNode;

	if(this->word == 0){ 
		Node node;
		if(last_letter==true){
			node.edge_sample=0;
		}
		this->mNode.insert( pair<string,Node>(comp,node) ); // Inserting a new Node.
	}else{
		itNode=this->mNode.find(comp);
		if(itNode!=this->mNode.end()){//Node was found
			itNode->second.emission+=1;		
			if(last_letter==false){
				itNode->second.edge_sample+=1;
			}
		}else{			//Node wasn't found
			Node node;
			if(last_letter==true){
				node.edge_sample=0;
			}
			this->mNode.insert( pair<string,Node>(comp,node) ); // Inserting a new Node.
		}	
	}
}

void Tree::updateEdge(Edge & edge,string comp){
	map <string,Edge>::iterator itEdge;

	if(this->word == 0){
		this->mEdge.insert( pair<string,Edge>(comp,edge) ); // Inserting a new Edge.
	}else{
		itEdge=this->mEdge.find(comp);
		if(itEdge!=this->mEdge.end()){//Edge was found
			itEdge->second.emission+=1;

		}else{			//Edge wasn't found
			this->mEdge.insert( pair<string,Edge>(comp,edge) ); // Inserting a new Edge.
		}	
	}
}

int Tree::calculate(vector <unsigned int> & value,double avg_size){
	map <string,Node>::iterator itNode;
	map <string,Edge>::iterator itEdge;

	int exp_, exp_score=0;
	char buffer[25],buffer2[50];
	double L,D,X,num;
	unsigned int sizeVal = value.size(),length;
	if(sizeVal > MAX_MARKOV_VALUE){
		sizeVal = MAX_MARKOV_VALUE;
	}

	if(this->sampleNode.size() == 0 ){ // There are no samples at all.
		return MIN_PROB;
	}

	for(unsigned int i=0 ;i < sizeVal ; i++){
		//Lowercase
		if(value[i]>64 && value[i]<91){
			value[i] += 32;
		}

		length = itoa (i,buffer);
		buffer[length++] ='+';
		length += itoa(value[i],buffer+length); 

		itNode=this->mNode.find(buffer);
		if(itNode!=this->mNode.end()){//Node was found
			num = ((itNode->second.emission)/(double)(this->sampleNode[i]));
		}else{			  //Node wasn't found
			if( this->sampleNode.size() > i ){		// There are samples to this index.			
				num = ( 1 / ((double)(this->sampleNode[i]+1)) );
			}else{						// There are no samples to this index.
				L = (double)(i+1);
				D = L - avg_size ;
				X = L / avg_size ;
				num =  L / (L + (D*X) );
			}
		}
		frexp(num , &(exp_)  );	
		if (exp_ > 0){exp_=0;}
		exp_score+=exp_;

		if(i == sizeVal-1)
			continue;

		memcpy(buffer2,buffer,length);
		buffer2[length++]='|';
		length += itoa (value[i+1],buffer2+length);

		itEdge=this->mEdge.find(buffer2);
		if(itEdge!=this->mEdge.end()){//Edge was found
			num =(  (itEdge->second.emission)  /  ((double)( this->mNode[itEdge->second.from_node_comp].edge_sample  ))   );
			frexp( num, &(exp_) );
			if (exp_ > 0){exp_=0;}

			exp_score+=exp_;
		}else{			  //Edge wasn't found
			exp_score-=4;
		}
	}

	return exp_score;
}

void Tree::print(){
	std::map <string,Node>::iterator itNode;  // Nodes
	std::map <string,Edge>::iterator itEdge;  // Egdes

	for(itNode = this->mNode.begin() ; itNode!=this->mNode.end() ; itNode++){
		itNode->second.print();
	}

	for(itEdge=this->mEdge.begin();itEdge!=this->mEdge.end();itEdge++){
		itEdge->second.print();
	}
	
	cout << "sampleNode"<< endl;
	for(unsigned int i=0;i<this->sampleNode.size();i++){
		cout << "  " << this->sampleNode[i] <<endl;
	}
}

void Tree::printCapacity(){
	cout << "-------------------------"<< endl;
	cout << "mNode.size():" << mNode.size() << endl;
	cout << "mEdge.size():" << mEdge.size() << endl;
	cout << "sampleNode.size():" << sampleNode.capacity() << endl;
	cout << "word:" << word << endl;
	cout << "-------------------------"<< endl;
}

