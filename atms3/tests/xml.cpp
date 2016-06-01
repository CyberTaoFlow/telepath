
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <vector>
#include <queue>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <limits.h>

#include <mxml.h>

using namespace std;


void parseXML(mxml_node_t *tree,string res,multimap<string,string> & xml_params){

	if (tree->value.element.name && (tree->child != NULL || tree->next != NULL) )
	{
		if(tree->next!=NULL){
			parseXML(tree->next,res,xml_params);
		}
		if(tree->child != NULL){
			if (!res.empty()){
				res.append("::") ;
			}
			res.append(tree->value.element.name) ;
			parseXML(tree->child,res,xml_params);
		}
	}
	if(tree->value.opaque && tree->child == NULL && tree->next == NULL )
	{
		xml_params.insert( pair<string,string>(res,tree->value.opaque) );
	}
}

unsigned int checkXML(char * buff){
	unsigned int i;	

	for(i=0;buff[i]!=0 && buff[i]==' ';i++);
	if(buff[i]==0){
		return 0;
	}
	
	if(buff[i+1]=='?' && buff[i+2]=='x' && buff[i+3]=='m' && buff[i+4]=='l'){
		i+=5;
	}else{
		return 0;
	}

	for(;;i++){
		if(buff[i]=='>'){
			break;
		}
		else if(buff[i]==0){
			return 0;
		}
	}
	return (i+1);
}

int main ()
{
	//char buffer[8192] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:ns7824=\"http://tempuri.org\"><SOAP-ENV:Body><GetYad2PopularCategories xmlns=\"http://tempuri.org/\"><b144_Rq><hdrRq><hdr_SearchType>yad2BusinessSearch</hdr_SearchType><hdr_ReturnResultCount>12</hdr_ReturnResultCount><hdr_PartnerId>yVnTEdKxwPzEm/lIOlffJw==</hdr_PartnerId></hdrRq><bdyRq><bdy_UserIp>80.74.102.18</bdy_UserIp><bdy_Referrer>http://www.yad2.co.il/Nadlan/sales.php?City=Ã¸Ã Ã¹Ã¥Ã¯ Ã¬Ã¶Ã©Ã¥Ã¯&amp;Neighborhood=&amp;HomeTypeID=5ooms=&amp;untilRooms=&amp;fromPrice=1000000&amp;untilPrice=2000000&amp;PriceType=1&amp;FromFloor=&amp;ToFloor=&amp;Info=</bdy_Referrer><bdy_UserAgent>Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)</bdy_UserAgent></bdyRq></b144_Rq></GetYad2PopularCategories></SOAP-ENV:Body></SOAP-ENV:Envelope>";
//	char buffer[8192] = " /><UserID>153031830</UserID><UserNameLowerCase /><FirstName>Maria</FirstName><LastName>Azzopardi</LastName><Company /><Address /><AddressCity>Whitestone</AddressCity><AddressState>New York</AddressState>";
	char * buffer = "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:ns7824=\"http://tempuri.org\"><data>777777777 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa rhyyyyyyyyyyyy 564555555555</data>";




	multimap<string,string> xml_params;
	multimap<string,string>::iterator it;

	mxml_node_t *tree;
	mxml_node_t *node;
	unsigned int k;

	unsigned int offset = checkXML(buffer);

	//cout <<buffer+offset <<endl;


	tree = mxmlLoadString(NULL, buffer+offset,MXML_OPAQUE_CALLBACK);
	if(tree!=NULL){
		parseXML(tree,"",xml_params);
		mxmlDelete( tree );
	}	

	cout << "-------------------" << endl;




	for( it=xml_params.begin() ; it!=xml_params.end() ; it++){
		cout << it->first <<"->"<<it->second<< endl;
	}





	cout << "========================= END ======================" << endl;
}


