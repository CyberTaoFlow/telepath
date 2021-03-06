#include "../json/json.h"
#include <map>
#include <string>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


using namespace std;

class Case{
public:
	char type;
	bool neg;
	vector <string> vals;

	Case(char neg){
		this->neg=neg;
	}
};

class pageFlow{
public:
	unsigned int pageID;
	map <unsigned int,string> atts;

	pageFlow(unsigned int pageID){
		this->pageID=pageID;
	}
};

void parseJsonFlow(const string & json, vector <pageFlow> & json_params){
        Json::Value root;
        Json::Reader reader;
        Json::Value::iterator it;
        bool parsed;
        string key,strTmp,val;
        unsigned int j,i;
	unsigned int att_id;
	map<unsigned int,string>::iterator itAtts;

        parsed = reader.parse(json, root, false);

        if(!parsed){
                return;
        }

        it=root.begin();
        for (i=0; i < root.size() ; i++) {

		if( !(*it).isObject() ){
                       	key = it.key().asString();
		}

                if( (*it).isObject() ){
                        parseJsonFlow( (*it).toStyledString(), json_params);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
                                if ( array[j].isNull() ){
                                        if (!strTmp.empty()){
                                                strTmp.push_back(',');
					}
                                        strTmp.append("null");
                                }
                                else if( array[j].isObject() ){
                                        parseJsonFlow( array[j].toStyledString(), json_params);
                                }else{
                                        if (!strTmp.empty()){
                                                strTmp.push_back(',');
					}
                                        strTmp.append(array[j].asString());
                                }
                        }
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }
			if(key[0]=='p'){
				pageFlow pf( (unsigned int)atoi( val.c_str() ) );
				json_params.push_back( pf );
			}
			else if(key[0]=='a'){
				if(json_params.size() > 0){	
					att_id = (unsigned int)atoi( val.c_str() ) ;	
					json_params[json_params.size()-1].atts.insert(pair<unsigned int,string>(att_id,""));
				}
			}
			else if(key[0]=='d'){
				if(json_params.size() > 0){
					itAtts = json_params[json_params.size()-1].atts.find(att_id);
					if( itAtts !=  json_params[json_params.size()-1].atts.end()){
						itAtts->second = val;
					}
				}
			}
                }
                it++;
        }
}


void parseJsonCase(const string & json, vector <Case> & cases){
        Json::Value root;
        Json::Reader reader;
        Json::Value::iterator it;
        bool parsed;
        string key,val;
        unsigned int j,i;

        parsed = reader.parse(json, root, false);

        if(!parsed){
                return;
        }

        it=root.begin();
        for (i=0; i < root.size() ; i++) {

		if( !(*it).isObject() ){
                       	key = it.key().asString();
		}

                if( (*it).isObject() ){
                        parseJsonCase( (*it).toStyledString(), cases);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {

                                if( array[j].isObject() ){
                                        parseJsonCase( array[j].toStyledString(), cases);
                                }else{
					cases[cases.size()-1].vals.push_back(array[j].asString());
                                }
                        }
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }
			if(key[0]=='t'){
				cases[cases.size()-1].type = val[0];
			}
			else if(key[0]=='n'){
				if(val[0] == 'f'){
					Case c(false);
					cases.push_back(c);
				}else{
					Case c(true);
					cases.push_back(c);
				}
				
			}
                }
                it++;
        }
}

void parseJSON(const string & json,string preKey,multimap<string,string> & json_params){
        Json::Value root;
        Json::Reader reader;
        Json::Value::iterator it;
        bool parsed;
        string key,strTmp,val;
        unsigned int j,i;

        parsed = reader.parse(json, root, false);

        if(!parsed){
                return;
        }

        it=root.begin();
        for (i=0; i < root.size() ; i++) {

                if(preKey.size()!=0){
                        key = preKey;
                        key.append("::");
                        key.append(it.key().asString());
                }else{
                        key = it.key().asString();
                }
		if ((*it).isNull())
		{
			json_params.insert(pair<string,string>(key,"null"));
		}
		else if( (*it).isObject() ){
                        parseJSON( (*it).toStyledString(), key,json_params);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
				if ( array[j].isNull() ){
					if (!strTmp.empty())
						strTmp.push_back(',');
					strTmp.append("null");
				}
				else if( array[j].isObject() ){
					parseJSON( array[j].toStyledString(), key,json_params);
				}else{
					if (!strTmp.empty())
						strTmp.push_back(',');	
                                       	strTmp.append(array[j].asString());
				}
                        }
                        json_params.insert(pair<string,string>(key,strTmp));
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }
                        json_params.insert(pair<string,string>(key,val));
                }
                it++;
        }
}

