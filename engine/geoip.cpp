#include <syslog.h>
#include "geoip.h"

using namespace std;

GeoIP *gi;

//opening GeoIP database
void open_geoIP_database(){

	if( access( "/opt/telepath/db/geoip/GeoIPCity.dat", F_OK ) != -1 ) {
		gi = GeoIP_open("/opt/telepath/db/geoip/GeoIPCity.dat", GEOIP_MEMORY_CACHE);
		if(gi==NULL){
			syslog(LOG_NOTICE,"ERROR[!!!] in the file /opt/telepath/db/geoip/GeoIPCity.dat .");
			exit(1);
		}else{
			syslog(LOG_NOTICE,"Using GeoIPCity.dat .");
		}
	}else{
		gi = GeoIP_open("/opt/telepath/db/geoip/GeoLiteCity.dat", GEOIP_MEMORY_CACHE);
		if(gi==NULL){
			syslog(LOG_NOTICE,"ERROR[!!!] in the file /opt/telepath/db/geoip/GeoLiteCity.dat . Maybe the file doesn't exist.");
			exit(1);
		}else{
			syslog(LOG_NOTICE,"Using GeoLiteCity.dat .");
		}
	}

}

void delete_geoIP_obj(){
	if(gi){
		GeoIP_delete(gi);
	}
}

string getCountry(string & IP){
	string country;
	GeoIPRecord *gir = GeoIP_record_by_addr(gi, IP.c_str());
	if(gir && gir->country_code){
		country = gir->country_code;
		GeoIPRecord_delete(gir);
		return country;
	}else{
		if (gir){
			GeoIPRecord_delete(gir);
		}
		return "00";
	}
}

Coordinate getCoordinate(string & IP){
	Coordinate c_;
	GeoIPRecord *gir = GeoIP_record_by_addr(gi, IP.c_str());
	if(gir && gir->country_code){
		c_.setCoordinate(gir->latitude,gir->longitude);
		GeoIPRecord_delete(gir);
		return c_;
	}else{
		if (gir){
			GeoIPRecord_delete(gir);
		}
		c_.setCoordinate(0,0);
		return c_;
	}
}

void getIpInfo(string & IP,string & country,string & city,Coordinate & c_){

	if(IP.size()<7){ // Broken IP
		country = "00";
		city = "Unknown";
		c_.setCoordinate(0,0);
		return;
	}

	GeoIPRecord *gir = GeoIP_record_by_addr(gi, IP.c_str());
	if(gir){
		if(gir->country_code){
			country = gir->country_code;
			c_.setCoordinate(gir->latitude,gir->longitude);
		}else{
			country = "00";
			c_.setCoordinate(0,0);
		}

		if(gir->city){
			city = gir->city;

			for(unsigned short i=0; i < city.size() ; i++){
				if(city[i]>0){
					if(city[i]==39){// "'"
						city[i]='`';
					}
				}else{
					switch(city[i])
					{
						case -4: // "ú"
						case -6: // "ú"
							city[i]='u';
							break;
						case -8:  // "ø"
						case -10: // "ö"
						case -13: // "ó"
							city[i]='o';
							break;
						case -15: // "ñ"
							city[i]='n';
							break;
						case -18: // "Ã"
						case -19: // "í"
						case -20: // "ì"
							city[i]='i';
							break;
						case -21:  // ë
						case -22:  // ê
						case -23:  // é
						case -24:  // è
							city[i]='e';
							break;
						case -25: // "ç"
							city[i]='c';
							break;
						case -26: // "æ"
						case -27: // "å"
						case -28: // "ä"
						case -29: // "ã"
						case -30: // "â"
						case -31: // "á"
							city[i]='a';
							break;
						case -33: // "ß"
							city[i]='B';
							break;
						case -36: // "Ü"
							city[i]='U';
							break;
						case -50: // "Î"
							city[i]='I';
							break;
						case -55: // "É"
							city[i]='E';
							break;
						case -59: // "Å"
							city[i]='A';
							break;



					}
				}
			}
		}else{
			city = "Unknown";
		}

		GeoIPRecord_delete(gir);
	}else{
		country = "00";
		city = "Unknown";
		c_.setCoordinate(0,0);
	}
}


