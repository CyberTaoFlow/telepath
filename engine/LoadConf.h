using namespace std;

void getAttType(string & str,boost::unordered_set<char> & types){
	if(str.size() != 0){
		if( str[0] >='G' && str[0] <='X' ){
			types.insert(str[0]);
			for(unsigned int i=1;i<str.size();i++){
				if( str[i] >='G' && str[i] <='X' ){
					types.insert(str[i]);
				}	
			}
		}
	}

}

void parseRulesJson(Json::Value & root,string & preKey,map <string,rule_group_data> & mRulesGroup,vector <Rule> & rules,Rule & rule,string & rule_name,vector <unsigned int> & rule_orders){

        Json::Value::iterator it;
        string key,strTmp,val,key2;
        unsigned int i,j,k;

        it=root.begin();
        for (i=0; i < root.size() ; i++) {

                if(preKey.size()!=0){
                        key2 = preKey;
                        key2.append(it.key().asString());
                }else{
                        key2 = it.key().asString();
                }

		if( !(*it).isObject() ){
                       	key = it.key().asString();
		}

                if( (*it).isObject() ){
                        parseRulesJson( (*it),key2,mRulesGroup,rules,rule,rule_name,rule_orders);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
                                if( array[j].isObject() ){
					preKey = it.key().asString();
                                        parseRulesJson( array[j],key2,mRulesGroup,rules,rule,rule_name,rule_orders);
                                }
				else if(array[j].isString()){
					if(key=="cmd"){
						val = array[j].asString();
						rule.cmds.push_back(val);
					}
				}
                        }
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }
			
			//syslog(LOG_NOTICE,"preKey:%s  key:%s  val:%s",preKey.c_str(),key.c_str(),val.c_str());

			if(preKey=="criteria"){
				switch(key[0]){
					case 'a':
						if(key=="aggregate"){
							rule.aggregate = (unsigned short)atoi(&val[0]);
							if(rule.criterion_hash != 0 ){
								if(rule.enable==true){
									rule_orders.push_back((unsigned int)rules.size());
									rules.push_back(rule);
								}
								rule.clean();
							}

							rule.criterion_hash = rules.size()+1;
						}
						else if(key=="action_name"){
							rule.action_name = val;
						}
						break;
					case 'c':
						rule.count = (unsigned short)atoi(&val[0]);
						break;
					case 'd':
						if(key=="domain"){
							rule.subdomain_name = val;
						}
						else if(key=="distance"){
							rule.distance = (unsigned short)atoi(&val[0]);
						}
						else if(key=="disable_db_save"){
							if(val[0]=='f'){
								rule.disable_db_save=false;
							}else{
								rule.disable_db_save=true;
							}
						}
						break;
					case 'e':
						if(key=="enable"){
							if(val[0]=='t'){
								rule.enable = true;
							}else{
								rule.enable = false;
							}
						}
						break;
					case 'k':
						rule.category = val;
						break;
					case 'I':
						rule.ip_str = val;
						break;
					case 'l':
						if(key=="location"){
							rule.str_match = val;
						}
						else if(key=="length"){
							rule.str_length = val;
						}
						break;
					case 'm':
						rule.method = val;
						getAttType(rule.method,rule.att_types);
						break;
					case 'n':
						if(key=="negate"){
							if(val[0]=='t'){
								rule.negate = true;
							}else{
								rule.negate = false;
							}
						}
						else if(key=="name"){
							rule_name = val;
						}
						break;
					case 'p':
						if(key=="personal"){
							if(val[0]=='t'){
								rule.personal = true;
							}else{
								rule.personal = false;
							}
						}
						else if(key=="pagename"){
							rule.page_name = val;
						}
						else if(key=="paramname"){
							rule.att_name = val;
						}

						break;
					case 'r':
						rule.radius = (double)atof(&val[0]);
						break;
					case 's':
						if(key=="str_match"){	
							rule.str_match = val;
							for(unsigned int i_lower=0;i_lower<rule.str_match.size();i_lower++){
								if(rule.str_match[i_lower]>64 && rule.str_match[i_lower]<91){
									rule.str_match[i_lower]+=32;
								}
							}
						}
						else if(key=="subtype"){
							rule.subtype = val;
						}
						else if(key=="score"){
							rule.threshold = (unsigned short)atoi(&val[0]);
							if(rule.enable==true){
								rule_orders.push_back((unsigned int)rules.size());
								rules.push_back(rule);


								for(k=0;k<rule_orders.size();k++){
									rules[rule_orders[k]].threshold = rule.threshold;
									rules[rule_orders[k]].criteria_count = (unsigned short)rule_orders.size();
									rules[rule_orders[k]].rule_name = rule_name;
								}
								rule_orders.clear();
								rule.clean();
							}
						}
						break;
					case 't':
						if(key=="type"){
							rule.type = val;
						}
						else if(key=="time"){
							rule.ts = (double)atof(&val[0]);
						}
						break;
					case 'u':
						rule.user = val;
						break;
				}
			}

			else if(preKey=="criteriaspecific"){
				if(key=="domain"){
					rule.subdomain_name = val;
				}
				else if(key=="pagename"){
					rule.page_name = val;
				}
				else if(key=="paramname"){
					rule.att_name = val;
				}
			}

			else if(preKey=="criteriainclusive"){
				rule.att_name = val;
			}

			else if(preKey=="criteriaOther"){
				if(key=="domain"){
					rule.anchor_app = val;
				}
				else if(key=="pagename"){
					rule.anchor_page = val;
				}
				else if(key=="paramname"){
					rule.anchor_param = val;
				}
			}

                }
                it++;
        }

}

