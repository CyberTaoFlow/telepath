
#include <vector>
#include <string>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <pcre.h>

#include "Range.h"
#include "Session.h"

using namespace std;

int hashCode(string &);
void rangeParser(string &,unsigned short &,unsigned short &);

class Rule{
public:

	unsigned int criterion_hash;
	string rule_name;
	string method;
	unsigned int subdomain_id;
	string subdomain_name;
	unsigned int page_id;
	string page_name;
	unsigned int att_id;
	string att_name;
	string user;
	string category;
	string type;
	string subtype;
	unsigned short threshold;
	unsigned short count;
	string anchor;
	string anchor_app;
	string anchor_page;
	string anchor_param;
	string dynamic;
	string str_match;
	string str_length;
	unsigned short str_length_low;
	unsigned short str_length_high;
	unsigned short aggregate;
	bool negate;
	string ip_str;
	vector <Range> vecRangeIP;
	unsigned short ip_neg;
	double ts;
	unsigned short distance;
	double radius;
	unsigned short personal;
	char final_type;
	double pattern_ts;
	string action_name;
	unsigned int action_id;
	bool enable;
	pcre *pcreRegex;
	bool validReg;
	vector <string> cmds;
	unsigned short criteria_count;

	boost::unordered_map <string,boost::unordered_set<int> >mCardinal_IP_att;
	boost::unordered_map <unsigned int,boost::unordered_set<int> >mCardinal_SID_att;
	boost::unordered_map <string,unsigned short> mCardinal_IP_pid;
	boost::unordered_map <unsigned int,boost::unordered_set<int> >mCardinal_att_att;
	boost::unordered_map <string,boost::unordered_set<int> >mCardinal_IP_User;
	boost::unordered_map <unsigned int,boost::unordered_set<int> >mCardinal_SID_User;
	boost::unordered_set <char> att_types;
	boost::unordered_map <string,alert> mCardinal_IP_flow;



	//--------------NEW--------------
	Rule();
	void clean();
	void ipRangeParser(string &);
	void setCriterionType();
	void print();
	void print_syslog();
	//-------------------------------


};

/*class longTermRule{
public:
	string ip; 
	unsigned int attID;
	string data;
	unsigned short ruleCount;
	unsigned int ruleGroup;
	unsigned short ruleid;
	unsigned short threshold;
	Session SID;
	unsigned int pageid;
	long long RID;

	longTermRule();
	longTermRule(string &,unsigned int,string &,Rule &,Session &,unsigned int,long long);
};*/

class rule_group_data{
public:
	vector<unsigned short> rules;
	vector<unsigned int> pages;
	string 	pages_str;
	short flag;
	short case_status; // 1=open , 0=closed.

	unsigned short action_email;
	unsigned short action_log;
	unsigned short action_syslog;
	unsigned int action_injection;
	
	rule_group_data();
	void insert_actions(unsigned short &,unsigned short &,unsigned short &,unsigned int &);
	void print();

};
