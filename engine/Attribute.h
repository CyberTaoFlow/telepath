
using namespace std;

class size_and_flag{
public:

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & size;
		ar & flag;
		ar & tokenOrNot;
	}
	unsigned short size;
	char flag;
	char tokenOrNot;

	size_and_flag(){
		tokenOrNot='n';
	}

	void print(){
		cout << "s:"<<this->size << "  f:"<<this->flag;
	}
};

class myAtt{
public:

	friend class boost::serialization::access;	
	std::string name;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & page_id;
		ar & name;
		ar & kind;
		ar & type;

		ar & tree;
		ar & numeric;
		ar & enumeration;
		ar & sizeMarkov;
		ar & lock;

		ar & num_of_nulls;
		ar & num_of_numeric;
		ar & num_of_values;
		ar & hash_values;
		ar & sumOfScores;
		ar & countOfScores;
		ar & weight;
	}

	std::vector <unsigned short> sizeMarkovVec;
	std::vector < vector <unsigned int> > values;
	std::vector < int> hash_values;
	std::vector <long long> RIDs;
	std::vector <short> exp_length;
	std::vector <int> exp_score;
	std::vector < size_and_flag > val_s_f;

	char kind; //H,G,P
	char type; // numeric,text,enum,Url,Free,undetermine.
	unsigned char lock; 
	unsigned int page_id; 
	Numeric numeric;
	Enumeration enumeration;
	Tree tree;
	Numeric sizeMarkov;
	char ifUpdate;
	unsigned int num_of_values;
	unsigned int num_of_numeric;
	unsigned int num_of_nulls;
	char newInHybrid;
	unsigned short newCounterInHybrid;
	long long sumOfScores;
	unsigned int countOfScores;
	double weight;

	myAtt(){
		this->lock=0;
		this->num_of_values=0;
		this->num_of_numeric=0;
		this->num_of_nulls=0;
		this->newCounterInHybrid=0;
		this->type='u';
		this->sumOfScores=0;
		this->countOfScores=0;
		this->weight=0;
	}

	void printCapacity(){
		cout << "Type:"<< this->type << endl;
		cout << "sizeMarkovVec: " << this->sizeMarkovVec.size() <<"  Cap:"<< this->sizeMarkovVec.capacity()  << endl;
		cout << "values: " << this->values.size()<<"  Cap:"<<this->values.capacity()  << endl;
		if(this->values.size()>0){
			cout << "\tvalue: " << this->values[0].size()<<"  Cap:"<<this->values[0].capacity()  << endl;
		}
		cout << "RIDs: " << this->RIDs.size() <<"  Cap:"<< this->RIDs.capacity()  << endl;
		cout << "exp_length: " << this->exp_length.size() <<"  Cap:"<< this->exp_length.capacity()  << endl;
		cout << "p: " << this->exp_score.size() <<"  Cap:"<< this->exp_score.capacity()  << endl;
		cout << "val_s_f: " << this->val_s_f.size() <<"  Cap:"<< this->val_s_f.capacity()  << endl;

		cout<< "Tree" << endl;
		cout<< "\tNodes:" << this->tree.mNode.size() << "   Edges:"<< this->tree.mEdge.size() << "  words:"<< this->tree.word << endl;
		
		cout<< "Enumeration" << endl;
		cout<< "\tList:" << this->enumeration.seen.size() <<  "   sizes:"<< this->enumeration.size << endl;

		//cout<< "URL" << endl;
		//cout<< "\tList:" << this->url.mDomain.size() << endl;		
	}
	
	void print(long long unsigned aid){

		cout << "\tpage_id: " << this->page_id << endl;
		cout << "\tname: " << this->name << endl;
		cout << "\tkind: " << this->kind << endl;
		cout << "\ttype: " << this->type << endl;
		cout << "\tValues: "  << endl;
		for (unsigned int i=0 ; i < this->values.size() ; i++ ){
			cout << "\t\t" << i <<")";
			for (unsigned int j=0 ; j < this->values[i].size() ; j++ ){
				if(j == this->values[i].size()-1 )
					cout << this->values[i][j] << endl;
				else							
					cout << this->values[i][j] << ",";
			}
		}
		cout << "\tRIDs: "  << endl;
		for (unsigned int j=0 ; j < this->RIDs.size() ; j++ )	
			cout << "\t\t" << j <<")"<< this->RIDs[j] << endl;
		cout << "\texp_length: "  << endl;
		for (unsigned int j=0 ; j < this->exp_length.size() ; j++ )	
			cout << "\t\t" << j <<")"<< this->exp_length[j] << endl;

		cout << "\exp_score:  " << this->exp_score.size() << endl;

		for (unsigned int k=0 ; k < this->exp_score.size() ; k++ ){
			cout << "\t\t" << k <<")"<< this->exp_score[k] << endl;
		}

		cout << "\ts_f:  " << this->val_s_f.size() << endl;

		for (unsigned int k=0 ; k < this->val_s_f.size() ; k++ ){
			cout << "\t\t" << k <<")  ";
			this->val_s_f[k].print();
			cout<<endl;
		}

		if(this->type == 'n'){
			cout << "\tNumeric: "  << endl;
			this->numeric.print();		

		}
		//if(this->type == 'U'){
		//	cout << "\tUrl: "  << endl;
		//	this->url.print();		
		//}

		if(this->type == 't'){
			cout << "\tTree: "  << endl;
			map <string,Node>::iterator itNode;
			cout << "\t\tNodes\n";
			for(itNode = this->tree.mNode.begin(); itNode!=this->tree.mNode.end();itNode++){
				cout << "\t\t\t";
				itNode->second.print();		
			}
			map <string,Edge>::iterator itEdge;
			cout << "\t\tEdges\n";
			for(itEdge = this->tree.mEdge.begin(); itEdge!=this->tree.mEdge.end();itEdge++){
				cout << "\t\t\t";
				itEdge->second.print();	
			}
			cout << "\t\tSample\n";
			for(unsigned int i=0;i<this->tree.sampleNode.size(); i++)
				cout << "\t\t\t" << i <<")"<< this->tree.sampleNode[i] << endl;
	
			cout << "\tWords:" <<this->tree.word <<endl;

			cout << "\tsizeMarkovVec\n";
			for(unsigned int j=0;j<this->sizeMarkovVec.size(); j++)
				cout << "\t\t" << j <<")"<< this->sizeMarkovVec[j] << endl;

			cout << "\tsizeMarkov\n";
				this->sizeMarkov.print();

		}

		if(this->type == 'e'){
			cout << "\tEnum: "  << endl;
			map < int,unsigned int>::iterator itSeen;
			cout << "\t\tSeen\n";
			for(itSeen = this->enumeration.seen.begin(); itSeen!=this->enumeration.seen.end();itSeen++){
				cout << itSeen->first <<" <-> "<<itSeen->second<<endl;
			}
			cout << "\t\tSize:" << this->enumeration.size << endl;
		}

	}
};

