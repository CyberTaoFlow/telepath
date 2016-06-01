
using namespace std;

boost::unordered_map <string,string> months;
boost::unordered_set <string> methods;
boost::unordered_map <string,char> const_attribute_types;

void initRegex(){
				//  # Visa | MasterCard | American Express | Diners Club | Discover | JCB .
	string cc_format = "^(?:4[0-9]{12}(?:[0-9]{3})?|5[1-5][0-9]{2}[-]*[0-9]{12}|3[47][0-9]{13}|3(?:0[0-5]|[68][0-9])[0-9]{11}|6(?:011|5[0-9]{2})[0-9]{12}|(?:2131|1800|35\\d{3})\\d{11})$";
	int erroroffset;
	const char *error;
	cc_regex = pcre_compile((char*)cc_format.c_str(), 0, &error, &erroroffset, NULL);
}

void initHybridRecord(){

	// The values of this parameter record the business action.
	map<unsigned int,myAtt> ::iterator itMyAttMap = myAttMap.find(18);
	if(itMyAttMap == myAttMap.end()){
		myAtt att;  //lock=0
		att.type = 't';
		att.kind = 'G';
		att.name = "hybridrecord";
		att.ifUpdate='y';
		att.page_id = 1;
		att.num_of_values++;
		myAttMap.insert( pair<unsigned int,myAtt>(18,att) );
	}

	itMyAttMap = myAttMap.find(19);
	if(itMyAttMap == myAttMap.end()){
		myAtt att;  //lock=0
		att.type = 't';
		att.kind = 'H';
		att.name = "not-found";
		att.ifUpdate='y';
		att.page_id = 0;
		att.num_of_values++;
		myAttMap.insert( pair<unsigned int,myAtt>(19,att) );
	}
}

// Months dictionary.
void initMonths(){
	months.insert(pair<string,string>("Jan","01"));
	months.insert(pair<string,string>("Feb","02"));
	months.insert(pair<string,string>("Mar","03"));
	months.insert(pair<string,string>("Apr","04"));
	months.insert(pair<string,string>("May","05"));
	months.insert(pair<string,string>("Jun","06"));
	months.insert(pair<string,string>("Jul","07"));
	months.insert(pair<string,string>("Aug","08"));
	months.insert(pair<string,string>("Sep","09"));
	months.insert(pair<string,string>("Oct","10"));
	months.insert(pair<string,string>("Nov","11"));
	months.insert(pair<string,string>("Dec","12"));
}

// All HTTP methods.
void initMethod(){
	methods.insert("GET");
	methods.insert("POST");
	methods.insert("HEAD");
	methods.insert("OPTIONS");
	methods.insert("PUT");
	methods.insert("DELETE");
	methods.insert("TRACE");
	methods.insert("CONNECT");
	methods.insert("PROPFIND");
	methods.insert("PROPPATCH");
	methods.insert("MKCOL");
	methods.insert("COPY");
	methods.insert("MOVE");
	methods.insert("LOCK");
	methods.insert("UNLOCK");
	methods.insert("POLL");
	methods.insert("REPORT");
	methods.insert("SUBSCRIBE");
	methods.insert("BMOVE");
	methods.insert("SEARCH");
}

// The following headers have a constant type.
// For example, "content-length" is always analysed as a numeric type.
void initAttType(){

	const_attribute_types.insert(pair<string,char>("content-length",'n'));// n = numeric.
	const_attribute_types.insert(pair<string,char>("accept",'e'));// e = enumeration.
	const_attribute_types.insert(pair<string,char>("accept-encoding",'e'));// e = enumeration.
	const_attribute_types.insert(pair<string,char>("x-forwarded-for",'e'));// e = enumeration.
	const_attribute_types.insert(pair<string,char>("accept-language",'e'));// e = enumeration.
	const_attribute_types.insert(pair<string,char>("pragma",'e'));// e = enumeration.
	const_attribute_types.insert(pair<string,char>("cache-control",'e'));// e = enumeration.
	const_attribute_types.insert(pair<string,char>("host",'e'));// e = enumeration.
	const_attribute_types.insert(pair<string,char>("connection",'e'));// e = enumeration.
	const_attribute_types.insert(pair<string,char>("user-agent",'t'));// t = text.
	const_attribute_types.insert(pair<string,char>("cookie",'t'));// t = text.

}