void getRule(char *ptr)
{

	Json::Value root;
	Json::Reader reader;
	bool parsed;

	string rule_json,prekey,rule_name;
	rule_name.clear();
	prekey.clear();

	Rule rule;
	vector <unsigned int> vec_num;
	vec_num.clear();
	if(ptr>0){
		if(ptr != NULL){
			rule_json = string(ptr);
			parsed = reader.parse(rule_json, root, false);

			if(!parsed){
				return;
			}

			parseRulesJson(root,prekey,mRulesGroup,rules,rule,rule_name,vec_num);
		}
	}
}

void parseRuleIDsJson(Json::Value & root,vector <string> & ids){
        Json::Value::iterator it;
        unsigned int i,j;
	string key,val;

        it=root.begin();
        for (i=0; i < root.size() ; i++) {
		if( !(*it).isObject() ){
                       	key = it.key().asString();
		}

                if( (*it).isObject() ){
                        parseRuleIDsJson((*it),ids);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
                                if( array[j].isObject() ){
                                        parseRuleIDsJson(array[j],ids);
                                }else{
				        if ( array[j].isString() ){
				                val = array[j].asString();
				        }else{
				                val = array[j].toStyledString();
				        }

					if(key == "hash"){
						pthread_mutex_lock(&mutexHashRules);
						setHashRules.insert((unsigned int)atoi(&val[0]));
						pthread_mutex_unlock(&mutexHashRules);
					}
				}
                        }
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }
			
			if(key == "_id"){
				ids.push_back(val);
			}

                }
                it++;
        }

}

void addRules()
{
	if(addRulesIDs.size() == 0){
		return;
	}

	CURL *curl;
	char url[200];
	unsigned int i,size;

	for(i=0;i<addRulesIDs.size();i++){
		sprintf(url,"%s/telepath-rules/rules/X%u/_source",es_connect.c_str(),addRulesIDs[i]);

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"GET");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getRule);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	size = (unsigned int)( rules.size() - addRulesIDs.size() );

	for(i=size;i<rules.size();i++){
		rules[i].setCriterionType();
		mRulesGroup[rules[i].rule_name].rules.push_back(rules[i].criterion_hash);

	}
}

