<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Users extends CI_Controller {
	
	function get_self() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		$user   = $this->ion_auth->user()->result();
		$user = $user[0];
		$user = array('id' => $user->id, 
 				      'login' => $user->username, 
					  'active' => $user->active == '1',
					  'email'  => $user->email,
					  'first_name' => $user->first_name,
					  'last_name'  => $user->last_name,
					  'company' => $user->company,
					  'phone' => $user->phone);

		return_success(array('user' => $user));
		
	}
	
	function set_self() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$data = array();
		
		// Only allow these values
		$data['first_name'] = $this->input->post('first_name');
		$data['last_name'] = $this->input->post('last_name');
		$data['email'] = $this->input->post('email');
		$data['company'] = $this->input->post('company');
		$data['phone'] = $this->input->post('phone');
		
		// PWD change
		if($this->input->post('password') && $this->input->post('password') != '') {
			$data['password'] = $this->input->post('password');
		}
		
		$user = $this->ion_auth->update($this->session->userdata('user_id'), $data);
		if(!$user) {
			return_fail($this->ion_auth->errors());
		}
		
		return_success();
	
	}
	
	
	function __construct()
	{
		parent::__construct();
	}
	
	function index() {

	}
	
	function get_list() {
	
		telepath_auth(__CLASS__, __FUNCTION__);
		$users = $this->ion_auth->users()->result();
		$ans   = array();
		foreach($users as $user) {
			$ans[] = array('id' => $user->id, 
						   'login' => $user->username, 
						   'created_on' => $user->created_on, 
						   'last_login' => $user->last_login, 
						   'active' => $user->active == '1',
						   'email'  => $user->email,
						   'first_name' => $user->first_name,
						   'last_name'  => $user->last_name,
						   'company' => $user->company,
						   'phone' => $user->phone);
		}
		
		return_success($ans);
		
	}

	function get_user() {
	
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$id = $this->input->post('id');
		
		if(intval($id) == 0) {
			return_fail('Bad user_id');
		}
		
		$user   = $this->ion_auth->user($id)->result();
		if(!$user || empty($user)) {
			return_fail('Bad user_id');
		}
		
		$user = $user[0];
		$user_extra = $user->extradata != '' ? json_decode($user->extradata, true) : array();
		$user = array('id' => $user->id, 
 				      'login' => $user->username, 
					  'active' => $user->active == '1',
					  'email'  => $user->email,
					  'first_name' => $user->first_name,
					  'last_name'  => $user->last_name,
					  'company' => $user->company,
					  'phone' => $user->phone);
		
		$groups = $this->ion_auth->get_users_groups($id)->result();
		$perm   = $this->acl->get_user_perm($id);
		$apps   = isset($user_extra['apps']) ? $user_extra['apps'] : array();
		$extra  = isset($user_extra['extra']) ? $user_extra['extra'] : array();
		
		//print_r($this->acl->user_extra);
		//die;
		
		return_success(array('user' => $user, 'groups' => $groups, 'perm' => $perm, 'apps' => $apps, 'extra' => $extra));
		
	}
	
	/* Add User */
	
	function add_user() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$post = $this->input->post();
		$data = $post['items']['user'];
		$user_id = $data['id'];
		$data['active'] = isset($data['active']) && $data['active'] == 'true' ? 1 : 0;
		
		if($user_id == 'new') {
			
			$login    = $data['login'];
			$password = $data['password'];
			$email 	  = $data['email'];
			
			unset($data['id']);
			unset($data['login']);
			unset($data['password']);
			unset($data['email']);
			unset($data['extradata']); // IMPORTANT NOT TO ALLOW THIS
			
			$user_id = $this->ion_auth->register($login, $password, $email, $data, isset($post['items']['groups']) ? $post['items']['groups'] : array());
			if(!$user_id) {
				return_fail($this->ion_auth->errors());
			}
			
		}
		
		// Update user groups
		$this->_upd_user_groups($user_id, $post);

		// Update user perm
		$this->_upd_user_perm($user_id, $post);
	
		// Update user apps
		$this->_upd_user_extra($user_id, $post);
	
		return_success(array('user_id' => $user_id));
		
	}

	function _upd_user_groups($user_id, $post) {
		
		// Clear OLD
		$this->ion_auth->remove_from_group(false, $user_id);
		// Set NEW
		$data = isset($post['items']['groups']) ? $post['items']['groups'] : array();
		if(!empty($data)) {
			foreach($data as $group_id) {
				$this->ion_auth->add_to_group($group_id, $user_id);
			}
		}
		
	}
	function _upd_user_perm($user_id, $post) {
		
		// Clear OLD
		$this->acl->clear_user_perm($user_id);
		// Set NEW
		$data = isset($post['items']['perm']) ? $post['items']['perm'] : array();
		
		if(!empty($data)) {
			foreach($data as $perm_id) {
				$this->acl->set_user_perm($user_id, $perm_id);
			}
		}
	
	}
	
	function _upd_user_extra($user_id, $post) {
		
		$allowed_apps = isset($post['items']['apps']) ? $post['items']['apps'] : array();
		$extra 	      = isset($post['items']['extra']) ? $post['items']['extra'] : array();
		
		$user = $this->ion_auth->update($user_id, array('extradata' => json_encode(array('apps' => $allowed_apps, 'extra' => $extra))));
		if(!$user) {
			return_fail($this->ion_auth->errors());
		}
			
	}
	
	function set_user() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$post = $this->input->post();
		$data = $post['items']['user'];
		$user_id = $data['id'];
		
		unset($data['id']);
		unset($data['extradata']); // IMPORTANT NOT TO ALLOW THIS
		
		$data['active'] = isset($data['active']) && $data['active'] == 'true' ? 1 : 0;
		
		if($user_id != 'new') {
			$user = $this->ion_auth->update($user_id, $data);
			if(!$user) {
				return_fail($this->ion_auth->errors());
			}
		}

		// Update user groups
		$this->_upd_user_groups($user_id, $post);

		// Update user perm
		$this->_upd_user_perm($user_id, $post);
		
		// Update user apps
		$this->_upd_user_extra($user_id, $post);
	
		return_success(array('user_id' => $user_id));
	
	}
	
	function del_user() {
		
		$id = $this->input->post('id');
		$this->ion_auth->delete_user($id);
		
		return_success();
	
	}
	
}