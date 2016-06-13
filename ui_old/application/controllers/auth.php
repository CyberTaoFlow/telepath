<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Auth extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function reset_password($code = null) {
		
		if (!$code)
		{
			show_404();
		}
		
		$user = $this->ion_auth->forgotten_password_check($code);
		
		$this->lang->load('auth');
		$this->load->helper('language');
		$this->load->library('form_validation');
		
		if ($user) {
		
			//if the code is valid then display the password reset form

			$this->form_validation->set_rules('new', $this->lang->line('reset_password_validation_new_password_label'), 'required|min_length[' . $this->config->item('min_password_length', 'ion_auth') . ']|max_length[' . $this->config->item('max_password_length', 'ion_auth') . ']|matches[new_confirm]');
			$this->form_validation->set_rules('new_confirm', $this->lang->line('reset_password_validation_new_password_confirm_label'), 'required');

			if ($this->form_validation->run() == false)
			{
				//display the form

				//set the flash data error message if there is one
				$this->data['message'] = (validation_errors()) ? validation_errors() : $this->session->flashdata('message');

				$this->data['min_password_length'] = $this->config->item('min_password_length', 'ion_auth');
				$this->data['new_password'] = array(
					'name' => 'new',
					'id'   => 'new',
				'type' => 'password',
					'pattern' => '^.{'.$this->data['min_password_length'].'}.*$',
				);
				$this->data['new_password_confirm'] = array(
					'name' => 'new_confirm',
					'id'   => 'new_confirm',
					'type' => 'password',
					'pattern' => '^.{'.$this->data['min_password_length'].'}.*$',
				);
				$this->data['user_id'] = array(
					'name'  => 'user_id',
					'id'    => 'user_id',
					'type'  => 'hidden',
					'value' => $user->id,
				);
				$this->data['csrf'] = $this->_get_csrf_nonce();
				$this->data['code'] = $code;

				//render
				$this->_render_page('auth/reset_password', $this->data);
			}
			else
			{
				// do we have a valid request?
				if ($this->_valid_csrf_nonce() === FALSE || $user->id != $this->input->post('user_id'))
				{

					//something fishy might be up
					$this->ion_auth->clear_forgotten_password_code($code);

					show_error($this->lang->line('error_csrf'));

				}
				else
				{
					// finally change the password
					$identity = $user->{$this->config->item('identity', 'ion_auth')};

					$change = $this->ion_auth->reset_password($identity, $this->input->post('new'));

					if ($change)
					{
						//if the password was successfully changed
						$this->session->set_flashdata('message', $this->ion_auth->messages());
						$this->logout();
						redirect('/', 'refresh');
					}
					else
					{
						$this->session->set_flashdata('message', $this->ion_auth->errors());
						redirect('auth/reset_password/' . $code, 'refresh');
					}
				}
			}
		}
		else
		{
			//if the code is invalid then send them back to the forgot password page
			$this->session->set_flashdata('message', $this->ion_auth->errors());
			redirect("auth/forgot_password", 'refresh');
		}
		
	}
	
	function _get_csrf_nonce()
	{
		$this->load->helper('string');
		$key   = random_string('alnum', 8);
		$value = random_string('alnum', 20);
		$this->session->set_flashdata('csrfkey', $key);
		$this->session->set_flashdata('csrfvalue', $value);

		return array($key => $value);
	}

	function _valid_csrf_nonce()
	{
		if ($this->input->post($this->session->flashdata('csrfkey')) !== FALSE &&
			$this->input->post($this->session->flashdata('csrfkey')) == $this->session->flashdata('csrfvalue'))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	
	function _render_page($view, $data=null, $render=false)
	{
	
		$this->viewdata = (empty($data)) ? $this->data: $data;

		$view_html = $this->load->view($view, $this->viewdata, $render);

		if (!$render) return $view_html;
	}

	
	public function forgotten_password() {
	
		$identity = $this->input->post('identity', true);
		
		if($this->ion_auth->forgotten_password($identity)) {
		
			return_json(array('success' => true));
			
		} else {
		
			return_json(array('success' => false, 'error' => $this->ion_auth->errors()));
			
		}
		
	}
	
	public function logout() {
		
		telepath_log('Telepath', 'logout', $this, array());
		
		$this->ion_auth->logout();
		
		return_json(array('success' => true));
		
	}
	
	public function login() {
		
		$username 	   = $this->input->post('username', true);
		$password 	   = $this->input->post('password', true);
		$password_raw  = $this->input->post('password_raw', true);
		$remember 	   = $this->input->post('remember', true) == 'true';
		
		$check_new     = $this->ion_auth->username_check($username);
		
		// Not yet registered with ION
		
		if($username && $password && $password_raw) {
		
		if(!$check_new) {
			
			$this->db->from('registered_users');
			$this->db->where('user', $username);
			$this->db->where('password', $password);
			
			if($this->db->count_all_results() == 1) {
				
				// Note the array(1) is setting default group for converted users to admin
				$user_id = $this->ion_auth->register($username, $password_raw, '', array(), array(1));
				
				if($user_id) {
					
					$this->ion_auth->login($username, $password_raw, $remember);
					
					telepath_log('Telepath', 'login', $this, array());
					
					return_json(array('success' => true));
					
				} else {
					
					return_json(array('success' => false, 'error' => $this->ion_auth->errors()));
				
				}
				
			} else {
					
				return_json(array('success' => false, 'error' => 'Could not login'));
				
			}
		
		} else {
		
			if($this->ion_auth->login($username, $password_raw, $remember)) {
				
				telepath_log('Telepath', 'login', $this, array());
				
				return_json(array('success' => true));
				
			} else {
				
				return_json(array('success' => false, 'error' => $this->ion_auth->errors()));
			
			}
		
		}
		
		} else {
		
			return_json(array('success' => false, 'error' => $this->ion_auth->errors()));
		
		}
	
	}
}