void getRulesIDs(char *ptr)
{
	CURL *curl;
	vector <string> ids;
 	string rule_json;
	char url[200];

	Json::Value root;
	Json::Reader reader;
	bool parsed;
	unsigned int i;

	ids.clear();

	if(ptr>0){
		if(ptr != NULL){
			rule_json = string(ptr);
			parsed = reader.parse(rule_json, root, false);

			if(!parsed){
				return;
			}

			parseRuleIDsJson(root,ids);
		}
	}

	if(ids.size()==0){
		noRulesToLoad = true;
		return;
	}

	for(i=0;i<ids.size();i++){
		sprintf(url,"%s/telepath-rules/rules/%s/_source",es_connect.c_str(),ids[i].c_str());
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"GET");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getRule);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	for(i=numRulesRotate*LOAD_RULES_BULK;i<rules.size();i++){
		rules[i].setCriterionType();
		mRulesGroup[rules[i].rule_name].rules.push_back(rules[i].criterion_hash);

		//rules[i].print();

	}

	/*map <string,rule_group_data>::iterator itRules;
	for(itRules = mRulesGroup.begin(); itRules != mRulesGroup.end();itRules++){
		cout << itRules->first<<":";
		itRules->second.print_syslog();
	}*/
}

void loadRules(){ // Load rule_groups from database.
	string output;

	if(rules_table_was_changed==0){
		es_get_config("/telepath-config/config/rules_table_was_changed_id/_source",output);
		rules_table_was_changed = (unsigned short)atoi(output.c_str());
	}

	pthread_mutex_lock(&mutexAddRules);
	if(rules_table_was_changed==0){
		addRules();
		addRulesIDs.clear();
		pthread_mutex_unlock(&mutexAddRules);
		return;
	}else{
		addRulesIDs.clear();
	}

	//---------Clean Old Rules---------
	for(unsigned int i_rules=0;i_rules < rules.size();i_rules++){
		if ( rules[i_rules].final_type == 'g' ){ // g = regex match.
			 pcre_free(rules[i_rules].pcreRegex);
		}
	}
	rules.clear();
	mRulesGroup.clear();
	rules_table_was_changed=0;
	es_insert("/telepath-config/config/rules_table_was_changed_id","{\"value\":\"0\"}");
	//---------------------------------
	CURL *curl;
	char postfields[100],url[200];
	unsigned int from=0;
	numRulesRotate=0;
	while(1){
		sprintf(url,"%s/telepath-rules/rules/_search?filter_path=hits.hits.fields,hits.hits._id",es_connect.c_str());
		sprintf(postfields,"{\"fields\":[\"hash\"],\"size\":%u,\"from\":%u}",LOAD_RULES_BULK,from);
		from += LOAD_RULES_BULK;

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"POST");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS,postfields);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getRulesIDs);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		numRulesRotate++;
		if(noRulesToLoad == true){
			noRulesToLoad = false;
			break;
		}

	}
	pthread_mutex_unlock(&mutexAddRules);
}

void parseActionsJson(Json::Value & root,string & preKey,vector <Action> & businessFlowVec,Action & bf,ActionPages & pf,string & fullpage,string & data){
        Json::Value::iterator it;
        unsigned int i,j;
	string key,val,key2,tmp_full;

        it=root.begin();
        for (i=0; i < root.size() ; i++) {

		key2 = it.key().asString();

		if( !(*it).isObject() ){
                       	key = it.key().asString();
		}

                if( (*it).isObject() ){
                        parseActionsJson((*it),key2,businessFlowVec,bf,pf,fullpage,data);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
				if( array[j].isObject() ){
					preKey = it.key().asString();
                                        parseActionsJson(array[j],key2,businessFlowVec,bf,pf,fullpage,data);
                                }
                        }
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }


			//cout << "\tpreKey:" <<preKey<< "\tkey:" <<key<< "\tval:" <<val<< endl;

			if(preKey.size()==0){
				if(key=="_type"){
					bf.pages.push_back(pf);
					tmp_full = bf.domain + bf.action_name;
					bf.action_id = (unsigned int)hashCode(tmp_full);
					businessFlowVec.push_back(bf);
					bf.clean();
					pf.clean();
				}
			}else{
				switch(key[0]){
					case 'a':
						if(key=="action_name"){
							if(bf.action_name.size() != 0){
								bf.pages.push_back(pf);
								tmp_full = bf.domain + bf.action_name;
								bf.action_id = (unsigned int)hashCode(tmp_full);

								bf.clean();
								pf.clean();
							}

							bf.action_name=val;
						}
						else if(key=="application"){
							bf.domain = val;
//cout << "fullpage0:"<<bf.domain << endl;				
						}
						break;
					case 'd':
						data = val;
						break;
					case 'n': // name
						tmp_full=fullpage+val;
//cout << "fullpage2:"<<tmp_full << " === "<<(unsigned int)hashCode(tmp_full)<< ":" << data << endl;

						pf.params[(unsigned int)hashCode(tmp_full)] = data;
						break;
					case 'p':
						if(preKey=="params"){
							bf.pages.push_back(pf);
							pf.params.clear();
						}
						fullpage = bf.domain;
						fullpage.append(val);
						pf.pageID = (unsigned int)hashCode(fullpage);

//cout << "fullpage1:"<<fullpage << " == "<< pf.pageID << endl;


						break;
				}
			}
		}
		it++;
        }

}

