
#ifndef _Session_h
#define _Session_h

#include "Page.h"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

using namespace std;


class alert{
public:
	double end_ts;
	double start_ts;
	boost::unordered_set <long long> rids;
	long long ridAlert;
	alert();
};

class Session{
public:
	string IP;			// For example : "81.218.185.126". The IP address of the last session request.
	unsigned int sid;		// The fingerprint/cookievalue produces a unique(there is a slight chance for colision) hash value for each session.
	char status;			// v = valid | i = invalid | u = unknown.
	unsigned int decimalIP;		// For example the decimal IP of "81.218.185.126" is 1373288830. Look for the function ipToNum.
	unsigned int sequence;		// The sequence of the first request in the session is 0, the second is 1 ,etc ...
	char update;			// The session path needs to be updated while this flag is up.
	int totalExp;
	double begin_ts;		// Only for production - because the engine erase pages.
	double elapsed_ts;		// time duration.
	char block,erase;
	unsigned short user_flag; 
	unsigned short sessionSize;
	vector <Page> vRequest;	// Requests sorted by time.
	bool validUser;
	string sUsername;
	bool logout;
	unsigned int logout_counter;
	boost::unordered_map<unsigned int,alert > RIDsPerFlow;
	string compared_link;
	double last_speed;

	Session();
	void reset();
};

#endif

