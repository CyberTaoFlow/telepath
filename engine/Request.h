
using namespace std;

class RequestValToInsert{
public:
	long long RID;
	unsigned int Sid;
	string user_name;
	string host_name;
	string page_name;
	string canonical_url;
	unsigned int sequence;
	double ts;
	string ip_orig;
	double ip_score;
	string ip_resp;
	double query_score;
	double flow_score;
	double geo_normal;
	double landing_normal;
	double avg_normal;
	unsigned short status_code;
	string country;
	string city;
	string SID_status;
	string protocol; // http or https.
	string method; // Get,Post,etc .
	string shard;
	string title;
	Coordinate c;
	double presence;
	short op_mode;
	unsigned int decimalIP;

	RequestValToInsert(){}
	void init(string & user_name,long long RID,unsigned int & Sid,unsigned int & index,string & page_name,string & host_name,double & ts,string & ip_orig,string & ip_resp,double flow_score,double query_score,double geo_normal,double landing_normal,unsigned short & status_code,string & country,string & city,char SID_status,string & protocol,string & method,Coordinate & c,unsigned int decimalIP,string & shard,string & title,double presence,string & canonical_url,short op_mode){ 
	boost::unordered_map <unsigned int,boost::unordered_map <string,UserReputation> >::iterator itReputationUsers;
	boost::unordered_map <string,UserReputation>::iterator itUsers;


	this->user_name=user_name;
	this->RID=RID;
	this->Sid=Sid;
	this->sequence=index;
	this->query_score=query_score;
	this->page_name=page_name;
	this->canonical_url=canonical_url;
	this->host_name=host_name;
	this->ts=ts;
	this->ip_orig=ip_orig;
	this->ip_resp=ip_resp;
	this->flow_score=flow_score;
	this->geo_normal=geo_normal;
	this->landing_normal=landing_normal;
	this->status_code=status_code;
	this->country=country;
	this->city=city;
	this->SID_status=SID_status;
	this->protocol=protocol;
	this->method=method;
	this->shard=shard;
	this->title=title;
	this->c=c;
	this->presence=presence;
	this->op_mode=op_mode;
	this->decimalIP=decimalIP;

	this->avg_normal = ( query_score + flow_score + landing_normal ) / 3;
	if(this->avg_normal < 0){
		this->avg_normal = 0;
	}
	else if(geo_normal > 0.6){
		this->avg_normal = sqrt(this->avg_normal);
	}

	}

};