void getAction(char *ptr)
{
	string prekey;
	prekey.clear();
	string action_json;
	Action bf;
	ActionPages pf;
	string fullpage,data;
	fullpage.clear();
	data.clear();

	Json::Value root;
	Json::Reader reader;
	bool parsed;

	if(ptr>0){
		if(ptr != NULL){
			action_json = string(ptr);
			parsed = reader.parse(action_json, root, false);

			if(!parsed){
				return;
			}

			parseActionsJson(root,prekey,businessFlowVec,bf,pf,fullpage,data);
		}
	}
}

void addActions()
{
	if(addActionIDs.size() == 0){
		return;
	}

	CURL *curl;
	char url[200];

	unsigned int i;

	for(i=0;i<addActionIDs.size();i++){
		sprintf(url,"%s/telepath-actions/actions/X%u",es_connect.c_str(),addActionIDs[i]);
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"GET");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getAction);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
}

void getActionIDs(char *ptr)
{
	CURL *curl;
	vector <string> ids;
	string action_json;
	char url[200];

	Json::Value root;
	Json::Reader reader;
	bool parsed;
	unsigned int i;

	ids.clear();

	if(ptr>0){
		if(ptr != NULL){
			action_json = string(ptr);
			parsed = reader.parse(action_json, root, false);

			if(!parsed){
				return;
			}

			parseRuleIDsJson(root,ids);
		}
	}

	if(ids.size()==0){
		noRulesToLoad = true;
		return;
	}
	for(i=0;i<ids.size();i++){
		sprintf(url,"%s/telepath-actions/actions/%s",es_connect.c_str(),ids[i].c_str());
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"GET");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getAction);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

}

void loadActions(){ // Load business logic from database.
	string output;

	if(business_flow_was_changed==0){
		es_get_config("/telepath-config/config/business_flow_was_changed_id/_source",output);
		business_flow_was_changed = (unsigned short)atoi(output.c_str());
	}

	pthread_mutex_lock(&mutexAddActions);
	if(business_flow_was_changed==0){
		addActions();
		addActionIDs.clear();
		pthread_mutex_unlock(&mutexAddActions);
		return;
	}else{
		addActionIDs.clear();
	}

	//---------Clean Old Actions---------
	businessFlowVec.clear();
	business_flow_was_changed=0;
	es_insert("/telepath-config/config/business_flow_was_changed_id","{\"value\":\"0\"}");
	//-----------------------------------

	CURL *curl;
	char postfields[100],url[200];
	unsigned int from=0;
	while(1){
		sprintf(url,"%s/telepath-actions/actions/_search?filter_path=hits.hits._id",es_connect.c_str());
		sprintf(postfields,"{\"size\":%u,\"from\":%u}",LOAD_RULES_BULK,from);
		from += LOAD_RULES_BULK;

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"POST");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS,postfields);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getActionIDs);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if(noRulesToLoad == true){
			noRulesToLoad = false;
			break;
		}

	}

	pthread_mutex_unlock(&mutexAddActions);

	/*boost::unordered_map <unsigned int,string>::iterator itr;
	for (unsigned int j=0;j<businessFlowVec.size();j++){
		cout<<"\n"<<businessFlowVec[j].action_name <<endl;
		for (unsigned int i=0;i<businessFlowVec[j].pages.size();i++)
		{
			cout<< "\t"<<businessFlowVec[j].pages[i].pageID <<endl;
			for ( itr = businessFlowVec[j].pages[i].params.begin() ; itr != businessFlowVec[j].pages[i].params.end() ; itr++){
				cout<<"\t\t"<<itr->first<<"="<<itr->second<<endl;
			}
		}
	}*/

}

