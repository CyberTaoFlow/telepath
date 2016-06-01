<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Config extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
			
	}
	
	public function _interfaces() {

		$lines = file('/proc/net/dev');
		$interfaces = array();
		for ($i = 2; $i < count($lines);  $i++) {
			$line = explode(':', $lines[$i]);
			$interfaces[] = trim($line[0]);
		}
		return $interfaces;
		
	}
	
	public function _applications($id = false) {
		
		$this->load->model('Apps');
		$this->load->model('PagesModel');
		$this->load->model('Attributes');
		
		$apps = $this->Apps->get_app_table('app_domain', 'asc', 0, 9999, $id);
		$ans  = array();
		
		if($apps && !empty($apps)) {
		
			foreach($apps as $app) {
		
				// Cast to array
				$app = (array) $app;
				$app_map = array();
				
				// We need logout page_id and logout att_id
				if(isset($app['logout_page_id']) && $app['logout_page_id'] != '0') {
					
					$page = $this->PagesModel->page_get($app['logout_page_id']);
					if($page) {
						$app_map['logout_page_name'] = $page->display_path;
					}
				}
				if(isset($app['logout_att_id']) && $app['logout_att_id'] != '0') {
					
					$att = $this->Attributes->get_att_by_id($app['logout_att_id']);
					if($att) {
						$app_map['logout_att_name'] = $att->att_name;
					}
					
				}
				
				
				// Drop the keys, convert to indexes
				$app = array_values($app);
				
				foreach($app as $key => $value) {
					$app_map['td' . $key] = $value;
				}
				
				$ans[] = $app_map;
				
			}
			
		}
	
		return $ans;
		
	}
	
	public function get_config() {
	
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		$this->load->model('ConfigModel');
		$this->load->model('RulesModel');
		
		$ans = $this->ConfigModel->get();
		
		if(isset($ans['workflow_states'])) { unset($ans['workflow_states']); }
		
		if($this->ion_auth->is_admin() || $this->acl->has_perm('Rules', 'get')) {
			$ans['rule_groups_names'] = $this->RulesModel->get_rule_groups();
		} else {
			$ans['rule_groups_names'] = array();
		}

		$ans['interfaces']        = $this->_interfaces();
		$ans['agents'] 			  = $this->ConfigModel->get_agents();
		$ans['regex'] 			  = $this->ConfigModel->get_regex();
		$ans['success']			  = true;
		
		return_json($ans);

	}
	
	public function get_app() {
				
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		$id = intval($this->input->post('id', true));
		$app = $this->_applications($id);
		if(empty($app)) {
			return_fail();
		}
		
		$ips = $this->Apps->get_ips($id);

		//$app[0]['app_ips'] = explode(',', $app[0]['app_ips']);
		$app[0]['app_ips'] = explode(',', $app[0]['td20']);
		foreach($ips as $ip) {
			$app[0]['app_ips'][] = $ip->resp_ip;
		}
		return_success($app);
		
	}
	
	// Execute Python -- Delete
	public function set_to_training_delete() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		exec('/opt/telepath/bin/delete_all_data.py');
		return_json(array('success' => true));
	}
	// Execute Python -- Backup
	public function set_to_training_backup() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		exec('/opt/telepath/bin/make_backup_and_delete_all_data.py');
		return_json(array('success' => true));
	}
	
	public function get_whitelist_ips() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$ans = array('success' => true, 'items' => array());
		
		$this->load->model('ConfigModel');
		$whitelist_ips = $this->ConfigModel->whitelist_get_ips();
		
		foreach($whitelist_ips as $key => $value) {
			$ans['items'][] = array ( 'td0' => $value );
		}
		
		$ans['total'] = count($ans['items']);
		
		return_json($ans);
	
	}
	
	public function get_ignorelist_ips() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$ans = array('success' => true, 'items' => array());
		
		$this->load->model('ConfigModel');
		$ignorelist_ips = $this->ConfigModel->ignorelist_get_ips();
		
		foreach($ignorelist_ips as $key => $value) {
			$ans['items'][] = array ( 'td0' => $value );
		}
		
		$ans['total'] = count($ans['items']);
		
		return_json($ans);
	
	}
	
	public function add_to_whitelist() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('ConfigModel');
		$ip = $this->input->post('ip', true);
		if($ip && strlen($ip) > 0) {
			$this->ConfigModel->whitelist_add_ip($ip);
			return_json(array('success' => true));
		} else {
			return_json(array('success' => false));
		}
	
	}
	
	public function get_scheduler() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('ConfigModel');
	
		$mode = $this->input->post('mode', true);
		
		$table = false;
		
		// Little mapping (TODO:: undo mapping)
		
		if($mode == 'get_schedule') 	   { $table = 'scheduler';		  }
		if($mode == 'get_report_schedule') { $table = 'report_scheduler'; }
		
		if($table) {
			return_json(array('scheduler' => $this->ConfigModel->get_scheduler($table), 'success' => true));
		}

	}
	
	public function set_scheduler() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('ConfigModel');
	
		$mode = $this->input->post('mode', true);
		$data = $this->input->post('data', true);
		
		$table = false;
		
		// Little mapping (TODO:: undo mapping)
		
		if($mode == 'get_schedule') 	   { $table = 'scheduler';		  }
		if($mode == 'get_report_schedule') { $table = 'report_scheduler'; }
		
		if($table) {
			return_json($this->ConfigModel->set_scheduler($table, $data));
		}
	
	}
	
	public function set_schedule() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		$mode = $this->input->post('mode', true);
	}
	
	public function get_num_of_studied() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('ConfigModel');
		$count = $this->ConfigModel->get_learning_so_far();
		return_json(array('success' => true, 'items' => $count));
		
	}
		
	public function set_config() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('ConfigModel');
			
		$config = $this->input->post(NULL, true);
			
		// Handle Whitelist
		
		if(isset($config['ip_whitelist'])) {
		
			$whitelist_new = explode(',', $config['ip_whitelist']);
			$whitelist_old = $this->ConfigModel->whitelist_get_ips();
			$whitelist_del = array_diff($whitelist_old, $whitelist_new);
			$whitelist_add = array_diff($whitelist_new, $whitelist_old);
			
			foreach($whitelist_del as $ip) {
				$this->ConfigModel->whitelist_delete_ip($ip);
			}
			foreach($whitelist_add as $ip) {
				if($ip != '') {
					$this->ConfigModel->whitelist_add_ip($ip);
				}
			}
			
			if(!empty($whitelist_del) || !empty($whitelist_add)) {
				// Update ATMS action
				$this->load->model('ConfigModel');
				$this->ConfigModel->update_action_code(99, 1);
			}
						
		}
		
		if(isset($config['ip_ignorelist'])) {
		
			$ignorelist_new = explode(',', $config['ip_ignorelist']);
			$ignorelist_old = $this->ConfigModel->ignorelist_get_ips();
			$ignorelist_del = array_diff($ignorelist_old, $ignorelist_new);
			$ignorelist_add = array_diff($ignorelist_new, $ignorelist_old);
			
			foreach($ignorelist_del as $ip) {
				$this->ConfigModel->ignorelist_delete_ip($ip);
			}
			foreach($ignorelist_add as $ip) {
				if($ip != '') {
					$this->ConfigModel->ignorelist_add_ip($ip);
				}
			}

		}
		

		$rewrite_mapping = array(
			'op_mode' => 'operation_mode',
			'analyzeTreeStruct' => 'analyze_tree_struct',
			'analyzeTreeValue' => 'analyze_tree_val',
			'analyzePosts' => 'analyze_posts',
			'analyzeGet' => 'analyze_gets',
			'analyzeHeaders' => 'analyze_headers',
			'enableReport' => 'enable_report',
			'syslog_server_address' => 'remote_syslog_ip',
			'reportGroups' => 'rule_groups_report',
			'useLoadBalancer' => 'load_balancer_on'
		);
		
		foreach($config as $key => $value) {
		
			switch($key) {
			
				// Operation mode
				case 'op_mode':
				case 'input_mode':
				
				// Training level
				case 'moveToProductionAfter':

				// Traffic Analysis
				case 'max_distance':
				case 'maxSession':
				case 'analyzeTreeStruct':
				case 'analyzeTreeValue':
				case 'analyzeSlowLandingSpeeds':
				case 'analyzePosts':
				case 'analyzeGet':
				case 'analyzeHeaders':
				
				// Headers
				case 'globalHeaders':
				case 'excludeGlobal':

				// Other
				case 'excludedIPs':
				case 'urlDetectRegex':
				
				// Reports
				case 'rep_user':
				case 'rep_pass':
				case 'MaxEvents':
				case 'smtp':
				case 'smtp_port':
				case 'repAllEvents':
				case 'reportWindow':
				case 'reportFreq':
				case 'enableReport':
				
				// Logs & Alerts
				case 'syslog_server_address':
				case 'write_to_syslog':
				case 'update_titles':
					
					// SAVE SETTING IF VALUE
					if($value != '') {
						$update_key   = isset($rewrite_mapping[$key]) ? $rewrite_mapping[$key] : $key;
						$this->ConfigModel->update($update_key, $value);
					}

				break;
				
				// Allow blank values
				
				case 'proxy_flag':
				case 'proxy_ip':
				case 'proxy_port':
					
					$this->ConfigModel->update($key, $value);
					
				break;
				
				case 'UserAgent' :
					
					if($value != '') {
						$this->ConfigModel->update_agents_regex('headers', $value);
					}
				
				case 'Extension' :
					
					if($value != '') {
						$this->ConfigModel->update_agents_regex('URL', $value);
					}
				
				break;
				
				
				case 'dontMapPage':
					
					// TODO: 0 case
					if($value != '') {
						$update_key   = isset($rewrite_mapping[$key]) ? $rewrite_mapping[$key] : $key;
						$this->ConfigModel->update($update_key, $value);
					}
				
				break;
				
				case 'addUnknownApp':
					
					if($value != '') {
						$update_key   = isset($rewrite_mapping[$key]) ? $rewrite_mapping[$key] : $key;
						$this->ConfigModel->update($update_key, $value);
						$this->ConfigModel->update_atms_actions(41);
					}
					
					
				break;
				
				case 'exclParameters':
					
					// TODO: 0 case
					if($value != '') {
						$update_key   = isset($rewrite_mapping[$key]) ? $rewrite_mapping[$key] : $key;
						$this->ConfigModel->update($update_key, $value);
						$this->ConfigModel->update_atms_actions(22);
					}
				
				break;
				
				case 'useLoadBalancer':
				
					if($value != '') {
						$update_key   = isset($rewrite_mapping[$key]) ? $rewrite_mapping[$key] : $key;
						$this->ConfigModel->update($update_key, $value);
						$this->ConfigModel->update_atms_actions(24);
					}
				
				break;
				
			}
		
		}
		
		if(isset($config['loadBalancerIP']) && isset($config['loadBalanceHead'])) {
			
			$this->ConfigModel->update('load_balancer_ip', $config['loadBalancerIP']);
			$this->ConfigModel->update('load_balancer_header', $config['loadBalanceHead']);
			
			if($config['loadBalancerIP'] == '' || $config['loadBalanceHead'] == '') {
				$this->ConfigModel->update('load_balancer_on', 0);
			}
			
			$this->ConfigModel->update_atms_actions(24);
			
		}
		
		// TODO:: Why both report group / groups?!
		if(isset($config['reportGroups']) && isset($config['reportGroup']) && $config['reportGroups'] != '' && $config['reportGroup'] != '') {
			$this->ConfigModel->update('rule_groups_report', $config['reportGroups']);
		}

		if(!isset($config['agents'])) { $config['agents'] = array(); }
		$this->ConfigModel->set_agents($config['agents']);
		
		// Done!
		return_json(array('success' => true, 'items' => array()));
	
	}
	
}
