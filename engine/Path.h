#ifndef _Path_h
#define _Path_h

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <map>
#include <GeoIP.h>
#include <GeoIPCity.h>
#include <pthread.h>

#include "NodeExtended.h"
#include "LinkExtended.h"
#include "DB-SCAN.h"

class Link;
class Page;
class Session;

using namespace std;

unsigned int compareStatus(vector <unsigned int>,unsigned int);
void checkMapPage(char &,unsigned short &);
void loadMaxSession(unsigned int);
string getCountry(string &);
//string getCity(string &);
Coordinate getCoordinate(string &);
void getIpInfo(string &,string &,string &,Coordinate &);

class Path{
public:

	friend class boost::serialization::access;
  	std::map <string,NodeExtended> mNodeExtended; // Unique key to each Page in the Path.
	std::map <string,LinkExtended> mLinkExtended; // Unique key to each Link in the Path.
	std::vector <unsigned int> sampleP; // Number of sample to each level in the Path.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mNodeExtended;  
		ar & mLinkExtended;     
		ar & sampleP;  
		ar & domain;  
  
		ar & behavior;
		ar & activate;
		ar & numOfSessions;  
		ar & group_id;        
		ar & location;
	}
	string domain;
	unsigned int behavior;
	unsigned long numOfSessions;
	unsigned short activate; // behavior group only!!!.
	unsigned short group_id; // To suspicious groups.

	//------Location--------
	Db_Scan location;
	//----------------------

	Path();
	Path(string&);
	Path(string&,unsigned int);
	Path(string&,unsigned short);
	void tokenize(Session &,short);
	void updatePage(Page &,short,short &);
	void updateLink(Link &,short &,unsigned int &,unsigned int &);
	void print();
	void printCapacity();

};

#endif
