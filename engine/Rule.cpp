#include "Rule.h"
#include <iostream>
#include <stdio.h>
#include <syslog.h>

using namespace std;


int hashCode(string & str)
{
	boost::hash<std::string> string_hash;
	return string_hash(str);
}


// Get a (-) separated string and convert it to a range.
void rangeParser(string & str,unsigned short & low ,unsigned short & high ){ 
	string diff;
	unsigned int flag=0;

	if(str.size()>10 || str.size()==0){
		low=1;
		high=0;
		return;
	}

	for( unsigned short i=0 ; i < str.size() ; i++){
		if(str[i]>57 || str[i]<48 ){
			if( str[i]!=45){
				low=1;
				high=0;
				return;
			}
		}
		if( str[i]<=57 && str[i]>=48){
			diff.push_back(str[i]);
		}
		if( str[i]==45 || i==str.size()-1){	 // sign (-)	
			if(flag==0){
				low = (unsigned short)atoi( diff.c_str() );
				diff.clear();
			}
			else if(flag==1){
				high = (unsigned short)atoi( diff.c_str() );
			}
			flag++;
		}
	}
	if(flag==1){
		high=low;
	}	
}

Rule::Rule(){
	this->subdomain_name.clear();
	this->criterion_hash = 0;
	this->rule_name.clear();
	this->ip_str.clear();
	this->subdomain_id=0;
	this->count=0;
	this->page_id=0;
	this->att_id=0;
	this->pattern_ts=0;
	this->ts=0;
	this->ip_neg=0;
	this->distance=0;
	this->radius=0;
	this->enable=true;
	this->action_id=0;
	this->action_name.clear();
	this->validReg=false;
	this->disable_db_save=false;
	this->threshold = 0;
	this->cmds.clear();
	this->script_params.clear();
	this->criteria_count=0;
}

void Rule::clean(){
	this->criterion_hash=0;
	this->rule_name.clear();
	this->method.clear();
	this->subdomain_id=0;
	this->subdomain_name.clear();
	this->page_id=0;
	this->page_name.clear();
	this->att_id=0;
	this->att_name.clear();
	this->user.clear();
	this->category.clear();
	this->type.clear();
	this->subtype.clear();
	this->count=0;
	this->anchor.clear();
	this->anchor_app.clear();
	this->anchor_page.clear();
	this->anchor_param.clear();
	this->dynamic.clear();
	this->str_match.clear();
	this->str_length.clear();
	this->str_length_low=0;
	this->str_length_high=0;
	this->negate=false;
	this->ip_str.clear();
	this->vecRangeIP.clear();
	this->ip_neg=0;
	this->ts=0;
	this->distance=0;
	this->radius=0;
	this->personal=0;
	this->pattern_ts=0;
	this->action_id=0;
	this->action_name.clear();
	this->enable=true;
	this->validReg=false;
	this->disable_db_save=false;
	this->mCardinal_IP_att.clear();
	this->mCardinal_SID_att.clear();
	this->mCardinal_IP_pid.clear();
	this->mCardinal_att_att.clear();
	this->mCardinal_IP_User.clear();
	this->mCardinal_SID_User.clear();
	this->att_types.clear();
	this->mCardinal_IP_flow.clear();
	this->threshold = 0;
	this->cmds.clear();
	this->script_params.clear();
	this->criteria_count=0;
	this->domain_block.clear();

}

void Rule::ipRangeParser(string & strIP){
	if(strIP.size() != 0){
		unsigned int end=0;
		for(unsigned int i=0;i<strIP.size();i++){
			if( (strIP[i] == ',') || (i == strIP.size()-1) ){
				string tmpIP,from,to;
				if(i == strIP.size()-1){
					tmpIP.assign(strIP.begin()+end,strIP.begin()+i+1);
				}else{
					tmpIP.assign(strIP.begin()+end,strIP.begin()+i);
				}
				end = i+1;

				if( ipSingleOrRange(tmpIP,from,to) == false ){
					unsigned int IP = ipToNum(tmpIP);
					Range rangeip( IP,IP );
					this->vecRangeIP.push_back(rangeip);
				}else{
					Range rangeip( ipToNum(from),ipToNum(to) );
					this->vecRangeIP.push_back(rangeip);
				}
			}
		}
	}
}

