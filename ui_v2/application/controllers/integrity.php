<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Integrity extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
	}
		
	public function index() {

		if($this->ion_auth->is_admin()) {
		
			$this->_alerts_ips();
			$this->_bad_subdomains();
			
		}
		
	}
	
	private function _bad_subdomains() {
		
		$this->db->select('subdomain_id, subdomain_name');
		$this->db->like('subdomain_name', ' ', 'both');
		$this->db->or_like('subdomain_name', '.', 'before');
		$this->db->from('subdomains');
		$res = $this->db->get()->result();
		if(!empty($res)) {
			echo '<h2>Integrity issue, bad subdomain name</h2>';
			foreach($res as $row) {
				echo 'Subdomain ID: ' . $row->subdomain_id . ' Name: ' . $row->subdomain_name . '<br>';
			}
		}
	
	}
	
	private function _alerts_ips() {
		
		$this->db->select('id, SID');
		$this->db->where('IP', '');
		$this->db->from('alerts_merge');
		$res = $this->db->get()->result();
		if(!empty($res)) {
			echo '<h2>Integrity issue, alerts without IP</h2>';
			foreach($res as $row) {
				echo 'Alert ID: ' . $row->id . ' SID: ' . $row->SID . '<br>';
			}
		}
	
	}
	
}