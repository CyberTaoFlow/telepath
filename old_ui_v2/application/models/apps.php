<?php

class Apps extends CI_Model {
	
	private $tableName = 'applications';
	
	function __construct()
	{
		parent::__construct();
	}
	
	function get_ips($id) {
		
		$this->db->select('resp_ip');
		$this->db->from('subdomain_ips');
		$this->db->join('subdomains', 'subdomains.subdomain_id = subdomain_ips.subdomain_id');
		$this->db->where('subdomains.app_id', $id);
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		$query  = $this->db->get();
		$result = $query->result();
		return $result;
		
	}
	
	public function add($data) {
		
		// Sanity check
		if(!isset($data['app_id'])) die;
		
		$app = $this->app_get($data['app_id']);
		
		if($app && !empty($app)) {
		
			// UPDATE
			$this->db->where('app_id', $data['app_id']);

			if(in_array($data['app_id'], $this->acl->allowed_apps) || $this->acl->all_apps()) {
				unset($data['app_id']);
				$this->db->update($this->tableName, $data);
			}

		} else {
		
			// INSERT
			$this->db->insert($this->tableName, $data); 
			
			// Automatically allow application created by the user to the allowed applications list
			$app_id = $this->db->insert_id();
			$user_extra = $this->acl->user_extra;
			if(!isset($user_extra['apps'])) { $user_extra['apps'] = array(); }
			$user_extra['apps'][] = $app_id;
			$this->ion_auth->update($this->session->userdata('user_id'), array('extradata' => json_encode($user_extra)) );
			
		}
		
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(117, 1);
	
	}
	
	public function get_last_id() {
		$this->db->select('MAX(app_id) as max', false);
		$this->db->from($this->tableName);
		$res = $this->db->get()->row_array();
		return $res['max'];
	}
	
	public function set_certificate($mode, $app_id, $file_name, $file_data) {
		
		$data = array('app_id' => $app_id, $mode . '_flag' => 1, $mode => base64_encode($file_data), $mode . '_path' => $file_name);
		
		$app = $this->app_get($app_id);
		
		if($app && !empty($app)) {
		
			// UPDATE
			$this->db->where('app_id', $app_id);
			
			if(in_array($data['app_id'], $this->acl->allowed_apps) || $this->acl->all_apps()) {
				unset($data['app_id']);
				$this->db->update($this->tableName, $data);
			}

		} else {
		
			// INSERT
			$this->db->insert($this->tableName, $data); 
			
		}
	
	}
	
	public function get_cookie_suggestion($app_id) {
		
		$this->db->select('cookie_suggestion');
		$this->db->from($this->tableName); // applications
		$this->db->where('app_id', $app_id);
		
		if(!$this->acl->all_apps()) {
			if(!in_array($app_id, $this->acl->allowed_apps)) {
				return array();
			}
		}
		
		$query = $this->db->get();
		return $query->result();
		
	}
	
	public function search($string = '', $count = false, $start = 0, $limit = 2048, $sortfield = 'app_domain', $sortorder = 'asc') {
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in($this->tableName . '.app_id', $this->acl->allowed_apps);
		}
		
		$this->db->from($this->tableName);

		if($string != '') {
			$this->db->where('app_domain LIKE', '%' . $string . '%');
			$this->db->or_where('display_name LIKE', '%' . $string . '%');
		}
		
