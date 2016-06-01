#include "../h/atmstypes.h"
#include <iostream>
#include <stdlib.h>


using namespace std;
map<string,string> const_header;

/**
 * sort_app_names() is a function to sort strings(APP names) in DESCENDING order by LENGTH
 *
 * @param i1 is a reference to type of vector
 * @param i2 is a reference to type of vector
 * @return true if first parameter should be first in resulting set
 */
bool sort_app_names(const vector<string>::value_type &i1,const vector<string>::value_type &i2)
{
	return i1.size()>i2.size();
}


/**
 * parse_url() function trims "http://", "https://", "www." and parameters from url
 *
 * @param str is an url
 * @return no return value
 */
void parse_url(std::string &str)
{
	string::size_type pos;

	pos=str.find("://");
	if(pos!=string::npos)
	{
		pos+=3;

	#ifdef EXCEPTION_CATCH
		try
		{
			str.erase(0,pos);
		}
		catch(exception &ex)
		{
			cout<<"void parse_url(std::string &str)1: "<<ex.what()<<endl;
			exit(1);
		}
	#else
		str.erase(0,pos);
	#endif

	}
	//remove "www." from the string
	pos=str.find("www.");
	if(pos!=string::npos)
	{
		pos+=4;
	#ifdef EXCEPTION_CATCH
		try
		{
			str.erase(0,pos);
		}
		catch(exception &ex)
		{
			cout<<"void parse_url(std::string &str)2: "<<ex.what()<<endl;
			exit(1);
		}
	#else
		str.erase(0,pos);
	#endif
	}

	pos=str.find('?');
	if(pos!=string::npos)
	{
	#ifdef EXCEPTION_CATCH
		try
		{
			if( pos < str.size() ){
				str.erase(pos);
			}
		}
		catch(exception &ex)
		{
			cout<<"void parse_url(std::string &str)3: "<<ex.what()<<endl;
			cout<<pos<<" str: "<<str<<endl;
			cout<<string::npos<<" : "<<sizeof(string::npos)<<endl;
			exit(1);
		}
	#else
		if( pos < str.size() ){
			str.erase(pos);
		}
	#endif
	}

}


/**
 * parse_quote() function adds ' if meets another quote
 *
 * @param str - string to pars
 * @return no return value
 */
void parse_quote(string &str)
{
	string::size_type pos=0;
	while(pos!=string::npos)
	{
		pos=str.find('\'',pos);
		if(pos!=string::npos)
		{
			if(str[pos+1]!='\'')
			{
				str.insert(pos+1,"\'");
			}
			pos+=2;
		}
	}
}

/**
 * parse_str_delim() function searches for delimiter and breaks to tokens
 *
 * @param data - string to parse
 * @param c -delimiter
 * @return string before first delimiter
 */
string parse_str_delim(string& data, char c)
{
#ifdef DEBUG
	printf("\nstring DalSock::parse_str(string& data)\n");
#endif
	string::size_type pos;

	pos=data.find(c);
	if(pos==string::npos )
	{
		string res=data;
		data.clear();
		return res;
	}
	string res;
	res.assign(data.c_str(), pos);
	data.erase(0,pos+1);

#ifdef DEBUG
	printf("\nstring DalSock::parse_str(string& data) - finish\n");
#endif
	return res;
}


/**
 * int_to_string() function converts int, long long, unsigned int, unsigned long long to string
 *
 * @param n number to convert
 * @return returns std::string
 */
string int_to_string(int n)
{
	char digit[12];
	sprintf(digit,"%d",n);
	return digit;
}

/**
 * int_to_string() function converts int, long long, unsigned int, unsigned long long to string
 *
 * @param n number to convert
 * @return returns std::string
 */
string int_to_string(long long n)
{
	char digit[22];
	sprintf(digit,"%lld",n);
	return digit;
}

/**
 * int_to_string() function converts int, long long, unsigned int, unsigned long long to string
 *
 * @param n number to convert
 * @return returns std::string
 */
string int_to_string(unsigned int n)
{
	char digit[22];
	sprintf(digit,"%u",n);
	return digit;
}

/**
 * int_to_string() function converts int, long long, unsigned int, unsigned long long to string
 *
 * @param n number to convert
 * @return returns std::string
 */
string int_to_string(unsigned long long n)
{
	char digit[22];
	sprintf(digit,"%llu",n);
	return digit;
}

/**
 * double_to_string() function converts double to string
 *
 * @param n number to convert
 * @return returns std::string
 */
string double_to_string(double n)
{
	char digit[25];
	sprintf(digit,"%f",n);
	return digit;
}


