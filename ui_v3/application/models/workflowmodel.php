<?php

class WorkflowModel extends CI_Model {

	function __construct()
	{
		parent::__construct();
	}
	
	public function get_requests($mode, $value, $time = false) {
		
		$this->load->model('AttributeScores');
		
		if(!$time) {
			// If no time was passed get 30 minutes back from the last request in DB (just in case)
			$this->db->select('date');
			$this->db->from('request_scores_merge');
			$this->db->limit(1);
			$this->db->order_by('date', 'DESC');
			$res = $this->db->get()->result();
			if(empty($res)) {
				return array();
			} else {
				$time = intval($res[0]->date) - 1800; // 60 * 30
				$time = 0;
			}
		}
		
		// If mode is URL convert it to either SID or IP mode
		if($mode == 'URL') {
			
			$this->load->model('RequestScores');
			
			$param_id  = 18; // Our magic "hybrid_record" param
			$RID = $this->AttributeScores->get_RID_by_param($param_id, $value);
			if(!$RID) {
				return array();
			}
			
			$mode  = 'SID';
			$value = $this->RequestScores->get_SID_by_RID($RID);

			// $mode  = 'IP';
			// $value = $this->RequestScores->get_IP_by_RID($RID);
			
			if(!$value) {
				return array();
			}
						
		} else {
			
			// Collect Request IDs
			$RIDs = array();
			
			// Init
			$this->db->select('RID');
			$this->db->from('request_scores_merge');
			$this->db->order_by('date', 'DESC');
			$this->db->limit(100);
			$this->db->where('date >=', $time);
			
			// Select
			if($mode == 'user') { $this->db->where('user_id', $value); }
			if($mode == 'IP')   { $this->db->where('user_ip', $value); }
			if($mode == 'SID')  { $this->db->where('SID', $value);     }
			
			// Validate + Gather
			$res = $this->db->get()->result();
			if(empty($res)) {
				return array();
			} else {
				foreach($res as $row) {
					$RIDs[] = $row->RID;
				}
			}
			
			// Collect Request Data
			$requests = array();
			
			$this->db->select('RID, SID, page_id, hostname, method, user_ip, date');
			$this->db->from('request_scores_merge');
			$this->db->where_in('RID', $RIDs);
			$this->db->order_by('date', 'DESC');
			$this->db->limit(100);
			
			$res = $this->db->get()->result();
			
			if(empty($res)) {
				return array();
			} else {
			
				// Populate parameters
				foreach($res as $row) {
					$row = (array) $row; // Typecasting
					$row['params'] = $this->AttributeScores->get_params_by_RID($row['RID']);
					$requests[] = $row;
				}
				return $requests;
				
			}
		
		}
	
	}
	
	public function get_last_ips() {
			
		$this->db->distinct();
		$this->db->select('user_ip as suggest');
		$this->db->limit(10);
		$this->db->from('request_scores_merge');
		$this->db->order_by('date', 'DESC');
		return $this->db->get()->result();
		
	}
	public function get_last_users() {
		
		$this->db->distinct();
		$this->db->select('user_id as suggest');
		$this->db->where('user_id !=', '');
		//$this->db->select('user_name as suggest');
		//$this->db->join('reputation_users', 'reputation_users.user_id = request_scores_merge.user_id');
		$this->db->limit(10);
		$this->db->from('request_scores_merge');
		$this->db->order_by('date', 'DESC');
		return $this->db->get()->result();
		
	}
	public function get_last_sids() {
		
		$this->db->distinct();
		$this->db->select('SID as suggest');
		$this->db->limit(10);
		$this->db->from('request_scores_merge');
		$this->db->order_by('date', 'DESC');
		return $this->db->get()->result();
		
	}

	public function page_get_freq($page_id) {
		$this->db->select('frequency');
		$this->db->from('page_diagram');
		$this->db->where('page', $page_id);
		$row = $this->db->get()->first_row();
		if($row && $row->frequency) {
			return intval($row->frequency);
		} else {
			return 0;
		}
	}
	
	public function get_workflow_graph($app_id, $frequency) {

		$this->db->select('page_from, page_to, seq, frequency')
			 ->select('(SELECT display_path from pages where page_from=page_id) as display_path_from', FALSE)
			 ->select('(SELECT display_path from pages where page_to=page_id) as display_path_to', FALSE);

		$this->workflow_query_params($app_id, $frequency);

		$query = $this->db->get();

		return $query->result();
				
	}

	public function get_workflow_by_page_from($app_id, $page_from, $frequency) {

		$this->db->select('page_to, seq, frequency')
			 ->select('(SELECT display_path from pages where page_to=page_id) as display_path_to', FALSE)
			 ->where('page_from', $page_from);
				
		$this->workflow_query_params($app_id, $frequency);

		$query = $this->db->get();
		
		return $query->result();

	}
	
