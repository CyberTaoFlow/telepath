<?php 

class acl
{

	var $ci;
	var $user         = array(); //Object  : Stores user data
	var $user_id;				 //Integer : Stores the ID of the current user
	var $user_groups  = array(); //Array : Stores the roles of the current user
	var $user_extra   = array(); //Array : Additional user access parameters (apps)
	var $perms 		  = array(); //Array : Stores the permissions for the user
	
	var $allowed_apps   = array();
	var $allowed_ranges = array();

	function __construct() {
		$this->ci = &get_instance();
	}
	
	// Determines if user has access to all apps
	function all_apps() {
		
		if($this->ci->input->is_cli_request() || ENVIRONMENT == 'development') {
			return true;
		}
	
		if(!$this->ci->ion_auth->logged_in()) return false;
		if($this->ci->ion_auth->is_admin()) return true;
		
		foreach($this->allowed_apps as $app_id) {
			if($app_id == 'All') {
				return true;
			}
		}
		
		return false;		
	}
	
	function has_perm($class, $function) {
		
		if(!$this->ci->ion_auth->logged_in()) return false;
		if($this->ci->ion_auth->is_admin()) return true;
		
		foreach($this->perms as $perm) {
			if($perm['class'] == $class && $perm['function'] == $function) {
				return true;
			}
		}
		
		return false;
			  
	}
	
	function get_master_list() {
		
		$master_list = array(
			'Telepath' => array(
				'get' => 'Access Telepath and view engine status.',
				'set' => 'Start and stop Telepath engine.'
			),
			'Dashboard' => array(
				'get' => 'Access dashboard screen.',
			),
			'Cases' => array(
				'get' => 'Access cases screen and requests details.',
				'set' => 'Modify cases.',
			),
			'Alerts' => array(
				'get' => 'Access alerts screen and search.',
				'del' => 'Delete alerts.',
			),
			'Suspects' => array(
				'get' => 'Access suspects list screen.'
			),
			'Applications' => array(
				'get' => 'View applications list.',
				'set' => 'Modify applications list.',
			),
			'Rules' => array(
				'get' => 'Access rules screen and categories.',
				'set' => 'Modify existing rules and categories.',
			),
			'Workflow' => array(
				'get' => 'Access business actions screen.',
				'set' => 'Modify actions and categories.',
			),
			'WebUsers' => array(
				'get' => 'View Web Users list.',
				'set' => 'Import users.'
			),
			'Advanced' => array(
				'set' => 'Change configurations and operation modes.'
			),
			'Search' => array(
				'get' => 'Use advanced search functionality.'
			),
			'Sessionflow' => array(
				'get' => 'Access session flow screen.'
			)
		);

		return $master_list;
	
	}
	
	function acl_cmp($a, $b) {
		
		$sort_table = array('Telepath', 'Dashboard', 'Alerts', 'Investigate', 'Workflow', 'Rules', 'Config', 'Applications', 'Debug');
		$a_pos = array_search($a, $sort_table);
		$b_pos = array_search($b, $sort_table);
		
		if($a_pos === false) $a_pos = 999;
		if($b_pos === false) $b_pos = 999;

		return $a_pos > $b_pos ? 1 : -1;
			
	}
	
	function get_short_acl() {
	
		$acl = $this->get_long_acl();
		$short_acl = array();

		foreach($acl as $className => $methods) {
						
			$short_m = array();
			
			foreach($methods as $m) {
				$m_parts = explode('_', $m, 2);
				if(!isset($short_m[$m_parts[0]])) { $short_m[$m_parts[0]] = array(); }
			}
			
			$short_acl[$className] = $short_m;
			
		}
	
		uksort($short_acl, array($this, "acl_cmp"));
		
		return $short_acl;
	
	}
	
	function get_long_acl() {
		
		$base  = realpath('.') . '/application/controllers';

		$files = scandir($base);
		$acl   = array();
		
		foreach($files as $file) {
		
			if($file == '.' || $file == '..' || $file == '.svn' || substr($file, -4) != '.php') {
				continue;
			}

			$className = ucfirst(substr($file, 0, -4));
			
			if($className == 'Auth') {
				continue;
			}
			
			$acl[$className] = array();
			
			include_once($base . '/' . $file);
			
			$methods = get_class_methods ($className);
			
			foreach($methods as $method) {
				if($method == '__construct' || $method == 'get_instance') {
					continue;
				}
				$acl[$className][] = $method;
			}
			
		}
	
		return $acl;
	
	}
	
