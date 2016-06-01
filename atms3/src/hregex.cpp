
#include "../h/hregex.h"

//initialization in the beginning
bool hregex::NeedToInit = true;
bool hregex::NeedToInitSQL = true;
bool hregex::NeedToInitFreeTextandURL = true;
bool hregex::NeedToInitUrl = true;

//initialization of static parameters
regex_t hregex::hostnameCompRegex;
regex_t hregex::urlCompRegex;
regex_t hregex::httpheaderCompRegex;
regex_t hregex::httpvalueCompRegex;

regex_t hregex::dataCompRegex;
regex_t hregex::sqlblacklistCompRegex;
regex_t hregex::freetextCompRegex;
regex_t hregex::urldetectionCompRegex;


/*
 * bool hregex::InitRegex()
 * 		compiling regexes for: hostnames, urls, httpheader, httpvalues
 * Parameters:
 * 		No
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::InitRegex()
{
	if(NeedToInit)
	{
		if(regcomp(&hostnameCompRegex,"^[a-zA-Z0-9\\._ \\-]*$",REG_EXTENDED) != 0 )
		{
			cout << "Failed to compile Hostname regex" << endl;
			return false;
		}

		if(regcomp(&urlCompRegex,"^/?[a-zA-Z0-9/_\\-]+[\\.]+[a-zA-Z]+[/]?+$",REG_EXTENDED) != 0)
		{
			cout << "Failed to compile URL regex." << endl;
			return false;
		}

		if(regcomp(&httpheaderCompRegex,"^[a-zA-Z0-9_]*$",REG_EXTENDED) != 0)
		{
			cout << "Failed to compile HTTP regex." << endl;
			return false;
		}

		if(regcomp(&httpvalueCompRegex,"^[a-zA-Z0-9\\.,_ \\-:*;=&?]*$",REG_EXTENDED) != 0)
		{
			cout << "Failed to compile HTTP regex." << endl;
			return false;
		}

		/*
		if(regcomp(&sqlblacklistCompRegex,"%27|%22|'|\"",REG_EXTENDED) != 0)
		{
			cout << "Failed to compile HTTP regex." << endl;
			return false;
		}
*/
		NeedToInit = false;
	}
	return true;
}


/*
 * bool hregex::InitSQLBlackListRegex(string sqlregex)
 * 		compiling regexes for: sql black list
 * Parameters:
 * 		sqlregex - regex to compile
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::InitSQLBlackListRegex(string sqlregex)
{
	if(NeedToInitSQL)
	{
		if(regcomp(&sqlblacklistCompRegex,sqlregex.c_str(),REG_EXTENDED) != 0)
		{
			cout << "Failed to compile InitSQLBlackListRegex regex." << endl;
			return false;
		}
		NeedToInitSQL = false;
	}
	return true;
}

//

/*
 * bool hregex::InitFreeTextRegexandURL(string freetextregex, string urldetection)
 * 		compiling regexes for: free text and url detection
 * Parameters:
 * 		freetextregex, urldetection - regex to compile
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::InitFreeTextRegexandURL(string freetextregex, string urldetection)
{
	if(NeedToInitFreeTextandURL)
	{
		if(regcomp(&freetextCompRegex,freetextregex.c_str(),REG_EXTENDED) != 0)
		{
			cout << "Failed to compile freetextCompRegex regex." << endl;
			return false;
		}
		if(regcomp(&urldetectionCompRegex,urldetection.c_str(),REG_EXTENDED) != 0)
		{
			cout << "Failed to compile urldetectionCompRegex." << endl;
			return false;
		}

		NeedToInitFreeTextandURL = false;
	}

	return true;
}

void hregex::freeRegex()
{
	if(NeedToInitFreeTextandURL == false)
	{
		regfree(&freetextCompRegex);
		regfree(&urldetectionCompRegex);
	}
}


/*
 * bool hregex::regexp (const char *sstring, regex_t *rgT)
 * 		wrapping function for comparing sstring to rgT
 * Parameters:
 * 		sstring - to compare
 * 		rgT - precompiled regex
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::regexp (const char *sstring, regex_t *rgT) 
{
	regmatch_t match;
	bool success = false;

        if (regexec(rgT,sstring,1,&match,0)==0)
        {
        	success = true;
        }
             
	return success;
}


/*
 * bool hregex::CheckHostnameField(string &hostname)
 * 		Compares hostname to precompiled regex
 * Parameters:
 * 		hostname
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::CheckHostnameField(string &hostname)
{
	if(NeedToInit)
	{
		cout << "Error: Regex class not init" << endl;
		return false;
	}
	return regexp(hostname.c_str(), &hostnameCompRegex);
}

/*
 * bool hregex::CheckURLField(string &url)
 * 		Compares URL to precompiled regex
 * Parameters:
 * 		url
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::CheckURLField(string &url)
{
	if(NeedToInit)
        {
                cout << "Error: Regex class not init" << endl;
                return false;
        }
        return regexp(url.c_str(), &urlCompRegex);
}

/*
 * bool hregex::CheckHTTPHeader(string &header)
 * 		Compares headers to precompiled regex
 * Parameters:
 * 		header
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::CheckHTTPHeader(string &header)
{
	if(NeedToInit)
        {
                cout << "Error: Regex class not init" << endl;
                return false;
        }
        return regexp(header.c_str(), &httpheaderCompRegex);
}

/*
 * bool hregex::CheckHTTPValue(string &value)
 * 		Compares values to precompiled regex
 * Parameters:
 * 		value
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::CheckHTTPValue(string &value)
{
	if(NeedToInit)
        {
                cout << "Error: Regex class not init" << endl;
                return false;
        }
        return regexp(value.c_str(), &httpvalueCompRegex);
}

/*
 * bool hregex::CheckSQLBlackListValue(string sqlblacklist)
 * 		Compares sqlblacklist to precompiled regex
 * Parameters:
 * 		sqlblacklist
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::CheckSQLBlackListValue(string & sqlblacklist)
{
	if(NeedToInitSQL)
	{
		cout << "Error: Regex class not init" << endl;
		return false;
	}
	return regexp(sqlblacklist.c_str(), &sqlblacklistCompRegex);
}

/*
 * bool hregex::CheckFreeTextValue(string &freetext)
 * 		Compares freetext to precompiled regex
 * Parameters:
 * 		freetext
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::CheckFreeTextValue(string &freetext)
{
	if(NeedToInitFreeTextandURL)
	{
			cout << "Not initialized: NeedToInitFreeTextandURL" << endl;
			return false;
	}
	return regexp(freetext.c_str(), &freetextCompRegex);
}


/*
 * bool hregex::CheckUrlDetectionValue(string &url)
 * 		Compares freetext to precompiled regex
 * 		Detects strings, which begin from "http:\\", "https:\\" ,"www."
 * Parameters:
 * 		url
 * Return value:
 * 		True on success, false - otherwise
 */
bool hregex::CheckUrlDetectionValue(string &url)
{
	if(NeedToInitFreeTextandURL)
	{
		cout << "Not initialized: NeedToInitFreeTextandURL" << endl;
		return false;
	}
    return regexp(url.c_str(), &urldetectionCompRegex);
}
