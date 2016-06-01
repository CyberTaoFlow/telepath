<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Applications extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
		$this->load->model('Apps');
	}
	
	public function get_subdomain() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$id   = $this->input->post('id', true);
		$id = explode('_', $id);
		if(count($id) != 2) {
			return_fail();
		}
		$id = intval($id[1]);
		
		$this->load->model('Subdomains');
		$ips = $this->Subdomains->get_ips($id);
		$ret = array();
		foreach($ips as $ip) {
			$ret[] = $ip->resp_ip;
		}
		
		
		return_success(array('ips' => $ret));
		
	}
	
	public function set_subdomain() {
	
	}
	
	public function get_path() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		$type = $this->input->post('type', true);
		$id   = intval($this->input->post('id', true));
		
		// Need to find page ID first
		if($type == 'param') {
			
			$this->load->model('Attributes');
			$att_data = $this->Attributes->get_att_by_id($id);
			if(!$att_data) {
				return_fail();
			}
			$page_id = $att_data->page_id;
			
		} else {
			$page_id = $id;
		}
		
		// OK, now load page details
		$this->load->model('PagesModel');
		$page_data = $this->PagesModel->page_get($page_id, true);
		if(!$page_data) {
			return_fail();
		}
		$page_data->page_id = $page_id;
		
		return_success($page_data);
		
	}
	
	public function get_app() {
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		return_success($this->Apps->app_get(intval($this->input->post('app_id'))));
	}
	
	public function get_autocomplete() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		$text = $this->input->post('text');
		$apps = $this->Apps->search($text, false, 0, 50);
		
		$res  = array(array('text' => 'All', 'id' => '-1'));
		if(!empty($apps)) {
			foreach($apps as $app) { 
				$res[] = array('text' => $app->display_name != '' ? $app->display_name : $app->app_domain, 'id' => $app->app_id);
			}
		}
		return_success($res);
		
	}
	
	public function get_subdomain_autocomplete() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		$this->load->model('Subdomains');
	
		$text 	    = $this->input->post('text');
		$subdomains = $this->Subdomains->search($text);
		
		$res  = array(array('text' => 'All', 'id' => '-1'));
		if(!empty($subdomains)) {
			foreach($subdomains as $domain) {
				$res[] = array('text' => $domain->subdomain_name, 'id' => $domain->app_id, 'sub_id' => $domain->subdomain_id, 'root' => $domain->root);
			}
		}
		return_success($res);
		
	}
	
	public function get_expand() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// Collect params
		$context = strtolower($this->input->post('context', true));
		$type 	 = $this->input->post('type', true);
		$id   	 = $this->input->post('id', true);
		$text 	 = $this->input->post('text', true);
		$search  = $this->input->post('search', true);
		$offset  = $this->input->post('offset', true);
		
		switch($context) {
			
			default:
			case 'applications':
			
				// Need to expand filestructure
			
				$search_mode = false;
				$search_string = '';
				
				if($search && 
				   is_array($search) && 
				   isset($search['text']) && 
				   $search['text'] != '') {
				   
					$searchmode = true;
					$searchparam = $search;
					$search_string = $search['text'];
					
					if($search['parameters'] == 'true') {
					
						$this->load->model('Params');
						$this->load->model('Paths');
						$this->load->model('Subdomains');
						
						$ans = $this->Params->searchParams($id, $search_string, $type !== 'application');
						
						$page_ids = array();
						foreach($ans as $row) {
							$page_ids[] = $row->page_id;
						}
						
						switch($type) {
						
							case 'application':
								$ans = $this->Subdomains->get($id, $page_ids);
								return_json(array('success' => true, 'app_id' => $id, 'subdomains' => $ans));
							break;
							case 'subdomain': 
								$ans = $this->Paths->expandPages($id, '/', $page_ids);
							break;
							case 'directory':
								$ans = $this->Paths->expandPages($id, '/' . $text, $page_ids);
							break;
							case 'page':
							
								$params = $this->Params->getParams($id, $search_string);
								return_json(array('success' => true, 'page_id' => $id, 'params' => $params));
								
							break;
							
						}
						
						$ans = $this->Paths->sortResult($ans);			
						return_json(array('success' => true, 'app_id' => $id, 'files' => $ans));
						
					}
					
				}
				
				switch($type) {
				
					case 'application':
						
						$this->load->model('Subdomains');
						$subdomains = $this->Subdomains->get($id); // APP_ID
						return_json(array('success' => true, 'app_id' => $id, 'subdomains' => $subdomains));
						
					break;
					
					case 'subdomain':
					
						$this->load->model('Paths');
						$files = $this->Paths->expand($id, '/', $search_string); // APP_ID
						// Expand Directories
						$files = $this->Paths->sortResult($files);
						return_json(array('success' => true, 'subdomain_id' => $id, 'files' => $files));
					
					break;
					case 'directory':
						// Expand Files
						$this->load->model('Paths');
						$files  = $this->Paths->expand($id, '/' . $text, $search_string); // APP_ID
						$files = $this->Paths->sortResult($files);
						return_json(array('success' => true, 'subdomain_id' => $id, 'files' => $files));

					break;
					case 'page':
						// Expand Params
						$this->load->model('Params');
						$params = $this->Params->getParams($id); // PAGE_ID
						return_json(array('success' => true, 'page_id' => $id, 'params' => $params));
						
					break;
					case 'param':
						// Nadda / Edit Params
						
					break;
					
				}

			break;
		}

	}
	
	public function get_apptree_index() {
		
		// Auth
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// Get post
		$search = $this->input->post('search', true);
		$start  = $this->input->post('start', true);
		$limit  = $this->input->post('limit', true);
		
		// Defaults
		if(!$start) { $start = 0; }
		if(!$limit) { $limit = 2048; }
		
		// Check if searching
		if($search && isset($search['text'])) {
		
			if($search['apps'] == 'true') {
			
				$search = $search['text'];
				$count = $this->Apps->search($search, true, $start, $limit);
				$res   = $this->Apps->search($search, false, $start, $limit);
				return_json(array('success' => true, 'items' => $res, 'total' => $count));
				
			}
			
			if($search['pages'] == 'true') {
				
				$search = $search['text'];
				$count = $this->Apps->search_by_page($search, true, $start, $limit);
				$res   = $this->Apps->search_by_page($search, false, $start, $limit);
				return_json(array('success' => true, 'items' => $res, 'total' => $count));
			
			}
			
			if($search['parameters'] == 'true') {
				
				$search = $search['text'];
				$count = $this->Apps->search_by_param($search, true, $start, $limit);
				$res   = $this->Apps->search_by_param($search, false, $start, $limit);
				return_json(array('success' => true, 'items' => $res, 'total' => $count));
			
			}
			
		} else {
			// Not searching for anything
			$count = $this->Apps->search('', true, $start, $limit);
			$res   = $this->Apps->search('', false, $start, $limit);
		
		}
		
		
		
		
		
		//echo($count);
		return_json(array('success' => true, 'items' => $res, 'total' => $count));
			
	}
	
	public function set_application_alias() {
		
		$app_id    = $this->input->post('app_id', true);
		$app_alias = $this->input->post('app_alias', true);
		$this->Apps->app_update($app_id, array('display_name' => $app_alias));
		$app	   = $this->Apps->app_get($app_id);
		
		return_json(array('success' => true, 'app' => $app[0]));
	
	}
	
	// Updates Application
	public function add_application() {
		
		//error_reporting(E_ALL);
		//ini_set("display_errors", 1);
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$data = array();
		for($i = 0; $i < 41 ; $i++) {
			$data[$this->_i2c('td' . $i)] = $this->input->post('td' . $i);
		}
		
		// 
		// AUTOMATIC WORKFLOW CREATION
		//
		
		$data['logout_page_cb'] = intval($this->input->post('logout_page_cb'));
		$data['logout_att_cb'] = intval($this->input->post('logout_att_cb'));
		$data['form_param_id'] = intval($data['form_param_id']);
		$data['form_authentication_redirect_page_id'] = intval($data['form_authentication_redirect_page_id']);
		// Collect data needed for workflow :: LOGIN
		$logout_parameters = array('logout_page_cb', 'logout_page_id', 'logout_att_cb', 'logout_att_id', 'logout_att_value');
		$logout_data	   = array();
		foreach($logout_parameters as $param) {
			$logout_data[$param] = $data[$param];
		}
		//print_r($logout_data);
		
		// Collect data needed for workflow :: LOGOUT
		$login_parameters = array('form_flag', 'form_param_id');
		$login_data 	  = array();
		foreach($login_parameters as $param) {
			$login_data[$param] = $data[$param];
		}
		
		// Create // Update automatic workflows
		$this->load->model('WorkflowModel');
		$this->WorkflowModel->autoflow($data['app_id'], 'Login', $login_data);
		$this->WorkflowModel->autoflow($data['app_id'], 'Logout', $logout_data);
		
		// Update actioncode
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(108, 1);
		
		// Cleanup
		unset($data['logout_page_cb']);
		unset($data['logout_att_cb']);
		
		//
		// END AUTOMATIC WORKFLOW CREATION
		//
		
		// Redmine issue #301 -- Extra HTTP(S) + Case casting
		$data['app_domain'] = str_replace(array('http://', 'https://'), array('', ''), strtolower($data['app_domain']));
		
		// Don't update certificates at this point
		unset($data['certificate']);
		unset($data['private_key']);
		
		$this->Apps->add($data);
		
		// REWRITE OUR NGINX.CONF
		$this->load->model('M_Nginx');
		$conf = $this->M_Nginx->gen_config();
		file_put_contents("/opt/telepath/openresty/nginx/conf/nginx.conf", $conf);
		
		$app = $this->Apps->app_get($data['app_id']);
		
		$this->load->model('ConfigModel');
		
		if($app && !empty($app)) {
		
			$app = $app[0];
			
			$ans = array(
				'success'  => true,
				'app_id'   => $data['app_id'],
				'ssl'      => $app->ssl_flag == '1' ? 'true' : 'false',
				'app_name' => $app->display_name
			);
	
			return_json($ans);
			
		} else {
			return_json(array('success' => false, 'error' => 'Application not found'));
		}
		
	}
	
	public function get_next_id() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		return_json(array('success' => true, 'app_id' => $this->Apps->get_last_id() + 1));
		
	}
	
	public function set_ssl_certificate() {
	
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		$this->_set_ssl('certificate');
	}
	public function set_ssl_private_key() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		$this->_set_ssl('private_key');
	}
	private function _set_ssl($mode) {
	
		$app_id  = $this->input->post('app_id', true);
		$waitMsg  = $this->input->post('waitMsg', true);
		$file_name  = $this->input->get('file_name', true);
		
		if($mode != 'certificate' && $mode != 'private_key') {
			echo json_encode(array('success' => false, 'error' => 'No such certificate upload mode'));
			die;
		}
		if(!$app_id) {
			echo json_encode(array('success' => false, 'error' => 'No application was selected, please try again'));
			die;
		}
		if(!isset($_FILES) || !isset($_FILES['file'])) {
			echo json_encode(array('success' => false, 'error' => 'No file was uploaded'));
			die;
		}
		if($_FILES['file']['error'] > 0) {
			echo json_encode(array('success' => false, 'error' => 'There was an error during upload'));
			die;
		}
		if($_FILES['file']['size'] > 1048576) {
			echo json_encode(array('success' => false, 'error' => 'File size cannot exceed 1MB'));
			die;
		}
		
		$file_name = $_FILES['file']['name'];
		$file_data = file_get_contents($_FILES['file']['tmp_name']);
		
		$this->Apps->set_certificate($mode, $app_id, $file_name, $file_data);	
		
		// Notice 'true', not true;
		return_json(array('success' => 'true', 'file' => $file_name));
	}
	
	public function get_cookie_suggestion() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$default = array('success' => true, 'items' => array());
		$default['items'][] = array('cookie' => 'PHPSESSID');
		$default['items'][] = array('cookie' => 'PHPSESSIONID');
		$default['items'][] = array('cookie' => 'JSESSIONID');
		$default['items'][] = array('cookie' => 'ASPSESSIONID');
		$default['items'][] = array('cookie' => 'ASP.NET_SessionId');
		$default['items'][] = array('cookie' => 'VisitorID');
		$default['items'][] = array('cookie' => 'SESS');
		$default['total'] = count($default['items']);
		
		$suggest = array('success' => true, 'items' => array());
		
		$app_id  = $this->input->get('app_id', true);
		
		$cookies = $this->Apps->get_cookie_suggestion($app_id);
		if(!empty($cookies)) {
			$cookies = $cookies[0]->cookie_suggestion;
			if(strlen($cookies) > 0) {
				$cookies = explode(',', $cookies);
				foreach($cookies as $cookie) {
					$suggest['items'][] = array('cookie' => $cookie);
				}
			}
		}
		
		$suggest['total'] = count($suggest['items']);
		if($suggest['total'] > 0) {
			return_json($suggest);
		} else {
			return_json($default);
		}

	}
	
	public function del_application() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// NOTE:: Can contain multiple comma seperated values (handled by model)
		
		$app_id = $this->input->post('app_id', true);
		
		if(!$app_id) {
			return_fail('No App ID specified');
		}
		
		$apps = $this->Apps->app_delete($app_id);
		
		// REWRITE OUR NGINX.CONF
		$this->load->model('M_Nginx');
		$conf = $this->M_Nginx->gen_config();
		file_put_contents("/opt/telepath/openresty/nginx/conf/nginx.conf", $conf);
		
		return_success();
		
	}
	
	// Just an alias
	public function get_list() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$apps = $this->Apps->index('app_domain', 'asc');
		$ans  = array(array('id'  => -1, 'domain' => 'All', 'display' => 'All'));
		
		foreach($apps as $app) {
			$ans[] = array(
				'id'      => intval($app->app_id),
				'domain'  => $app->app_domain,
				'display' => $app->display_name
			);
		}
		
		return_success($ans);
		
	}
	
	public function get_apps_combobox_general() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		$this->_get_apps_combobox();
	}
	public function get_apps_combobox_general_without_all() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		$this->_get_apps_combobox(false);
	}
	
	private function _get_apps_combobox($with_all = true) {
		
		$sortorder  = $this->input->get('sortorder', true);
		$sortfield  = $this->input->get('sortfield', true);
		$sort 		= $this->input->get('sort', true);
		$sort       = json_decode($sort, true);
		
		if($sort && is_array($sort) && !empty($sort)) {
			$sortfield = $sort[0]['property'];
			$sortorder = $sort[0]['direction'];
		}
		
		$sortfield = $this->_i2c($sortfield);
		
		$this->load->model('Apps');
		
		if(!$sortfield) {
			$sortfield = 'app_domain';
		}
		if(!$sortorder) {
			$sortorder = 'asc';
		}
		
		$apps = $this->Apps->index($sortfield, $sortorder);
		
		$ans = array();
		
		if($with_all) {
			$ans[] = array(
				'id'  => '-1',
				'app' => 'All',
				'ssl' => 0
			);
		}
		foreach($apps as $app) {
			$ans[] = array(
				'id'  => intval($app->app_id),
				'app' => $app->app_domain,
				'ssl' => intval($app->ssl_flag)
			);
		}

		return_success($ans);
		
	}
	
	private function _i2c($i) {
			
		$i_to_c = array(
			"td0" => 'app_id',
			"td1" => 'app_domain',
			"td2" => 'display_name',
			"td3" => 'login_att_id',
			"td4" => 'logged_condition',
			"td5" => 'condition_value',
			"td6" => 'logout_page_id',
			"td7" => 'logout_att_id',
			"td8" => 'logout_att_value',
			"td9" => 'AppCookieName',
			"td10" => 'cpt_name',
			"td11" => 'cpt_val',
			"td12" => 'ntlm',
			"td13" => 'global_per_app',
			"td14" => 'exclude_group_headers',
			"td15" => 'global_pages',
			"td16" => 'certificate',
			"td17" => 'private_key',
			"td18" => 'ssl_flag',
			"td19" => 'ssl_server_port',
			"td20" => 'app_ips',
			"td21" => 'ssl_certificate_password',
			"td22" => 'cpt_injected_header_name',
			"td23" => 'basic_flag',
			"td24" => 'digest_flag',
			"td25" => 'form_flag',
			"td26" => 'form_param_id',
			"td27" => 'form_param_name',
			"td28" => 'form_authentication_flag',
			"td29" => 'form_authentication_cookie_flag',
			"td30" => 'form_authentication_redirect_flag',
			"td31" => 'form_authentication_redirect_page_id',
			"td32" => 'form_authentication_redirect_page_name',
			"td33" => 'form_authentication_redirect_response_range',
			"td34" => 'form_authentication_body_flag',
			"td35" => 'form_authentication_body_value',
			"td36" => 'form_authentication_cookie_name',
			"td37" => 'form_authentication_cookie_value',
			"td38" => 'cookie_suggestion',
			"td39" => 'certificate_path',
			"td40" => 'private_key_path',
			//"td41" => 'app_type',
		);
		
		if(!in_array($i, array_values($i_to_c))) {
		
			if(isset($i_to_c[$i])) {
				return $i_to_c[$i];
			} else {
				return 'app_id';
			}
		
		} else {
		
			return $i;
			
		}
		
	}
	
}