	public function get_workflow_diagram_json($page_id, $frequency) {
		
		$this->db->select('j_from, j_to, frequency')
			 ->select('(SELECT json FROM json_diagram WHERE id=j_from) as j_from_json', FALSE)
			 ->select('(SELECT structure FROM json_diagram WHERE id=j_from) as j_from_structure', FALSE)
			 ->select('(SELECT json FROM json_diagram WHERE id=j_to) as j_to_json', FALSE)
			 ->select('(SELECT structure FROM json_diagram WHERE id=j_to) as j_to_structure', FALSE)
			 ->from('flow_diagram_json')
			 ->where('page', $page_id)
			 ->where('frequency >=', $frequency);

		$query = $this->db->get();
		
		return $query->result();

	}

	private function workflow_query_params($app_id, $frequency) {

		$this->db->from('flow_diagram')
			 ->where('frequency >=', $frequency)
			 ->where('hostname', $app_id)
			 ->order_by('seq', 'desc')
			 ->order_by('page_from', 'asc')
			 ->order_by('frequency', 'desc');
			 
		if(!$this->acl->all_apps()) {
			$this->db->where_in('hostname', $this->acl->allowed_apps);
		}

	}
	
	// APP RELATED QUERIES
	// TODO:: Perhaps create app model
	// -----------------------------------------	
	// SELECT count(*) FROM applications WHERE app_id
	public function app_get_count($app_id) {
		
		$this->db->where('app_id', $app_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->from('applications');

		return $this->db->count_all_results();

	}
	
	// SELECT app_domain FROM applications WHERE app_id
	public function app_get_domain($app_id) {
		
		$this->db->select('app_domain')
			 ->from('applications')
			 ->where('app_id', $app_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();

		return $query->result();		

	}
	

	// PAGE RELATED QUERIES
	// TODO:: Perhaps create page model
	// -----------------------------------------	

	// SELECT display_path from pages where page_id
	public function page_get_display_name($page_id, $with_app_id = false) {
		
		$columns = 'display_path' . ($with_app_id ? ',app_id' : '');
		
		$this->db->select($columns)
			 ->from('pages')
			 ->where('page_id', $page_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();
		$result = $query->result();
		
		if($result && $result[0] && $result[0]->display_path) {
			return $with_app_id ? $result[0] : $result[0]->display_path;
		} else {
			return ' ';
		}
		
	}

	// SELECT title FROM pages WHERE page_id
	public function page_get_name($page_id) {
		
		$this->db->select('title')
			 ->from('pages')
			 ->where('page_id', $page_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();

		return $query->result();

	}
	
	public function page_set_name($page_id, $new_name) {
	
		$this->db->where('page_id', $page_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->update('pages', array('title' => $new_name));
	
	}
	
	// Count combination of the following 2 queries returning boolean
	public function page_is_web_service($page_id) {
		return $this->page_count_flow_diagram_json($page_id) + $this->page_count_attributes($page_id) > 0 ? TRUE : FALSE;
	}
	
	// For is Webservice
	// SELECT count(*) FROM flow_diagram_json WHERE page
	public function page_count_flow_diagram_json($page_id) {
		
		$this->db->where('page', $page_id);
		$this->db->from('flow_diagram_json');

		return $this->db->count_all_results();

	}
	
	// For is Webservice
	// SELECT count(*) FROM attributes WHERE page_id AND (att_source='X' OR att_source='J')
	public function page_count_attributes($page_id) {
		
		$this->db->where('page_id', $page_id);
		$this->db->where_in('att_source', array('X','J'));
		$this->db->from('attributes');

		return $this->db->count_all_results();
		
	}

	// For is Expandable
	// SELECT count(*) FROM flow_diagram WHERE page_from AND hostname
	public function count_flow_diagram_page_from($page_id, $app_id) {
		
		$this->db->where('page_from', $page_id);
		$this->db->where('hostname', $app_id);
		$this->db->from('flow_diagram');
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('hostname', $this->acl->allowed_apps);
		}
		
		return $this->db->count_all_results();
	
	}
	
	// For is Expandable
	// SELECT count(*) FROM flow_diagram_json WHERE page
	public function count_flow_diagram_json_page($page_id) {
		
		$this->db->where('page', $page_id);
		$this->db->from('flow_diagram_json');

		return $this->db->count_all_results();
		
	}

	// For is Expandable
	// SELECT count(*) FROM flow_diagram_json WHERE j_from
	public function count_flow_diagram_json_j_from($page_id) {
		
		$this->db->where('j_from', $page_id);
		$this->db->from('flow_diagram_json');

		return $this->db->count_all_results();
		
	}

}



?>