void bbb(const string & json,string preKey,multimap<string,string> & json_params){
        Json::Value root;
        Json::Reader reader;
        Json::Value::iterator it;
        bool parsed;
        string key,strTmp,val;
        unsigned int j,i;

	if(preKey.size()==0){
		if(json[0] == '['){
			string new_json = json;
			new_json.erase(0,1);
			new_json.erase(new_json.size()-1,1);
      			parsed = reader.parse(new_json, root, false);
			cout <<new_json<<endl;
		}else{
      			parsed = reader.parse(json, root, false);
		}
	}else{
      		parsed = reader.parse(json, root, false);
	}

	cout <<"11"<<endl;

        if(!parsed){
                return;
        }
	cout <<"111"<<endl;
        it=root.begin();


        for (i=0; i < root.size() ; i++) {

                if(preKey.size()!=0){
                        key = preKey;
                        key.append("::");
                        key.append(it.key().asString());
                }else{
                        key = it.key().asString();
                }
		if ((*it).isNull())
		{
			json_params.insert(pair<string,string>(key,"null"));
		}
		else if( (*it).isObject() ){
                        bbb( (*it).toStyledString(), key,json_params);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
				if ( array[j].isNull() ){
					if (!strTmp.empty())
						strTmp.push_back(',');
					strTmp.append("null");
				}
				else if( array[j].isObject() ){
					bbb( array[j].toStyledString(), key,json_params);
				}else{
					if (!strTmp.empty()){
						strTmp.push_back(',');	
					}

				        if ( array[j].isString() ){
cout <<array[j].asString()<<endl;
				                strTmp.append(array[j].asString());
				        }else{
cout <<array[j].toStyledString()<<endl;
						strTmp.append(array[j].toStyledString());
				        }

				}
                        }
                        json_params.insert(pair<string,string>(key,strTmp));
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }
                        json_params.insert(pair<string,string>(key,val));
                }
                it++;
        }
}

int main()
{
	//string str = "[{\"type\": \"IP\", \"negate\": false, \"value\": [\"111.111.111.111-222.222.222.222\",\"33.33.33.33\" ] }, {\"type\": \"country\", \"negate\": true,\"value\": [\"EU\",\"AD\"]},{\"type\": \"application\", \"negate\": false, \"value\": [\"321\",\"21\"] } ]";

//[{"type": "IP", "negate": false, "value": ["111.111.111.111-222.222.222.222","33.33.33.33" ] }, {"type": "country", "negate": true,"value": ["EU","AD"]},{"type": "application", "negate": false, "value": ["321","21"] } ]

	const char * json_str="[{\"jsonrpc\":\"2.0\",\"method\":\"AppStarLevelApi.getLevelToplist\",\"params\":[34,100],\"id\":8}]";

	//vector <Case> json_vec;

	//const char * data;
	//data = str.c_str();

	multimap<string,string> json_params2;

	bbb(json_str,"",json_params2);
	//parseJson(json_str,"",json_params2);
	
	/*for (unsigned int i=0;i<json_vec.size();i++)
	{
		cout<<"type:" <<json_vec[i].type <<endl;
		cout<<"neg1:"<<json_vec[i].neg <<endl;

		for ( unsigned int j=0;j<json_vec[i].vals.size();j++){
			cout<<"\t"<<json_vec[i].vals[j]<<endl;
		}
	}
	printf("done\n");
	*/




/*
	map <unsigned int,string>::iterator itr;
	for (unsigned int j=0;j<businessFlowVec.size();j++){
		cout<<businessFlowVec[j].bFlowID <<endl;
		for (unsigned int i=0;i<businessFlowVec[j].pages.size();i++)
		{
			cout<< "\t"<<businessFlowVec[j].pages[i].pageID <<endl;
			for ( itr = businessFlowVec[j].pages[i].atts.begin() ; itr != businessFlowVec[j].pages[i].atts.end() ; itr++){
				cout<<"\t\t"<<itr->first<<"="<<itr->second<<endl;
			}
		}
	}
*/


}