	function init_current_acl() {
		
		$groups_array  = array();
		
		$this->user_id    = $this->ci->ion_auth->get_user_id();
		
		if($this->user_id) {
			$this->user       = $this->ci->ion_auth->user($this->user_id)->result();
			if(!isset($this->user[0])) { return; }
			$this->user       = (array) $this->user[0];
		} else {
			return;
		}
		
		$parsed = $this->user['extradata'] != '' ? json_decode($this->user['extradata'], true) : false;
		
		$this->user_extra = $parsed ? $parsed : array();
		
		
		
		$users_groups = $this->ci->ion_auth->get_users_groups($this->user_id)->result();
		
		foreach ($users_groups as $group) { 
			$groups_array[] = $group->id; 
		}
		
		$this->user_groups = $groups_array;
		
		// Build Group Perm
		if (count($this->user_groups) > 0) {
			$this->perms = array_merge($this->perms,$this->get_group_perm($this->user_groups));
		}
		
		// Build User Perm
		$this->perms = array_merge($this->perms,$this->get_user_perm($this->user_id));
		
		
		// ------------------------------------------
		// Build User Allowed Apps and Ranges
		// ------------------------------------------
		
		$user_apps   = is_array($this->user_extra) && isset($this->user_extra['apps']) ? $this->user_extra['apps'] : array();
		$user_ranges = is_array($this->user_extra) && isset($this->user_extra['ranges']) ? $this->user_extra['ranges'] : array();
		
		$groups_data = $this->ci->ion_auth->groups($groups_array)->result();
		
		foreach($groups_data as $group) {
			if(isset($group->extradata) && $group->extradata != '') {
				$group_extra = json_decode($group->extradata, true);
				if($group_extra && !empty($group_extra)) {
					
					if(isset($group_extra['apps']) && !empty($group_extra['apps'])) {
						$user_apps = array_merge($user_apps, $group_extra['apps']);
					}
					
					if(isset($group_extra['ranges']) && !empty($group_extra['ranges'])) {
						$user_ranges = array_merge($user_ranges, $group_extra['ranges']);
					}
					
				}
			}
		}
		
		$user_apps = array_unique($user_apps);
		$user_ranges = array_unique($user_ranges);
		
		$this->allowed_apps   = $user_apps;
		$this->allowed_ranges = $user_ranges;

	}

	function get_user_acl($user_id) {
		
		$group_array  = array();
		$ans          = array();
		
		$users_groups = $this->ci->ion_auth->get_users_groups($user_id)->result();
		foreach ($users_groups as $group) { $groups_array[] = $group->id; }
		
		// Build Group Perm
		if (count($this->user_groups) > 0) {
			$ans = array_merge($ans, $this->get_group_perm($groups_array));
		}
		
		// Build User Perm
		return array_merge($ans,$this->get_user_perm($user_id));
		
	}
	
	// Get permission name from permID
	
	function get_perm_data($perm_id) {
		$this->ci->db->select('class, function');
		$this->ci->db->where('id', floatval($perm_id));
		$sql = $this->ci->db->get('ci_perm_data',1);
		$data = $sql->result();
		return (array) $data[0];
	}
	
	/* GROUP RELATED */
	
	// Clear Group Permissions
	function clear_group_perm($group_id) {
		$this->ci->db->where('group_id', $group_id)->delete('ci_group_perm');
	}
	
	// Set Group Permission
	function set_group_perm($group_id, $perm_id) {
		$this->ci->db->insert('ci_group_perm', array('group_id' => $group_id, 'perm_id' => $perm_id, 'value' => 1));
	}
	
	// Get Group Permissions
	function get_group_perm($user_groups) {
	
		$this->ci->db->where_in('group_id',$user_groups);
		$sql = $this->ci->db->get('ci_group_perm');
		$data = $sql->result();
		
		$perms = array();
		foreach( $data as $row ) {
			
			$perm_data  = $this->get_perm_data($row->perm_id);
			
			$perm_formatted = array(
				'id'    => $row->perm_id,
				'class' => $perm_data['class'],
				'function' => $perm_data['function'], 
				'inheritted' => true, 
				'value' => $row->value == '1'
			);
			
			$perms[$perm_data['class'] . '::' . $perm_data['function']] = $perm_formatted;
			
		}
		return $perms;
	}
	
	/* USER RELATED */
	
	// Clear User Permissions
	function clear_user_perm($user_id) {
		$this->ci->db->where('user_id', $user_id)->delete('ci_user_perm');
	}
	
	// Apply User Permission
	function set_user_perm($user_id, $perm_id) {
		$this->ci->db->insert('ci_user_perm', array('user_id' => $user_id, 'perm_id' => $perm_id, 'value' => 1));
	}
	
	// Get User Permissions
	function get_user_perm($user_id) {

		$this->ci->db->where('user_id',floatval($user_id));
		$sql = $this->ci->db->get('ci_user_perm');
		$data = $sql->result();

		$perms = array();
		foreach( $data as $row ) {
		
			$perm_data  = $this->get_perm_data($row->perm_id);
			
			$perm_formatted = array(
				'id'    => $row->perm_id,
				'class' => $perm_data['class'],
				'function' => $perm_data['function'], 
				'inheritted' => false, 
				'value' => $row->value == '1'
			);
			
			$perms[$perm_data['class'] . '::' . $perm_data['function']] = $perm_formatted;
			
		}
		return $perms;
	}

}

?>