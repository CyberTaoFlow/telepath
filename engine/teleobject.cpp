#include <boost/date_time/posix_time/posix_time.hpp>
#include "teleobject.h"

using namespace std;


using boost::posix_time::time_input_facet;
using std::locale;
const locale inputs[] = {
	locale(locale::classic(), new time_input_facet("%d/%m/%Y:%H:%M:%S")),
	locale(locale::classic(), new time_input_facet("%d/Jan%m/%Y:%H:%M:%S")),
	locale(locale::classic(), new time_input_facet("%Y-%m-%d %H:%M:%S")),
	locale(locale::classic(), new time_input_facet("%Y%m%d%H%M%S")),
	locale(locale::classic(), new time_input_facet("%Y%m%d%H%M")) };
const size_t formats = sizeof(inputs)/sizeof(inputs[0]);

boost::unordered_map <string,string>::iterator itMonth;

extern bool validMethod(string&);
extern void findAppID(string&,string&);
extern boost::unordered_map <string,string> months;



bool ifValidIP(string & ip){
	int counter=0;
	for(unsigned int i=0;i<ip.size();i++){
		if(ip[i]=='.'){
			counter++;
		}
	}

	if(counter==3){
		return true;
	}else{
		return false;
	}
}

void cutSuffix(string & url,string & gets){
	for(unsigned int i=0;i<url.size();i++){
		if(url[i]=='?'){
			gets.insert( gets.begin(),url.begin()+i,url.end() );
			url.erase(i,url.size());
		}
	}
}

string ptime_to_time_t(boost::posix_time::ptime t)
{
       char tmp[15];
       static boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
       sprintf(tmp,"%u", (unsigned int)((t-epoch).ticks() / boost::posix_time::time_duration::ticks_per_second()) );
       return tmp;
}


TeleObject::TeleObject()
{
}


TeleObject::TeleObject(const TeleObject &other)
{
	this->mParams=other.mParams;
	mAttr=other.mAttr;
}

TeleObject& TeleObject::operator=(const TeleObject &other)
{
	if(this==&other) return *this;

	this->mParams=other.mParams;
	mAttr=other.mAttr;

	return *this;
}

void TeleObject::clear()
{
	this->mAttr.clear();
	this->mParams.clear();
}

TeleObject::~TeleObject()
{}

bool TeleObject::parseApache(string & str,unsigned long long & att_id,string & domain){

	int mode=0,mode2;
	string newstr,month,gets,ext,ip,ua;
	struct Attribute tAttr;

	for(unsigned int i=0;i<str.size();i++){
		switch (mode){
			case 0:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					this->mParams.insert(pair<char,string>('a'/*UserIP*/,newstr));
					ip = newstr;
					newstr.clear();
					mode++;

					if( !ifValidIP(ip) ){
						return false;
					}
				}
				break;
			case 1:
				if(str[i]=='['){
					mode++;
				}
				break;
			case 2:


				if( month.size()<3 && ( (str[i]>='A' && str[i]<='Z') || (str[i]>='a' && str[i]<='z') ) ){
					month.push_back(str[i]);
				}
				if(str[i]!=']'){
					newstr.push_back(str[i]);
					if(month.size()==3){
						itMonth = months.find(month);
						newstr.insert(newstr.size(),itMonth->second);
						month.clear();
					}
				}else{
					for(size_t i=0; i<formats; ++i)
					{
						std::istringstream ss(newstr);
						ss.imbue(inputs[i]);
						boost::posix_time::ptime this_time;
						ss >> this_time;

						if(this_time != boost::posix_time::not_a_date_time){
							this->mParams.insert(pair<char,string>('b'/*TimeStamp*/, ptime_to_time_t(this_time) ));
							break;
						}
					}
					
					newstr.clear();
					mode++;
				}

				break;
			case 3:
				if(str[i]=='"'){
					mode++;
				}
				break;
			case 4:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					if( !validMethod(newstr) ){
						newstr.clear();
					}

					this->mParams.insert(pair<char,string>('h'/*Method*/,newstr));
					newstr.clear();
					mode++;
				}

				break;
			case 5:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					cutSuffix(newstr,gets);
					for(int i=newstr.size()-1 ; i>0 ; i--){
						if(newstr[i]=='.'){
							break;
						}else{
							ext.insert(ext.begin(),newstr[i]);
						}
					}

					ext.clear();
					this->mParams.insert(pair<char,string>('c'/*Page*/,newstr));

					mode2=0;
					string name,value;
					for(unsigned int ii=1;ii<gets.size();ii++){
						switch(mode2){
							case 0:
								if(gets[ii]!='='){
									if(gets[ii]!='&'){
										name.push_back(gets[ii]);
									}
								}else{
									mode2++;
								}
								break;
							case 1:
								if(gets[ii]!='&' && ii!=gets.size()-1 ){
									value.push_back(gets[ii]);
								}else{
									if(ii==gets.size()-1){
										value.push_back(gets[ii]);
									}
									mode2=0;
									tAttr.value=value;
									tAttr.name=name;
									tAttr.attribute_source='G';
									att_id--;
									this->mAttr.insert(pair<unsigned long long, struct Attribute>(att_id,tAttr));
									name.clear();
									value.clear();
								}
								break;
						}
					}

					newstr.clear();
					gets.clear();
					mode++;
				}

				break;
			case 6:
				if(str[i]!='/'){
					newstr.push_back(str[i]);
				}else{
					if(newstr.size()>5){
						newstr.clear();
					}

					this->mParams.insert(pair<char,string>('i'/*Protocol*/,newstr));
					newstr.clear();
					mode++;
				}

				break;
			case 7:
				if(str[i]==' '){
					mode++;
				}
				break;
			case 8:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					this->mParams.insert(pair<char,string>('d'/*StatusCode*/,newstr));
					newstr.clear();
					mode++;
				}

				break;
			case 9:
				if(str[i]=='"'){
					mode++;
				}
				break;
			case 10:
				if(str[i]!='"'){
					newstr.push_back(str[i]);
				}else{
					tAttr.value=newstr;
					tAttr.name="referer";
					tAttr.attribute_source='H';
					att_id--;
					this->mAttr.insert(pair<unsigned long long, struct Attribute>(att_id,tAttr));
					newstr.clear();
					mode++;
				}

				break;
			case 11:
				if(str[i]=='"'){
					mode++;
				}
				break;
			case 12:
				if(str[i]!='"'){
					newstr.push_back(str[i]);
				}else{
					tAttr.value=newstr;
					tAttr.name="user-agent";
					tAttr.attribute_source='H';
					att_id--;
					this->mAttr.insert(pair<unsigned long long, struct Attribute>(att_id,tAttr));
					ua = newstr;
					newstr.clear();
					mode++;
				}

				break;

		}
	}

	if(this->mParams['b'/*TimeStamp*/].size()==0){
		return false;
	}

	if(this->mParams['f'/*App*/].size()==0){
		string appid;
		this->mParams.insert(pair<char,string>('f'/*App*/,domain));
		findAppID(domain,appid);
		this->mParams.insert(pair<char,string>('g'/*AppID*/,appid));
	}

	return true;

}

