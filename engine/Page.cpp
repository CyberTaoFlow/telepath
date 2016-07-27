#include "Tree.h"
#include "Page.h"

using namespace std;

// Defult Constrator.
Page::Page(){}
Page::Page(boost::unordered_map<char,string> & mParams,unsigned int index,unsigned int current_PageID,long long current_RID,string & current_IP,string & UserID_){
	char buffer[35];
	unsigned int length;

	this->emission=1;
	this->link_sample=1;
	this->parsedHost=0;
	this->parsedUser=0;
	this->parsedHostPro=0;
	this->parsedUserPro=0;
	this->presence=0;
	this->tainted=0;

	this->index=index;
	this->user=UserID_;
	this->ID= current_PageID;
	this->RID= current_RID;
	this->user_ip=current_IP;
	this->domain_id= mParams['g'/*AppID*/];
	this->subdomain_id= (unsigned int)atoi(mParams['y'/*SubDomainID*/].c_str() );
	this->ts = (double)atof(mParams['b'/*TimeStamp*/].c_str() ) ;
	this->status_code= (unsigned short)atoi(mParams['d'/*StatusCode*/].c_str() ) ;
	this->resp_ip=mParams['u'/*RespIP*/];
	this->protocol=mParams['i'/*Protocol*/];
	this->method=mParams['h'/*Method*/];
	this->shard=mParams['S'/*Shard*/];
	this->title=mParams['m'/*Title*/];
	this->page_name=mParams['c'/*Page*/];
	this->sha256_sid=mParams['E'/*SHA256_SID*/];
	if(mParams['C'/*Canonical URL*/].size()==0){
		this->canonical_url=mParams['c'/*Page*/];
	}else{
		this->canonical_url=mParams['C'/*Canonical URL*/];
	}
	this->host_name=mParams['f'/*App*/];

	length = itoa (this->index,buffer);
	buffer[length++] = '+';
	length += itoa(this->ID,buffer+length);

	this->compare = buffer;

}

// Printing.
void Page::print(){
	cout <<"\tID:"<<ID << "  name:"<<"  index:" <<index <<"  emission:"<< emission<<"  link_sample:"<< link_sample<<"   user_ip:"<<user_ip;
	cout<<endl;
	cout <<"\tparsedHost:"<<parsedHost<< "  parsedUser:"<<parsedUser <<"  RID:"<<RID<<"  ts:"<<ts<<"   status_code:"<<status_code<<"  tainted:"<<tainted<<endl<<endl;
}


