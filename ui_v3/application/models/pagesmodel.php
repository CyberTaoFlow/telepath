<?php

class PagesModel extends CI_Model {
	
	private $tableName   	   = 'pages';
	private $field_page_id     = 'page_id';
	private $field_page_app_id = 'app_id';
	private $field_page_path   = 'display_path';
	private $field_page_title  = 'title';
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function get_page_by_id($page_id) {
	
		$this->db->select($this->field_page_path . ',' . $this->field_page_app_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->from($this->tableName); // pages
		$this->db->where('page_id', $page_id);
		$query = $this->db->get();
		return $query->result();
		
	}
	
	public function page_get($page_id) {
	
		$this->db->select($this->field_page_app_id . ',' . $this->field_page_path . ',' . $this->field_page_title);
		$this->db->from($this->tableName); // pages
		$this->db->where('page_id', $page_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();
		$result = $query->result();
		if($result && !empty($result)) {
			return $result[0];
		} else {
			return false;
		}

	}
	
	function page_search_display_path($search) {
		
		$this->db->select('page_id');
		$this->db->from($this->tableName);
		$this->db->where('display_path LIKE', '%' . $search . '%');
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->limit(99);
		
		$query = $this->db->get();
		$result = $query->result();
		
		$ans = array();
		foreach($result as $row) {
			$ans[] = $row->page_id;
		}
		
		return $ans;
		
	}
	
	function page_search($page_name, $app_id, $mode) {

		$this->db->distinct();
		$this->db->select('display_path, page_id');
		$this->db->from($this->tableName);
	
		switch($mode) {
			case 'Contains':
				$regex = '^/[a-zA-Z0-9_.,-/]*[a-zA-Z0-9_.,-]*' . $page_name . '[a-zA-Z0-9_.-]*$';
			break;
			case 'Equals':
				$regex = '^/[a-zA-Z0-9_.,-/]*' . $page_name . '$';
			break;
		}

		$this->db->where('app_id', $app_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->where('path REGEXP', $regex);
		
		$query = $this->db->get();

		return $query->result();
		
	}
	
	function page_check_edge($page_id_from, $page_id_to) {
		
		$this->db->where('page_from', $page_id_from);
		$this->db->where('page_to', $page_id_to);
		$this->db->where('frequency >=', 0);
		$this->db->from('flow_diagram');
		return $this->db->count_all_results();
		
	}
	
	public function page_update($page_id, $data) {
	
		$this->db->where('page_id', $page_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->update($this->tableName, $data);
	
	}
		
}

?>
