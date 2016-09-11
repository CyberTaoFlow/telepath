#include <GeoIP.h>
#include <GeoIPCity.h>

#include "Coordinate.h"

using namespace std;

//!  Opening GeoIP Database .
/*!
	This function opens the comlete GeoIP version file(/opt/telepath/db/geoip/GeoIPCity.dat) while available. If this file is not exist, Telepath would use the demo GeoIP version file(/opt/telepath/db/geoip/GeoLiteCity.dat).

	\param None
	\return void
*/
void open_geoIP_database();

//!  Deleting GeoIP Obejects + Freeing Memory.
/*!
*/
void delete_geoIP_obj();

//! Getting Country Code Information .
/*!
	\param None
	\return Country code string (2 characters).
*/
string getCountry(string&);

//! Getting Coordinate Information .
/*!
        \param None
        \return Coordinate information - latitude & longitude.
*/
Coordinate getCoordinate(string & IP);

//! Getting all the Location Information for a given IPv4.
/*!
        \param IP a C++ string agrument.
	\param country a C++ string agrument.
	\param city a C++ string agrument.
	\param c_ as a Coordinate object.
	\return void
*/
void getIpInfo(string&,string&,string&,Coordinate&);

