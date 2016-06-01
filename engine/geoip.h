#include <GeoIP.h>
#include <GeoIPCity.h>

#include "Coordinate.h"

using namespace std;

void open_geoIP_database();
void delete_geoIP_obj();
string getCountry(string&);
Coordinate getCoordinate(string & IP);
void getIpInfo(string&,string&,string&,Coordinate&);