void Rule::setCriterionType(){
	string fullname;
	this->ipRangeParser(this->ip_str);
	rangeParser(this->str_length,this->str_length_low,this->str_length_high);
	const char *error;
	int erroroffset;

	switch(this->category[0]){
		case 'b':
			this->method=this->type;
			this->final_type='X';
			break;
		case 'B':
			this->final_type='B';
			break;
		case 'g':
			if(this->type[0]=='i'){//inside
				this->negate=false;
			}
			else if(this->type[0]=='o'){//outside
				this->negate=true;
			}
			this->final_type='G';
			break;
		case 'p':
			if(this->subtype=="parameter"){
				this->anchor=this->type;
				this->final_type='p';
				if(this->anchor[0]=='O'){
					fullname.clear();
					fullname = this->anchor_app + this->anchor_page + this->anchor_param;
					char tmp[50];
					sprintf(tmp,"%u",(unsigned int)hashCode(fullname));
					this->anchor=tmp;

					this->anchor_app.clear();
					this->anchor_page.clear();
					this->anchor_param.clear();
				}
				fullname.clear();
				fullname = this->subdomain_name + this->page_name + this->att_name;
				this->att_id = (unsigned int)hashCode(fullname);

				this->att_name.clear();
			}
			else if(this->subtype=="page"){
				this->anchor=this->type;
				this->final_type='P';
				fullname.clear();
				fullname = this->subdomain_name + this->page_name;
				this->page_id = (unsigned int)hashCode(fullname);

			}
			else if(this->subtype=="user"){
				this->anchor=this->type;
				this->dynamic="User";
				this->final_type='X';
			}

			else if(this->subtype=="action"){
				fullname.clear();
				this->anchor=this->type;
				this->final_type='F';
				fullname = this->subdomain_name + this->action_name;
				this->action_id = (unsigned int)hashCode(fullname);
				this->action_name.clear();
			}
			break;
		case 'P':
			switch(this->subtype[0]){
				case 'r':
					if(this->subtype=="regex"){
						this->final_type='g';

						this->pcreRegex = pcre_compile((char*)this->str_match.c_str(), 0, &error, &erroroffset, NULL);
						if (this->pcreRegex != NULL) {
							this->validReg=true;
						}else{
							this->validReg=false;
						}
					}
					else if(this->subtype=="rangelength"){
						this->final_type=this->subtype[0];
					}
					break;
				case 'c':
					rangeParser(this->str_match,this->str_length_low,this->str_length_high);
					this->final_type=this->subtype[0];
					break;
				case 'd':
				case 'e':
				case 'f':
				case 'h':
				case 's':
				case 'N':
					this->final_type=this->subtype[0];
					break;

			}
			if(this->subdomain_name.size() != 0){
				fullname.clear();
				fullname = this->subdomain_name + this->page_name;
				this->page_id = (unsigned int)hashCode(fullname);
				fullname.append(this->att_name);
				this->att_id = (unsigned int)hashCode(fullname);
				this->att_name.clear();
			}

			break;
	}
	this->subdomain_name.clear();
	this->page_name.clear();
	this->subtype.clear();
	this->category.clear();
	this->type.clear();
	//Range tempRange;
	//if(!this->ip_block_to.empty){
	//	tempRange.Range(ip_block_to,ip_block_from);
	//	this->vecRangeIP.insert(vecRangeIP.size()+1,tempRange);
	//}
	//this->print_syslog();
}

void Rule::print(){
	unsigned int i;
	cout << "criterion_hash:" << this->criterion_hash;
	cout << "\nrule_name:" << this->rule_name;
	cout << "\nmethod:" << this->method;
	cout << "\nsubdomain_id:" << this->subdomain_id;
	cout << "\nsubdomain_name:" << this->subdomain_name;
	cout << "\npage_id:" << this->page_id;
	cout << "\npage_name:" << this->page_name;
	cout << "\natt_id:" << this->att_id;
	cout << "\natt_name:" << this->att_name;
	cout << "\nuser:" << this->user;
	cout << "\ncategory:" << this->category;
	cout << "\ntype:" << this->type;
	cout << "\nsubtype:" << this->subtype;
	cout << "\nthreshold:" << this->threshold;
	cout << "\ncount:" << this->count;
	cout << "\nanchor:" << this->anchor;
	cout << "\nanchor_app:" << this->anchor_app;
	cout << "\nanchor_page:" << this->anchor_page;
	cout << "\nanchor_param:" << this->anchor_param;
	cout << "\ndynamic:" << this->dynamic;
	cout << "\nstr_match:" << this->str_match;

	cout << "\nstr_length:" << this->str_length;
	cout << "\nstr_length_low:" << this->str_length_low;
	cout << "\nstr_length_high:" << this->str_length_high;
	cout << "\naggregate:" << this->aggregate;
	cout << "\nnegate:" << this->negate;
	for(i=0;i<this->vecRangeIP.size();i++){
		cout << "\n\tvecRangeIP:";
		this->vecRangeIP[i].print();
	}
	cout << "\nip_neg:" << this->ip_neg;
	cout << "\ntime:" << this->ts;
	cout << "\ndistance:" << this->distance;
	cout << "\nradius:" << this->radius;
	cout << "\npersonal:" << this->personal;
	cout << "\nfinal_type:" << this->final_type;
	cout << "\npattern_ts:" << this->pattern_ts;
	cout << "\naction_id:" << this->action_id;
	cout << "\naction_name:" << this->action_name;
	cout << "\nvalidReg:" << this->validReg;
	cout << "\n-------------------------" << endl;
}

void Rule::print_ruleCmd(){

		syslog(LOG_NOTICE,"CMDS PRINT: rule_name:%s",this->rule_name.c_str());
		for(unsigned int i=0;i<this->cmds.size();i++){
			syslog(LOG_NOTICE,"cmds: %s",this->cmds[i].c_str());
		}

}

