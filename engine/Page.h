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

	//!  Boost Serialization Library.
	/*!
		Using the boost serialization library for serializing/deserializing the Page object to/from the disk.
	*/
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & index;     
		ar & parsedHost;
		ar & parsedUser;
		ar & parsedHostPro;  
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

	//!  The Sequence of the Request
	/*!
		For example:
		\n  0 - is the first request in the session.
	*/
	unsigned int index;

	//!  If The Request was Analyzed by the Domain Path in Training Mode.
	/*!
		"0" or "1".
	*/
	char parsedHost;

	//!  If The Request was Analyzed by the User Path in Training Mode.
	/*!
		"0" or "1".
	*/
	char parsedUser;

	//!  If The Request was Analyzed by the Domain Path Production Mode.
	/*!
		"0" or "1".
	*/
	char parsedHostPro;

	//!  Page ID.
	/*!
		This number is created by hash function.
		\sa hashCode()
	*/
	unsigned int ID;

	//!  Request ID.
	/*!
		Random signed number(8 bytes).
	*/
	long long RID;

	//!  Time Stamp.
	/*!
		The epoch of the request.
	*/
	double ts;

	//!  HTTP Status Code.
	/*!
		\n For example : 200 
	*/
	unsigned short status_code;

	//!  Tainted Request (HTTP Status Code is greater than 400).
	/*!
		"0" or "1".
	*/
	char tainted;

	//!  The number of times this key(Page ID) exists in that level/index.
	/*!*/
	unsigned int emission;

	//!  The number of times this key(Page ID) in that level/index is not the last request in the session(so far).
	/*!*/
	unsigned int link_sample;

	//!  Source IP.
	/*!
		For example : "81.218.185.126".
	*/
	string user_ip;

	//!  Destination IP.
	/*!
		For example : "8.8.8.8".
	*/
	string resp_ip;

	//!  The Unique Key of Request.
	/*!
		For example : 
		\n
		\n The Page ID of "/index.php: is 5342523 so the unique key of the request when it's the first request of the session is "0+5342523".
	*/
	string compare;

	//!  Username.
	/*!
		For example : "admin".
	*/
	string user;

	//!  Protocol/Port.
	/*!
		For example : "HTTP" or "HTTPS".
	*/
	string protocol;

	//!  Method.
	/*!
		For example : "GET" or "POST".
	*/
	string method;

	//!  
	/*!*/
	double presence;

	//!  
	/*!*/
	unsigned int full_rule_hash;

	//!  ElasticSearch Index Name.
	/*!
		For example : 20150101 .
	*/
	string shard;

	//!  The HTML Title.
	/*!
		example : "Hybrid Security Main Page" .
	*/
	string title;

	//!  The Page URL.
	/*!
		For example : "/index.php"
	*/
	string page_name;

	//!  Canonical URL.
	/*!
		\sa https://support.google.com/webmasters/answer/139066?hl=en
	*/
	string canonical_url;

	//!  Domain/Subdomain Name.
	/*!
		For example : "hybridsec.com" .
	*/
	string host_name;

	//!  Domain ID.
	/*!
		For example : 534 . 
	*/
	string domain_id;

	//!  Subdomain ID.
	/*!
		For example : 432 .
	*/
	unsigned int subdomain_id;

	//!  
	/*!*/
	string sha256_sid;

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	Page();

	//!  Constrator.
	/*!
		Initializing the object fields with their default values.
		\param mParams as a C++ STL unordered_map argument.
		\param index as an unsigned integer argument.
		\param current_PageID as an unsigned integer argument.
		\param current_RID as a long long argument.
		\param current_IP as a C++ string argument.
		\param UserID_ as a C++ string argument.
	*/
	Page(boost::unordered_map<char,string> &,unsigned int,unsigned int,long long,string &,string &);

	//! Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\param none
		\return void
	*/
	void print();

};

#endif
