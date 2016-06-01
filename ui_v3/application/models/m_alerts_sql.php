<?php

class M_Alerts extends CI_Model {
	
	private $tableName = 'alerts_merge';
	
	function __construct() {
		parent::__construct();
	}
	
	public function alerts_for_RIDS($RIDS = array()) {
		if(empty($RIDS)) {
			return array();
		}
		
		// Get Alerts
		$this->db->select('rule_id, rule_groups.name, rule_groups.description, sid, ip, RID, t1.page_id, att_name, att_alias, numeric_score, literal_score, value, date, rule_group');
		$this->db->select('(SELECT aspect from rules where id=t1.rule_id) as aspect', false);
		$this->db->from('rule_groups,session_alerts t1');
		$this->db->join('attributes', 't1.att_id = attributes.att_id', 'left');
		$this->db->where('t1.rule_group=rule_groups.id', null, false);
		
		$this->db->join('pages', 't1.page_id=pages.page_id');
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		$this->db->where_in('RID', $RIDS);
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
		
		return $alerts;
		
	}
	
	public function alert_for_RID($RID) {
		
		$this->db->where('RID', $RID);
		$this->db->from('session_alerts');
		$res = $this->db->get()->result();
		return empty($res) ? false : $res[0];
	
	}
	
	public function get($limit = false) {
	
		if($limit) { $this->db->limit($limit); }
		
		$q = $this->db->get('alerts');
		return $q->result();
		
	}
	
	public function count_alerts($to, $from, $rule_group, $apps = array(), $case_id = -1) {
	
		if(!$this->acl->all_apps()) {
			$this->db->where_in('hostname', $this->acl->allowed_apps);
		}
		
		$this->db->from($this->tableName);
		$this->db->where('date >=', $from);
		$this->db->where('date <=', $to);
		
		if(!empty($apps)) {
			$this->db->where_in('hostname', $apps);
		}
		
		if(intval($case_id) > -1) {
			$this->db->join('case_alerts','case_alerts.alert_id=' . $this->tableName . '.id', 'right');
			$this->db->where('case_alerts.case_id', $case_id);
		}

		$c = $this->db->count_all_results();
		//echo $this->db->last_query() . '<br>';
		return $c;
		
		
	}
	
	public function time_diff($date, $count, $mode) {
		$diff = $this->db->query('SELECT UNIX_TIMESTAMP(DATE_SUB(FROM_UNIXTIME(' . $date . ') ,INTERVAL "' . $count . '" ' . $mode . ')) as diff')->result();
		return $diff[0]->diff;
	}
	
	public function get_time_chart($range, $apps = array(), $case_id = -1) {
		
		$chart = array();
		$gap = get_gap($range);
		$rule_group_id = 0;		
				
		switch($gap) {
		
			case 'hoursToDay':
				$index = 1;
				while($index <= 24) {
					$from  = $this->time_diff($range['end'], $index - 1, 'HOUR');
					$to    = $this->time_diff($range['end'], $index, 'HOUR');
					$count = $this->count_alerts($from, $to, $rule_group_id, $apps, $case_id);
					$chart[] = array($count, $to);
					$index += 1;
				}
			break;
			case 'dayToWeek':
				$index = 1;
				while($range['start'] <= $this->time_diff($range['end'], $index, 'DAY')) {
					$from  = $this->time_diff($range['end'], $index - 1, 'DAY');
					$to    = $this->time_diff($range['end'], $index, 'DAY');
					$count = $this->count_alerts($from, $to, $rule_group_id, $apps, $case_id);
					$chart[] = array($count, $to);
					$index += 1;
				}
				
				//$to = $this->time_diff($range['end'], $index - 1, 'DAY');
				//$count = $this->count_alerts($range['start'], $to, $rule_group_id, $apps, $case_id);
				//$chart[] = array($count, $range['start']);
				
			break;
			case 'weekToMonth':
				$index = 3;
				while($range['start'] <= $this->time_diff($range['end'], $index, 'DAY')) {
					$from  = $this->time_diff($range['end'], $index - 3, 'DAY');
					$to    = $this->time_diff($range['end'], $index, 'DAY');
					$count = $this->count_alerts($from, $to, $rule_group_id, $apps, $case_id);
					$chart[] = array($count, $to);
					$index += 3;
				}
				
				$to = $this->time_diff($range['end'], $index - 3, 'DAY');
				$count = $this->count_alerts($range['start'], $to, $rule_group_id, $apps, $case_id);
				$chart[] = array($count, $range['start']);
				
			break;
			case 'monthTo6months':
				$index = 2;
				while($range['start'] <= $this->time_diff($range['end'], $index, 'WEEK')) {
					$from  = $this->time_diff($range['end'], $index - 2, 'WEEK');
					$to    = $this->time_diff($range['end'], $index, 'WEEK');
					$count = $this->count_alerts($from, $to, $rule_group_id, $apps, $case_id);
					$chart[] = array($count, $to);
					$index += 2;
				}
				
				$to = $this->time_diff($range['end'], $index - 2, 'WEEK');
				$count = $this->count_alerts($range['start'], $to, $rule_group_id, $apps, $case_id);
				$chart[] = array($count, $range['start']);
				
			break;
			case '6monthsToYear':
				$index = 1;
				while($range['start'] <= $this->time_diff($range['end'], $index, 'MONTH')) {
					$from  = $this->time_diff($range['end'], $index - 1, 'MONTH');
					$to    = $this->time_diff($range['end'], $index, 'MONTH');
					$count = $this->count_alerts($from, $to, $rule_group_id, $apps, $case_id);
					$chart[] = array($count, $to);
					$index += 1;
				}
				
				$to = $this->time_diff($range['end'], $index - 1, 'MONTH');
				$count = $this->count_alerts($range['start'], $to, $rule_group_id, $apps, $case_id);
				$chart[] = array($count, $range['start']);
				
			break;
			case 'aYearPlus':
				$index = 3;
				while($range['start'] <= $this->time_diff($range['end'], $index, 'MONTH')) {
					$from  = $this->time_diff($range['end'], $index - 3, 'MONTH');
					$to    = $this->time_diff($range['end'], $index, 'MONTH');
					$count = $this->count_alerts($from, $to, $rule_group_id, $apps, $case_id);
					$chart[] = array($count, $to);
					$index += 3;
				}
				
				$to = $this->time_diff($range['end'], $index - 3, 'MONTH');
				$count = $this->count_alerts($range['start'], $to, $rule_group_id, $apps, $case_id);
				$chart[] = array($count, $range['start']);
				
			break;
		
		}
		
		$result = array();
		if(!empty($chart)) {
			foreach($chart as $row) {
				$row[1] = $row[1] * 1000;
				$result[] = array_reverse($row);
			}
		}
		
		return $result;
		
	}
	
