<?php

class DashboardModel extends CI_Model {
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function getLimits() {
		
		$this->db->select('MIN(date) as date_min', FALSE);
		$this->db->select('MAX(date) as date_max', FALSE);
		$this->db->from('request_scores_merge');
		$res = $this->db->get();
		return $res->result();
	
	}
	
	public function setDashboardOrder($order) {
		
		foreach($order as $key => $value) {
			$this->db->update('dashboard_order', array('panelId' => $value) , array('index' => $key));
		}
		
	}

	public function setSettings($date1, $date2, $refresh) {
	
		$this->db->update('config', array('value' => $date1)    , array('name' => 'time_start'));
		$this->db->update('config', array('value' => $date2)    , array('name' => 'time_end'));
		$this->db->update('config', array('value' => $refresh)  , array('name' => 'timerInterval'));
	
	}
	
	public function getRange($scale = '') {
		
		// Sort out range
		
		$q1 = "select value as date from config where name='time_start'";
		$q2 = "select value as date from config where name='time_end'";
		
		$date1 = $this->db->query($q1)->result();
		$date1 = $date1[0]->date;
		
		$date2 = $this->db->query($q2)->result();
		$date2 = $date2[0]->date;
		
		return array('from' => $date1, 'to' => $date2);
		
	}
	
	public function getAnomalies($range, $app_ids = array(), $subdomain_ids = array()) {

		// Get rule groups
		
		$this->load->model('RulesModel');
		$rule_groups = $this->RulesModel->get_rule_groups(true);
		
		$this->db->where_in('rule_group', array_keys($rule_groups));
		
		// Restrain to just selected hostnames / subdomains in addition to allowed ones
		if(!empty($app_ids))        { $this->db->where_in('hostname', $app_ids); }
		if(!empty($subdomain_ids))  { $this->db->where_in('subdomain_id', $subdomain_ids); }
		if(!$this->acl->all_apps()) { $this->db->where_in('hostname', $this->acl->allowed_apps); }
		
		$this->db->select('count(*) as data');
		$this->db->select('rule_group as id');
		$this->db->from('alerts_merge');
		$this->db->where('date >=', $range['from']);
		$this->db->where('date <=', $range['to']);
		$this->db->group_by('rule_group');
		$this->db->order_by('data DESC');
		$this->db->limit(10);
		
		$query = $this->db->get();
		
		$result = $query->result_array();

		foreach($rule_groups as $rule_group_id => $rule_group_name) {
			foreach($result as $index => $row) {
				if($row['id'] == $rule_group_id) {
					$result[$index]['name'] = $rule_group_name;
				}
			}
		}
		
		// Return top 10 only
		
		return $result;
		
	}
	
	public function getSuspects($range, $app_ids = array(), $subdomain_ids = array(), $limit = 25, $useTableSplit = true) {
		
		$this->db->select('user_ip as IP, date, city, country, avg_score as score, SID, user_id,query_score,flow_score,landing_normal,geo_normal');
		$this->db->from('request_scores_merge');
		$this->db->where('date >=', $range['from']);
		$this->db->where('date <=', $range['to']);
		$this->db->where('country !=', '00');
		
		$this->db->where('avg_score >', 0.85);
		$this->db->order_by('avg_score DESC');
		

		// Restrain to just selected hostnames / subdomains in addition to allowed ones
		if(!empty($app_ids))        { $this->db->where_in('hostname', $app_ids); }
		if(!$this->acl->all_apps()) { $this->db->where_in('hostname', $this->acl->allowed_apps); }
		if(!empty($subdomain_ids))  { $this->db->where_in('subdomain_id', $subdomain_ids); }
			
		$this->db->limit($limit * 10);
		$query = $this->db->get();
		$result = $query->result();
		
		$ips = array();
		foreach($result as $key => $row) {
			if(isset($ips[$row->IP])) {
				unset($result[$key]);
			} else {
				$ips[$row->IP] = true;
			}
		}
		
		return array_slice($result, 0, $limit);
				
	}
	
	public function get_map($range, $app_ids = array(), $subdomain_ids = array()) {
		
		$this->db->select('country, count(`country`) AS sum_count');
		$this->db->where('date >=', $range['from']);
		$this->db->where('date <=', $range['to']);
		$this->db->group_by('country');
		$this->db->order_by('sum_count DESC');
		$this->db->from('alerts_merge');
		
		if(!empty($app_ids))        { $this->db->where_in('hostname', $app_ids); }
		if(!$this->acl->all_apps()) { $this->db->where_in('hostname', $this->acl->allowed_apps); }
		if(!empty($subdomain_ids))  { $this->db->where_in('subdomain_id', $subdomain_ids); }
		
		$query = $this->db->get();
		
		return $query->result();
		
	}
	