void parseAppsJson(Json::Value & root,string & preKey,string & domain,unsigned int & learning_so_far,unsigned int & operation_mode,unsigned int & move_to_production,unsigned short & redirect_mode,string & redirect_page,unsigned short & redirect_status_code,unsigned short & body_value_mode,unsigned short & basic_mode,unsigned short & digest_mode,unsigned short & ntlm_mode,string & body_value_html,string & form_param_name,unsigned short & cookie_mode,string & cookie_name,string & cookie_value,unsigned short & cookie_value_appearance,unsigned short & top_level_domain){
        Json::Value::iterator it;
        unsigned int i,j;
	string key,val,key2,tmp_full;

        it=root.begin();
        for (i=0; i < root.size() ; i++) {

		key2 = it.key().asString();

		if( !(*it).isObject() ){
                       	key = it.key().asString();
		}

                if( (*it).isObject() ){
                        parseAppsJson((*it),key2,domain,learning_so_far,operation_mode,move_to_production,redirect_mode,redirect_page,redirect_status_code,body_value_mode,basic_mode,digest_mode,ntlm_mode,body_value_html,form_param_name,cookie_mode,cookie_name,cookie_value,cookie_value_appearance,top_level_domain);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
				if( array[j].isObject() ){
					preKey = it.key().asString();
                                        parseAppsJson(array[j],key2,domain,learning_so_far,operation_mode,move_to_production,redirect_mode,redirect_page,redirect_status_code,body_value_mode,basic_mode,digest_mode,ntlm_mode,body_value_html,form_param_name,cookie_mode,cookie_name,cookie_value,cookie_value_appearance,top_level_domain);
                                }
                        }
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }

			if(key=="host"){
				domain = val;
			}
			else if(key=="learning_so_far"){
				learning_so_far = (unsigned int)atoi(&val[0]);
			}
			else if(key=="operation_mode"){
				operation_mode = (unsigned int)atoi(&val[0]);
			}
			else if(key=="move_to_production"){
				move_to_production = (unsigned int)atoi(&val[0]);  
			}
			else if(key=="redirect_mode"){
				redirect_mode = (unsigned short)atoi(&val[0]);
			}
			else if(key=="redirect_page"){
				redirect_page = val;
			}
			else if(key=="redirect_status_code"){
				redirect_status_code = (unsigned short)atoi(&val[0]);
			}
			else if(key=="body_value_mode"){
				body_value_mode = (unsigned short)atoi(&val[0]);
			}
                        else if(key=="basic_mode"){
                                basic_mode = (unsigned short)atoi(&val[0]);
                        }
                        else if(key=="digest_mode"){
                                digest_mode = (unsigned short)atoi(&val[0]);
                        }
                        else if(key=="ntlm_mode"){
                                ntlm_mode = (unsigned short)atoi(&val[0]);
                        }
			else if(key=="body_value_html"){
				body_value_html = val;
			}
			else if(key=="form_param_name"){
				form_param_name = val;
				for(unsigned int i_lower=0;i_lower<form_param_name.size();i_lower++){
					if(form_param_name[i_lower]>64 && form_param_name[i_lower]<91){
						form_param_name[i_lower] += 32;
					}
				}
			}
			else if(key=="cookie_mode"){
				cookie_mode = (unsigned short)atoi(&val[0]);
			}
			else if(key=="cookie_name"){
				cookie_name = val;
			}
			else if(key=="cookie_value"){
				cookie_value = val;
			}
			else if(key=="cookie_value_appearance"){
				cookie_value_appearance = (unsigned short)atoi(&val[0]);
			}
			else if(key=="top_level_domain"){
				top_level_domain = (unsigned short)atoi(&val[0]);
			}
		}
		it++;
        }
}

