#include "../h/config.h"
#include <arpa/inet.h>

//extern RSA * loadRSA(string & priv_key, string & pwd);


Config::Config(int id){
	this->lb_flag = 1;
	this->id = id;
	this->finger_prints_num = 0;
	this->session_timeout = 86400; //in seconds 
	this->mSslPorts.insert(ntohs(80));
	this->mSslPorts.insert(ntohs(443));
}


/**
 * Merge() function copies all the fields of conf2 to this fields
 * 
 * @param conf2 is a pointer to the config object from which to copy all the fields
 * @return No return values.
 */
void Config::Merge(Config * conf2)
{
	//std::pair< map<class AppKey, struct AppData, AppMatchPredicate>::iterator ,bool> ret;
	//map<class AppKey, struct AppData, AppMatchPredicate>::iterator itr;
	//map<class AppKey, struct AppData, AppMatchPredicate>::iterator itr2;
	sLB_IPaddr = conf2->sLB_IPaddr;
	lb_flag = conf2->lb_flag;
	lb_header = conf2->lb_header;
	sniffer_conf = conf2->sniffer_conf;
	finger_prints_num = conf2->finger_prints_num;
	mConfig = conf2->mConfig;
	mfingerprints = conf2->mfingerprints;
	excludedIP = conf2->excludedIP;
	session_timeout = conf2->session_timeout;
	excluded_params = conf2->excluded_params;
	mSslPorts = conf2->mSslPorts;
	mAppData = conf2->mAppData;
	mAppIPData = conf2->mAppIPData;
}

/**
 * GetRSA() function gets the RSA private key from the mAppIPData map.
 *
 * @param ip server IP to look for
 * @param net_port server port to look for
 */
RSA * Config::GetRSA(const char * ip, unsigned int net_port)
{
	map<string, AppIpData>::iterator itr;
	itr = mAppIPData.find(ip);
	if (itr == mAppIPData.end())
		return NULL;
	if (net_port == itr->second.net_port)
		return itr->second.rsa;
	return NULL;
}

/*	
	// merge mAppData
	AppKey app;
        app.search_subdomain = false;
	for (itr = conf2.mAppData.begin(); itr != conf2.mAppData.end(); itr++)
	{
		app.name = itr->first.name;
		itr2 = mAppData.find(app);
		if (itr2 == mAppData.end())
		{
			ret = eAppData.insert(pair<AppKey,struct AppData>(app,itr->second));
			if (ret->first->second.ssl_enabled)
			{
				// copy of the config class must have its own copy of certificate loaded !
				ret->first->second.rsa = loadRSA(itr2->second.private_key, itr2->second.certificate_password);
			}
				
			continue;
		}
		// this application already their
		itr->second.global = itr2->second.global;	
		itr->second.global_per_app = itr2->second.global_per_app;
		itr->second.cookie = itr2->second.cookie;
		itr->second.id = itr2->second.id;
		itr->second.sExcludeGlobalHeaders = itr2->second.sExcludeGlobalHeaders;
		itr->second.mdynPageRegex2 = itr2->second.mdynPageRegex2;
		if (itr2->second.ssl_enabled)
		{
			itr->ssl_enabled = itr2->second.ssl_enabled;
			itr->second.ssl_server_port = itr2->second.ssl_server_port;
			itr->second.ssl_server_port_net = itr2->second.ssl_server_port_net;
			if (itr->second.certificate != itr2->second.certificate &&
				itr->second.private_key != itr2->second.private_key &&
				itr->second.certificate_password != itr2->second.certificate_password)
			{
				// load certificate again !
				// do not free RSA structure !
				itr->secon.rsa = loadRSA(itr2->second.private_key, itr2->second.certificate_password);
				itr->second.certificate = itr2->second.certificate;
				itr->second.private_key = itr2->second.private_key;
				itr->second.certificate_password = itr2->second.certificate_password;
			}else {
				// same ceritificate was already loaded
				// do not load it again
			}
		} else {
			itr->second.rsa = NULL;
			itr->second.ssl_enabled = false;
		}
	}
}
*/

//Config::Config(Config* c,int id){
//	this->id = id;
//	this->finger_prints_num = 0;
//	this->lb_flag = 1;
//	
//}