	// MAP -- Bad Points
	public function getBadPoints($range, $app_ids = array(), $subdomain_ids = array()) {
		
		$this->db->distinct();
		$this->db->select('latitude,longitude,id');
		$this->db->from('alerts_merge');
		$this->db->where('date >=', $range['from']);
		$this->db->where('date <=', $range['to']);
		$this->db->order_by('id DESC');
		$this->db->limit(100);
		
		// Restrain to just selected hostnames / subdomains in addition to allowed ones
		if(!empty($app_ids))        { $this->db->where_in('hostname', $app_ids); }
		if(!$this->acl->all_apps()) { $this->db->where_in('hostname', $this->acl->allowed_apps); }
		if(!empty($subdomain_ids))  { $this->db->where_in('subdomain_id', $subdomain_ids); }
			
		$query = $this->db->get();
		return $query->result();
			
	}
	
	// MAP -- Clusters
	public function getClusters($range, $app_ids = array(), $subdomain_ids  = array()) {
		
		$this->db->select('x_centoid,y_centoid,radius');
		$this->db->from('clusters');
		
		// Restrain to just selected hostnames / subdomains in addition to allowed ones
		if(!empty($app_ids))        { $this->db->where_in('hostname', $app_ids); }
		if(!$this->acl->all_apps()) { $this->db->where_in('hostname', $this->acl->allowed_apps); }
		// if(!empty($subdomain_ids))  { $this->db->where_in('subdomain_id', $subdomain_ids); } // Not applicable
		
		$query = $this->db->get();
		return $query->result();
		
	}
	
	public function getCountries($range, $app_ids = array(), $subdomain_ids = array()) {
		
		// Get rule groups
		
		$this->load->model('RulesModel');
		$rule_groups = $this->RulesModel->get_rule_groups(true);
		
		$this->db->where_in('rule_group', array_keys($rule_groups));
		
		$this->db->select('count(*) as sum');
		$this->db->select('country');
		$this->db->from('alerts_merge');
		$this->db->where('date >=', $range['from']);
		$this->db->where('date <=', $range['to']);
		$this->db->group_by('country');
		$this->db->order_by('sum DESC');
		$this->db->limit(10);
		
		// Restrain to just selected hostnames / subdomains in addition to allowed ones
		if(!empty($app_ids))        { $this->db->where_in('hostname', $app_ids); }
		if(!$this->acl->all_apps()) { $this->db->where_in('hostname', $this->acl->allowed_apps); }
		if(!empty($subdomain_ids))  { $this->db->where_in('subdomain_id', $subdomain_ids); } // Not applicable	
		
		$query = $this->db->get();
		return $query->result();
				
	}
	
	public function getRuleCountByRange($from, $to, $rule_group_id, $app_ids = array(), $subdomain_ids = array()) {
		
		$this->db->from('alerts_merge');
		$this->db->where('date >=', intval($from));
		$this->db->where('date <=', intval($to));
		
		if(is_array($rule_group_id)) {
			$this->db->where_in('rule_group', $rule_group_id);
		} else {
			$this->db->where('rule_group', $rule_group_id);
		}
		
		// Restrain to just selected hostnames / subdomains in addition to allowed ones
		if(!empty($app_ids))        { $this->db->where_in('hostname', $app_ids); }
		if(!$this->acl->all_apps()) { $this->db->where_in('hostname', $this->acl->allowed_apps); }
		if(!empty($subdomain_ids))  { $this->db->where_in('subdomain_id', $subdomain_ids); } // Not applicable
		
		return $this->db->count_all_results();
		
	}
	