void updateApp(char *ptr)
{
	string prekey,domain;
	prekey.clear();
	string action_json,redirect_page,body_value_html,form_param_name,cookie_name,cookie_value;
	boost::unordered_map <string,AppMode>::iterator itAppMode;
	unsigned int learning_so_far,operation_mode,move_to_production;
	unsigned short redirect_mode,redirect_status_code,body_value_mode,basic_mode,digest_mode,ntlm_mode,cookie_mode,cookie_value_appearance,top_level_domain;
	Json::Value root;
	Json::Reader reader;
	bool parsed;

	if(ptr>0){
		if(ptr != NULL){
			action_json = string(ptr);
			parsed = reader.parse(action_json, root, false);
			if(!parsed){
				return;
			}

			parseAppsJson(root,prekey,domain,learning_so_far,operation_mode,move_to_production,redirect_mode,redirect_page,redirect_status_code,body_value_mode,basic_mode,digest_mode,ntlm_mode,body_value_html,form_param_name,cookie_mode,cookie_name,cookie_value,cookie_value_appearance,top_level_domain);
			pthread_mutex_lock(&mutexAppMode);
				itAppMode = mAppMode.find( domain );
				if( itAppMode != mAppMode.end() ){
					if(itAppMode->second.mode!=1 && operation_mode==1){
						itAppMode->second.counter=0;
						itAppMode->second.mode=operation_mode;
					}
					else if(itAppMode->second.mode==1 && operation_mode==3){
						syslog(LOG_NOTICE,"Application %s cannot move to hybrid mode directly from learning - moved to production mode instead.",domain.c_str());
						itAppMode->second.mode=2;
					}else{
						itAppMode->second.mode=operation_mode;
					}

					itAppMode->second.move_to_production=move_to_production;
					itAppMode->second.redirect_mode=redirect_mode;
					itAppMode->second.redirect_page=redirect_page;
					itAppMode->second.redirect_status_code=redirect_status_code;
					itAppMode->second.body_value_mode=body_value_mode;
					itAppMode->second.basic_mode=basic_mode;
					itAppMode->second.digest_mode=digest_mode;
					itAppMode->second.ntlm_mode=ntlm_mode;
					itAppMode->second.body_value_html=body_value_html;
					itAppMode->second.form_param_name=form_param_name;
					itAppMode->second.cookie_mode=cookie_mode;
					itAppMode->second.cookie_name=cookie_name;
					itAppMode->second.cookie_value=cookie_value;
					itAppMode->second.cookie_value_appearance=cookie_value_appearance;
					itAppMode->second.top_level_domain=top_level_domain;
				}else{
					operation_mode=1;
					AppMode am(operation_mode,learning_so_far,move_to_production,redirect_mode,redirect_page,redirect_status_code,body_value_mode,basic_mode,digest_mode,ntlm_mode,body_value_html,form_param_name,cookie_mode,cookie_name,cookie_value,cookie_value_appearance,top_level_domain);
					mAppMode.insert(pair<string,AppMode>(domain,am));
				}
			pthread_mutex_unlock(&mutexAppMode);
		}
	}
}

void getApp(char *ptr)
{
	string prekey,domain;
	prekey.clear();
	string action_json,redirect_page,body_value_html,form_param_name,cookie_name,cookie_value;
	unsigned int learning_so_far=0,operation_mode=1,move_to_production=1000000;
	unsigned short redirect_mode=0,redirect_status_code=0,body_value_mode=0,basic_mode=0,digest_mode=0,ntlm_mode=0,cookie_mode=0,cookie_value_appearance=0,top_level_domain=1;
	Json::Value root;
	Json::Reader reader;
	bool parsed;

	if(ptr>0){
		if(ptr != NULL){
			action_json = string(ptr);
			parsed = reader.parse(action_json, root, false);

			if(!parsed){
				return;
			}
			parseAppsJson(root,prekey,domain,learning_so_far,operation_mode,move_to_production,redirect_mode,redirect_page,redirect_status_code,body_value_mode,basic_mode,digest_mode,ntlm_mode,body_value_html,form_param_name,cookie_mode,cookie_name,cookie_value,cookie_value_appearance,top_level_domain);
			AppMode am(operation_mode,learning_so_far,move_to_production,redirect_mode,redirect_page,redirect_status_code,body_value_mode,basic_mode,digest_mode,ntlm_mode,body_value_html,form_param_name,cookie_mode,cookie_name,cookie_value,cookie_value_appearance,top_level_domain);
			pthread_mutex_lock(&mutexAppMode);
				mAppMode.insert(pair<string,AppMode>(domain,am));
			pthread_mutex_unlock(&mutexAppMode);
		}
	}
}

