#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <string.h>
#include <set>
#include <vector>
#include "../h/atmstypes.h"
#include <openssl/rsa.h>


using namespace std;

/**
 * AppData structure contains application (website) related information. For example ssl certificates, etc...
 */
struct AppData
{
        unsigned int global;
	unsigned int global_per_app;
	set<string> cookie_names;
        unsigned int id;
        //unsigned int id;
        set<string> sExcludeGlobalHeaders; //headers, which should be excluded
        //map<string, struct dynamicPageRegex> mdynPageRegex2;    //regex for pages which will be gouped
        // ssl info
        bool ssl_enabled;
        unsigned int ssl_server_port;
        unsigned int ssl_server_port_net;
        set<string> ssl_server_ips;
        string certificate_password;
        string certificate;
        string private_key;
	string login_msg;
	RSA * rsa;
};

/**
 * AppIpData data is used to store infroamtion about RSA private keys for an IP address.
 */
struct AppIpData
{
	unsigned int id;
	unsigned int net_port;
	RSA * rsa;
};


class AppKey {
public:
  string name;
  bool search_subdomain;
};

class AppMatchPredicate {
public:
bool operator()(const class AppKey& lhs, const class AppKey& rhs) const {
        //return strcmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
        if (rhs.search_subdomain )
        {
                //printf("search subdomain %s - %s!\n", lhs.name.c_str(), rhs.name.c_str());
                return strncmp(lhs.name.c_str(), rhs.name.c_str(), lhs.name.size()) > 0;
        } else if (lhs.search_subdomain)
        {
                //printf("search subdomain2 %s - %s!\n", lhs.name.c_str(), rhs.name.c_str());
                return strncmp(lhs.name.c_str(), rhs.name.c_str(), rhs.name.size()) > 0;
        } else if (lhs.name > rhs.name) {
                return true;
       }
      //} else if (lhs.name > rhs.name) {
      return false;
  }
};


/**
 * Application main class.
 */
class Config{
public:
	Config(int);
	int id;
	void Merge( Config * conf2 );
	RSA * GetRSA(const char * ip, unsigned int net_port);
	//----LOAD BALANCING-----------
	//structures to save data about load balancing
	set<string> sLB_IPaddr;
	size_t lb_flag;
	vector<string> lb_header;
	//----LOAD BALANCING-----------
	map<string,string> sniffer_conf;
	int finger_prints_num;
	//-------APP DATA - cookie -id --------------
	//structures to save data about applications
	map<class AppKey, struct AppData, AppMatchPredicate> mAppData;
	map<string, AppIpData> mAppIPData;
	set<unsigned int> mSslPorts;
	//vector of application's names - to store sorted array of applications
	//vector <string> vAppNames;
	map<string,string> mConfig;
	map<string,string> mfingerprints;
	set<string> excludedIP;
	int session_timeout;
	map<string,vector<struct excluded_param> > excluded_params;
};







#endif /*CONFIG_H_*/