/**
 * init_lookuptable() function is used to inilialize lookup table for UTF8 decoding
 *
 * @return no return value
 */
void init_lookuptable()
{
	const_header.insert(pair<string,string>("accept-encoding","1"));
	const_header.insert(pair<string,string>("accept-charset","2"));
	const_header.insert(pair<string,string>("x-varnish","3"));
	const_header.insert(pair<string,string>("x-forwarded-for","4"));
	const_header.insert(pair<string,string>("user-agent","5"));
	const_header.insert(pair<string,string>("referer","6"));
	const_header.insert(pair<string,string>("host","7"));
	const_header.insert(pair<string,string>("cookie","8"));
	const_header.insert(pair<string,string>("client_ip","9"));
	const_header.insert(pair<string,string>("accept-language","10"));
	const_header.insert(pair<string,string>("accept","11"));
	const_header.insert(pair<string,string>("content-type","14"));
	const_header.insert(pair<string,string>("content-length","15"));
	const_header.insert(pair<string,string>("authorization","16"));
	const_header.insert(pair<string,string>("setcookie","17"));
}


/**
 * xml_node() function creates xml string line: < nodename > nodevalue < / nodename >
 *
 * @param nodename - string to fetch data
 * @param nodevalue - tag to fetch data from
 * @return string representing XML node
 */

string xml_node(string nodename,string nodevalue)
{
#ifdef DEBUG
	printf("string map_to_xml(string, map<string, string>*)\n");
#endif
	string xmlnode="<"+nodename+">"+nodevalue+"</"+nodename+">";
	return xmlnode;
#ifdef DEBUG
	printf("string :map_to_xml(string, map<string, string>*) finished\n");
#endif

}




/**
 * get_tag_value() function searches for the first tag in str and returns its value
 *
 * @param str conatins all xml message
 * @param opentag "<tag" to fetch data from
 * @param closetag "</tag"
 * @return  std::string with tag value
 */

string get_tag_value(string & str, const char * opentag, const char * closetag)
{
        string tmp;
	string::size_type openpos, closepos;
	unsigned int token_len;

	openpos=str.find(opentag,0);
	if(openpos==string::npos)
	{
		return tmp;
	}
	openpos+= strlen(opentag); //.size();

	closepos=str.find(closetag,openpos);
	token_len=closepos-(openpos);
	if(token_len <= 0)
	{
		return tmp;
	}
	tmp.append(str.c_str()+openpos, token_len);

	return tmp;
}


string get_tag_value(const char * str, const char * opentag, const char * closetag)
{
        string tmp;
	const char * openpos;
	const char* closepos;
	unsigned int token_len;

	openpos=strstr(str+0, opentag);
	if(openpos==0)
	{
		return tmp;
	}
	openpos+= strlen(opentag); //.size();
	closepos=strstr(openpos, closetag);

	token_len=closepos-openpos;
	if(token_len <= 0)
	{
		return tmp;
	}
	tmp.append(openpos, token_len);

	return tmp;
}


unsigned int utf16_to_utf8(unsigned int utf_16){
		unsigned int utf_8;
		unsigned int hi_b,mid_b,lo_b;

		//UTF16(U+0000 to U+007F) == 0xxxxxxx ==> 1-byte UTF8(0xxxxxxx)
		if(utf_16>0x0000 && utf_16<0x007F)
		{
			utf_8 = utf_16;
		}
		// UTF16(0x00000000 — 0x0000007F)==00000yyy yyxxxxxx ==> 2-byte UTF8(110yyyyy 10xxxxxx)
		else if(utf_16>0x0080 && utf_16<0x07FF)
		{
			//now hi_b & lo_b - will be bytes of UTF8
			hi_b=0b11000000 | (utf_16>>6);
			lo_b=0b10000000 | (utf_16 & 0b0000000000111111);
			utf_8=hi_b*256+lo_b;				
		}
		// UTF16(U+0800 to U+FFFF)==zzzzyyyy yyxxxxxx ==> 3-byte UTF8(1110zzzz 10yyyyyy 10xxxxxx)
		else if(utf_16>0x0800 && utf_16<0xFFFF)
		{
			//now hi_b & lo_b - will be bytes of UTF8
			hi_b= 0b11100000 | (utf_16>>12);
			mid_b= 0b10000000 | ((utf_16>>6) & 0b00000000000111111);
			lo_b=0b10000000 | (utf_16 & 0b0000000000111111);
			utf_8=hi_b*256*256+mid_b*256+lo_b;
		}
	
		return utf_8;
}



