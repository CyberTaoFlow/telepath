<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Rules extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
		$this->load->model('RulesModel');
	}
	
	public function _fail() {
		return_json(array('success' => false, 'total' => 0, 'items' => array()));
	}
	
	public function _success() {
		return_json(array('success' => true, 'total' => 0, 'items' => array()));
	}
	
	public function _get_alert_rule_categories() {
	
		//$this->_get_rule_groups_categories();
		$ans  = array('items' => array(), 'total' => 0, 'success' => true);
		
		$categories = $this->RulesModel->get_categories();
		
		if($categories && is_array($categories) && !empty($categories)) {
			
			foreach($categories as $category) {
				
				$expandable = false;
				$groups     = $this->RulesModel->get_groups($category->id);
				
				if($groups && is_array($groups) && !empty($groups)) {
					foreach($groups as $group) {
						if(intval($group->num) > 0) {
							$expandable = true;
						}
					}
				}
				
				if($expandable) {
					$ans['items'][] = array(
						'id'       => intval($category->id),
						'category' => $category->name,
					);
				}
				
			}
			
		}
		
		$ans['total'] = count($ans['items']);
		return_json($ans);
				
	}
	public function _expand_alert_category() {
		$this->_get_rule_groups_list();
	}
	
	public function index() {
	
		// Get mode from either POST
		$mode = $this->input->post('mode', true);
		
		// Or GET
		if(!$mode) {
			$mode = $this->input->get('mode', true);
		}
		
		switch($mode) {
			
			// Alerts Screen
			case 'get_alert_rule_categories':
				// All categories having alerts
				$this->_get_alert_rule_categories();
			break;
			
			case 'expand_alert_category':
				// All rules having alerts
				$this->_expand_alert_category();
			break;
			
			// Rules Screen
			
			case 'wizard_submit':
				telepath_auth(__CLASS__, 'add_rule');
				$this->_wizard_submit();
			break;
			
			case 'get_rule_information':
				telepath_auth(__CLASS__, 'get_rule');
				$this->_get_rule_information();
			break;
			
			case 'get_group_information':
				telepath_auth(__CLASS__, 'get_group_info');
				$this->_get_group_information();
			break;
			
			case 'expand_group':
				telepath_auth(__CLASS__, 'get_group_expand');
				$this->_expand_group();
			break;
			
			case 'expand_category':
				telepath_auth(__CLASS__, 'get_category_expand');
				$this->_expand_category();
			break;
			
			case 'get_groups_per_category':
				telepath_auth(__CLASS__, 'get_category_groups');
				$this->_get_groups_per_category();
			break;
			
			case 'create_new_category':
				telepath_auth(__CLASS__, 'add_category');
				$this->_create_new_category();
			break;
			
			case 'update_category':
				telepath_auth(__CLASS__, 'set_category');
				$this->_update_category();
			break;
			
			case 'create_new_group':
				telepath_auth(__CLASS__, 'add_group');
				$this->_create_new_group();
			break;
			
			case 'disable_category_rules':
			case 'enable_category_rules': 
				telepath_auth(__CLASS__, 'set_category_toggle');
				$this->_toggle_category_rules($mode);
			break;
			
			case 'disable_group_rules':
			case 'enable_group_rules':
				telepath_auth(__CLASS__, 'set_group_toggle');
				$this->_toggle_group_rules($mode);
			break;
			
			case 'get_rule_groups_categories':
				telepath_auth(__CLASS__, 'get_rule_groups_categories');
				$this->_get_rule_groups_categories();
			break;
			
			case 'get_rule_groups_list':
				telepath_auth(__CLASS__, 'get_rule_groups_list');
				$this->_get_rule_groups_list();
			break;
			
			case 'add_parameter_to_rule_group':
				telepath_auth(__CLASS__, 'set_rule_parameter');
				$this->_add_parameter_to_rule_group();
			break;
			
			case 'update_group':
				telepath_auth(__CLASS__, 'set_group');
				$this->_update_group();
			break;
			
			case 'update_rule':
				telepath_auth(__CLASS__, 'set_rule');
				$this->_update_rule();
			break;
			
			case 'delete_rule':
				telepath_auth(__CLASS__, 'del_rule');
				$this->_delete_rule();
			break;
			
			case 'delete_group':
				telepath_auth(__CLASS__, 'del_group');
				$this->_delete_group();
			break;
			
			case 'delete_category':
				telepath_auth(__CLASS__, 'del_category');
				$this->_delete_category();
			break;
			
			case 'create_new_rule_in_group':
				telepath_auth(__CLASS__, 'add_new_rule');
				$this->_create_new_rule_in_group();
			break;
			
			case 'move_rule_to_group':
				telepath_auth(__CLASS__, 'set_rule_group');
				$this->_move_rule_to_group();
			break;
			
			case 'move_group_to_category':
				telepath_auth(__CLASS__, 'set_rule_category');
				$this->_move_group_to_category();
			break;
			
			case 'copy_group_to_category':
				telepath_auth(__CLASS__, 'add_group_copy');
				$this->_copy_group_to_category();
			break;
			
			case 'check_group_duplication':
				telepath_auth(__CLASS__, 'get_group_duplication');
				$this->_check_group_duplication();
			break;
	
		}
		
	}
	
	// TODO:: Merge with generic functions
	// Handle wizard submit
	private function _wizard_submit_validate() {
		
		// TODO:: Import basic validation from FieldsValidation(form)
		// Check rule name
		// if($this->RulesModel->get_rule_by_name($this->input->post('general_rulename'))) {
		//	return_json(array('success' => false, 'error_message' => 'Rule name already exists'));
		// }
		
	}
	
	private function _wizard_submit_category() {
		
		$category_mode = $this->input->post('category_mode');
		$category_id   = -1;
		
		if($category_mode == 'new') {
			$category_id = $this->RulesModel->create_category($this->input->post('category_name'));
		} else {
			$category_id = $this->input->post('category_id');
		}
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		return $category_id;
	
	}
	private function _wizard_submit_group($category_id) {
		
		$group_mode = $this->input->post('group_groupmode');
		
		if($group_mode == 'new') {
			
			// Check collision
			if($this->RulesModel->find_group_by_name($this->input->post('group_nametextfield'))) {
				return_json(array('success' => false, 'error_message' => 'Group already exists'));
			} else {	
			
				// Create blank group
				$group_id = $this->RulesModel->create_group($this->input->post('group_nametextfield'), 
															$this->input->post('group_desctextfield'), 
															$category_id, '1', '75');
				$group_data = array();
			
				// Group Params
				$group_data['alert_param_ids'] = $this->input->post('general_param_ids');
				$group_data['score_numeric'] = null;
				$group_data['score_literal'] = null;
				$group_data['businessflow_id'] = '-1';
				
				// Group Action Log
				$group_data['action_log'] 			   = $this->input->post('group_action_log')		         == '1' ? '1' : '0';
				$group_data['action_syslog'] 		   = $this->input->post('group_action_syslog') 		     == '1' ? '1' : '0';
				$group_data['action_header_injection'] = $this->input->post('group_action_header_injection') == '1' ? '1' : '0';
				
				// Group Action Email
				$group_actionEmailCB = $this->input->post('group_action_email');
				if($group_actionEmailCB && $group_actionEmailCB == '1') {
					$group_data['action_email'] = '1';
					$group_data['email_recipient'] = $this->input->post('group_action_email_field');
				} else {
					$group_data['action_email'] = '0';
					$group_data['email_recipient'] = '';
				}
			
				$this->RulesModel->update_group($group_id, $group_data);
				
			}
			
		} else {
			$group_id = $this->input->post('group_ExistsGroupsCombobox');
		}
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		return $group_id;
		
	}
	
	private function _wizard_submit_rule() {
		
		$rule_data = array();
		
		$rule_data['enable_rule'] = '1';
		
		$rule_data['name'] 		  = $this->input->post('general_rulename');
		$rule_data['description'] = $this->input->post('general_description');
		$rule_data['owner'] 	  = $this->input->post('general_owner');
		$rule_data['seq_index']   = $this->input->post('general_StartIndexField');
		$rule_data['appearance']  = $this->input->post('general_AppearanceField');
		
		
		// APP
		$rule_app_cb    = $this->input->post('general_ApplicationCheckbox');
		if($rule_app_cb && $rule_app_cb == '1') {
			$rule_data['app_id'] = $this->input->post('general_ApplicationCombobox');
		} else {
			$rule_data['app_id'] = '';
		}
		
		// IP
		$rule_ip_cb     = $this->input->post('general_ipcheckbox');
		if($rule_ip_cb && $rule_ip_cb == '1') {
			$rule_data['user_ip'] = $this->input->post('general_iptextfield');
		} else {
			$rule_data['user_ip'] = '';
		}
		
		// IP Negate flag
		$rule_ip_neg   = $this->input->post('ip_neg');
		if($rule_ip_neg && $rule_ip_neg == '1') {
			$rule_data['ip_neg'] = 1;
		} else {
			$rule_data['ip_neg'] = 0;
		}
		
		// USER
		$rule_user_cb   = $this->input->post('general_usercheckbox');
		if($rule_user_cb && $rule_user_cb == '1') {
			$rule_data['user'] = $this->input->post('general_usertextfield');
		} else {
			$rule_data['user'] = '';
		}
		
		// SCORE
		$rule_score_type    = $this->input->post('general_scoretype');
		if($rule_score_type == 'numeric') {
			$rule_data['numeric_score'] = $this->input->post('general_scoretype_numeric');
			$rule_data['literal_score'] = '';
		} else {
			$rule_data['numeric_score'] = '';
			$rule_data['literal_score'] = $this->input->post('general_scoretype_literal');
		}
				
		// TYPE
		$rule_type = $this->input->post('general_RuleTypeCombo');
		$rule_data['rule_type'] = $rule_type;

		switch($rule_type) {
		
			case 'Aspect':
				
				$rule_data['aspect'] = $this->input->post('Aspect_AspectTypeCombobox');
				$rule_data['personal'] = $this->input->post('Aspect_Personal') == '1' ? '1' : '0';
				
			break;
			
			case 'Pattern':
				
				$rule_pattern_anchor_type = $this->input->post('Pattern_Anchor');
				$rule_data['anchor_type'] = $rule_pattern_anchor_type;
				
				switch($rule_pattern_anchor_type) {
				
					case 'IP':
						$rule_data['anchor'] = $rule_pattern_anchor_type;
					break;
					case 'SID':
						$rule_data['anchor'] = $rule_pattern_anchor_type;
					break;
					case 'Other': 
						$rule_data['anchor'] = $this->input->post('Pattern_OtherTextField');
					break;
				
				}
		
				$rule_data['dynamic_type'] = $this->input->post('Pattern_DynamicType');
				$rule_data['att_id']      = $this->input->post('Pattern_DynamicTextField');
				$rule_data['page_id']     = $this->input->post('Pattern_PageTextField');
				$rule_data['business_id'] = $this->input->post('Pattern_FlowSelect');

				$rule_data['pr_attrType'] = '';
				
				if($rule_data['att_id'] != '')      { $rule_data['pr_attrType'] = 'param'; }
				if($rule_data['page_id'] != '')     { $rule_data['pr_attrType'] = 'Page'; }
				if($rule_data['business_id'] != '') { $rule_data['pr_attrType'] = 'Flow'; }								
				
				$rule_data['count']    		= $this->input->post('Pattern_CountTextField');
				$rule_data['timetype'] 		= $this->input->post('Pattern_TimeWindowCombo');
				
				$rule_pattern_timewindow    = $this->input->post('Pattern_TimeWindowTextField');
				
				switch($rule_data['timetype']) {
				
					case 'sec':
						$rule_data['time']  = $rule_pattern_timewindow;
					break;
					case 'min':
						$rule_data['time']  = $rule_pattern_timewindow * 60;
					break;
					case 'hours':
						$rule_data['time']  = $rule_pattern_timewindow * 60 * 60;
					break;
					
				}

			break;
			
			case 'ParameterRule':
				
				// TODO:: Check where these params go
				//ParameterRule_ScoreType = form.getvalue('ParameterRule_ScoreType')
				//ParameterRule_scoretype_numeric = form.getvalue('ParameterRule_scoretype_numeric')
				//ParameterRule_ScoreType_literal = form.getvalue('ParameterRule_ScoreType_literal')
				//ParameterRule_StringMatchScore = form.getvalue('ParameterRule_StringMatchScore')
	
				$rule_param_att          = $this->input->post('ParameterRule_AttributeTextField');
				$rule_param_raw          = $this->input->post('ParameterRule_AttributeTextFieldDisplay');
				$rule_param_page         = $this->input->post('ParameterRule_PageTextField');
				$rule_param_att_type     = $this->input->post('ParameterRule_AttributeTypeRadio');
				$rule_param_string_match = $this->input->post('ParameterRule_StringMatchTextField');
				$rule_param_length       = $this->input->post('ParameterRule_LengthCombo');
				$rule_param_exact        = $this->input->post('ParameterRule_exactlength');
				$rule_param_range_low    = $this->input->post('ParameterRule_rangelengthlow');
				$rule_param_range_high   = $this->input->post('ParameterRule_rangelengthhigh');
				$rule_param_not_signal   = $this->input->post('ParameterRule_StringMatchNot');
				$rule_param_similarity   = $this->input->post('ParameterRule_StringSimlarity');
				$rule_param_aspect_type  = $this->input->post('Aspect_AspectTypeCombobox');
				
				// Aspect
				if($rule_param_aspect_type && $rule_param_aspect_type != '') {
					$rule_data['aspect'] = $rule_param_aspect_type;
				} else {
					$rule_data['aspect'] = '';
				}
				
				// Param
				if($rule_param_att && $rule_param_att != '') {
					$rule_data['att_id'] = $rule_param_att;
				} else {
					$rule_data['att_id'] = '';
					
					if($rule_param_raw && $rule_param_raw != '') {
						$rule_data['att_name'] = $rule_param_raw;
					}
					
				}
				
				// Page
				if($rule_param_page && $rule_param_page != '') {
					$rule_data['page_id'] = $rule_param_page;
				} else {
					$rule_data['page_id'] = '';
				}
				
				
				// Parameter Type
				$rule_data['pr_attrType'] = $rule_param_att_type;
				
				switch($rule_param_att_type) {
				
					case 'fuzzylength':
						$rule_data['str_length']     = $rule_param_length;
						$rule_data['str_similarity'] = $rule_param_similarity;
					break;
					case 'exactlength':
						$rule_data['str_length']     = $rule_param_length;
					break;
					case 'distance_similarity':
						$rule_data['str_similarity'] = $rule_param_similarity;
					break;
					case 'rangelength':
						$rule_data['str_length']     = intval($rule_param_range_low) . '-' . intval($rule_param_range_high);
						$rule_data['str_similarity'] = $rule_param_similarity;
					break;
					case 'gex':
					case 'stringmatch':
						$rule_data['not_signal']     = $rule_param_not_signal;
						$rule_data['str_match']      = $rule_param_string_match;
					break;
					
				}
				
			break;
			
			case 'Geo':
				
				$rule_geo_type   = $this->input->post('GeoRule_Type');
				$rule_geo_radius = $this->input->post('GeoRule_Radius');
				$rule_geo_time   = $this->input->post('GeoRule_Time');
				$rule_geo_allow  = $this->input->post('GeoRule_Allow');
				$rule_geo_block  = $this->input->post('GeoRule_Block');
				
				$rule_data['pr_attrType'] = 'GEO';
				
				switch($rule_geo_type) {
					
					case 'Velocity':
					
						$rule_data['radius'] 	 = $rule_geo_radius;
						$rule_data['time']   	 = $rule_geo_time;
						$rule_data['str_match']  = '';
						$rule_data['not_signal'] = '0';
						
					break;
					case 'Allow':
						
						$rule_data['str_match']  = $rule_geo_allow;
						$rule_data['not_signal'] = '1';
						$rule_data['radius'] 	 = '0';
						$rule_data['time']   	 = '0';
						
					break;
					case 'Block':
						
						$rule_data['str_match']  = $rule_geo_block;
						$rule_data['not_signal'] = '0';
						$rule_data['radius'] 	 = '0';
						$rule_data['time']   	 = '0';
						
					break;
				
				}
				
			break;
			
			case 'Bot':
				
				$rule_data['str_match'] = $this->input->post('Bot_Type');
				
			break;
			
		}
		
		return $rule_data;
		
	}
	
	private function _wizard_submit() {
		
		// Validate form
		$this->_wizard_submit_validate();
		
		// Get category ID
		$category_id = $this->_wizard_submit_category();
		
		// Get group ID
		$group_id    = $this->_wizard_submit_group($category_id);
		
		// Gather Data
		$rule_data = $this->_wizard_submit_rule($group_id);
		$rule_data['rule_group']  = $group_id;
		
		// Create rule
		$rule_id = $this->RulesModel->create_rule($rule_data);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		return_json(array('success' => true, 'error_message' => '', 'category_id' => $category_id, 'group_id' => $group_id, 'rule_id' => $rule_id));
		
	}
	
	/*
	
	Mode: get_rule_information
	 
	 Invoked:
		When we want to load a rule information (after clicking a rule node)

	 Prints:
		a json with all the fields on the rules table and the values
		
	*/
	
	private function _get_rule_information() {
	
		$rule_id = $this->input->post('rule_id', true);
		
		$rule = $this->RulesModel->get_rule_by_id($rule_id);
		
		if($rule) {
			
			// Cast to array
			$rule = (array) $rule;		
			
			// Fix diff
			$rule['ruletype'] = $rule['rule_type'];
			
			$json_fields = array("name", "aspect", "description", "anchor", "att_id", "str_match", "business_id",
								 "numeric_score", "literal_score", "count", "seq_index", "time", "user",
								 "user_ip", "rule_group", "occurence", "owner", "rule_type", "timetype",
								 "app_id", "personal", "anchor_type", "pr_attrType", "str_length", "appearance",
								 "str_similarity", "enable_rule", "not_signal", "radius", "ip_neg", "page_id", "dynamic_type", "att_name");
			
			// Copy from RULE to JSON
			
			$json = array('success' => true);
			
			foreach($json_fields as $field) {
				//if(isset($rule[$field])) {
					$json[$field] = $rule[$field];
				//}
			}
			
			// Post process JSON
			$json['ruletype'] = $json['rule_type'];
			unset($json['rule_type']);
			
			$json['business_id'] = intval($json['business_id']);
			$json['rule_group'] = intval($json['rule_group']);
			$json['page_id'] = intval($json['page_id']);
			$json['radius'] = intval($json['radius']);
			$json['enable_rule'] = intval($json['enable_rule']);
			$json['str_similarity'] = intval($json['str_similarity']);
			$json['appearance'] = intval($json['appearance']);
			$json['not_signal'] = intval($json['not_signal']);
			$json['ip_neg'] = intval($json['ip_neg']);
			$json['personal'] = intval($json['personal']);

			$this->load->model('Attributes');
			$this->load->model('PagesModel');
			
			$page_data = $rule['page_id'] != 0 ? $this->PagesModel->page_get($rule['page_id']) : false;
			$att_data  = $rule['att_id'] != 0 ? $this->Attributes->get_att_by_id($rule['att_id']) : false;
			
			switch($rule['rule_type']) {
			
				case 'Pattern':

					if($rule['anchor_type'] == 'Other') {
						$other_data = $this->Attributes->get_att_by_id($rule['anchor']);
						$json["p_anchor_displayname"] = $other_data ? $other_data->att_name : null;
					}
					$json["p_dynamic_displayname"]    = $att_data  ? $att_data->att_name 	  : null;
					$json["p_page_displayname"]       = $page_data ? $page_data->display_path : null;
				
				break;
				
				case 'ParameterRule':
					
					$json["pr_attribute_displayname"]    = $att_data  ? $att_data->att_name : ($rule['att_name'] ? $rule['att_name'] : null);
					$json["p_page_displayname"]       = $page_data ? $page_data->display_path : null;
					
				break;
				
				case '':
					
					$json['ruletype'] = "Built In";
					
				break;
			
			}
			
						
		} else {
			$this->_fail();
		}

		return_json($json);
		
	}

	/*
	
	Mode: get_group_information
	 
	 Invoked:
		When we want to load a group information (after clicking a group node)

	 Prints:
		a json with all the fields on the rule_Groups table and the values
		
	*/
	
	private function _get_group_information() {
		
		$group_id = $this->input->post('rulesgroup_id', true);

		$group = $this->RulesModel->get_group_by_id($group_id);
		
		if($group) {
			
			// Cast to array
			$group = (array) $group;
			
			// Fix diff
			$group['desc'] = $group['description'];
			$group['action_email_field'] = $group['email_recipient'];
			
			$json_fields = array("name", "desc", "score_numeric", "score_literal", "action_email", "action_email_field", 
						"action_log", "action_syslog", "action_header_injection", "businessflow_id");
			
			// Copy from GROUP to JSON
			
			$json = array('success' => true);
			
			foreach($json_fields as $field) {
				$json[$field] = $group[$field];
			}
			
			$json['action_email'] = intval($json['action_email']);
			$json['businessflow_id'] = intval($json['businessflow_id']);
			$json['action_syslog'] = intval($json['action_syslog']);
			$json['action_log'] = intval($json['action_log']);
			$json['action_header_injection'] = intval($json['action_header_injection']);
			
			if($json['score_numeric']) {
				$json['score_numeric'] = intval($json['score_numeric']);
			}
			
			$json["alert_param_ids"] = array();
			
			if(isset($group["alert_param_ids"]) && $group["alert_param_ids"] != '') {
				$this->load->model('Attributes');
				$atts = $this->Attributes->get_atts_by_ids(explode(',', $group["alert_param_ids"]));
				if($atts) {
					foreach($atts as $att) {
						$json["alert_param_ids"][] = array('td1' => $att->att_id, 'td0' => $att->att_name);
					}
				}
			}

		} else {
			$this->_fail();
		}
	
		return_json($json);

	}
	
	/*
	
	 Mode: expand_group
	 
	 Invoked:
		When we want to load rules for a certain group(after expanding a group node)
	 
	 Parameters:
	 	group_id - the group id expanded
	 
	 Prints:
	 	An object of extjs response form {"items": [{td0:td0_val...}], "total": int, "success": "true"/"false"}
		
	*/
	
	private function _expand_group() {
		
		$group_id = $this->input->post('group_id', true);
		
		if(!$group_id) {
			$this->_fail();
		}
		
		$rules = $this->RulesModel->get_rules_by_group_id($group_id);
		
		$ans = array('items' => array(), 'total' => 0, 'success' => true);
		
		if($rules && is_array($rules) && !empty($rules)) {
		
			foreach($rules as $rule) {
				
				$ans['items'][] = array(
					'td0' => intval($rule->id),
					'td1' => $rule->name,
					'td2' => $rule->description,
					'td3' => intval($rule->rule_group),
					'td4' => intval($rule->category_id)
				);
			
			}
		
		}
		
		$ans['total'] = count($ans['items']);
		
		return_json($ans);
		
	}
	
	/*
	
	 Mode: expand_category
	 
	 Invoked:
		When we want to load rule groups for a certain category(after expanding a category node)
	 
	 Parameters:
	 	category_id - the category id expanded
	 
	 Prints:
	 	An object of extjs response form {"items": [{td0:td0_val...}], "total": int, "success": "true"/"false"}
		
	*/
	
	private function _expand_category() {
		
		$category_id = $this->input->post('category_id', true);
		
		$groups = $this->RulesModel->get_groups_by_category_id($category_id);

		$ans = array('items' => array(), 'total' => 0, 'success' => true);
		
		$brute_limit = 1000;
		$brute_token = 'Login Brute Force Attempt';
		
		if($groups && is_array($groups) && !empty($groups)) {
			
			$hover = '';
			
			foreach($groups as $group) {
				
				$name = $group->name;
				
				if(strpos($group->name, $brute_token) > -1) {
					
					if($brute_limit-- < 0)
						continue;
					
					// Start copying rules
					$rules = $this->RulesModel->get_rules_by_group_id($group->id);
					
					if($rules && !empty($rules)) {
						
						// Load & Cast
						foreach($rules as $rule) {
							$rule_data = (array) $this->RulesModel->get_rule_by_id($rule->id);
							if(!empty($rule_data)) {
							
								$this->load->model('Subdomains');
								$res = $this->Subdomains->get($rule_data['app_id'], array($rule_data['page_id']));
								if($res && !empty($res)) {
									
									$hover = $res[0]->subdomain_name . trim(str_replace($brute_token, '', $name));
									$name  = 'Login Brute Force /' . array_pop(explode('/', str_replace($brute_token, '', $name)));
									break;
									
								}
								continue;
							}
						}
					
					}
					
				}
				
				$ans['items'][] = array(
					'td0' => intval($group->id),
					'td1' => $name,
					'td2' => intval($group->category_id),
					'hover' => $hover
				);
			
			}
		
		}
		
		$ans['total'] = count($ans['items']);
		
		return_json($ans);

	}
	
	/*
	 // DEPRECATED -- USE EXPAND CATEGORY INSTEAD
	 
	 Mode: get_groups_per_category
	 
	 Invoked:
		When we want to load rule groups for a certain category(after expanding a category node)
	 
	 Parameters:
	 	category_id - the category id expanded
	 
	 Prints:
	 	An object of extjs response form {"items": [{id:group_id,group:group_name,td2:category_id...}], "total": int, "success": "true"/"false"}
		
	*/
	
	private function _get_groups_per_category() {
	
		$category_id = $this->input->get('category_id', true);
		
		$groups = $this->RulesModel->get_groups_by_category_id($category_id);
		
		$ans = array('items' => array(), 'total' => 0, 'success' => true);
		
		if($groups && is_array($groups) && !empty($groups)) {
		
			foreach($groups as $group) {
				
				$ans['items'][] = array(
					'id' => intval($group->id),
					'group' => $group->name,
					'td2' => intval($group->category_id)
				);
			
			}
		
		}
		
		$ans['total'] = count($ans['items']);
		
		return_json($ans);
		
	}
	
	/*
	
	 Mode: create_new_category
	 
	 Invoked:
		When we want to create a new category
	 
	 Parameters:
	 	category_name - the category name to add
	 
	 Prints:
	 	An object of extjs response form {"items": [], "total": int, "success": "true"/"false"}
		
	*/
	
	private function _create_new_category() {
	
		$category_name = $this->input->post('category_name', true);
	
		$ans  = array('items' => array(), 'total' => 0, 'success' => true);
		
		// Check if name is taken by another category
		$category = $this->RulesModel->get_category_by_name($category_name);
		if($category && is_array($category) && !empty($category)) {
			$this->_fail();
		}
		
		// Create a new category
		$ans['category_id'] = $this->RulesModel->create_category($category_name);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		return_json($ans);
		
	}
	
	/* 
	
	 Mode: update_category
	 
	 Invoked:
		When we want to edit a category
	 
	 Parameters:
	 	category_name - the category name to save
		category_id - the category id to edit
	 
	 Prints:
	 	An object of extjs response form {"items": [], "total": int, "success": "true"/"false"}
		
	*/
	
	private function _update_category() {
		
		$category_id = $this->input->post('category_id', true);
		$category_name = $this->input->post('category_name', true);
	
		// Check if name is taken by another category
		$category = $this->RulesModel->get_category_by_name($category_name);
		if($category && is_array($category) && !empty($category) && $category[0]->id != $category_id) {
			$this->_fail();
		}
		
		$this->RulesModel->update_category($category_id, $category_name);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
		
	}

	/*
	
	Mode: create_new_group
	 
	 Invoked:
		When we want to create a new group
	 
	 Parameters:
	 	group_name - the group name to add
		category_id - the category to add the group to
	 
	 Prints:
	 	An object of extjs response form {"items": [], "total": int, "success": "true"/"false",group_id:int}
		
	*/
	
	private function _create_new_group() {
		
		$group_name = $this->input->post('group_name', true);
		$category_id = $this->input->post('category_id', true);
		
		$ans  = array('items' => array(), 'total' => 0, 'success' => true);
		
		$group = $this->RulesModel->find_group_by_name($group_name);
		
		if($group && is_array($group) && !empty($group)) {
			$this->_fail();
		}

		$ans['group_id'] = $this->RulesModel->create_group($group_name, '-', $category_id, '1', '75');
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		return_json($ans);

	}
	
	/*
	
	 Mode: disable_category_rules | enable_category_rules
	 
	 Invoked:
		When we want to enable/disable all groups under a category
	 
	 Parameters:
		category_id - the category under which to disable/enable
	 
	 Prints:
	 	An object of extjs response form {"items": [], "success": "true"/"false"}
		
	*/
	
	private function _toggle_category_rules($mode) {
		
		$category_id = $this->input->post('category_id', true);
		
		if($mode == 'enable_category_rules') {
			$value = '1';
		} else {
			$value = '0';
		}
		
		$groups = $this->RulesModel->get_groups_by_category_id($category_id);
		
		if($groups && is_array($groups) && !empty($groups)) {
		
			foreach($groups as $group) {
				
				$this->RulesModel->toggle_rules_by_group($group->id, $value);
				
			}
			
		}
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
			
	}
	
	/*
	
	 Mode: disable_category_rules | enable_category_rules
	 
	 Invoked:
		When we want to enable/disable all rules under a group
	 
	 Parameters:
		group_id - the group under which to disable/enable
	 
	 Prints:
	 	An object of extjs response form {"items": [], "success": "true"/"false"}
		
	*/
	
	private function _toggle_group_rules($mode) {
		
		$group_id = $this->input->post('group_id', true);
		
		if($mode == 'enable_group_rules') {
			$value = '1';
		} else {
			$value = '0';
		}
	
		$this->RulesModel->toggle_rules_by_group($group_id, $value);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
	
		$this->_success();
		
	}
	
	/*
	
	 Mode: get_rule_groups_categories (to load to the tree on start)
	 
	 Invoked:
		When we want to load to the tree on start
	 
	 Parameters:
	 
	 Prints:
	 	An object of extjs response form {"items": [{category:str,id:int},.....,{}], "success": "true"/"false"}
		
	*/
	
	private function _get_rule_groups_categories() {
	
		$ans  = array('items' => array(), 'total' => 0, 'success' => true);
		
		$categories = $this->RulesModel->get_categories();
		
		if($categories && is_array($categories) && !empty($categories)) {
			
			foreach($categories as $category) {
			
				$ans['items'][] = array(
					'id'       => intval($category->id),
					'category' => $category->name
				);
				
			}
			
		}
		
		$ans['total'] = count($ans['items']);
		
		return_json($ans);
	
	}

	/*
	
	 Mode: get_rule_groups_list 
	 
	 Invoked:
		When we want to get the list of all the rule groups on the rule group table with no filter

	 Prints:
	 	An object of extjs response form {"items": [{group:str,id:int},.....,{}], "success": "true"/"false"}
		
	*/
	
	private function _get_rule_groups_list() {
		
		$ans  = array('items' => array(), 'total' => 0, 'success' => true);
		$cat  = intval($this->input->post('category_id'));
		
		$groups = $this->RulesModel->get_groups($cat);
		
		if($groups && is_array($groups) && !empty($groups)) {
			
			foreach($groups as $group) {
			
				$ans['items'][] = array(
					'id'    => intval($group->id),
					'text' => $group->name . ' (' . $group->num . ')'
				);
				
			}
			
		}

		$ans['total'] = count($ans['items']);
		
		return_json($ans);

	}
	
	/*
	
		Mode: add_parameter_to_rule_group

		Invoked:
			This function is called when we add parameters in Alerts
		
	*/
	
	private function _add_parameter_to_rule_group() {
		
		$group_id     = $this->input->get('rule_group_id', true);
		$parameter_id = $this->input->get('parameter_id', true);
		
		$this->RulesModel->add_group_param($group_id, $parameter_id);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
		
	}
	
	/*
	
	Mode: update_group
	 
	 Invoked:
		When a group is saved in the rule editor
	 
	 Parameters:
	 	
	 
	 Prints:
	 	ans - an array,to load to store form [success:true]
		
	*/
	
	private function _update_group() {
		
		$group_id = $this->input->post('groupid');
		
		if(!$group_id) {
			$this->_fail();
		}
		
		$group_data = array();
		
		$group_data['name'] = $this->input->post('groupname');
		
		if(!$group_data['name']) {
			$this->_fail();
		}
		
		$group_data['description'] = $this->input->post('groupdesc');
		
		// Check Group Duplication
		$group_chk = $this->RulesModel->find_group_by_name($group_data['name']);

		if($group_chk && isset($group_chk->id) && ($group_chk->id != $group_id)) {
			return_json(array('success' => false, 'exists' => true));
		}
		
		// Group Params
		$group_data['alert_param_ids'] = $this->input->post('general_param_ids');

		// DEPRECATED CODE
		/*
		// Group Score
		$group_scoreType = $this->input->post('group_scoretype');
		if($group_scoreType == 'literal') {
			$group_data['score_numeric'] = null;
			$group_data['score_literal'] = $this->input->post('group_scoretype_literal');
		} else {
			$group_data['score_numeric'] = $this->input->post('group_scoretype_numeric');
			$group_data['score_literal'] = null;
		}

		// Group Flow
		$group_flowComboCB = $this->input->post('group_flowcheckbox');
		if($group_flowComboCB && $group_flowComboCB == '1') {
			$group_data['businessflow_id'] = $this->input->post('group_flowCombo');
		} else {
			$group_data['businessflow_id'] = '-1';
		}
		*/
		
		$group_data['score_numeric'] = null;
		$group_data['score_literal'] = null;
		$group_data['businessflow_id'] = '-1';
		
		// Group Action Log
		$group_data['action_log'] 			   = $this->input->post('action_log')		       == '1' ? '1' : '0';
		$group_data['action_syslog'] 		   = $this->input->post('action_syslog') 		   == '1' ? '1' : '0';
		$group_data['action_header_injection'] = $this->input->post('action_header_injection') == '1' ? '1' : '0';
		
		// Group Action Email
		$group_actionEmailCB = $this->input->post('action_email');
		if($group_actionEmailCB && $group_actionEmailCB == '1') {
			$group_data['action_email'] = '1';
			$group_data['email_recipient'] = $this->input->post('action_email_address');
		} else {
			$group_data['action_email'] = '0';
			$group_data['email_recipient'] = '';
		}
		
		$this->RulesModel->update_group($group_id, $group_data);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
		
	}
	
	/*
	
	Mode: update_rule
	 
	 Invoked:
		When a rule is saved in the rule editor
	 
	 Parameters:
	 	
	 
	 Prints:
	 	An object of extjs response form {"items": [{group:str,id:int},.....,{}], "success": "true"/"false"}
		
	*/
	
	private function _update_rule() {
	
		$rule_id   = $this->input->post('ruleid');
		$rule_data = array();
		
		$rule_data['name'] 		  = $this->input->post('rulename');
		$rule_data['description'] = $this->input->post('ruledesc');
		$rule_data['owner'] 	  = $this->input->post('ruleowner');
		$rule_data['seq_index']   = $this->input->post('rulestartindex');
		$rule_data['appearance']  = $this->input->post('ruleAppearance');
		$rule_data['enable_rule'] = $this->input->post('ruleEnable');
		
		// APP
		$rule_app_cb    = $this->input->post('rule_applicationcheckbox');
		if($rule_app_cb && $rule_app_cb == '1') {
			$rule_data['app_id'] = $this->input->post('rule_applicationcombo');
		} else {
			$rule_data['app_id'] = '';
		}
		
		// IP
		$rule_ip_cb     = $this->input->post('rule_ipcheckbox');
		if($rule_ip_cb && $rule_ip_cb == '1') {
			$rule_data['user_ip'] = $this->input->post('rule_iptext');
		} else {
			$rule_data['user_ip'] = '';
		}
		
		$rule_ip_neg     = $this->input->post('rule_ipneg');
		if($rule_ip_neg && $rule_ip_neg == '1') {
			$rule_data['ip_neg'] = 1;
		} else {
			$rule_data['ip_neg'] = 0;
		}

		
		// USER
		$rule_user_cb   = $this->input->post('rule_usernamecheckbox');
		if($rule_user_cb && $rule_user_cb == '1') {
			$rule_data['user'] = $this->input->post('rule_usernametext');
		} else {
			$rule_data['user'] = '';
		}
		
		// SCORE
		$rule_score_type    = $this->input->post('rule_scoretype');
		if($rule_score_type == 'numeric') {
			$rule_data['numeric_score'] = $this->input->post('rule_scoretype_numeric');
			$rule_data['literal_score'] = '';
		} else {
			$rule_data['numeric_score'] = '';
			$rule_data['literal_score'] = $this->input->post('rule_scoretype_literal');
		}
		// TYPE
		$rule_type = $this->input->post('rule_ruletypecombo');
		$rule_data['rule_type'] = $rule_type;

		switch($rule_type) {
			
			case 'Aspect':
				
				$rule_data['aspect'] = $this->input->post('rule_aspect_aspecttype');
				$rule_data['personal'] = $this->input->post('rule_aspect_personal') == '1' ? '1' : '0';
				
			break;
			
			case 'Pattern':
				
				$rule_pattern_anchor_type = $this->input->post('Pattern_Anchor');
				$rule_data['anchor_type'] = $rule_pattern_anchor_type;
				
				switch($rule_pattern_anchor_type) {
				
					case 'IP':
						$rule_data['anchor'] = $rule_pattern_anchor_type;
					break;
					case 'SID':
						$rule_data['anchor'] = $rule_pattern_anchor_type;
					break;
					case 'Other': 
						$rule_data['anchor'] = $this->input->post('Pattern_OtherTextField');
					break;
				
				}
				
				$rule_data['dynamic_type'] = $this->input->post('Pattern_DynamicType');
				$rule_data['att_id']       = $this->input->post('Pattern_DynamicTextField');
				$rule_data['page_id']      = $this->input->post('Pattern_PageTextField');
				$rule_data['business_id']  = $this->input->post('Pattern_FlowSelect');

				$rule_data['pr_attrType'] = '';
				
				if($rule_data['att_id'] != '')      { $rule_data['pr_attrType'] = 'param'; }
				if($rule_data['page_id'] != '')     { $rule_data['pr_attrType'] = 'Page'; }
				if($rule_data['business_id'] != '') { $rule_data['pr_attrType'] = 'Flow'; }								
				
				$rule_data['count']    		= $this->input->post('Pattern_CountTextField');
				$rule_data['timetype'] 		= $this->input->post('Pattern_TimeWindowCombo');
				
				$rule_pattern_timewindow    = $this->input->post('Pattern_TimeWindowTextField');
				
				switch($rule_data['timetype']) {
				
					case 'sec':
						$rule_data['time']  = $rule_pattern_timewindow;
					break;
					case 'min':
						$rule_data['time']  = $rule_pattern_timewindow * 60;
					break;
					case 'hours':
						$rule_data['time']  = $rule_pattern_timewindow * 60 * 60;
					break;
					
				}

			break;
			
			case 'ParameterRule':
				
				$rule_param_att          = $this->input->post('ParameterRule_AttributeTextField');
				$rule_param_raw			 = $this->input->post('ParameterRule_AttributeTextRaw');
				$rule_param_page         = $this->input->post('ParameterRule_PageTextField');
				$rule_param_att_type     = $this->input->post('tParameterRule_AttributeTypeRadio');
				$rule_param_string_match = $this->input->post('tParameterRule_StringMatchTextField');
				$rule_param_length       = $this->input->post('tParameterRule_LengthCombo');
				$rule_param_exact        = $this->input->post('tParameterRule_exactlength');
				$rule_param_range_low    = $this->input->post('tParameterRule_rangelengthlow');
				$rule_param_range_high   = $this->input->post('tParameterRule_rangelengthhigh');
				$rule_param_not_signal   = $this->input->post('tParameterRule_NotSignal');
				$rule_param_similarity   = $this->input->post('tParameterRule_stringsimilarity');
				$rule_param_aspect_type  = $this->input->post('rule_aspect_aspecttype');
				
				// Aspect
				if($rule_param_aspect_type && $rule_param_aspect_type != '') {
					$rule_data['aspect'] = $rule_param_aspect_type;
				} else {
					$rule_data['aspect'] = '';
				}
				
				// Param
				if($rule_param_att && $rule_param_att != '') {
					$rule_data['att_id'] = $rule_param_att;
				} else {
					$rule_data['att_id'] = '';
				}
				
				// Param (Raw)
				if($rule_param_raw && $rule_param_raw != '' && $rule_data['att_id'] == '') {
					$rule_data['att_name'] = $rule_param_raw;
				} else {
					$rule_data['att_name'] = '';
				}
				
				// Page
				if($rule_param_page && $rule_param_page != '') {
					$rule_data['page_id'] = $rule_param_page;
				} else {
					$rule_data['page_id'] = '';
				}
				
				
				// Parameter Type
				$rule_data['pr_attrType'] = $rule_param_att_type;
				
				switch($rule_param_att_type) {
				
					case 'fuzzylength':
						$rule_data['str_length']     = $rule_param_length;
						$rule_data['str_similarity'] = $rule_param_similarity;
					break;
					case 'exactlength':
						$rule_data['str_length']     = $rule_param_length;
					break;
					case 'distance_similarity':
						$rule_data['str_similarity'] = $rule_param_similarity;
					break;
					case 'rangelength':
						$rule_data['str_length']     = intval($rule_param_range_low) . '-' . intval($rule_param_range_high);
						$rule_data['str_similarity'] = $rule_param_similarity;
					break;
					case 'gex':
					case 'stringmatch':
						$rule_data['not_signal']     = $rule_param_not_signal;
						$rule_data['str_match']      = $rule_param_string_match;
					break;
					
				}
				
			break;
			
			case 'Geo':
				
				$rule_geo_type   = $this->input->post('rule_geo_type');
				$rule_geo_radius = $this->input->post('rule_geo_radius');
				$rule_geo_time   = $this->input->post('rule_geo_time');
				$rule_geo_allow  = $this->input->post('rule_geo_allow');
				$rule_geo_block  = $this->input->post('rule_geo_block');
				
				$rule_data['pr_attrType'] = 'GEO';
				
				switch($rule_geo_type) {
					
					case 'Velocity':
					
						$rule_data['radius'] 	 = $rule_geo_radius;
						$rule_data['time']   	 = $rule_geo_time;
						$rule_data['str_match']  = '';
						$rule_data['not_signal'] = '0';
						
					break;
					case 'Allow':
						
						$rule_data['str_match']  = $rule_geo_allow;
						$rule_data['not_signal'] = '1';
						$rule_data['radius'] 	 = '0';
						$rule_data['time']   	 = '0';
						
					break;
					case 'Block':
						
						$rule_data['str_match']  = $rule_geo_block;
						$rule_data['not_signal'] = '0';
						$rule_data['radius'] 	 = '0';
						$rule_data['time']   	 = '0';
						
					break;
				
				}
				
			break;
			
			case 'Bot':
				
				$rule_data['str_match'] = $this->input->post('Bot_bot_type');
				
			break;
			
		}
		
		$this->RulesModel->update_rule($rule_id, $rule_data);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
	
	}
	
	private function _delete_rule() {
		
		$rule_id = $this->input->post('ruleid');
		if(!$rule_id) {
			$this->_fail();
		}
		
		$this->RulesModel->delete_rule($rule_id);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();

	}
	
	private function _delete_group() {
	
		$group_id = $this->input->post('groupid');
		if(!$group_id) {
			$this->_fail();
		}
		
		$this->RulesModel->delete_group($group_id);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
		
	}
	
	private function _delete_category() {
		
		$category_id = $this->input->post('category_id');
		if(!$category_id) {
			$this->_fail();
		}
		
		$this->RulesModel->delete_category($category_id);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
		
	}
	
	private function _create_new_rule_in_group() {
	
		$ans  = array('items' => array(), 'total' => 0, 'success' => true);
	
		$group_id = $this->input->post('groupId');
		$ans['items']['rule_id'] = $this->RulesModel->create_new_rule($group_id);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		return_json($ans);
		
	}
	
	private function _move_rule_to_group() {
	
		$rule_id  = $this->input->post('ruleId');
		$group_id = $this->input->post('groupId');
		$this->RulesModel->update_rule($rule_id, array('rule_group' => $group_id));
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
		
	}
	
	private function _move_group_to_category() {
		
		$group_id    = $this->input->post('group_id');
		$category_id = $this->input->post('category_id');
		
		$this->RulesModel->update_group($group_id, array('category_id' => $category_id));
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
		
	}
	
	private function _copy_group_to_category() {
	
		// Copy this group
		$group_id = $this->input->post('group_id');
		
		// To this category
		$category_id = $this->input->post('category_id');
		
		// Load group + Array Cast
		$group_data = (array) $this->RulesModel->get_group_by_id($group_id);
		
		if(!$group_data) {
			$this->_fail();
		}
		// Load category
		$category = $this->RulesModel->get_category_by_id($category_id);
		if(!$category) {
			$this->_fail();
		}
		
		// New name
		$group_data['name'] = $group_data['name'] . '_in_' . $category->name;
		
		// Create new category
		$new_group_id = $this->RulesModel->create_group($group_data['name'], 
														$group_data['description'], 
														$category_id, '1', '75');
		
		if(!$new_group_id) {
			$this->_fail();
		}
		
		// Update new category
		$this->RulesModel->update_group($new_group_id, $group_data);
		
		// Start copying rules
		$rules = $this->RulesModel->get_rules_by_group_id($group_id);
		
		if($rules) {
			
			// Load & Cast
			foreach($rules as $rule) {
				$rule_data = (array) $this->RulesModel->get_rule_by_id($rule->id);
				$rule_data['rule_group'] = $new_group_id;
				unset($rule_data['id']);
				$this->RulesModel->create_rule($rule_data);
			}
		
		} else {
			// Empty group -- no rules to copy.
		}
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(90, 1);
		
		$this->_success();
		
	}
	
	private function _check_group_duplication() {
	
		$group_name = $this->input->post('group_name', true);
				
		$ans  = array('success' => true, 'exists' => false);

		$group = $this->RulesModel->find_group_by_name($group_name);
		
		if($group && is_array($group) && !empty($group)) {
			$ans['exists'] = true;
		}
		
		return_json($ans);

	}
	
}