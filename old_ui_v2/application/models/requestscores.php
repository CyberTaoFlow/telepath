<?php

class RequestScores extends CI_Model {
	
	private $base_table_name = 'request_scores_merge';
	
	function __construct()
	{
		parent::__construct();
	}
	
	function get_investigate_details_by_RID($RID, $useTableSplit = true, $split_from, $split_to) {
		
		$this->db->select('query_score, flow_score, landing_normal, geo_normal, avg_score, status_code, method');
		$this->db->select('user_ip, city, country, protocol, user, ' . $this->base_table_name . '.user_id');
		
		$this->db->join('reputation_users', $this->base_table_name . '.user_id = reputation_users.user_id', 'left');
		
		$this->db->select('pages.page_id, pages.display_path, pages.title');
		$this->db->join('pages', 'pages.page_id = ' . $this->base_table_name . ".page_id", 'left');

		$this->db->select('subdomains.subdomain_name');
		$this->db->join('subdomains', 'subdomains.subdomain_id = ' . $this->base_table_name . '.subdomain_id', 'left');
		
		$this->db->select('applications.app_domain');
		$this->db->join('applications', 'applications.app_id = ' . $this->base_table_name . '.hostname', 'left');
		
		$this->db->select("FROM_UNIXTIME(`" . $this->base_table_name . "`.`date`, '%H:%i:%s %c/%e/%Y') as time", false);

		if(!$this->acl->all_apps()) {
			$this->db->where_in($this->base_table_name . ".hostname", $this->acl->allowed_apps);
		}
		
		$this->db->from($this->base_table_name);
		$this->db->where('RID', $RID);
		
		$query  = $this->db->get();
		return $query->result();

	}
	
	function __build_query() {
	
		$this->db->select('RID, SID, seq_index');
		$this->db->select('(select app_domain from applications where app_id=hostname) as app_domain', false);
		$this->db->select('(select ssl_flag from applications where app_id=hostname) as ssl_flag', false);
		$this->db->select('pages.page_id, pages.display_path, pages.title');
		$this->db->select('CAST(' . $this->base_table_name . '.query_score*100 AS SIGNED) as query_score', false);
		$this->db->select('CAST(' . $this->base_table_name . '.flow_score*100 AS SIGNED) as flow_score', false);
		$this->db->select('CAST(landing_normal*100 AS SIGNED) as landing_normal', false);
		$this->db->select('CAST(geo_normal*100 AS SIGNED) as geo_normal', false);
		$this->db->select('user_ip, date, hostname');
		$this->db->from($this->base_table_name);
		$this->db->join('pages', $this->base_table_name . '.page_id=pages.page_id', 'left');
		
		$this->db->select('subdomains.subdomain_name');
		$this->db->join('subdomains','request_scores_merge.subdomain_id=subdomains.subdomain_id', 'left');
		
		$this->db->select('reputation_users.score as user_score, reputation_users.user');
		$this->db->join('reputation_users', $this->base_table_name . '.user_id = reputation_users.user_id', 'left');
		
		$this->db->order_by('date');
		
	}
	
	function get_requests_by_SID($SID, $limit = 1000) {
		
		$this->__build_query();
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		$this->db->limit($limit);
		$this->db->where('sid', $SID);
		$query = $this->db->get();
		return $query->result();
	
	}
	
	function get_requests_by_IP($IP) {
		
		$this->__build_query();
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		$this->db->limit(1000);
		$this->db->where('user_ip', $IP);
		$query = $this->db->get();
		return $query->result();
	
	}
	
	function get_requests_by_user($uid) {
		
		$this->__build_query();
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		$this->db->limit(1000);
		$this->db->where('user_id', $uid);
		$query = $this->db->get();
		return $query->result();
	
	}
	
	// Gets User IP by Request ID
	function get_IP_by_RID($RID) {
		
		$this->db->select('user_ip, date');
		$this->db->from($this->base_table_name);
		$this->db->where('RID', $RID);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in($this->base_table_name . ".hostname", $this->acl->allowed_apps);
		}
		
		$query  = $this->db->get();
		
		if($query->num_rows > 0) {
			$result = $query->result();
			return $result[0];
		}

	}
	
	// Gets Session ID by Request ID
	function get_SID_by_RID($RID, $useTableSplit = true) {
		
		$this->db->select('SID, date');
		$this->db->from($this->base_table_name);
		$this->db->where('RID', $RID);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in($this->base_table_name . ".hostname", $this->acl->allowed_apps);
		}
		
		$query  = $this->db->get();
		
		if($query->num_rows > 0) {
			$result = $query->result();
			return $result[0];
		}
		
		return false;
		
	}
	
	function get_pages_by_IP($IP, $min_date = false) {
		
		$this->db->select('rs.page_id, RID, date');
		$this->db->from($this->base_table_name . ' as rs');
		
		// Connect to pages to get path
		$this->db->select('pages.path as path, pages.title as title');
		$this->db->from('pages');
		$this->db->where('pages.page_id = rs.page_id', NULL, false);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		$this->db->where('user_ip', $IP);
		$this->db->order_by('date', 'asc');
		
		if($min_date) {
			$this->db->where('date >', $min_date);
		}
					
		$query  = $this->db->get();
		return $query->result();
		
	}
	
	function get_pages_by_SID($SID, $min_date = false, $useTableSplit = true) {
		
		$this->db->select('rs.page_id, RID, date');
		$this->db->from($this->base_table_name . ' as rs');
		
		// Connect to pages to get path
		$this->db->select('pages.path as path, pages.title as title');
		$this->db->from('pages');
		$this->db->where('pages.page_id = rs.page_id', NULL, false);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		
		$this->db->where('SID', $SID);
		$this->db->order_by('date', 'asc');
		
		if($min_date) {
			$this->db->where('date >', $min_date);
		}
					
		$query  = $this->db->get();
		return $query->result();
		
	}
		
}

?>