	public function getSessionAvgByRange($from, $to, $app_id = array()) {
		
		$this->db->select('SUM(count) as avg', false);
		$this->db->from('session_stats_hosts');
		$this->db->where('epoch >=', intval($from));
		$this->db->where('epoch <=', intval($to));
		
		if(!empty($app_id)) {
			$this->db->where_in('hostname', $app_id);
		}
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('hostname', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();
		$result = $query->result();
						
		$result = $result[0]->avg == NULL ? 0 : intval($result[0]->avg);
		
		$result = intval($result / (($to - $from) / 60));
		
		return $result;

	}
	
	public function getTimeDifference($date, $count, $mode) {
		
		$diff = $this->db->query('SELECT UNIX_TIMESTAMP(DATE_SUB(FROM_UNIXTIME(' . $date . ') ,INTERVAL "' . $count . '" ' . $mode . ')) as diff')->result();
		return $diff[0]->diff;
		
	}
	
	public function getRanges($range, $gap, $scale) {
		
		$scopes = array(
			'minutesToHour' => array(
				'offset' => 61,
				'mod'    => 10,
				'type'   => 'minutes'
			),
			'hoursToDay' => array(
				'offset' => 26,
				'mod'    => 1,
				'type'   => 'hours'
			),
			'dayToWeek' => array(
				'offset' => 8,
				'mod'    => 1,
				'type'   => 'days'
			),
			'weekToMonth' => array(
				'offset' => 34,
				'mod'    => 3,
				'type'   => 'days'
			),
			'monthTo6months' => array(
				'offset' => 7,
				'mod'    => 1,
				'type'   => 'month'
			),
			'6monthsToYear' => array(
				'offset' => 13,
				'mod'    => 1,
				'type'   => 'month'
			),
			'aYearPlus' => array(
				'offset' => 31,
				'mod'    => 3,
				'type'   => 'month'
			)
		);
		
		$range_scope = intval(($range['to'] - $range['from']) / 7);
		for($i = 0; $i < 7; $i++) {
			
			$from = $range['from'] + ($i * $range_scope);
			$to	  = $range['from'] + (($i + 1) * $range_scope);
			$ranges[] = array($from, $to);
			
		}
		//print_r($ranges);
		return $ranges;
		
		switch($scale) {
			case 'Year':
				$scope = '6monthsToYear';
			break;
			case 'Month':
				$scope = 'weekToMonth';
			break;
			case 'Week':
				$scope = 'dayToWeek';
			break;
			case 'Day':
				$scope = 'hoursToDay';
			break;
			case 'Hour':
			case 'Now':
				$scope = 'minutesToHour';
			break;
			case 'Custom Range':
			
			break;
		}
		
		$scope = $scopes[$scope];
		$ranges = array();
		$index = $scope['offset'];
		while($index > 1) {
			$index-= $scope['mod'];
			$from = strtotime('-' . $index . ' ' . $scope['type'], $range['to']);
			$to   = strtotime('-' . ($index - 1) . ' ' . $scope['type'], $range['to']);
			$ranges[] = array($from, $to);
		}
		return $ranges;
		
	}
	
	public function getTrends($range, $app_ids = array(), $subdomain_ids = array(), $gap, $scale) {
	
		$this->load->model('RulesModel');
		//$rule_groups = $this->RulesModel->get_rule_groups(true);
		$rule_groups = array();
		$categories = $this->RulesModel->get_categories();
		if($categories && is_array($categories) && !empty($categories)) {
			
			foreach($categories as $category) {
				
				$groups     = $this->RulesModel->get_groups($category->id);
				if($groups && is_array($groups) && !empty($groups)) {
					foreach($groups as $group) {
						if(intval($group->num) > 0) {
							$rule_groups[$group->id] = $group->name;
						}
					}
				}
								
			}
			
		}
		
		
		$ans = array();
		
		$brute_force_processed = false;
		
		foreach($rule_groups as $rule_group_id => $rule_group_name) {
			
			// Cleanup for EXT
			$rule_group_name = str_replace('  ', ' ', $rule_group_name);
			$rule_group_name = str_replace(':', '', $rule_group_name);
			$rule_group_name = str_replace('.php', '', $rule_group_name);
			
			if(strpos($rule_group_name, 'Login Brute Force Attempt') === 0) {
				
				if($brute_force_processed) {
					continue;
				}
			
				// Scan all rules at this point, collect Login Brute Force ID's, query once
				$ids = array();
				foreach($rule_groups as $id => $name) {
					if(strpos($name, 'Login Brute Force Attempt') === 0) {
						$ids[] = $id;
					}
				}
				
				$brute_force_processed = true;
				
				// Rewrite our loop vars
				$rule_group_name = 'Login Brute Force Attempt'; // Skip the <page> part
				$rule_group_id   = $ids;
				
			}
			
			$start_index    = 0;
			$trends			= array();
			
			$ranges = $this->getRanges($range, $gap, $scale);
			foreach($ranges as $r) {
				$count = $this->getRuleCountByRange($r[0], $r[1], $rule_group_id, $app_ids, $subdomain_ids);
				$trends[] = array($count, $r[1]);
			}
	
			$ans[$rule_group_name] = array_reverse($trends);
			
		}
		
		return $ans;
		
	}
	
	public function getSessions($range, $app_ids = array(), $subdomain_ids = array(), $gap, $scale) {
	
		$sessions 		= array();
		$ranges = $this->getRanges($range, $gap, $scale);

		foreach($ranges as $r) {
			$count = $this->getSessionAvgByRange($r[0], $r[1], $app_ids);
			$sessions[] = array($count, $r[1]);
		}
		
		return array_reverse($sessions);
		
	}
	
	public function getDashboardOrder() {
		
		$this->db->select('panelId');
		$this->db->from('dashboard_order');
		$this->db->order_by('index ASC');
		$query = $this->db->get();
		return $query->result();
		
	}
	
	public function getETA() {
		
		$this->db->select('value');
		$this->db->from('config');
		$this->db->where('name', 'ETA');
		$query = $this->db->get();
		return $query->result();
	
	}
	
}

?>
