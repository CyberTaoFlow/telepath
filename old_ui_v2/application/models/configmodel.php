<?php

class ConfigModel extends CI_Model {
	
	private $tableName = 'config';
	private $agentsRegexTableName = 'agents_regex';
	private $whitelistIPsTableName = 'whitelist_ip';
	private $ignorelistIPsTableName = 'ignorelist_ip';
	
	public function set_atms_actions($atms_codes) {
		
		// Roi requested to disable updating ATMS actions
		return;
		
		$field = 'atmsActions';
		
		$data = array(
               'value' => $atms_codes,
        );

		$this->db->where('name', $field);
		$this->db->update($this->tableName, $data); 

	}
	
	public function get_atms_actions() {
		
		$field = 'atmsActions';
		
		$atms_actions = $this->db->select('value')
					  		 ->from($this->tableName)
							 ->where('name', $field);
							
		$query = $this->db->get();
		$ans = $query->result();
		if($ans && $ans[0] && $ans[0]->value) {
			return $ans[0]->value;
		} else {
			return '';
		}

	}

	public function update_atms_actions($new_code) {
		
		$codes = $this->get_atms_actions();
		$codes = explode(',', $codes);
		
		$new_codes = array();
		$found = false;
		
		foreach($codes as $code) {
			if($code == $new_code) {
				return;
			}
		}
		
		$codes[] = $new_code;
		$codes = implode(',', $codes);
				
		$this->set_atms_actions($codes);
		
	}
	
	function __construct()
	{
		parent::__construct();
	}
	
	// key for 1 value, otherwise = all
	public function update_action_code($action_code, $value) {
		$this->db->where('action_code', $action_code);
		$this->db->update($this->tableName, array('value' => $value));
	}
	
	public function get($key = false) {

		$this->db->select('name, value');
		$this->db->from($this->tableName);
		
		if($key) {
			$this->db->where('name', $key);
		}
		 
		$query = $this->db->get();
		$config = $query->result();
		
		$ans = array();
		if(is_array($config) && !empty($config)) {
			foreach($config as $conf) {
				$ans[$conf->name] = $conf->value;
			}
		} else {
			return false;
		}

		return $ans;
		
	}
	
	public function set_agents($agents) {
		
		// Empty, clear all records
		if(empty($agents)) {
		
			$this->db->where('1=1', null, false);
			$this->db->delete('agents');
			return;
			
		} else {

			// Has items, compare to current
			$current_list = $this->get_agents();
		
			foreach($agents as $id => $agent) {
				if($agent['idx'] == NULL) { // No IDX == NEW
					unset($agent['idx']);
					$this->db->insert('agents', $agent);
					unset($agents[$id]);
				}
			}
			
			foreach($current_list as $current_agent) {
				
				$found = false;				
				foreach($agents as $agent) {
					
					if($agent['idx'] == $current_agent->idx) {
						// Update Task
						$found = true;
						$this->db->where('idx', $agent['idx']);
						unset($agent['idx']);
						$this->db->update('agents', $agent);
					}
					
				}
				
				if(!$found) {
					// Not found -- Remove Task
					$this->db->where('idx', $current_agent->idx);
					$this->db->delete('agents');
				}
				
			}
			
		}
				
	}
	
	public function get_agents() {
		$this->db->select('idx, agent_id, FilterExpression, NetworkInterface');
		$this->db->from('agents');
		$query = $this->db->get();
		$result = $query->result();
		return $result;
	}
	
	public function get_regex() {
		
		$ans = array();
		$this->db->select('header_name, regex');
		$this->db->from('agents_regex');
		$query = $this->db->get();
		$result = $query->result();
		foreach($result as $row) {
			$ans[$row->header_name] = $row->regex;
		}
		return $ans;
		
	}
	
	public function get_learning_so_far() {
	
		$field = 'learning_so_far';
		
		$atms_actions = $this->db->select('value')
					  		 ->from($this->tableName)
							 ->where('name', $field);
							
		$query = $this->db->get();
		$ans = $query->result();
		if($ans && $ans[0] && $ans[0]->value) {
			return $ans[0]->value;
		} else {
			return '';
		}

	}
	
