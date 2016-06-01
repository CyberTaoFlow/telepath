#include "teletypes.h"
#include <iostream>
#include <stdlib.h>


using namespace std;

boost::unordered_map <string,unsigned short> global_header;
boost::unordered_set<string> domain_header;
boost::unordered_set<string> const_method;
boost::unordered_map <char,string> const_types;
boost::unordered_set<string> const_content_type;


/*
 * string int_to_string(int n)
 * 		Converts int, long long, unsigned int, unsigned long long to string
 * Parameters:
 * 		Integer number
 * Return value:
 * 		String
 */
string int_to_string(int n)
{
	char digit[12];
	sprintf(digit,"%d",n);
	return digit;
}

string int_to_string(long long n)
{
	char digit[22];
	sprintf(digit,"%lld",n);
	return digit;
}

string int_to_string(unsigned int n)
{
	char digit[22];
	sprintf(digit,"%u",n);
	return digit;
}

string int_to_string(unsigned long long n)
{
	char digit[22];
	sprintf(digit,"%llu",n);
	return digit;
}

string int_to_string(double n)
{
	char digit[22];
	sprintf(digit,"%f",n);
	return digit;
}


/*
 * string double_to_string(double n)
 * 		Converts double to string
 * Parameters:
 * 		Integer number
 * Return value:
 * 		String
 */
string double_to_string(double n)
{
	char digit[25];
	sprintf(digit,"%f",n);
	return digit;
}


/*
 * unsigned int to_utf8(string index)
 * 		Converting one element to UTF8
 * Parameters:
 * 		No
 * Return value:
 * 		Doesn't return value
 */

unsigned int to_utf8(string & index)
{
	unsigned int i;
	i = strtoul( index.c_str() + 1,NULL,16 );
	if(i>255){
		return 0;
	}else{
		return i;
	}
}


unsigned int to_utf8(const char * index)
{
	unsigned int i;
	i = strtoul( index + 1,NULL,16 );
	if(i>255){
		return 0;
	}else{
		return i;
	}
}

/*
 * string init_lookuptable(int n)
 * 		1) Initiate a dictionry of global heaedrs.
		2) Initiate a dictionry of domain heaedrs.
		3) Initiate a dictionry of all HTTP methods.
		4) Initiate a dictionry of all HTTP methods.
 * Parameters:
 * 		No
 * Return value:
 * 		Doesn't return value
 */

void init_lookuptable()
{
	global_header.insert(pair<string,unsigned short>("accept-encoding",1));
	global_header.insert(pair<string,unsigned short>("accept-charset",2));
	global_header.insert(pair<string,unsigned short>("x-varnish",3));
	global_header.insert(pair<string,unsigned short>("x-forwarded-for",4));
	//global_header.insert(pair<string,unsigned short>("user-agent",5));
	global_header.insert(pair<string,unsigned short>("x-requested-with",6));
	//global_header.insert(pair<string,unsigned short>("host",7));
	//global_header.insert(pair<string,unsigned short>("cookie",8));
	global_header.insert(pair<string,unsigned short>("sip",7));
	global_header.insert(pair<string,unsigned short>("client-ip",8));
	global_header.insert(pair<string,unsigned short>("client_ip",9));
	global_header.insert(pair<string,unsigned short>("accept-language",10));
	global_header.insert(pair<string,unsigned short>("accept",11));
	global_header.insert(pair<string,unsigned short>("accept-datetime",12));
	global_header.insert(pair<string,unsigned short>("proxy-connection",13));
	//global_header.insert(pair<string,unsigned short>("content-type",14));
	//global_header.insert(pair<string,unsigned short>("content-length",15));
	global_header.insert(pair<string,unsigned short>("authorization",16));
	global_header.insert(pair<string,unsigned short>("connection",17));
	global_header.insert(pair<string,unsigned short>("hybridrecord",18));

	//------------------------------------

	domain_header.insert("user-agent");
	domain_header.insert("host");
	domain_header.insert("cookie");
	domain_header.insert("referer");
	domain_header.insert("if-modified-since");
	domain_header.insert("cache-control");
	domain_header.insert("from");
	domain_header.insert("pragma");
	domain_header.insert("via");
	domain_header.insert("dnt");
	domain_header.insert("date");
	domain_header.insert("proxy-authorization");
	domain_header.insert("x-wap-profile");
	domain_header.insert("x-wap-profile-diff");
	domain_header.insert("x-att-deviceid");

	//------------------------------------

	const_types.insert(pair<char,string>('F',"Free"));
	const_types.insert(pair<char,string>('n',"numeric"));
	const_types.insert(pair<char,string>('e',"enum"));
	const_types.insert(pair<char,string>('t',"text"));
	const_types.insert(pair<char,string>('u',"undetermined"));

	//------------------------------------

	const_method.insert("GET");
	const_method.insert("POST");
	const_method.insert("HEAD");
	const_method.insert("OPTIONS");
	const_method.insert("PUT");
	const_method.insert("DELETE");
	const_method.insert("TRACE");
	const_method.insert("CONNECT");
	const_method.insert("PROPFIND");
	const_method.insert("MKCOL");
	const_method.insert("COPY");
	const_method.insert("MOVE");
	const_method.insert("LOCK");
	const_method.insert("UNLOCK");
	const_method.insert("POLL");
	const_method.insert("REPORT");
	const_method.insert("SUBSCRIBE");
	const_method.insert("BMOVE");
	const_method.insert("SEARCH");

	//------------------------------------

	const_content_type.insert("application/x-www-form-urlencoded");
	const_content_type.insert("application/x-www-form-urlencoded; charset=utf-8");

	const_content_type.insert("text/plain");
	const_content_type.insert("text/plain;charset=utf-8");
	const_content_type.insert("text/plain; charset=utf-8");

	//const_content_type.insert("text/xml");
	//const_content_type.insert("text/xml; charset=utf-8");
	//const_content_type.insert("text/xml; charset=`utf-8`");

	const_content_type.insert("text/json");
	const_content_type.insert("application/json");
	const_content_type.insert("application/json; charset=utf-8");
	const_content_type.insert("application/json-rpc");

	const_content_type.insert("text/html");
	const_content_type.insert("text/html; charset=utf-8");
	const_content_type.insert("text/html; charset=iso-8859-1");

	const_content_type.insert("application/x-java-archive");



}


