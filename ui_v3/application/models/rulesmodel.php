<?php

class RulesModel extends CI_Model {
	
	private $tableName            = 'rules';
	private $groups_tableName     = 'rule_groups';
	private $categories_tableName = 'rule_group_categories';
	
	function __construct()
	{
		parent::__construct();
	}

	public function get_rule_groups($assoc = false) {

		$this->db->select('id, name');
		$this->db->from($this->groups_tableName);
		
		$query = $this->db->get();
		$groups = $query->result();
		
		return $groups;
		
		/*
		$ans = array();
		
		if($assoc) {
			foreach($groups as $group) {
				$ans[$group->id] = $group->name;
			}
		} else {
			foreach($groups as $group) {
				$ans[] = array($group->name, intval($group->id));
			}
		}
		return $ans;
		*/
		
	}
	
	public function get_rule_by_id($rule_id) {
						
		$fields = array("name", "aspect", "description", "anchor", "att_id", "str_match", 
						"numeric_score", "literal_score", "count", "seq_index", "time", "user",
						"user_ip", "rule_group", "occurence", "owner", "rule_type", "timetype", "business_id",
						"app_id", "personal", "anchor_type", "pr_attrType", "str_length", "appearance",
						"str_similarity", "enable_rule", "not_signal", "radius", "page_id", "id");
					
		$this->db->select(implode(',', $fields));
		$this->db->from($this->tableName);
		$this->db->where("id", $rule_id);
		$this->db->limit(1);
		$query = $this->db->get();
		
		if($query->num_rows == 1) {
			$result = $query->result();
			return $result[0];
		} else {
			return false;
		}
		
	}
	
	public function get_group_by_id($group_id) {
				
		$fields = array("name", "description", "score_numeric", "score_literal", "action_email", "email_recipient", 
						"action_log", "action_syslog", "action_header_injection", "businessflow_id", "alert_param_ids");
		
		$this->db->select(implode(',', $fields));
		$this->db->from($this->groups_tableName);
		$this->db->where("id", $group_id);
		$this->db->limit(1);
		$query = $this->db->get();
		
		if($query->num_rows == 1) {
			$result = $query->result();
			return $result[0];
		} else {
			return false;
		}
	
	}
	
	public function get_rules_by_group_id($group_id) {

		$this->db->select('id, name, description, rule_group');
		$this->db->select('(SELECT category_id FROM rule_groups WHERE rule_groups.id="' . intval($group_id) . '") as category_id', false);
		$this->db->from($this->tableName);
		$this->db->where('rule_group', intval($group_id));
		$this->db->order_by('name ASC');
		
		$query = $this->db->get();
		
		if($query->num_rows > 0) {
			$result = $query->result();
			return $result;
		} else {
			return false;
		}
		
	}
	
	public function get_groups_by_category_id($category_id) {
		
		$this->db->select('id, name, category_id');
		$this->db->from($this->groups_tableName);
		$this->db->where('category_id', $category_id);
		$this->db->order_by('name ASC');
		
		$query = $this->db->get();
		
		if($query->num_rows > 0) {
			$result = $query->result();
			return $result;
		} else {
			return false;
		}
		
	}
	
	public function get_rule_by_name($name) {
		
		$this->db->select('id, name');
		$this->db->from($this->tableName);
		$this->db->where('name', $name);
	
		$query = $this->db->get();
		
		if($query->num_rows > 0) {
			$result = $query->result();
			return $result;
		} else {
			return false;
		}
		
	}
	
	public function get_category_by_name($name) {
		
		$this->db->select('id, name');
		$this->db->from($this->categories_tableName);
		$this->db->where('name', $name);
	
		$query = $this->db->get();
		
		if($query->num_rows > 0) {
			$result = $query->result();
			return $result;
		} else {
			return false;
		}
		
	}
	
	public function get_category_by_id($id) {
		
		$this->db->select('name');
		$this->db->from($this->categories_tableName);
		$this->db->where('id', $id);
	
		$query = $this->db->get();
		
		if($query->num_rows > 0) {
			$result = $query->result();
			return $result[0];
		} else {
			return false;
		}
		
	}
	
	public function create_category($category_name) {
	
		$this->db->insert($this->categories_tableName, array('name' => $category_name)); 
		return $this->db->insert_id();
		
	}
	
	public function update_category($category_id, $category_name) {
		
		$this->db->where('id', $category_id);
		$this->db->update($this->categories_tableName, array('name' => $category_name)); 
		
	}
	
	public function find_group_by_name($name) {
		
		$this->db->select("id");
		$this->db->from($this->groups_tableName);
		$this->db->where("name", $name);
		$this->db->limit(1);
		$query = $this->db->get();
		
		if($query->num_rows == 1) {
			$result = $query->result();
			return $result[0];
		} else {
			return false;
		}
	
	}
	
