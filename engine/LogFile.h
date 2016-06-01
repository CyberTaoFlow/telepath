using namespace std;


class LogFile{
public:

	string filename;
	string domain;
	

	LogFile(){}
	LogFile(string & filename,string & domain){
		this->filename=filename;
		this->domain=domain;
	}

	//void print(){
	//	cout << "Filename:"<< this->filename <<  "  Domain:"<<  domain<<endl;
	//}

};
