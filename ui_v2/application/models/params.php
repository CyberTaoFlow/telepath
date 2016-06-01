<?php

class Params extends CI_Model {
	
	private $tableName   	   = 'attributes';
	private $field_att_id	   = 'att_id';
	private $field_att_name    = 'att_name';
	private $field_att_alias   = 'att_alias';
	private $field_att_mask    = 'mask';
	private $field_att_type    = 'att_source';
	
	function __construct()
	{
		parent::__construct();
	}
		
	public function getParams($page_id, $search_string = '') {
		
		$this->db->select($this->field_att_id . ',' .    // ID
						  $this->field_att_name . ',' .  // Name
						  $this->field_att_alias . ',' . // Alias
						  $this->field_att_mask . ',' .  // Masked?
						  $this->field_att_type);        // Type == G | P | H | J
						  
		$this->db->from($this->tableName); // pages
		
		$this->db->join('pages', 'attributes.page_id = pages.page_id', 'left');
		
		$this->db->where('pages.page_id', $page_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		if($search_string != '') {
			$this->db->where($this->field_att_name . ' LIKE', '%' . $search_string . '%');
		}
		
		$this->db->limit(512, 0);
		$query = $this->db->get();
		$result = $query->result();
		
		return $result;
	
	}
	
	public function searchParams($id, $search_string, $search_sub = false) {
		
		$this->db->select('attributes.page_id');
		$this->db->from($this->tableName);
		$this->db->join('pages', 'attributes.page_id = pages.page_id', 'left');
		
		if($search_sub) {
		
			$this->db->where('pages.subdomain_id', $id);
		
		} else {
			
			$this->db->where('pages.app_id', $id);
		}
		
		$this->db->where($this->field_att_name . ' LIKE', '%' . $search_string . '%' );
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();
		
		$result = $query->result();
		return $result;
				
	}
		
}

?>