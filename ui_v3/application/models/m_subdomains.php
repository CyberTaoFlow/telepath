<?php

class M_Subdomains extends CI_Model {
	
	private $tableName = 'subdomains';
	
	function __construct()
	{
		parent::__construct();
	}
	
	function get($app_id) {
	
		$this->db->distinct();
		$this->db->select('pages.subdomain_id');
		$this->db->select('subdomains.subdomain_name');
		$this->db->join('subdomains', 'pages.subdomain_id = subdomains.subdomain_id');
		$this->db->from('pages');
		$this->db->where('pages.app_id', $app_id);
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		$query  = $this->db->get();
		$result = $query->result();
		
		return $result;
		
	}
	

}

?>