bool TeleObject::parseIIS(string & str,unsigned long long & att_id,string & domain){

	int mode=0,mode2;
	string newstr,month,gets,ext,ip,ua;
	struct Attribute tAttr;

	for(unsigned int i=0;i<str.size();i++){
		switch (mode){
			case 0:
				if(str[i]==' '){
					mode++;
				}
				newstr.push_back(str[i]);

				break;
			case 1:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					for(size_t i=0; i<formats; ++i)
					{
						std::istringstream ss(newstr);
						ss.imbue(inputs[i]);
						boost::posix_time::ptime this_time;
						ss >> this_time;

						if(this_time != boost::posix_time::not_a_date_time){
							this->mParams.insert(pair<char,string>('b'/*TimeStamp*/, ptime_to_time_t(this_time) ));
							break;
						}
					}
					
					newstr.clear();
					mode++;
				}
				break;
			case 2:
				if(str[i]==' '){
					mode++;
				}
				break;
			case 3:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					if( !validMethod(newstr) ){
						newstr.clear();
					}
					this->mParams.insert(pair<char,string>('h'/*Method*/,newstr));
					newstr.clear();
					mode++;
				}
				break;
			case 4:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					cutSuffix(newstr,gets);
					for(int i=newstr.size()-1 ; i>0 ; i--){
						if(newstr[i]=='.'){
							break;
						}else{
							ext.insert(ext.begin(),newstr[i]);
						}
					}

					ext.clear();
					this->mParams.insert(pair<char,string>('c'/*Page*/,newstr));

					mode2=0;
					string name,value;
					for(unsigned int ii=1;ii<gets.size();ii++){
						switch(mode2){
							case 0:
								if(gets[ii]!='='){
									name.push_back(gets[ii]);
								}else{
									mode2++;
								}
								break;
							case 1:
								if(gets[ii]!='&' && ii!=gets.size()-1 ){
									value.push_back(gets[ii]);
								}else{
									if(ii==gets.size()-1){
										value.push_back(gets[ii]);
									}
									mode2=0;
									tAttr.value=value;
									tAttr.name=name;
									tAttr.attribute_source='G';
									att_id--;
									this->mAttr.insert(pair<unsigned long long, struct Attribute>(att_id,tAttr));
									name.clear();
									value.clear();
								}
								break;
						}
					}

					newstr.clear();
					gets.clear();
					mode++;
				}
				break;
			case 5:
				if(str[i]==' '){
					mode++;
				}
				break;
			case 6:
				if(str[i]==' '){
					mode++;
				}
				break;
			case 7:
				if(str[i]==' '){
					mode++;
				}
				break;
			case 8:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					this->mParams.insert(pair<char,string>('a'/*UserIP*/,newstr));
					ip = newstr;
					newstr.clear();
					mode++;

					if( !ifValidIP(ip) ){
						return false;
					}
				}
				break;
			case 9:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					tAttr.value=newstr;
					tAttr.name="user-agent";
					tAttr.attribute_source='H';
					att_id--;
					this->mAttr.insert(pair<unsigned long long, struct Attribute>(att_id,tAttr));
					ua = newstr;
					newstr.clear();
					mode++;
				}
				break;
			case 10:
				if(str[i]!=' '){
					newstr.push_back(str[i]);
				}else{
					this->mParams.insert(pair<char,string>('d'/*StatusCode*/,newstr));
					newstr.clear();
					mode++;
				}
				break;
		}

	}

	if(this->mParams['b'/*TimeStamp*/].size()==0){
		return false;
	}

	this->mParams.insert(pair<char,string>('i'/*Protocol*/,"http"));
	if(this->mParams['f'/*App*/].size()==0){
		string appid;
		this->mParams.insert(pair<char,string>('f'/*App*/,domain));
		findAppID(domain,appid);
		this->mParams.insert(pair<char,string>('g'/*AppID*/,appid));
	}

	return true;

}


