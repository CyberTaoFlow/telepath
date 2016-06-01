<?php

class AttributeScores extends CI_Model {
	
	private $base_table_name = 'attribute_scores_merge';
	
	function __construct()
	{
		parent::__construct();
	}
	
	function get_investigate_params_by_RID($RID) {
	
		$this->db->select('attributes.att_name, attributes.att_source,' . $this->base_table_name . '.att_id, ' . $this->base_table_name . '.data, ' . $this->base_table_name . '.attribute_score_normal, attributes.att_alias, attributes.noisy, attributes.mask, attributes.structure');
		$this->db->order_by('attribute_score_normal DESC');
		
		$this->db->from($this->base_table_name);
		$this->db->where('RID', $RID);
		$this->db->join('attributes', $this->base_table_name.'.att_id = attributes.att_id', 'left');
		
		if(!$this->acl->all_apps()) {
			$this->db->join('pages', 'attributes.page_id=pages.page_id', 'left');
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		$query  = $this->db->get();
		return $query->result();
		
	}
	
	function get_specified_param_by_RIDs($param_ids, $RIDs) {
		
		$this->db->select('data,att_name,attributes.att_id,att_alias,mask');
		$this->db->distinct();
		$this->db->from($this->base_table_name);
		$this->db->where_in('RID', $RIDs);
		$this->db->where_in('attributes.att_id', $param_ids);
		$this->db->join('attributes', $this->base_table_name.'.att_id = attributes.att_id', 'left');
		
		if(php_sapi_name() !== 'cli' && !$this->acl->all_apps()) {
			$this->db->join('pages', 'attributes.page_id=pages.page_id', 'left');
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		$query  = $this->db->get();
		return $query->result();
	
	}
	
	function get_params_by_RID($RID, $expand = true) {
	
		if($expand) {
			$this->db->select('attributes.att_name, attributes.att_source,' . $this->base_table_name . '.att_id, ' . $this->base_table_name . '.data');
		} else {
			$this->db->select($this->base_table_name . '.att_id, ' . $this->base_table_name . '.data');
		}
					
		$this->db->from($this->base_table_name);
		$this->db->where('RID', $RID);
		
		if($expand) {
			$this->db->join('attributes', $this->base_table_name . '.att_id = attributes.att_id', 'left');
			$this->db->where('attributes.att_source !=', 'H');
		}
		
		$query  = $this->db->get();
		return $query->result();
		
	}
	
	// Gets Request ID by Param Key, Param Value
	function get_RID_by_param($param_id, $param_value, $useTableSplit = true, $single_result = true) {
		
		$ans = array();
		
		// To have to ability to abort this query by SID
		$this->db->where('/* SID:' . $this->session->userdata('session_id') . '; */1=1', NULL, false);
		
		$this->db->select('RID');
		$this->db->from($this->base_table_name);
		$this->db->where('att_id', $param_id);
		
		if($param_value != '') {
			$this->db->where('data', $param_value);	
		}
		
		$query  = $this->db->get();
		$this->db->limit(1000);
		
		if($single_result) {
			if($query->num_rows > 0) {
				$result = $query->result();
				return $result[0]->RID;
			}
		} else {
			return $query->result();
		}

	}
		
}

?>