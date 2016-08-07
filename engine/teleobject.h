#ifndef _teleobject_h
#define _teleobject_h

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <string>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <algorithm>

#include "teletypes.h"

using namespace std;

//unsigned int get_vec_length(vector< vector<unsigned int> >);

struct Attribute
{
	unsigned int hash;
	string name;
	string value;
	char attribute_source;
	char analysis;
	vector<unsigned int> vec_value;
};



class TeleObject
{
public:
	boost::unordered_map<char,string> mParams;
	boost::unordered_multimap<unsigned int,struct Attribute> mAttr;

	//------mParams Keys------

	//'a' = UserIP.
	//'b' = TimeStamp.
	//'c' = Page.
	//'d' = StatusCode.
	//'e' = SID.
	//'f' = App.
	//'g' = AppID.
	//'h' = Method.
	//'i' = Protocol.
	//'j' = RID.
	//'k' = PageID.
	//'m' = Title.
	//'n' = CookieStatus.
	//'o' = CleanSID.
	//'p' = SetCookie.
	//'q' = AgentID.
	//'r' = UserID.
	//'s' = ReqSeq.
	//'t' = Encoding.
	//'u' = RespIP.
	//'v' = LoginMsg.
	//'w' = HttpAuthType.
	//'x' = HttpUser.
	//'y' = SubDomainID.
	//'z' = User-Agent.

	TeleObject();

	TeleObject& operator=(const TeleObject&);
	TeleObject(const TeleObject&);
	~TeleObject();

	bool parseApache(string&,unsigned long long&,string&);
	bool parseIIS(string&,unsigned long long&,string&);

	void clear();

};

#endif