void Rule::print_ruleParams(){
		syslog(LOG_NOTICE,"Params PRINT: rule_name:%s",this->rule_name.c_str());
		for(unsigned int i=0;i<this->script_params.size();i++){
			syslog(LOG_NOTICE,"params: %s",this->script_params[i].c_str());
		}
		for(unsigned int i=0;i<this->script_path.size();i++){
			syslog(LOG_NOTICE,"path: %s",this->script_path[i].c_str());
		}
}

void Rule::print_syslog(){
	unsigned int i;
	syslog(LOG_NOTICE,"criterion_hash:%u",this->criterion_hash);
	syslog(LOG_NOTICE,"rule_name:%s",this->rule_name.c_str());
	syslog(LOG_NOTICE,"method:%s",this->method.c_str());
	syslog(LOG_NOTICE,"subdomain_id:%u",this->subdomain_id);
	syslog(LOG_NOTICE,"subdomain_name:%s",this->subdomain_name.c_str());
	syslog(LOG_NOTICE,"page_id:%u",this->page_id);
	syslog(LOG_NOTICE,"page_name:%s",this->page_name.c_str());
	syslog(LOG_NOTICE,"att_id:%u",this->att_id);
	syslog(LOG_NOTICE,"user:%s",this->user.c_str());
	syslog(LOG_NOTICE,"category:%s",this->category.c_str());
	syslog(LOG_NOTICE,"type:%s",this->type.c_str());
	syslog(LOG_NOTICE,"subtype:%s",this->subtype.c_str());
	syslog(LOG_NOTICE,"threshold:%hu",this->threshold);
	syslog(LOG_NOTICE,"count:%hu",this->count);
	syslog(LOG_NOTICE,"anchor:%s",this->anchor.c_str());
	syslog(LOG_NOTICE,"anchor_app:%s",this->anchor_app.c_str());
	syslog(LOG_NOTICE,"anchor_page:%s",this->anchor_page.c_str());
	syslog(LOG_NOTICE,"anchor_param:%s",this->anchor_param.c_str());
	syslog(LOG_NOTICE,"dynamic:%s",this->dynamic.c_str());
	syslog(LOG_NOTICE,"str_match:%s",this->str_match.c_str());
	syslog(LOG_NOTICE,"str_length:%s",this->str_length.c_str());
	syslog(LOG_NOTICE,"str_length_low:%hu",this->str_length_low);
	syslog(LOG_NOTICE,"str_length_high:%hu",this->str_length_high);
	syslog(LOG_NOTICE,"aggregate:%hu",this->aggregate);
	syslog(LOG_NOTICE,"negate:%u",(unsigned)this->negate);
	syslog(LOG_NOTICE,"ip_str:%s",this->ip_str.c_str());
	for(i=0;i<this->vecRangeIP.size();i++){
		syslog(LOG_NOTICE,"vecRangeIP: ");
		this->vecRangeIP[i].print_syslog();
	}
	syslog(LOG_NOTICE,"ip_neg:%hu",this->ip_neg);
	syslog(LOG_NOTICE,"time:%f",this->ts);
	syslog(LOG_NOTICE,"distance:%hu",this->distance);
	syslog(LOG_NOTICE,"radius:%f",this->radius);
	syslog(LOG_NOTICE,"personal:%hu",this->personal);
	syslog(LOG_NOTICE,"final_type:%c",this->final_type);
	syslog(LOG_NOTICE,"pattern_ts:%f",this->pattern_ts);
	syslog(LOG_NOTICE,"action_id:%u",this->action_id);
	syslog(LOG_NOTICE,"action_name:%s",this->action_name.c_str());
	syslog(LOG_NOTICE,"validReg:%u",(unsigned)this->validReg);
	syslog(LOG_NOTICE,"this->criteria_count:%hu",this->criteria_count);
	syslog(LOG_NOTICE,"Domian to block:%s",this->domain_block.c_str());
	syslog(LOG_NOTICE,"------------------------------------------------------");
}


/*longTermRule::longTermRule(){}
longTermRule::longTermRule(string & ip,unsigned int aid,string & data,Rule & rule,Session & SID,unsigned int pageid,long long RID){
	this->ip = ip;
	this->attID = aid;
	this->data = data;
	this->ruleCount = rule.count;
	this->ruleGroup = rule.group;
	this->ruleid = rule.ID;
	this->threshold = rule.Threshold;
	this->SID = SID;
	this->pageid = pageid;
	this->RID = RID;
}*/

	
rule_group_data::rule_group_data(){
	this->flag=0;
	this->case_status=0;
}

void rule_group_data::insert_actions(unsigned short & email_,unsigned short & log_,unsigned short & syslog_,unsigned int & injection_){
	this->action_email = email_;
	this->action_log = log_;
	this->action_syslog = syslog_;
	this->action_injection = injection_;
}

void rule_group_data::print(){
	unsigned int i;
	cout<<"\nRules:";
	for(i=0;i<this->rules.size();i++){
		cout << "\t" <<this->rules[i] ;
	}
	cout<<endl;
}