		if(!$count) {
			
			//$this->db->join('pages', 'pages.app_id = applications.app_id', 'left');
			//$this->db->where('pages.page_id IS NOT NULL');
			//$this->db->group_by($this->tableName . '.app_id');
			
			$this->db->select($this->tableName . '.app_id, app_domain, ssl_flag, display_name, app_type');
			$this->db->order_by($sortfield, $sortorder);
			$this->db->limit($limit, $start);
			$query = $this->db->get();
			return $query->result();
			
		} else {
		
			return $this->db->count_all_results();
			
		}
		
	}
	
	public function search_by_page($string = '', $count = false, $start = 0, $limit = 2048, $sortfield = 'app_domain', $sortorder = 'asc') {
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->from($this->tableName);
		$this->db->join('pages', 'pages.app_id = ' . $this->tableName . '.app_id', 'left');
		
		$this->db->group_by($this->tableName . '.app_id');
		
		if($string != '') {
			$this->db->where('pages.path LIKE', '%' . $string . '%');
		}
		
		if(!$count) {
			
			$this->db->distinct();
			$this->db->select($this->tableName . '.app_id, app_domain, ssl_flag, display_name');
			$this->db->order_by($sortfield, $sortorder);
			$this->db->limit($limit, $start);
			$query = $this->db->get();
			return $query->result();
			
		} else {
		
			return $this->db->count_all_results();
			
		}
		
	}
	
	public function search_by_param($string = '', $count = false, $start = 0, $limit = 2048, $sortfield = 'app_domain', $sortorder = 'asc') {
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->from($this->tableName);
		$this->db->join('pages', 'pages.app_id = ' . $this->tableName . '.app_id', 'left');
		$this->db->join('attributes', 'attributes.page_id = pages.page_id', 'left');
		
		$this->db->group_by($this->tableName . '.app_id');
		
		if($string != '') {
			$this->db->where('attributes.att_name LIKE', '%' . $string . '%');
		}
		
		if(!$count) {
			
			$this->db->distinct();
			$this->db->select($this->tableName . '.app_id, app_domain, ssl_flag, display_name');
			$this->db->order_by($sortfield, $sortorder);
			$this->db->limit($limit, $start);
			$query = $this->db->get();
			return $query->result();
			
		} else {
		
			return $this->db->count_all_results();
			
		}
		
		
	}
	
	public function index($sortfield = 'app_domain', $sortorder = 'asc') {
		
		$this->db->select($this->tableName . '.app_id, app_domain, ssl_flag, display_name');
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		//$this->db->join('pages', 'pages.app_id = applications.app_id', 'left');
		//$this->db->where('pages.page_id IS NOT NULL');
		//$this->db->group_by($this->tableName . '.app_id');
		
		$this->db->from($this->tableName);
		$this->db->order_by($sortfield, $sortorder);
		$this->db->limit(2048);
		$query = $this->db->get();
		
		return $query->result();

	}
	
	public function app_delete($app_id) {
		
		// Handle MultiApp Delete
		$app_ids = explode(',', $app_id);
		$to_delete = array();
		
		if($this->acl->all_apps()) {
			$to_delete = $app_ids;
		} else {
			foreach($app_ids as $id) {
				if(in_array($id, $this->acl->allowed_apps)) {
					$to_delete[] = $id;
				}
			}
		}
		
		// Delete the application
		$this->db->where_in('app_id', $to_delete);
		$this->db->delete($this->tableName); 
		
		// Update Agents table
		$this->db->where('1=1', null, false);
		$this->db->update('agents', array('newConfig' => 1));
		
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(117, 1);

	}
	
	public function app_update($app_id, $data) {
	
		if(in_array($app_id, $this->acl->allowed_apps) || $this->acl->all_apps()) {
			$this->db->where('app_id', $app_id);
			$this->db->update($this->tableName, $data);
		}

		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(117, 1);
			
	}
	
	public function app_get($app_id) {
	
		$this->db->select('app_domain, ssl_flag, display_name');
		$this->db->from($this->tableName); // applications
		$this->db->where('app_id', $app_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();
		return $query->result();
	
	}
	
	public function get_app_table($sortfield, $sortorder, $start, $limit, $id = false) {

		 $fields = array(
			 "app_id",
			 "app_domain",
			 "display_name",
			 "login_att_id",
			 "logged_condition",
			 "condition_value",
			 "logout_page_id",
			 "logout_att_id",
			 "logout_att_value",
			 "AppCookieName",
			 "cpt_name",
			 "cpt_val",
			 "ntlm",
			 "global_per_app",
			 "exclude_group_headers",
			 "global_pages",
			 "certificate_flag",
			 "private_key_flag",
			 "ssl_flag",
			 "ssl_server_port",
			 "app_ips",
			 "ssl_certificate_password",
			 "cpt_injected_header_name",
			 "basic_flag",
			 "digest_flag",
			 "form_flag",
			 "form_param_id",
			 "form_param_name",
			 "form_authentication_flag",
			 "form_authentication_cookie_flag",
			 "form_authentication_redirect_flag",
			 "form_authentication_redirect_page_id",
			 "form_authentication_redirect_page_name",
			 "form_authentication_redirect_response_range",
			 "form_authentication_body_flag",
			 "form_authentication_body_value",
			 "form_authentication_cookie_name",
			 "form_authentication_cookie_value",
			 "cookie_suggestion",
			 "certificate_path",
			 "private_key_path",
			 "app_type"
		 );
		 
		 $this->db->select(implode(',', $fields));
		 $this->db->from($this->tableName);
		 
		 if($id) {
			$this->db->where('app_id', $id);
		 }
		 
		 if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		 
		 $this->db->order_by($sortfield, $sortorder);
		 $this->db->limit($limit, $start);
		 $query = $this->db->get();
		 return $query->result();
		 
	}

}

?>