	public function update($key, $value) {
		
		$this->db->where('name', $key);
		$this->db->update($this->tableName, array('value' => $value));
		
	}
	
	public function insert($key, $value) {
	
		$this->db->insert($this->tableName, array('name' => $key, 'value' => $value));
		
	}

	// Update regex in agents_regex table (headers, URL)
	public function update_agents_regex($header_name, $regex) {
		
		$this->db->where('header_name', $header_name);
		$this->db->update($this->agentsRegexTableName, array('regex' => $regex));
		
	}
	
	public function whitelist_get_ips() {
		
		$this->db->select('user_ip');
		$this->db->distinct();
		$this->db->from($this->whitelistIPsTableName);
		$query = $this->db->get();
		$result = $query->result();
		$ans = array();
		foreach($result as $row) {
			$ans[] = $row->user_ip;
		}
		return $ans;
		
	}
	
	public function whitelist_delete_ip($ip) {
		
		$this->db->where('user_ip', $ip);
		$this->db->from($this->whitelistIPsTableName);
		$this->db->delete();
		
	}
	
	public function whitelist_add_ip($ip) {
		
		$this->db->insert($this->whitelistIPsTableName, array('user_ip' => $ip)); 
		return $this->db->insert_id();
		
	}
	
	public function ignorelist_get_ips() {
		
		$this->db->select('user_ip');
		$this->db->distinct();
		$this->db->from($this->ignorelistIPsTableName);
		$query = $this->db->get();
		$result = $query->result();
		$ans = array();
		foreach($result as $row) {
			$ans[] = $row->user_ip;
		}
		return $ans;
		
	}
	
	public function ignorelist_delete_ip($ip) {
		
		$this->db->where('user_ip', $ip);
		$this->db->from($this->ignorelistIPsTableName);
		$this->db->delete();
		
	}
	
	public function ignorelist_add_ip($ip) {
		
		$this->db->insert($this->ignorelistIPsTableName, array('user_ip' => $ip)); 
		return $this->db->insert_id();
		
	}
	
	public function get_scheduler($mode) {
		
		// Sanity Check
		$table = false;		
		if($mode == 'scheduler') { $table = 'scheduler'; }
		if($mode == 'report_scheduler') { $table = 'report_scheduler'; }
		if(!$table) return;	
		
		$sql = "SELECT * FROM " . $table; 
		$q   = $this->db->query($sql);
		$ans = array();
		
		while ($row = mysql_fetch_array($q->result_id, MYSQL_ASSOC)) {
			$key = array_shift($row);
			$ans[$key] = $row;
		}
		
		return $ans;
		
	}
	
	public function set_scheduler($mode, $data) {
		
		// Sanity Check
		$table = false;		
		if($mode == 'scheduler') { $table = 'scheduler'; }
		if($mode == 'report_scheduler') { $table = 'report_scheduler'; }
		if(!$table) return;	

		// VALIDATE DATA
		$weekday = array("Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday");
		$tmp     = array();
		
		// Check All Days are present and no extra keys
		foreach($data as $key => $value) {
			if(in_array($key, $weekday)) {
				$tmp[$key] = true;
			}
		}
		if(count($tmp) != 7) {
			return array('success' => false);
		}
		
		// Check All Hours are present
		foreach($data as $key => $value) {
			if(count($value) != 24) {
				return array('success' => false);
			}
		}
		
		// Check All Hours are numeric
		foreach($data as $key1 => $value1) {
			foreach($value1 as $key2 => $value2) {
				if(intval($value) === 0 || intval($value) === 1) {
					$data[$key1][$key2] = intval($value2);
				} else {
					return array('success' => false);
				}
			}
		}
		
		// Done sanity check, lets do queries
		
		foreach($data as $day => $schedule) { // SUN, MON..
			
			$this->db->where('day', $day);
			
			$updates = array();
			foreach($schedule as $hour => $mode) { // 0 => 0, 1 => 0, 2 => 1 ...
				$updates[$hour] = $mode;
			}
			
			$this->db->update($table, $updates);
			
		}

		return array('success' => true);
			
	}
	
}

?>