void getAppIDs(char *ptr)
{
	CURL *curl;
	vector <string> ids;
	string app_json;
	char url[200];

	Json::Value root;
	Json::Reader reader;
	bool parsed;
	unsigned int i;

	ids.clear();

	if(ptr>0){
		if(ptr != NULL){
			app_json = string(ptr);
			parsed = reader.parse(app_json, root, false);

			if(!parsed){
				return;
			}

			parseRuleIDsJson(root,ids);
		}
	}

	if(ids.size()==0){
		noAppsToLoad = true;
		return;
	}
	for(i=0;i<ids.size();i++){
		snprintf(url,sizeof(url)-1,"%s/telepath-domains/domains/%s/_source",es_connect.c_str(),ids[i].c_str());
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"GET");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getApp);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

}

void loadApps(){ // Load business logic from database.
	es_insert("/telepath-config/config/app_list_was_changed_id/","{\"value\":\"0\"}");
	pthread_mutex_lock(&mutexAppMode);
		mAppMode.clear();
	pthread_mutex_unlock(&mutexAppMode);


	CURL *curl;
	char postfields[100],url[200];
	unsigned int from=0;
	while(1){
		sprintf(url,"%s/telepath-domains/domains/_search?filter_path=hits.hits._id",es_connect.c_str());
		sprintf(postfields,"{\"size\":%u,\"from\":%u}",LOAD_RULES_BULK,from);
		from += LOAD_RULES_BULK;

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"POST");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS,postfields);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getAppIDs);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if(noAppsToLoad == true){
			noAppsToLoad = false;
			break;
		}

	}
}

/*
void parseCasesJson(Json::Value & root,string & preKey,vector <Case> & vCases,Case & case_,SubCase & subcase){

        Json::Value::iterator it;
        string key,strTmp,val,key2;
        unsigned int i,j;

        it=root.begin();
        for (i=0; i < root.size() ; i++) {

		key2 = it.key().asString();

		if( !(*it).isObject() ){
                       	key = it.key().asString();
		}

                if( (*it).isObject() ){
                        parseCasesJson( (*it),key2,vCases,case_,subcase);
                }
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
				if( array[j].isObject() ){
					preKey = it.key().asString();
                                        parseCasesJson( array[j],key2,vCases,case_,subcase);
                                }
                        }
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }
			
			if(preKey.size()==0){
				if(key=="_type"){
					vCases.push_back(case_);
					case_.clean();
				}
			}else{
				switch(key[0]){
					case 'c':
						if(key.size()==9){
							if(case_.subcase.size() != 0){
								vCases.push_back(case_);
								case_.clean();
							}

							case_.case_name=val;
						}
						break;

					case 'n':
						if(val[0]=='t'){
							subcase.neg = true;
						}else{
							subcase.neg = false;
						}
					case 't':
						subcase.type = val[0];
						break;
					case 'v':
						subcase.vals = val;
						case_.subcase.push_back(subcase);
						break;
				}
			}



                }
                it++;
        }

}


void getCases(char *ptr)
{
        Json::Value root;
        Json::Reader reader;
        bool parsed;

	string prekey;
	prekey.clear();
	string case_json;

	Case case_;
	SubCase subcase;

	if(ptr>0){
		if(ptr != NULL){
			case_json = string(ptr);
			parsed = reader.parse(case_json, root, false);

			if(!parsed){
				return;
			}

			parseCasesJson(root,prekey,vCases,case_,subcase);

			for (unsigned int k=0;k<vCases.size();k++){
				vCases[k].init();
			}
		}
	}
}

void loadCases(){ // Load business logic from database.
	if(case_list_was_changed==0){	
		return;
	}

	case_list_was_changed=0;
	es_insert("/telepath-config/config/case_list_was_changed_id","{\"value\":\"0\"}");
	vCases.clear();

	CURL *curl;
	curl = curl_easy_init();
	curl_easy_setopt(curl,CURLOPT_CUSTOMREQUEST,"GET"); 
	curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:/telepath-config/cases/cases_id");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getCases);
	curl_easy_perform(curl);
	curl_easy_cleanup(curl); 
}

*/