/*
 * 		Converts string to array of UTF8 codes
 * Parameters:
 * 		data - string of data
 * 		result - vector of UTF8 codes
 * Return value:
 * 		0 - on success; 1 - if values is noisy, t.e. some artificial value.
 */
void string_to_utf8(string & data, vector<unsigned int>& result)
{
	unsigned int sz=data.size(); // saving length of string

	result.clear();
	result.reserve( sz*3 );
	unsigned int i=0;
	unsigned int uiTmp=0;
	unsigned int tmp, hi_b, lo_b, mid_b, res_b;
	char ind[]= {'\0','\0','\0','\0'};
	char b1[] = {'\0','\0','\0','\0'};
	char b2[] = {'\0','\0','\0','\0'};
	unsigned short utf16_code=0;

	while(i<sz)	//running through all string
	{
		ind[1]='\0';
		ind[2]='\0';
		tmp=hi_b=lo_b=mid_b=res_b=0;


		//%d7%99 - UTF8; %u05db - UTF16
		//so looking for UTF8 and not for UTF16
		if(data[i]=='%' && data[i+1]!='u')
		{

			ind[1]=data[i+1];
			ind[2]=data[i+2];
			tmp=to_utf8(ind);//saving first found element to tmp

			if(tmp>=0 && tmp <= 127)	//1-byte ascii symbol
			{
				result.push_back(tmp);
				i+=3;
				continue;
			}
			else if(tmp>=192 && tmp <= 223)// 2-bytes UTF8
			{					//so looking for second byte
				i+=3;			//moving iterator to the next element
				if(i>=sz)		//if iterator more then size of string, so we have only one byte, which is not UTF8, so mark it as noisy
				{
					result.push_back(tmp);
					return;
				}

				else if(data[i]!='%')	//if we cannot find '%' in the next byte - mark as noisy
				{
					result.push_back(tmp);
					return; //or maybe better to return
				}


				//checking every byte
				++i;
				if(i<sz)
					ind[1]=data[i];
				else
				{
					result.push_back(tmp);
					return;
				}

				++i;
				if(i<sz)
					ind[2]=data[i];
				else
				{
					result.push_back(tmp);
					return;
				}


				hi_b=tmp;
				lo_b=to_utf8(ind);

				uiTmp=hi_b*256+lo_b;		//building 2-bytes integer

				result.push_back(uiTmp);
				++i;
				continue;
			}
			else if(tmp>=224 && tmp<=239) // 3-bytes UTF8
			{
				if(sz-i>=8)//checking, if after 1st byte there are 2 more
				{

					hi_b=tmp;
					//saving three bytes: hi is in tmp, mid and lo
					b1[1]=data[i+4];
					b1[2]=data[i+5];
					mid_b=to_utf8(b1);

					b2[1]=data[i+7];
					b2[2]=data[i+8];
					lo_b=to_utf8(b2);

					//if sequence of bytes doesn't meet UTF8, i convert each byte to number
					if( !((mid_b>=128 && mid_b <= 191) && (lo_b>=128 && lo_b <= 191)) )
					{

						result.push_back(hi_b);
						result.push_back(mid_b);
						result.push_back(lo_b);
						return; //added 17.01.2012
						i+=9;
						continue;
					}

					//else
					uiTmp=hi_b*256*256+mid_b*256+lo_b;
					result.push_back(uiTmp);

					i+=9;
					continue;
				}
				else
				{

					result.push_back(tmp);
					i+=3;
					return; //added 17.01.2012
					continue;
				}
			}
			else  //should delete this else???
			{
/*
				if(tmp == 192 || tmp == 193 || tmp>=245 && tmp<=255)
				{
					i+=3;
					continue;
				}
*/
				result.push_back(tmp);
				i+=3;
				continue;
			}
		}

		//looking for UTF16
		else if(data[i]=='%' && data[i+1]=='u')
		{
			i+=2;
			//getting low and high bytes of UTF16
			ind[1]=data[i];
			ind[2]=data[i+1];
			hi_b=to_utf8(ind);
			ind[1]=data[i+2];
			ind[2]=data[i+3];
			lo_b=to_utf8(ind);

			utf16_code=hi_b*256+lo_b;	//calculating

			//UTF16(U+0000 to U+007F) == 0xxxxxxx ==> 1-byte UTF8(0xxxxxxx)
			if(utf16_code>0x0000 && utf16_code<0x007F)
			{
				result.push_back(utf16_code);
			}
			// UTF16(0x00000000 — 0x0000007F)==00000yyy yyxxxxxx ==> 2-byte UTF8(110yyyyy 10xxxxxx)
			else if(utf16_code>0x0080 && utf16_code<0x07FF)
			{
				//now hi_b & lo_b - will be bytes of UTF8
				hi_b=0b11000000 | (utf16_code>>6);
				lo_b=0b10000000 | (utf16_code & 0b0000000000111111);
				tmp=hi_b*256+lo_b;
				result.push_back(tmp);
			}
			// UTF16(U+0800 to U+FFFF)==zzzzyyyy yyxxxxxx ==> 3-byte UTF8(1110zzzz 10yyyyyy 10xxxxxx)
			else if(utf16_code>0x0800 && utf16_code<0xFFFF)
			{
				//now hi_b & lo_b - will be bytes of UTF8
				hi_b= 0b11100000 | (utf16_code>>12);
				mid_b= 0b10000000 | ((utf16_code>>6) & 0b00000000000111111);
				lo_b=0b10000000 | (utf16_code & 0b0000000000111111);
				tmp=hi_b*256*256+mid_b*256+lo_b;
				result.push_back(tmp);
			}
			i+=4;
		}

		else//if it's simply ascii
		{
			result.push_back(data[i]);
			++i;
		}
	}

}