	public function get_distribution_chart($alerts) {
	
		$dist   = array();
		$result = array();
		$max    = 5;
		
		foreach($alerts as $alert) {
			$key = $alert->name;
			if(!isset($dist[$key])) { $dist[$key] = 0; }
			$dist[$alert->name]++;
		}

		if(!empty($dist)) {
		
			arsort($dist);
			$i = 0;
			$other = 0;
			
			foreach($dist as $label => $count) {
			
				$i++;
				if($i < $max) {
					$result[] = array('label' => str_replace('_', ' ', $label), 'data' => intval($count));
				} else {
					$other += intval($count);
				}
				
			}
			
			if($other > 0) {
				$result[] = array('label' => 'Other', 'data' => $other);
			}
			
		}

		return $result;

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
			if(isset($filter['rule']) && is_array($filter['rule']) && !empty($filter['rule'])) {
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
					$this->db->where('alerts.country', $val);
				break;
				case 'IP':
					$this->db->where('alerts.IP', $val);
				break;
				case 'SID':
					$this->db->where('alerts.SID', $val);
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
		
		$table = $this->tableName;
			
		$this->db->from($table . ', rule_groups');
		$this->db->where($table . '.rule_group = rule_groups.id', null, false);
		
		if(in_array($variable, array('IP', 'country', 'SID', 'msg_id'))) {
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
			if(isset($filter['from_date']) && intval($filter['from_date']) > 0) {
				$this->db->where('date >=', $filter['from_date']);
			}
			// TO
			if(isset($filter['to_date']) && intval($filter['to_date']) > 0) {
				$this->db->where('date <=', $filter['to_date']);
			}
			// APP_ID
			if(isset($filter['apps']) && !empty($filter['apps'])) {
				$this->db->where_in('hostname', $filter['apps']);
			}
			// IP
			if(isset($filter['ip']) && is_array($filter['ip']) && !empty($filter['ip'])) {
				$this->db->where_in('IP', $filter['ip']);
			}
			// CaseID
			if(isset($filter['cid']) && intval($filter['cid']) > -1) {
				$this->db->join('case_alerts','case_alerts.alert_id=' . $this->tableName . '.id', 'right');
				$this->db->where('case_alerts.case_id', intval($filter['cid']));
			}
			
		}

		$this->db->select('SQL_CALC_FOUND_ROWS ' . $table . '.id', false);
		$this->db->select('date,SID, user, IP, name, description, average_score, counter,country,city');
		$this->db->select('(SELECT app_domain from applications WHERE app_id=hostname) as hostname', false);
		$this->db->select('rule_group');
				
		
		$this->db->limit($limit, $start);
		
		
		if($sortfield == 'type') {
			$this->db->order_by('rule_groups.name', $sortorder);
		} else {
			$this->db->order_by($sortfield . ' ' . $sortorder);
			if($sortfield == 'country') { $this->db->order_by('city' . ' ' . $sortorder); }
		}
		
		$items = $this->db->get()->result();
		$total = $this->db->query('SELECT FOUND_ROWS() as total;')->row_array();
		$total = $total['total'];
		
		return array('items' => $items, 'total' => $total);
		
	}
	
}

?>