void parseBotIDsJson(Json::Value & root,vector <string> & ids){
        Json::Value::iterator it;
        unsigned int i,j;
        string key,val;

        it=root.begin();
        for (i=0; i < root.size() ; i++) {
		if( !(*it).isObject() ){
                        key = it.key().asString();
                }
                if( (*it).isObject() ){
                        parseBotIDsJson((*it),ids);
		}
                else if( (*it).isArray() ){
                        Json::Value array = (*it);
                        for (j=0; j < array.size(); j++) {
                                if( array[j].isObject() ){
                                        parseBotIDsJson(array[j],ids);
                                }else{
                                }
                        }
            
                }else{
                        if ( (*it).isString() ){
                                val = (*it).asString();
                        }else{
                                val = (*it).toStyledString();
                        }
                        
			if(key == "_id"){
				ids.push_back(val);
                        }

                }
                it++;
        }

}

void getTorIPs(char *ptr)
{
	CURL *curl;
	vector <string> ids;
	string bot_json;

	Json::Value root;
	Json::Reader reader;
	bool parsed;
	unsigned int i;

	ids.clear();

	if(ptr>0){
		if(ptr != NULL){
			bot_json = string(ptr);
			parsed = reader.parse(bot_json, root, false);

			if(!parsed){
				return;
			}

			parseBotIDsJson(root,ids);
		}
	}

	if(ids.size()==0){
		noBotsToLoad = true;
		return;
	}

	for(i=0;i<ids.size();i++){
		sTorIntelligenceIP.insert(ids[i]); 
	}
}

void getBotIPs(char *ptr)
{
        CURL *curl;
        vector <string> ids;
        string bot_json;

        Json::Value root;
        Json::Reader reader;
        bool parsed;
        unsigned int i;

        ids.clear();

        if(ptr>0){
                if(ptr != NULL){
                        bot_json = string(ptr);
                        parsed = reader.parse(bot_json, root, false);

                        if(!parsed){
                                return;
                        }

                        parseBotIDsJson(root,ids);
                }
        }

        if(ids.size()==0){
                noBotsToLoad = true;
                return;
        }

        for(i=0;i<ids.size();i++){
                sBotIntelligenceIP.insert(ids[i]);
        }
}

void loadBotIntelligence(){ // Load bot intelligence from database.
	if(bot_intelligence==0){
		return;
	}else{
		bot_intelligence=0;
		es_insert("/telepath-config/config/bot_intelligence_id","{\"value\":\"0\"}");
	}
	sTorIntelligenceIP.clear();

        //---------------------------------
        
	CURL *curl;
	char postfields[100],url[200];
	unsigned int from=0;
	while(1){
		sprintf(url,"%s/telepath-tor-ips/tor/_search?filter_path=hits.hits._id",es_connect.c_str());
		sprintf(postfields,"{\"size\":%u,\"from\":%u}",LOAD_BOTS_BULK,from);
		from += LOAD_BOTS_BULK;

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"POST");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS,postfields);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getTorIPs);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if(noBotsToLoad == true){
			noBotsToLoad = false;
			break;
		}
	}
	
	from=0;
	while(1){
		sprintf(url,"%s/telepath-bad-ips/bad/_search?filter_path=hits.hits._id",es_connect.c_str());
		sprintf(postfields,"{\"size\":%u,\"from\":%u}",LOAD_BOTS_BULK,from);
		from += LOAD_BOTS_BULK;

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,"POST");
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS,postfields);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getBotIPs);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if(noBotsToLoad == true){
			noBotsToLoad = false;
			break;
		}
	}
}

