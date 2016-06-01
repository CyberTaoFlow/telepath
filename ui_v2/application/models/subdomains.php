<?php

class Subdomains extends CI_Model {
	
	private $tableName = 'subdomains';
	
	function __construct()
	{
		parent::__construct();
	}
	
	function get_ips($id) {
		
		$this->db->select('resp_ip');
		$this->db->from('subdomain_ips');
		$this->db->join('subdomains', 'subdomains.subdomain_id = subdomain_ips.subdomain_id');
		$this->db->where('subdomain_ips.subdomain_id', $id);
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		$query  = $this->db->get();
		$result = $query->result();
		return $result;
		
	}
	
	function get_ids_by_app($app_id) {
		$this->db->select('subdomain_id');
		$this->db->from('subdomains');
		$this->db->where('app_id', $app_id);
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		$query  = $this->db->get();
		$result = $query->result();
		return $result;
	}
	
	function get($app_id, $page_ids = array()) {
	
		$this->db->distinct();
		$this->db->select('pages.subdomain_id');
		$this->db->select('subdomains.subdomain_name');
		$this->db->join('subdomains', 'pages.subdomain_id = subdomains.subdomain_id');
		$this->db->from('pages');
		
		if(!empty($page_ids)) {
			$this->db->where_in('pages.page_id', $page_ids);
		}
		
		$this->db->order_by('subdomains.subdomain_name');
		$this->db->where('pages.app_id', $app_id);
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		$query  = $this->db->get();
		$result = $query->result();
		
		return $result;
		
	}
	
	function search($string) {
		
		$this->db->select('subdomain_name, subdomain_id, subdomains.app_id, applications.app_domain as root');
		$this->db->from('subdomains');
		$this->db->join('applications', 'subdomains.subdomain_name = applications.app_domain', 'left');
		
		
		if($string != '') {
			$this->db->where('subdomains.subdomain_name LIKE', '%' . $string . '%');
		}
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('subdomains.app_id', $this->acl->allowed_apps);
		}
		
		$this->db->order_by('applications.app_domain, subdomains.subdomain_name');
		
		$query  = $this->db->get();
		$result = $query->result();
		
		if(!empty($result)) {
			foreach($result as $row) {
				$row->root = $row->root != '' ? true : false;
			}
		}
		
		return $result;
	
	}
	

}

?>
