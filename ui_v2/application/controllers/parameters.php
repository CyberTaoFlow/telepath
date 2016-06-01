<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Parameters extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
		
		// Init DB
		$this->load->database();		
		// Init Model
		$this->load->model('ParametersModel');
		
	}
	
	function get_global_headers() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$this->load->model('GlobalHeadersModel');
		$headers = $this->GlobalHeadersModel->get_global_headers();
		
		$ans = array('items' => array(), 'success' => true);
		
		foreach($headers as $header) {
			$ans['items'][] = array('header' => $header->att_name, 'value' => $header->att_id);
		}
		
		$ua_ids = array();
		$this->db->select('att_id');
		$this->db->where('att_name', 'user-agent');
		$this->db->from('attributes');
		$query = $this->db->get();
		$result = $query->result();
		if(!empty($result)) {
			foreach($result as $row) {
				$ua_ids[] = $row->att_id;
			}
		}
		$ans['items'][] = array('header' => 'user-agent', 'value' => implode(',', $ua_ids));

		return_json($ans);

	}
	
	function set_parameter_alias() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$this->load->model('ParametersModel');
	
		$att_id    = $this->input->post('att_id', true);
		$att_alias = $this->input->post('att_alias', true);
		$this->ParametersModel->att_update($att_id, array('att_alias' => $att_alias));
		$param     = $this->ParametersModel->att_get($att_id);
		return_json(array('success' => true, 'param' => $param[0]));
		
	}
	
	function set_parameter_config() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$this->load->model('ParametersModel');
	
		$att_id    = $this->input->post('att_id', true);
		$att_mask  = $this->input->post('att_mask', true) == '0' ? '0' : '1';
		$this->ParametersModel->att_update($att_id, array('mask' => $att_mask));
		$param     = $this->ParametersModel->att_get($att_id);
		return_json(array('success' => true, 'param' => $param[0]));
	
	}
	
}