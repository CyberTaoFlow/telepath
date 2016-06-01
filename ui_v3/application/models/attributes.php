<?php

class Attributes extends CI_Model {
	
	private $tableName = 'attributes';
	
	function __construct()
	{
		parent::__construct();
	}
	
	function get_att_by_id($att_id) {
		
		// App Check
		$this->db->select('att_name, att_source, att_alias, noisy, page_id');
		$this->db->from($this->tableName);
		$this->db->where('att_id', $att_id);

		$query  = $this->db->get();
		
		$result = $query->result();
		if($result && !empty($result)) {
			return $result[0];
		} else {
			return false;
		}
		
	}
	
	function get_atts_by_ids($att_id) {
	
		$this->db->select('att_id, att_name, att_source, att_alias, noisy');
		$this->db->from($this->tableName);
		$this->db->where_in('att_id', $att_id);
		
		$query  = $this->db->get();
		
		$result = $query->result();
		if($result && !empty($result)) {
			return $result;
		} else {
			return false;
		}
		
	}
		
}

?>