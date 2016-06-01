#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string>
#include <iostream>
#include <string.h>

#ifndef __HREGEX_H__
#define __HREGEX_H__
using namespace std;

class hregex
{

private:	
	static bool NeedToInit;						//flags for different initializations
	static bool NeedToInitSQL;
	static bool NeedToInitFreeTextandURL;
	static bool NeedToInitUrl;

	//variable to store compiled regex
	static regex_t hostnameCompRegex;
	static regex_t urlCompRegex;
	static regex_t dataCompRegex;
	static regex_t httpheaderCompRegex;
	static regex_t httpvalueCompRegex;
	static regex_t sqlblacklistCompRegex;
	static regex_t freetextCompRegex;
	static regex_t urldetectionCompRegex;

public:
	static bool regexp(const char *sstring, regex_t *rgt);
	//Compares values to precompiled regex
	static bool CheckHostnameField(string &hostname);
	static bool CheckURLField(string &url);
	static bool CheckHTTPHeader(string &httpheader);
	static bool CheckHTTPValue(string &httpvalue);
	static bool CheckSQLBlackListValue(string &sqlblacklist);
	static bool CheckFreeTextValue(string& );
	static bool CheckUrlDetectionValue(string& );

	static bool InitRegex();
	static bool InitSQLBlackListRegex(string);
	static bool InitFreeTextRegexandURL(string,string );
	static void freeRegex();
};



#endif