// In each Node there is an ASCII char.

unsigned int lenHelper2(unsigned int x) { 
    // this is either a fun exercise in optimization 
    // or it's extremely premature optimization.
    if(x>=100000) {
        if(x>=10000000) {
            if(x>=1000000000) return 10;
            if(x>=100000000) return 9;
            return 8;
        }
        if(x>=1000000) return 7;
        return 6;
    } else {
        if(x>=1000) {
            if(x>=10000) return 5;
            return 4;
        } else {
            if(x>=100) return 3;
            if(x>=10) return 2;
            return 1;
        }
    }
}

unsigned int itoa2(unsigned int val,char *buf){

	unsigned int s=lenHelper2(val);
 	char *ptr = buf+s;

	*ptr = '\0';

	do
		*--ptr = '0' + (val % 10);
   	while(val/=10);
	return s;
}



/*
 * void unicodeParser2(vector <unsigned int> & shortVec,string & str)
 * 		Creates comma separated string of vector values
 * Parameters:
 * 		shortVec - vector of integers
 * 		str - result string
 * Return value:
 * 		null.
 */

// Get a vector of numbers and put them in a string sperate by commas.
void unicodeParser2(vector <unsigned int> & shortVec,string & str){ 
	char tmp[15];
	unsigned int sizeV = shortVec.size();

	char buffer[sizeV* 12+1];
	unsigned int length = 0,len;
	for( unsigned int i=0 ; i<sizeV ;i++){
		if(i != sizeV-1 ){	
			len = itoa2(shortVec[i],tmp);	
			tmp[len++] =',';
			tmp[len] ='\0';
		}else{
			len = itoa2(shortVec[i],tmp);				
		}


		memcpy(buffer + length, tmp, len ); 
		length+=len;
	}
	
	buffer[length]='\0';
	str = buffer;


}



