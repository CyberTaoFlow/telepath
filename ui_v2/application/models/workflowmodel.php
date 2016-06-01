<?php

class WorkflowModel extends CI_Model {

	function __construct()
	{
		parent::__construct();
	}
	
	// Automatic workflow creation
	public function autoflow($app_id, $flow_name, $flow_data) {
		
		$flow = array('app_id' => $app_id, 'group_name' => $flow_name);
		
		// Clear that flow on specific app
		if($this->acl->all_apps() || in_array($app_id, $this->acl->allowed_apps)) {
			$this->db->delete('business_flow', $flow);
		}
		
		// Assemble json 
		switch($flow_name) {
			case 'Login':
				// Checkbox states that no flow is defined, return here
				if($flow_data['form_flag'] == '0') { return; }
				
				// We have attribute id here, need page id for flow
				$this->load->model('Attributes');
				$att_data = $this->Attributes->get_att_by_id($flow_data['form_param_id']);
				// Error, specified attribute wasnt found
				if(!$att_data) { return; }
				
				$flow['pages'] = json_encode(array(array('page_id' => intval($att_data->page_id), 'params' => array(array('att_id' => intval($flow_data['form_param_id']), 'data' => '*')))));
				
			break;
			case 'Logout':
				// Checkbox states that no flow is defined, return here
				if($flow_data['logout_page_cb'] == 0 && $flow_data['logout_att_cb'] == 0) { return; }
				
				// We have either param_id + page_id
				if($flow_data['logout_att_cb'] == 1) {
					$flow['pages'] = json_encode(
							array(
								array('page_id' => intval($flow_data['logout_page_id']), 
									'params' => array(
											array('att_id' => $flow_data['logout_att_id'], 'data' => ($flow_data['logout_att_value'] != '' ? $flow_data['logout_att_value'] : '*'))
									)
								)
							)
						);
				} else {
				// Or just page_id
					$flow['pages'] = json_encode(array(array('page_id' => intval($flow_data['logout_page_id']))));
				}
				
				
			break;
			case 'Registration':
				// TODO:: Handle registrations
			break;
		}
		// Save
		if($this->acl->all_apps() || in_array($app_id, $this->acl->allowed_apps)) {
			$this->db->insert('business_flow', $flow); 
			return $this->db->insert_id();
		}
	
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
