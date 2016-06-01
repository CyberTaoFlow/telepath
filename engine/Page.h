#ifndef _Page_h
#define _Page_h

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/unordered_map.hpp>

using namespace std;



class Page{
public:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & index;     
		ar & parsedHost;  
		ar & parsedUser;        
		ar & parsedHostPro;  
		ar & parsedUserPro;    
		ar & ID;     
		ar & RID;    
		ar & ts;  
		ar & status_code;     
		ar & tainted;  
		ar & emission;        
		ar & link_sample;  
		ar & user_ip;        
		ar & compare;   
		ar & user;

		ar & shard;
		ar & title;
		ar & page_name;
		ar & host_name;
		ar & domain_id;

	}

	unsigned int index;			// For example : 0 - is the first request in the session.
	char parsedHost;			// For example : 0 or 1.
	char parsedUser; 			// For example : 0 or 1.
	char parsedHostPro; 			// For example : 0 or 1.
	char parsedUserPro; 			// For example : 0 or 1.
	unsigned int ID; 			// Page id.
	long long RID; 				// Request id.
	double ts;				// Time Stamp
	unsigned short status_code;		// For example : 200.
	char tainted;    			// For example : 0 or 1.
	unsigned int emission;
	unsigned int link_sample;
	string user_ip;				// For example : "81.218.185.126".
	string resp_ip;				// For example : "8.8.8.8".
	string compare;				// For example : "0+55".
	string user;				// For example : "admin".
	string protocol;			// For example : "HTTP" or "HTTPS".
	string method; 				// For example : "GET" or "POST" etc ...
	double presence;
	unsigned int full_rule_hash;

	string shard;				// ElasticSearch index name. For example : 20150101 .
	string title;				// For example : "hybrid security main page"
	string page_name;			// For example : "/index.php" .
	string canonical_url;
	string host_name;			// For example : "hybridsec.com" .
	string domain_id;			// For example : 534 .
	unsigned int subdomain_id;		// For example : 432 .


	Page();
	Page(boost::unordered_map<char,string> &,unsigned int,unsigned int,long long,string &,string &);
	void print();

};

#endif

