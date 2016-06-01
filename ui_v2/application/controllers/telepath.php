<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Telepath extends CI_Controller {
	
	private $processName = 'telewatchdog';
	private $startFunction = 'sudo /opt/telepath/bin/telepath start';
	private $stopFunction = 'sudo /opt/telepath/bin/telepath stop';
	private $checkFunction = 'sudo /opt/telepath/bin/telepath check';
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function activity() {
	
		$flag = intval($this->input->post('flag'));
		$this->load->model('ConfigModel');
		$this->ConfigModel->update('make_report', $flag);
		return_success();
		
	}
	
	public function check() {
	
		$this->load->model('ConfigModel');
		$key = $this->input->post('key');
		$this->ConfigModel->update('licence_key', $key);
		
		// This code should work but currently using the other way around
		 exec($this->checkFunction . " > /dev/null &");
		// Allow 3 second for engine to validate the key
		 sleep(3);
		
		//exec($this->startFunction . " > /dev/null &");
		//sleep(10);
		//exec($this->stopFunction . " > /dev/null &");
		
		$valid = $this->ConfigModel->get('licence_valid');
		$valid = $valid['licence_valid'];
		
		return_json(array('success' => true, 'valid' => $valid));
		
	}
	
	private function _install_auth() {
		
		$this->load->dbforge();
		
		/*$this->dbforge->drop_table('ci_users');
		$this->dbforge->drop_table('ci_groups');
		$this->dbforge->drop_table('ci_users_groups');
		$this->dbforge->drop_table('ci_perm_data');
		$this->dbforge->drop_table('ci_groups_perm');
		$this->dbforge->drop_table('ci_users_perm');
		$this->dbforge->drop_table('ci_login_attempts');*/
				
		$tables = $this->db->list_tables();

		if(!in_array('ci_users', $tables)) {
			
			$this->dbforge->add_field('id');

			$fields = array(
				'ip_address' 	  => array('type' => 'VARBINARY', 'constraint' => '16', 'null' => TRUE), // IP
                'username'  	  => array('type' => 'VARCHAR', 'constraint' => '100'),   // Username
				'password'  	  => array('type' => 'VARCHAR', 'constraint' => '80'),   // Password
				'salt'      	  => array('type' => 'VARCHAR', 'constraint' => '40', 'null' => TRUE),   // Password
				'email'     	  => array('type' => 'VARCHAR', 'constraint' => '254', 'null' => TRUE),  // Email Address
				'activation_code' => array('type' => 'VARCHAR', 'constraint' => '40', 'null' => TRUE), // Activation Code
			    'forgotten_password_code' => array('type' => 'VARCHAR', 'constraint' => '40', 'null' => TRUE), // Forgotten password code
				'forgotten_password_time' => array('type' => 'INT', 'constraint' => '11', 'unsigned' => TRUE, 'null' => TRUE), // Forgotten password stamp
				'remember_code' => array('type' => 'VARCHAR', 'constraint' => '40', 'null' => TRUE), // Remember Code
				'created_on' => array('type' => 'INT', 'constraint' => '11', 'unsigned' => TRUE, 'null' => TRUE), // Creation Timestamp
				'last_login' => array('type' => 'INT', 'constraint' => '11', 'unsigned' => TRUE, 'null' => TRUE), // Last Login
				'active'     => array('type' => 'TINYINT', 'constraint' => '1', 'unsigned' => TRUE, 'null' => TRUE), // Boolean Active
				'first_name' => array('type' => 'VARCHAR', 'constraint' => '50', 'null' => TRUE), // First Name
				'last_name' => array('type' => 'VARCHAR', 'constraint' => '50', 'null' => TRUE), // Last Name
				'company'   => array('type' => 'VARCHAR', 'constraint' => '100', 'null' => TRUE), // Company
				'phone'     => array('type' => 'VARCHAR', 'constraint' => '20', 'null' => TRUE), // Phone Number
            );
			
			$this->dbforge->add_field($fields);
			$this->dbforge->add_key('username');
			$this->dbforge->add_key('email');
			
			$this->dbforge->create_table('ci_users', TRUE);

		}
		if(!in_array('ci_groups', $tables)) {
			
			$this->dbforge->add_field('id');
			
			$fields = array(
                'name'        => array('type' => 'VARCHAR', 'constraint' => '32'),  // Group Name
				'description' => array('type' => 'VARCHAR', 'constraint' => '512', 'null' => TRUE), // Group Description
				'extradata'   => array('type' => 'VARCHAR', 'constraint' => '4048', 'null' => TRUE) // Additional storage per group
            );
			
			$this->dbforge->add_field($fields);			
			$this->dbforge->add_key('name');
			
			$this->dbforge->create_table('ci_groups', TRUE);
			$this->db->insert('ci_groups', array('name' => 'admin', 'description' => 'Administrator'));
			$this->db->insert('ci_groups', array('name' => 'members', 'description' => 'General User'));
			
		}
		
		if(!in_array('ci_users_groups', $tables)) {
			
			$this->dbforge->add_field('id');
			$fields = array(
				'user_id'     => array('type' => 'INT',    'constraint' => '9', 'unsigned' => true), // USER ID
				'group_id'    => array('type' => 'INT',    'constraint' => '9', 'unsigned' => true) // GROUP ID
			);
			
			$this->dbforge->add_field($fields);
			$this->dbforge->add_key('user_id');
			$this->dbforge->add_key('group_id');
			
			$this->dbforge->create_table('ci_users_groups', TRUE);
			
		}
		
		if(!in_array('ci_perm_data', $tables)) {

			$this->dbforge->add_field('id');
			
			$fields = array(
                'class'       => array('type' => 'VARCHAR', 'constraint' => '64', 'null' => TRUE),   // Permission Class
				'function'    => array('type' => 'VARCHAR', 'constraint' => '64', 'null' => TRUE),   // Permission Function
				'alias'    	  => array('type' => 'VARCHAR', 'constraint' => '128', 'null' => TRUE),  // Permission Alias
				'description' => array('type' => 'VARCHAR', 'constraint' => '512', 'null' => TRUE),  // Permission Description
				'params'      => array('type' => 'VARCHAR', 'constraint' => '1024', 'null' => TRUE)  // Permission Alias
            );
			
			$this->dbforge->add_field($fields);	
			$this->dbforge->create_table('ci_perm_data', TRUE);
			
		}
		
		if(!in_array('ci_group_perm', $tables)) {
			
			$this->dbforge->add_field('id');
			
			$fields = array(
                'group_id'    => array('type' => 'INT',    'constraint' => '5', 'unsigned' => true),  				 // GROUP ID
				'perm_id' 	  => array('type' => 'INT',    'constraint' => '5', 'unsigned' => true), 				 // PERM ID
				'value'   	  => array('type' => 'TINYINT','constraint' => '1', 'unsigned' => true, 'default' => 0) // 0 == False, 1 == TRUE
            );
			
			$this->dbforge->add_field($fields);			
			$this->dbforge->add_key('group_id');
			$this->dbforge->add_key('perm_id');
			
			$this->dbforge->create_table('ci_group_perm', TRUE);

		}
		
		if(!in_array('ci_user_perm', $tables)) {
			
			$this->dbforge->add_field('id');
			
			$fields = array(
                'user_id'     => array('type' => 'INT',    'constraint' => '5', 'unsigned' => true),  				 // USER ID
				'perm_id' 	  => array('type' => 'INT',    'constraint' => '5', 'unsigned' => true), 				 // PERM ID
				'value'   	  => array('type' => 'TINYINT','constraint' => '1', 'unsigned' => true, 'default' => 0) // 0 == False, 1 == TRUE
            );
			
			$this->dbforge->add_field($fields);			
			$this->dbforge->add_key('user_id');
			$this->dbforge->add_key('perm_id');
			
			$this->dbforge->create_table('ci_user_perm', TRUE);
			
		}
		
		if(!in_array('ci_login_attempts', $tables)) {
			
			$this->dbforge->add_field('id');
			
			$fields = array(
				'ip_address' => array('type' => 'VARBINARY', 'constraint' => '16'), // IP
				'login'      => array('type' => 'VARCHAR', 'constraint' => '100'),  // Login
				'time' 		 => array('type' => 'INT', 'constraint' => '11', 'unsigned' => TRUE, 'null' => TRUE) // Time
			);
			
			$this->dbforge->add_field($fields);		

			$this->dbforge->create_table('ci_login_attempts', TRUE);
			
		}
		
	}
	
	public function dbforge() {
		$this->_install_auth();
		$this->ion_auth->logout();
	}
	
	public function index() {
		
		global $atms_file;
		if(!file_exists($atms_file)) { 
			$this->load->view('setup');
			return;
		}
		
		$tables = $this->db->list_tables();
		if(!in_array('ci_login_attempts', $tables)) {
			$this->_install_auth();
		}
		
		// Split Tables
		$split_tables = array('request_scores', 'alerts', 'attribute_scores');
		
		foreach($split_tables as $base) {
			
			// Kill any threads to merge tables
			$o = array();
			$q = $this->db->query('SHOW FULL PROCESSLIST');
			foreach($q->result_array() as $k => $v) { $o[] = $v; }
			if(!empty($o)) {
				foreach($o as $v) {
					if($v['db'] == 'telepath' && strpos($v['Info'], $base . '_merge') > -1) {
						$this->db->query('KILL ' . $v['Id']);
					}
				}
			}
		
			// Current Tables
			$o = array($base);
			$q = $this->db->query('SHOW TABLES LIKE "' . $base . '_from_%"');
			foreach($q->result_array() as $k => $v) { $o = array_merge($o, array_values($v)); }
			
			if(in_array($base . '_merge' , $tables)) {
				// Current Structure
				$structure = $this->db->query('SHOW CREATE TABLE ' . $base . '_merge')->result_array();
				$structure = $structure[0]['Create Table'];
				$structure = substr($structure, strpos($structure, 'UNION=') + 7, -1);
				$structure = explode(',', $structure); // Populated with table names that make up the split
			} else {
				$structure = array(); // Currently split table is empty
			}

			// If we have more tables now than in merge
			if(count($o) > count($structure)) {
				
				// Drop it first (if any)
				if(in_array($base . '_merge' , $tables)) {
					$this->db->query('DROP TABLE ' . $base . '_merge');
				}
			
				$res = $this->db->query('SHOW CREATE TABLE ' . $base)->result();
				$res = (array) $res[0];
				$res = $res['Create Table'];
				$res = explode("\n", $res);
				foreach($res as $i => $v) {
					if(strpos($v, 'FULLTEXT') > -1) {
						unset($res[$i]);
						// If last index drop trailing comma from previous line
						if($i == count($res) - 1) {	$res[$i - 1] = substr($res[$i - 1], 0, -1);	}
					}
				}
				$res = implode("\n", $res);
				$res = str_replace($base, $base . '_merge', $res);
				
				// Create regular table
				$this->db->query($res);
				// UNION
				$this->db->query('ALTER TABLE ' . $base . '_merge ENGINE=MERGE, UNION=(' . implode(',', $o) . ')');

			}
			
		}
		
		
		// Fix users extradata
		if(in_array('ci_users', $tables)) {
			if (!$this->db->field_exists('extradata', 'ci_users')) {
				$this->load->dbforge();
				$this->dbforge->add_column('ci_users', array('extradata' => array('type' => 'VARCHAR', 'constraint' => '4048', 'null' => TRUE)));
			}
		}
		
		// Fix alerts msg_id > id
		foreach($tables as $table) {
			if(substr($table, 0, 6) == 'alerts') {
			
				if ($this->db->field_exists('msg_id', $table)) {
				
					$this->load->dbforge();
					$fields = array('msg_id' => array(
										'name' => 'id',
										'type' => 'INT',
										'constraint' => 20, 
										'unsigned' => TRUE,
										'auto_increment' => TRUE
										)
									);
					$this->dbforge->modify_column($table, $fields);
					
				}
			}
		}
	
		telepath_auth(__CLASS__, __FUNCTION__);
		
		if(is_file('/usr/sbin/ver')) {
			include(APPPATH.'errors/error_db.php');
			return;
		}
		
		// Figure Licence
		$this->load->model('ConfigModel');
		$licence_key   = $this->ConfigModel->get('licence_key');
		$licence_valid = $this->ConfigModel->get('licence_valid');
		$this->load->view('index', array('licence_key' => $licence_key['licence_key'], 
										 'licence_valid' => $licence_valid['licence_valid']));

	}
	
	public function get_about() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
			
		$this->load->model('ConfigModel');
		$config = $this->ConfigModel->get();
		$fields = array('last_updated_date' => 1, 'atms_version' => 1, 'engine_version' => 1);
		$ans = array_intersect_key($config, $fields);
		$ans['success'] = true;

		return_json($ans);
		
	}
	
	public function get_status() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		exec("pgrep " . $this->processName, $output, $return);
		$found = $return == 0;
		return_json(array('success' => true, 'status' => $found));
		
	}
	
	public function start() {
	
		telepath_auth(__CLASS__, __FUNCTION__);
	    exec($this->startFunction . " > /dev/null &");
		return_json(array('success' => true, 'status' => 'started'));
		
	}
	
	public function stop() {
	
		telepath_auth(__CLASS__, __FUNCTION__);
        exec($this->stopFunction . " > /dev/null &");
		return_json(array('success' => true, 'status' => 'stopped'));
		
	}
	
	public function audit() {
		
		if($this->ion_auth->is_admin()) {
			
			$ans = array();
		
			$audit_raw   = file_get_contents('/tmp/telepath_ui_log');
			$audit_lines = explode("\n", $audit_raw);
			$total       = count($audit_lines) - 1;
			
			$start = intval($this->input->get('start'));
			$limit = intval($this->input->get('limit'));
			
			$offset = 0;
			$count  = 0;
			
			foreach($audit_lines as $audit_line) {
				
				if($offset >= $start) {

					$audit_data = json_decode($audit_line, true);
					$audit_data['userdata'] = $audit_data['userdata']['identity'];
					$ans[] = $audit_data;
					$count++;
					
				}
				
				if($count >= $limit) {
					break;
				}
				
				$offset++;
				
			}
			
			return_json(array('success' => true, 'total' => $total, 'items' => $ans, 'count' => count($ans)));
			
			return_success($ans);
			
		}

	}
	
}

?>
