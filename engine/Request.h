
using namespace std;

//! RequestValToInsert class
/*!
	This class represents the fields that the Engine inserts to the Elasticsearch.
*/
class RequestValToInsert{
public:

	//!  Request ID. 
	/*!*/
	long long RID;

	//!  Session ID.
	/*!*/
	unsigned int Sid;

	//!  Encrypted Session ID from Suricata.
	/*!*/
	string sha256_sid;

	//!  Username.
	/*!*/
	string user_name;

	//!  Domains.
	/*!
		For Example: \n \n
		Url: "www.hybridsec.com/index.php?k1=v1&k2=v2"
		Host Name: "hybridsec.com"
	*/
	string host_name;

	//!  Page Name. 
	/*!
		For Example: \n \n
		Url: "www.hybridsec.com/index.php?k1=v1&k2=v2"
		Page Name: "/index.php"
	*/
	string page_name;

	//! Canonical Url.
	/*!
		\sa https://support.google.com/webmasters/answer/139066?hl=en
	*/
	string canonical_url;

	//!  Index.
	/*!
		The order of the user session flow.
	*/
	unsigned int sequence;

	//!  Time Stamp.
	/*!
		Epoch micro second format.
	*/
	double ts;

	//!  Source IP.
	/*!*/
	string ip_orig;

	//!  Aggragate IP score.
	/*!*/
	double ip_score;

	//!  Destination IP.
	/*!*/
	string ip_resp;

	//!  Query Score.
	/*!*/
	double query_score;

	//!  Flow Score.
	/*!*/
	double flow_score;

	//!  Geographical Score.
	/*!*/
	double geo_normal;

	//!  Speed Score.
	/*!*/
	double landing_normal;

	//!  Average Score.
	/*!*/
	double avg_normal;

	//!  Status Code.
	/*!*/
	unsigned short status_code;

	//!  Country Code.
	/*!*/
	string country;

	//!  City.
	/*!*/
	string city;

	//!  Session ID Status.
	/*!
		Possible Values: Valid, Invalid and Unknown.
	*/
	string SID_status;

	//!  Protocal
	/*!
		Possible Values: HTTP and HTTPS.
	*/
	string protocol;

	//!  Method
	/*!
		Possible Values: Head,Get and Post.
	*/
	string method; // Get,Post,etc .

	//!  Index name.
	/*!
		For Example: "20160101" \n 
		The full index name is "telepath-20160101".
	*/
	string shard;

	//!  Title Page.
	/*!*/
	string title;

	//!  Latitude and Longitude.
	/*!*/
	Coordinate c;

	//!  Presence Score.
	/*!*/
	double presence;

	//!  Operation Mode.
	/*!
		Possible Values: 1,2 and 3. \n
		1 - Learning.\n
		2 - Production. \n
		3 - Hybrid. \n
	*/
	short op_mode;

	//!  Decimal IP Representation.
	/*!*/
	unsigned int decimalIP;

	//!  User Scores.
	/*!*/
	string user_scores_string;

	//!  Default Constractor.
	/*!*/
	RequestValToInsert(){}

	//!  Initializing Object Fields.
	/*!
		Initializing all the object fields with the argument values.
		\param user_name as a C++ string argument.
		\param RID as a long long argument.
		\param Sid as a unsigned integer argument.
		\param sha256_sid as a C++ string argument.
		\param index as a unsigned integer argument.
		\param page_name as a C++ string argument.
		\param host_name as a C++ string argument.
		\param ts as a double argument.
		\param ip_orig as a C++ string argument.
		\param ip_resp as a C++ string argument.
		\param flow_score as a double argument.
		\param query_score as a double argument.
		\param geo_normal as a double argument.
		\param landing_normal as a double argument.
		\param status_code as a unsigned short argument.
		\param country as a C++ string argument.
		\param city as a C++ string argument.
		\param SID_status as a character argument.
		\param protocol as a C++ string argument.
		\param method as a C++ string argument.
		\param c as a Coordinate object.
		\param decimalIP as a unsigned integer argument.
		\param shard as a C++ string argument.
		\param title as a C++ string argument.
		\param presence as a double argument.
		\param canonical_url as a C++ string argument.
		\param op_mode as a short argument.
		\param user_scores_string as a C++ string argument.
	*/
	void init(string & user_name,long long RID,unsigned int & Sid,string & sha256_sid,unsigned int & index,string & page_name,string & host_name,double & ts,string & ip_orig,string & ip_resp,double flow_score,double query_score,double geo_normal,double landing_normal,unsigned short & status_code,string & country,string & city,char SID_status,string & protocol,string & method,Coordinate & c,unsigned int decimalIP,string & shard,string & title,double presence,string & canonical_url,short op_mode,string & user_scores_string){ 
	boost::unordered_map <unsigned int,boost::unordered_map <string,UserReputation> >::iterator itReputationUsers;
	boost::unordered_map <string,UserReputation>::iterator itUsers;


	this->user_name=user_name;
	this->RID=RID;
	this->Sid=Sid;
	this->sha256_sid=sha256_sid;
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
	this->user_scores_string=user_scores_string;

	this->avg_normal = ( query_score + flow_score + landing_normal ) / 3;
	if(this->avg_normal < 0){
		this->avg_normal = 0;
	}
	else if(geo_normal > 0.6){
		this->avg_normal = sqrt(this->avg_normal);
	}

	}

};
