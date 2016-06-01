<?php

class BusinessflowModel extends CI_Model {
	
	private $tableName = 'business_flow';
	private $categoriesTableName = 'business_flow_categories';
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function categories_by_app($app_id, $extra) {
		
		$this->db->select('id, name');
		$this->db->from($this->categoriesTableName);
		$this->db->where('app_id', $app_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();
		
		if($query->num_rows > 0) {
			return $query->result();
		} else {
			return array();
		}
	
	}
	
	public function category_find_by_name($category_name) {
		
		$this->db->select('id');
		$this->db->from($this->categoriesTableName);
		$this->db->where('name', $category_name);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();
		
		if($query->num_rows == 1) {
			$result = $query->result();
			return $result[0];
		} else {
			return false;
		}
		
	}
	
	public function category_rename($category_id, $category_name) {
		
		// Validate
		// if same name -> error
		if($this->category_find_by_name($category_name)) {
			return false;
		}
		
		$this->db->where('id', $category_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->update($this->categoriesTableName, array('name' => $category_name));

		return true;
		
	}
	
	public function category_create($application_id, $category_name) {
		
		// Validate
		if($this->category_find_by_name($category_name)) {
			return false;
		}
		
		if($this->acl->all_apps() || in_array($application_id, $this->acl->allowed_apps)) {
			$this->db->insert($this->categoriesTableName, array('app_id' => $application_id, 'name' => $category_name));
			return $this->db->insert_id();
		}
		
		return false;
		// if same name -> error
	
	}
	
	public function category_delete($category_id) {
		
		// Do Delete FLOWS
		$this->db->from($this->tableName);
		$this->db->where('category_id', $category_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->delete();
		
		// Do Delete Category
		$this->db->from($this->categoriesTableName);
		$this->db->where('id', $category_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->delete();
		
		return true;
		
	}
	
	public function category_add_flow($category_id, $flow_id) {
		
		$this->db->where('id', $flow_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->update($this->tableName, array('category_id' => $category_id));
		
		return true;
		
	}

	
	// Get all groups
	function groups_get_all() {
	
		$this->db->select('group_name, id, category_id, app_id');
		$this->db->from('business_flow');
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->order_by('group_name', 'asc');
		$query = $this->db->get();
		
		return $query->result();
	}
	
	// Get all groups by APP ID
	function groups_by_app($app_id, $with_pages = false) {
	
		$this->db->select('group_name, id, category_id' . ($with_pages ? ',pages' : ''));
		$this->db->where('app_id', $app_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->from('business_flow');
		$this->db->order_by('group_name', 'asc');
		
		$query = $this->db->get();
		
		return $query->result();
	}
	
	// Where id = group_id from business flow	
	function group_query_param($group_id) {
	
		$this->db->where('id', $group_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->from('business_flow');
		
	}

	// Group Check by ID
	function group_id_check($group_id) {
	
		$this->group_query_param($group_id);
		return $this->db->count_all_results();
		
	}
	
	// Group Get by ID
	function group_get_by_id($group_id) {
	
		$this->db->select('group_name, app_id, category_id, pages');
		$this->group_query_param($group_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();
		
		return $query->result();
		
	}
	
	// Group Get by NAME
	function group_get_by_name($group_name, $app_id = false, $exclude_id = false) {
		
		$this->db->select('id, group_name, pages, category_id');
		$this->db->where('group_name', $group_name);
		
		if($app_id !== false) {
			$this->db->where('app_id', $app_id);
		}
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}

		$this->db->from('business_flow');
		
		if($exclude_id !== false) {
			$this->db->where('id !=', $exclude_id);
		}
		
		$query = $this->db->get();
		
		return $query->result();
		
	}

	// Group Create
	function group_create($group_name, $app_id, $pages) {

		$group_data = array(
			'group_name' => $group_name,
			'app_id' => $app_id,
			'pages' => $pages
		);
		
		if($this->acl->all_apps() || in_array($app_id, $this->acl->allowed_apps)) {
			$this->db->insert('business_flow', $group_data); 
			return $this->db->insert_id();
		}
		
		return false;

	}
	
	// Group Update
	function group_update($group_name, $pages, $group_id) {
		
		$group_data = array(
			'group_name' => $group_name,
			'pages' => $pages
		);
		
		$this->db->where('id', $group_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->update('business_flow', $group_data); 
		
	}

	// Group Delete
	function group_delete($group_id) {
	
		$this->group_query_param($group_id);
		$this->db->delete(); 
	
	}

}

?>