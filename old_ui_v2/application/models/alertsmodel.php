<?php

class AlertsModel extends CI_Model {
	
	private $tableName = 'alerts_merge';
	
	function __construct() {
		parent::__construct();
	}
	
	public function get_ips() {
		
		$this->db->select('IP');
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('hostname', $this->acl->allowed_apps);
		}
		
		$this->db->distinct();
		$this->db->from($this->tableName);
		$this->db->order_by('INET_ATON(IP) ASC');
		
		$query = $this->db->get();
		return $query->result();
	
	}
	
	public function get_session_flow_params($sid, $RIDS) {
		
		$ans    = array();
		
		// convert to array
		$RIDS = explode(',', $RIDS);
		
		$this->db->select('rule_group,alert_param_ids');
		$this->db->from('session_alerts,rule_groups');
		$this->db->where('sid', $sid);
		$this->db->where('session_alerts.rule_group=rule_groups.id', null, false);
		$query = $this->db->get();
		
		$rule = $query->result();

		$rule_group_id = $rule[0]->rule_group;
		$alert_param_ids = $rule[0]->alert_param_ids;
		
		if($alert_param_ids && strlen($alert_param_ids) > 0) {
			
			$this->load->model('AttributeScores');
		
			$alert_param_ids = explode(',', $alert_param_ids);
			foreach($RIDS as $RID) {
				
				$ans[$RID] = array();
				
				$params = $this->AttributeScores->get_specified_param_by_RIDs($alert_param_ids, $RID);
				
				if(!empty($params)) {
				
					foreach($params as $param) {
					
						$ans[$RID][] = array(
						
							'att_id'    => $param->att_id,
							'att_name'  => $param->att_name,
							'att_alias' => $param->att_alias,
							'val'       => $param->data,
							'mask'      => $param->mask
							
						);
					
					}
				
				} else {
				
					$this->db->distinct();
					$this->db->select('value,att_name,attributes.att_id,att_alias');
					$this->db->from('session_alerts,attributes');
					$this->db->where('attributes.att_id=session_alerts.att_id', null, false);
					$this->db->where('rule_group', $rule_group_id);
					$this->db->where('RID', $RID);
					$this->db->where('value !=', '');
					$query  = $this->db->get();
					$params = $query->result();
					
					if(!empty($params)) {
						
						foreach($params as $param) {
							
							$ans[$RID][] = array(
								'att_id'    => $param->att_id,
								'att_name'  => $param->att_name,
								'att_alias' => $param->att_alias,
								'val'       => $param->data
							);
							
						}
						
					}
					
				}
				
			}
			
		}
		
		return $ans;
	}
	
	public function get_session_flow($sid, $ip, $group_id) {
	
		// Nodes by sid
		$RIDS = array();
	
		// Here we need to check rule anchor 
		// Default
		$anchor = 'SID';
		$this->load->model('RulesModel');
		
		// Load group
		$rule_ids = $this->RulesModel->get_rules_by_group_id($group_id);
		if(!empty($rule_ids)) {
			foreach($rule_ids as $row) {
				$rule_id   = $row->id;
				$rule_data = $this->RulesModel->get_rule_by_id($rule_id);
				if(!empty($rule_data)) {
					if($rule_data->rule_type == 'Pattern' && $rule_data->anchor_type == 'IP') {
						$anchor = 'IP';
					}
					if($rule_data->rule_type == 'Pattern' && $rule_data->anchor_type == 'user') {
						$anchor = 'user';
						$requests = $this->RequestScores->get_requests_by_SID($sid);
						foreach($requests as $request) {
							if(intval($request->user_id) > 0) {
								$user_id = intval($request->user_id);
							}
						}
					}
				}
			}
		}
		

	
		$this->load->model('RequestScores');
		
		switch($anchor) {
			case 'SID':
				$requests = $this->RequestScores->get_requests_by_SID($sid);
			break;
			case 'IP':
				$requests = $this->RequestScores->get_requests_by_IP($ip);
			break;
			case 'user':
				$requests = $this->RequestScores->get_requests_by_user($user_id);
			break;
		}
		
		if(empty($requests)) {
			return array('requests' => array(), 'alerts' => array());
		}
		
		$this->load->model('AttributeScores');
		
		foreach($requests as $req) {
			$RIDS[] = $req->RID;
			
			if($req->page_id == 0 || $req->page_id == NULL) {
				$data = $this->AttributeScores->get_specified_param_by_RIDs(array(19), array($req->RID), true);
				if($data && !empty($data) && isset($data[0]->data) ) {
					$req->display_path = $data[0]->data;
					$req->title        = '';
				}
			}
			
		}
		
		$this->load->model('InvestigateModel');
		$flows = $this->InvestigateModel->flows_for_RIDS($RIDS);
		
		foreach($requests as $req) {
			foreach($flows as $flow) {
				if($req->RID == $flow->RID) {
					$req->business_id     = $flow->business_id;
					$req->business_status = $flow->status;
				}
			}
		}
				
		$this->db->select('rule_id, rule_groups.name, sid, ip, RID, t1.page_id, att_name, att_alias, numeric_score, literal_score, value, date, rule_group');
		$this->db->select('(SELECT aspect from rules where id=t1.rule_id) as aspect', false);
		$this->db->from('rule_groups,session_alerts t1');
		$this->db->join('attributes', 't1.att_id = attributes.att_id', 'left');
		$this->db->where('t1.rule_group=rule_groups.id', null, false);
		
		$this->db->join('pages', 't1.page_id=pages.page_id', 'left');
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		// These RIDS already have the correct SID/IP/user anchor applied
		switch($anchor) {
			case 'SID':
			case 'IP':
			case 'user':
				$this->db->where_in('RID', $RIDS);
			break;
		}
		
		$this->db->order_by('date');
		
		$this->db->limit(1000);
		
		$query = $this->db->get();
		$alerts = $query->result();
		
		foreach($alerts as $alert) {
		
			if($alert->att_alias != '') {
				$alert->att_name = $alert->att_alias;
			}
			if($alert->att_name == '' && ($alert->name == 'Bot Intelligence' || $alert->name == 'Geo Limitation')) {
				$alert->att_name = 'IP';
			}
			
		}
				
		return array('requests' => $requests, 'alerts' => $alerts);
	
	}
	
	public function delete($ids_to_delete) {

		if(!$this->acl->all_apps()) {
			$this->db->where_in('hostname', $this->acl->allowed_apps);
		}
	
		$this->db->where_in('id', $ids_to_delete);
		$this->db->from($this->tableName);
		$this->db->delete();
		
	}
	
	public function get_attributes_for_alert($SID, $group_id, $param_ids) {

		$ans = array();
		
		$this->db->select('RID, date');
		$this->db->from('session_alerts');
		$this->db->where('rule_group', $group_id);
		$this->db->where('SID', $SID);
		$this->db->limit(200);
		$query  = $this->db->get();
		$result = $query->result();
		
		$min = 0;
		$max = 0;
		$RIDs = array();
		
		// Get MIN/MAX for tablesplit
		
		foreach($result as $row) {
			$RIDs[] = $row->RID;			
		}
		
		$this->load->model('AttributeScores');
		
		$params = array();
		
		if(!empty($RIDs)) {
			$params = $this->AttributeScores->get_specified_param_by_RIDs($param_ids, $RIDs);
		}

		if(!empty($params)) {
			
			foreach($params as $param) {
			
				$ans[] = array(
					
					'par'    => $param->att_alias != '' ? $param->att_alias : $param->att_name,
					'val'    => $param->data,
					'par_id' => $param->att_id
					
				);
			
			}
		
		} else {
		
			$this->db->distinct();
			$this->db->select('value,att_name,attributes.att_id,att_alias');
			$this->db->from('session_alerts,attributes');
			$this->db->where('attributes.att_id=session_alerts.att_id', null, false);
			$this->db->where('rule_group', $group_id);
			$this->db->where('SID', $SID);
			$this->db->where('value !=', '');
			$query  = $this->db->get();
			$params = $query->result();
			
			if(!empty($params)) {
				
				foreach($params as $param) {
					
					$ans[] = array(
					
						'par'    => $param->att_alias != '' ? $param->att_alias : $param->att_name,
						'val'    => $param->value,
						'par_id' => $param->att_id
					
					);
					
				}
				
			}
			
		}
		
		return $ans;
		
	}
	
	public function get_alert_group_params() {	
		
		$params = array();
		$this->db->select('id,alert_param_ids');
		$this->db->from('rule_groups');
		$this->db->where('alert_param_ids !=', '');
		$query = $this->db->get();
		$result = $query->result();
		
		foreach($result as $row) {
			$params[$row->id] = $row->alert_param_ids;
		}
		
		return $params;
	
	}
	
	
	public function get_report_alerts() {
		
		$this->db->select('DISTINCT(alerts.SID), alerts.date,alerts.IP,rule_groups.name,rule_groups.description,pages.display_path,applications.app_domain,alerts.average_score,rule_groups.email_recipient,email,syslog,session_alerts.RID,rule_groups.alert_param_ids,alerts.id', false);
		$this->db->from('alerts,rule_groups,session_alerts,pages,applications');
		$this->db->where('alerts.rule_group=rule_groups.id', null, false);
		$this->db->where('session_alerts.rule_group=alerts.rule_group', null, false);
		$this->db->where('session_alerts.SID=alerts.SID', null, false);
		$this->db->where('session_alerts.page_id=pages.page_id', null, false);
		$this->db->where('applications.app_id=alerts.hostname', null, false);
		$this->db->where('applications.app_id=pages.app_id', null, false);
		$this->db->where('(email=1 or syslog=1)', null, false);
		$this->db->limit(100);
		
		$query = $this->db->get();
		$result = $query->result_array();
		
		return $result;
		
	}
	
	
	public function mark_report_alerts() {
		
		$this->db->where('email', '1');
		$this->db->or_where('syslog', '1');
		$this->db->update('alerts', array('email' => '0', 'syslog' => '0'));
		
	}
	
	
	public function get_alerts($variable, $val, $sortfield, $sortorder, $start, $limit, $filter) {
		
		$count_queries = array();
		$data_queries  = array();
		
		$total = 0;
		$ans   = array();
		$offset = 0;
		
		// Gather rule groups ids
		$rule_group_ids = array();
		
		if($filter) {
			if($filter['rule'] && is_array($filter['rule']) && !empty($filter['rule'])) {
				foreach($filter['rule'] as $rule) {
					switch($rule['type']) {
						case 'category':
							$this->load->model('RulesModel');
							$groups = $this->RulesModel->get_groups_by_category_id($rule['id']);
							if($groups && !empty($groups)) {
								foreach($groups as $group) {
									$rule_group_ids[] = $group->id;
								}
							}
						break;
						case 'group':
							$rule_group_ids[] = $rule['id'];
						break;
					}
				}
			}
		}
		
		// Figure out what rule groups we're quering for
		if($variable == 'country' || $variable == 'IP' || $variable == 'SID') {
				
			$this->db->select('rule_group');
		
			switch($variable) {
				
				case 'country':
					$this->db->distinct();
					$this->db->where($this->tableName . '.country', $val);
				break;
				case 'IP':
					$this->db->where($this->tableName . '.IP', $val);
				break;
				case 'SID':
					$this->db->where($this->tableName . '.SID', $val);
				break;
		
			}
			
			$this->db->from($this->tableName);
			$query  = $this->db->get();
			$result = $query->result();
			
			foreach($result as $row) {
				$rule_group_ids[] = $row->rule_group;
			}
			
		}
		
		if(in_array($variable, array('Rule', 'alerts_score_High', 'alerts_score_Low', 'alerts_score_Medium'))) {
			$rule_group_ids[] = $val;
		}
		
		$this->db->start_cache();
		
		$table = $this->tableName;
			
		$this->db->from($table . ', rule_groups');
		$this->db->where($table . '.rule_group = rule_groups.id', null, false);
		
		if(in_array($variable, array('IP', 'country', 'SID', 'id'))) {
			$this->db->where($table . '.' . $variable, $val);
		}
		if(!empty($rule_group_ids)) {
			$this->db->where_in($table . '.rule_group', $rule_group_ids);
		}
		if(!$this->acl->all_apps()) {
			$this->db->where_in('hostname', $this->acl->allowed_apps);
		}

		if($filter) {
			
			// FROM
			if($filter['from_date'] && intval($filter['from_date']) > 0) {
				$this->db->where('date >=', $filter['from_date']);
			}
			// TO
			if($filter['to_date'] && intval($filter['to_date']) > 0) {
				$this->db->where('date <=', $filter['to_date']);
			}
			
			// APP Search
			if(isset($filter['apps']) && !empty($filter['apps'])) {
				$this->db->where_in('hostname', $filter['apps']);
			}
			
			// SUB Search
			if(isset($filter['subs']) && !empty($filter['subs'])) {
				$this->db->where_in($this->tableName . '.subdomain_id', $filter['subs']);
			}
			
			// TEXT Search
			if(isset($filter['freetext_app']) && $filter['freetext_app'] != '') {
				$this->db->like('subdomains.subdomain_name', $filter['freetext_app']);
			}
			
			// IP
			if($filter['ip'] && is_array($filter['ip']) && !empty($filter['ip'])) {
				$this->db->where_in('IP', $filter['ip']);
			}
			
		}
	
		// Join to subdomains table
		$this->db->select('subdomains.subdomain_name as subdomain');
		$this->db->join('subdomains', 'subdomains.subdomain_id=' . $table . '.subdomain_id', 'left');
	
		// #1 only count
		$this->db->stop_cache();
		$q_count = $this->db->get();
		$r_count = $q_count->num_rows();

		// #2 get data
		$this->db->select($table . '.id,date,SID,user,IP,name,description,average_score,counter,country,city');
		
		// Keeping this for compatability, TODO: remove in the future
		$this->db->select('applications.app_domain as hostname');
		$this->db->join('applications', 'applications.app_id=' . $table . '.hostname', 'left');
		
		
		$this->db->select('rule_group');
		
		// Limit
		$this->db->limit($limit, $start);
		
		// Sort
		$this->db->order_by($sortfield . ' ' . $sortorder);
		if($sortfield == 'country') { $this->db->order_by('city' . ' ' . $sortorder); }
		if($sortfield == 'hostname') { $this->db->order_by('subdomain' . ' ' . $sortorder); }
		
		// Query
		$q_items = $this->db->get();
		
		// Debug
		// echo $this->db->last_query();
				
		$r_items = $q_items->result();
		
		$this->db->flush_cache();
		
		return array('items' => $r_items, 'total' => $r_count);
		
	}
	
}

?>
