<?php
/*
class Apps extends CI_Model {
	
	private $tableName = 'applications';
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function get_app($app_id) {

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
			 "ldap_conn",
			 "certificate_path",
			 "private_key_path"
		 );
		 
		 $this->db->select(implode(',', $fields));
		 $this->db->from($this->tableName);
		 
		 $this->db->where('app_id', $app_id);
		 
		 if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		 
		 $query = $this->db->get();
		 return $query->result();
		 
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

	public function index($sortfield = 'app_domain', $sortorder = 'asc', $search = '') {
		
		$this->db->select('app_id, app_domain, ssl_flag, display_name');
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		// Secure
		$search = $this->db->escape_like_str($search);
		
		if($search != '') {
			$this->db->where("(display_name LIKE '%$search%' OR app_domain LIKE '%$search%')", null, false);
		}
		
		$this->db->from($this->tableName);
		$this->db->order_by($sortfield, $sortorder);
		$this->db->limit(999);
		
		$query = $this->db->get();
		
		return $query->result();

	}
	
	public function app_delete($app_id) {
		
		if(in_array($app_id, $this->acl->allowed_apps) || $this->acl->all_apps()) {
			
			// Delete the application
			$this->db->delete($this->tableName, array('app_id' => $app_id)); 
			
			// Update Agents table
			$this->db->where('1=1', null, false);
			$this->db->update('agents', array('newConfig' => 1));
			
			// Update ATMS actions
			$this->load->model('ConfigModel');
			
			$this->ConfigModel->update_atms_actions(41);
			$this->ConfigModel->update_atms_actions(22);
		
		}

	}
	
	public function app_update($app_id, $data) {
	
		$this->db->where('app_id', $app_id);
		if(in_array($app_id, $this->acl->allowed_apps) || $this->acl->all_apps()) {
			$this->db->update($this->tableName, $data);
		}

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
	
	public function get_app_table($sortfield, $sortorder, $start, $limit) {

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
			 "ldap_conn",
			 "certificate_path",
			 "private_key_path"
		 );
		 
		 $this->db->select(implode(',', $fields));
		 $this->db->from($this->tableName);
		 
		 if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		 
		 $this->db->order_by($sortfield, $sortorder);
		 $this->db->limit($limit, $start);
		 $query = $this->db->get();
		 return $query->result();
		 
	}

}
*/
?>