	public function create_rule($rule_data) {
		
		$this->db->insert($this->tableName, $rule_data); 
		return $this->db->insert_id();
		
	}
	
	public function create_group($name, $description, $category_id, $action_log, $score_numeric ) {
		
		$data = array('name' => $name, 
					  'description' => $description, 
					  'category_id' => $category_id, 
					  'action_log' => $action_log, 
					  'score_numeric' => $score_numeric);	
					  
		$this->db->insert($this->groups_tableName, $data); 
		return $this->db->insert_id();
		
	}
	
	public function toggle_rules_by_group($group_id, $value) {
		
		$this->db->where('rule_group', $group_id);
		$this->db->update($this->tableName, array('enable_rule' => $value)); 
	
	}
	
	public function get_categories() {
		
		$this->db->select('id, name');
		$this->db->from($this->categories_tableName);
		$this->db->order_by('name ASC');
	
		$query = $this->db->get();
		
		if($query->num_rows > 0) {
			$result = $query->result();
			return $result;
		} else {
			return false;
		}
	
	}
	
	public function get_groups() {
		
		$this->db->select("id, name");
		$this->db->select("count(*) num", false);
		$this->db->from("rule_groups, alerts");
		$this->db->where("rule_groups.id = alerts.rule_group");
		$this->db->group_by("id");
		$this->db->order_by("num DESC");
		
		$query = $this->db->get();
		
		if($query->num_rows > 0) {
			$result = $query->result();
			return $result;
		} else {
			return false;
		}
	
	}
	
	public function add_group_param($group_id, $param_id) {

		// Get current state
		$group = $this->get_group_by_id($group_id);
		if($group) {
			
			$params_arr = array();
			
			$params = $group['alert_param_ids'];
			
			if($params != '') {
				$params_arr = explode(',', $params);
			}
			
			if(!in_array($param_id, $params_arr)) {
				
				$params_arr[] = $param_id;
				$params = implode(',', $params_arr);
				
				$this->db->where('id', $group_id);
				$this->db->update($this->groups_tableName, array('alert_param_ids' => $params)); 
			
			}
					
		}
	
	}
	
	public function update_group($group_id, $group_data) {
		
		$this->db->where('id', $group_id);
		$this->db->update($this->groups_tableName, $group_data); 
	
	}
	
	public function update_rule($rule_id, $rule_data) {
		
		$this->db->where('id', $rule_id);
		$this->db->update($this->tableName, $rule_data); 
	
	}
	
	public function delete_rule($rule_id) {
		
		// DEL RULE
		$this->db->where('id', $rule_id);
		$this->db->delete($this->tableName); 
	
	}
	
	public function delete_group($group_id) {
		
		// DEL RULES
		$this->db->where('rule_group', $group_id);
		$this->db->delete($this->tableName); 
		
		// DEL GROUP
		$this->db->where('id', $group_id);
		$this->db->delete($this->groups_tableName); 
		
	}
	
	public function delete_category($category_id) {
		
		// DEL GROUPS
		$this->db->select('id');
		$this->db->from($this->groups_tableName); 
		$this->db->where('category_id', $category_id);
		$query  = $this->db->get();
		$result = $query->result();
		
		if(!empty($result)) {
			foreach($result as $row) {
				$this->delete_group($row->id);
			}
		}
		
		// DEL CATEGORY
		$this->db->where('id', $category_id);
		$this->db->delete($this->categories_tableName); 
		
	}
	
	public function create_new_rule($group_id) {
		
		$this->db->select('MAX(id) as id', false);
		$this->db->from('rules');
		$query = $this->db->get();
		$result = $query->result();
		
		if(!$result || empty($result)) {
			$last_id = 0;
		} else {
			$last_id = intval($result[0]->id) + 1;
		}
		
		$data = array("id" => $last_id, 
					  "name" => '', 
					  "aspect" => '', 
					  "description" => '', 
					  "appearance" => 1, 
					  "anchor_type" => '', 
					  "anchor" => '', 
					  "att_id" => '', 
					  "pr_attrType" => '', 
					  "str_match" => '', 
					  "not_signal" => 0, 
					  "str_length" => '', 
					  "str_similarity" => 0, 
					  "page_id" => 0, 
					  "business_id" => 0, 
					  "numeric_score" => '', 
					  "literal_score" => '', 
					  "count" => '', 
					  "seq_index" => '', 
					  "time" => '', 
					  "radius" => 0, 
					  "user" => '', 
					  "user_ip" => '', 
					  "app_id" => '', 
					  "rule_group" => $group_id, 
					  "occurence" => '', 
					  "owner" => '', 
					  "rule_type" => '', 
					  "timetype" => NULL, 
					  "personal" => '', 
					  "enable_rule" => 1
					);
					
		$this->db->insert('rules', $data);
		
		return $last_id;
		
	}
	
	
}

?>
