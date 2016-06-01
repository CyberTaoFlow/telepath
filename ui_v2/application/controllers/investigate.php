<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Investigate extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
		$this->load->model('InvestigateModel');
	}
	
	function get_investigate_cancel() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$sid = $this->session->userdata('session_id');
		$ans = $this->InvestigateModel->cancel_query($sid);
		
		return_json($ans);

	}
	
	/*
	
	Mode: 	delete_profile
	 
	 Invoked:
		when we want to delete a saved search profile	
	 
	 Parameters:
		id - profile id
		
	*/
	
	function del_profile() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$id = $this->input->post('id', true);
		if($id) {
			$this->InvestigateModel->delete_profile($id);
		}

		$this->get_profile_list();
	}
	
	/*
	
	 Mode: 	save_profile
	 
	 Invoked:
		when we want to save search profile	
	 
	 Parameters:
		everything in form

	*/
	
	function set_profile() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$form = $this->input->post(NULL, true);
		
		$param = '';
		$name  = '';
		
		foreach($form as $key => $value) {
		
			switch($key) {
				case 'mode': // Do Nothing
				break;
				
				case 'Profile Name':
				case 'Profile_Name':
					$name = $value;
				break;
				
				default:
					$param .= $key . '=' . $value . ';';
				break;
			}

		}
		
		if($name != '' && $param != '') {
			$this->InvestigateModel->save_profile($name, $param);
		}
		
		$this->get_profile_list();
	}
	
	/*
	
	 Mode: 	get_profile_list
	 
	 Invoked:
		Gets the list of all saved profiles for the save priofile table	
	 
	 Parameters:
	 
	 
	 Prints:
	 	An object of extjs response form {"items": [{id:profile id ,name:profile name}....,{id:profile id ,name:profile name}], "total": int, "success": "true"/"false"}

	*/
	
	function get_profile_list() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$ans = array('success' => true, 'items' => array());
		$profiles = $this->InvestigateModel->get_profile_list();
		if($profiles && !empty($profiles)) {
			foreach($profiles as $profile) {
				$ans['items'][] = array(
					'id'   => $profile->id,
					'name' => $profile->name
				);
			}
		}
		$ans['total'] = count($ans['items']);
		
		return_json($ans);
		
	}
	
	/*

	 Mode: 	get_profile_details
	 
	 Invoked:
		when we want to fields of a saved profile
	 
	 Parameters:
		id - profile id
	 	 
	 Prints:
	 	An object of extjs response form {"items": [{desc:val},...,{desc:val}],"hidden":{hidden_field:val,...,hidden_field:val} "total": int, "success": "true"/"false"}		

	*/
	
	function get_profile_details() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$id = $this->input->get('id', true);
		if($id) {
			$profile = $this->InvestigateModel->get_profile_details($id);
		} else {
			return_json(array('success' => false, 'error' => 'ID attribute is missing'));
		}
		
		return_json($profile);
		
	}
	
	// Main Investigate Handler
	function get_investigate() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('AttributeScores');
		
		$ans = array('items' => array());
		
		// Configure Defaults
		$search_param = array(
			"fromdate" 			=> "",
			"todate" 			=> "",
			"app_id" 			=> "",
			"subdomain_ids" 	=> "",
			"user_id" 			=> "",
			"session_id" 		=> "",
			"ip_id" 			=> "",
			"page_num" 			=> "",
			"page_rows_number"	=> "",
			"sort_field" 		=> "td14", // Date
			"sort_order" 		=> "",
			"rid" 				=> "",
			"learningmode" 		=> "",
			"SID" 				=> "",
			"user"				=> "",
			"user_ip" 			=> "",
			"city" 				=> "",
			"country"			=> "",
			"status_code"		=> "",
			"date" 				=> "",
			"user_agent" 		=> "",
			"hostname" 			=> "",
			"seq_index" 		=> "",
			"SID_status" 		=> "",
			"path" 				=> "",
			"path_mode" 		=> "",
			"attribute_filter"  => "",
			"attribute_filter_val" => "",
			"flow" 				=> "",
			"score_field_num" 	=> 0,
			"score_field_type" 	=> "more",
			"score_field" 		=> "avg_score",
			"method" 			=> "",
			"criteria"			=> array()
		);
		
		// Sortfield Index
		$sortfield_index = array(
			'td0' => "seq_index",
			'td1' => "user",
			'td2' => "INET_ATON(user_ip)",
			'td3' => "SID",
			'td4' => "city",
			'td5' => "country",
			'td6' => "query_score",
			'td7' => "query_score",
			'td8' => "flow_score",
			'td9' => "landing_normal",
			'td10' => "geo_score",
			'td11' => "week_day_score",
			'td12' => "hour_score",
			'td13' => "status_code",
			'td14' => "date",
			'td15' => "subdomain_name",
			'td16' => "path",
			'td17' => "SID_status",
			'td19' => "avg_score",
			'td20' => "method",
			'td21' => "protocol",
			'td22' => "title"
		);
		
		// Parse Post Values
		$search_post = $this->input->post(NULL, true);

		$search_post_allowed = array('learningmode', 'sort_order', 'sort_field', 'fromdate', 
									 'todate', 'app_id', 'page_num', 'page_rows_number', 'subdomain_ids',
									 'rid', 'SID', 'user', 'user_ip', 'city', 'country', 'status_code',
									 'date', 'user_agent', 'path', 'hostname', 'seq_index', 
									 'SID_status', 'path_mode', 'attribute_filter', 'attribute_filter_val',
									 'flow', 'score_field_num', 'score_field_type', 'score_field', 'method', 'criteriaNum',
									 'user_not','useragent_not','sid_not','ip_not','city_not','country_not','action_not','method_not','session_alerts');
		
		
		$only_count = false;
		
		if(is_array($search_post)) {
			foreach($search_post as $key => $value) {
				if($key == 'selected_apps') {
					$search_param[$key] = json_decode($value, true);
				}
				if(in_array($key, $search_post_allowed)) {
					$search_param[$key] = $value;
				}
				if($key == 'only_count') {
					$only_count = true;
				}
			}
		}
		
		$all_apps = false;
	
		// Pass 1, see if ALL was selected
		if(isset($search_param['selected_apps']) && !empty($search_param['selected_apps'])) {
			foreach($search_param['selected_apps'] as $app) {
				if($app['id'] == '-1' || $app['sub_id'] == '-1') {
					$all_apps = true;
					if($app['text'] != 'All') {
						$search_param['freetext_app'] = $app['text'];
					}
				}
			}
		}
		if($all_apps) {
			unset($search_param['selected_apps']);
		}

		$got_apps = false;
		$got_subs = false;
		
		// Pass 2, see what types of selection we have
		if(isset($search_param['selected_apps']) && !empty($search_param['selected_apps'])) {
			foreach($search_param['selected_apps'] as $app) {
				if($app['root'] == '1') {
					$got_apps = true;
				} else {
					$got_subs = true;
				}
			}
		}
		
		$filter_apps = array();
		$filter_subs = array();
		
		$this->load->model('subdomains');
		
		// Pass 3, collect ids for query based on determined type
		if(isset($search_param['selected_apps']) && !empty($search_param['selected_apps'])) {
			foreach($search_param['selected_apps'] as $app) {
			
				// Only have root domains
				if($app['root'] && $got_apps && !$got_subs) {
					$filter_apps[] = $app['id'];
				}
				
				// Have mixed roots and subs
				if($app['root'] && $got_apps && $got_subs) {
					$subs = $this->subdomains->get_ids_by_app($app['id']);
					if(!empty($subs)) {
						foreach($subs as $sub) {
							$filter_subs[] = $sub->subdomain_id;
						}
					}
				}
				
				// Is a subdomain
				if(!$app['root']) {
					$filter_subs[] = $app['sub_id'];
				}
			}
		}
		
		$search_param['apps'] = $filter_apps;
		$search_param['subs'] = $filter_subs;
		
		// Post Process
		$search_param['fromdate'] = intval($search_param['fromdate']);
		$search_param['todate'] = intval($search_param['todate']);
		
		$search_param['sort_field'] = array_key_exists($search_param['sort_field'], $sortfield_index) ? 
									  $sortfield_index[$search_param['sort_field']] : 'date';
		
		// Process Criteria
		if(isset($search_param['criteriaNum'])) {
		
			for($i = 0; $i < intval($search_param['criteriaNum']); $i++) {
			
				if(isset($search_post['score_field'      . $i]) &&
				   isset($search_post['score_field_type' . $i]) &&
				   isset($search_post['score_field_num'  . $i])) {

				   $search_param['criteria'][] = array(
				
						'score_field'      => $search_post['score_field'      . $i],
						'score_field_type' => $search_post['score_field_type' . $i],
						'score_field_num'  => $search_post['score_field_num'  . $i],
						'relation'		   => isset($search_post['relation' . $i . '_' . ($i + 1)]) ? 
													$search_post['relation' . $i . '_' . ($i + 1)] : false
				   );
				   
				}
				
			}
		}
		
		// Start Investigate
		$investigate_arr = $this->InvestigateModel->investigate($search_param, true, $only_count);
		
		// Prevent duplicates
		$rids = array();
		
		if($only_count) {
			
			$total = 0;
			
			if(is_array($investigate_arr) && !empty($investigate_arr)) {
				foreach($investigate_arr as $investigate) {
					$total+= $investigate[0];
				}
			}
			
			$ans['total'] = $total;
		
			// Output
			return_json($ans);
			
		}
		// ELSE ...
		
		if(is_array($investigate_arr) && !empty($investigate_arr)) {
			
			foreach($investigate_arr as $investigate) {
				
				if(is_array($investigate_arr) && !empty($investigate_arr)) {
				
					foreach($investigate[0] as $row) {
						
						if(!in_array($row->RID, $rids)) {
							
							if($row->page_id == 0) {
								$data = $this->AttributeScores->get_specified_param_by_RIDs(array(19), array($row->RID), true, $search_param['fromdate'], $search_param['todate']);
								if($data && !empty($data) && isset($data[0]->data) ) {
									$row->display_path = $data[0]->data;
									$row->title        = '';
								}
							}
							
							$ans['items'][] = array(
								'td0'  => $row->seq_index,
								'td1'  => $row->user_id,
								'td2'  => $row->user_ip,
								'td3'  => $row->SID,
								'td4'  => $row->city,
								'td5'  => $row->country,
								'td6'  => intval(floatval($row->query_score)    * 100) . '',
								'td7'  => intval(floatval($row->query_score)    * 100) . '',
								'td8'  => intval(floatval($row->flow_score)     * 100) . '',
								'td9'  => intval(floatval($row->landing_normal) * 100) . '',
								'td10' => intval(floatval($row->geo_normal)     * 100) . '',
								'td13' => $row->status_code,
								'td14' => $row->time,
								'td15' => $row->hostname,
								'td16' => $row->display_path,
								'td17' => $row->SID_status, // $row->SID_status == 'invalid' ? '' :
								'td18' => $row->RID,
								'td19' => intval(floatval($row->avg_score)      * 100) . '',
								'td20' => $row->method,
								'td21' => $row->protocol,
								'td22' => $row->title,
								'td23' => isset($row->business_id) ? $row->business_id : '',
								'td24' => $row->app_id . ',' . $row->subdomain_id,
							);
							
							$rids[] = $row->RID;
							
						}
					
					}
				
				}
				
			}
			
		}
		
		$ans['count'] = count($ans['items']);
		
		// If we're not searching for a specific flow, connect to flows at this point
		
		if($search_param['flow'] == '') {
			
			// Check to see if user have workflows read access
			if(!$this->ion_auth->is_admin()) {
				if(!$this->acl->has_perm('Workflow', 'get')) {
					return_json($ans);
					die;
				}
			}
			
			$RIDS = array();
			foreach($ans['items'] as $item) {
				$RIDS[] = $item['td18'];
			}
				
			$flows = $this->InvestigateModel->flows_for_RIDS($RIDS);

			if($flows && !empty($flows)) {

				foreach($flows as $flow) {
					foreach($ans['items'] as $id => $item) {
						if($item['td18'] == $flow->RID) {
							$item['td23'] = $flow->business_id;
							$ans['items'][$id] = $item;
						}
					}
				}

			}
			
		}
		
		// Connect to users, attach user reputation
		
		$user_ids = array();
		foreach($ans['items'] as $id => $item) {
			if(intval($item['td1']) > 0) {
				$user_ids[] = $item['td1'];
			} else {
				$ans['items'][$id]['td1'] = ''; // Remove the default 0
			}
		}
		if(!empty($user_ids)) {
			$this->db->select('user_id, user, score');
			$this->db->from('reputation_users');
			$this->db->where_in('user_id', $user_ids);
			$query  = $this->db->get();
			$result = $query->result();
			if(!empty($result)) {
				foreach($result as $row) {
					
					foreach($ans['items'] as $id => $item) {
						if(intval($item['td1']) == intval($row->user_id)) {
							$item['td1'] = $row->user;
							$item['td24'] = $row->score;
							$ans['items'][$id] = $item;
						}
					}
					
				}
			}
		}
		
		
		// Output
		return_json($ans);
		
	}
	
	/*
	
	 Function: get_params_for_request
	 
	 Invoked:
		when we want to show the parameters of a request on the window
	 
	 Parameters:
		results - an sql query response tuple
		ans  - an array to which add the new results 
		rdis - an array of rids already added, to prevent duplicates
	 
	 
	 Prints JSON:
	 	'Details' => array(), 
		'Scores' => array(), 
		'Params' => array()
	*/
	
	function _get_params_for_request($RID, $date) {
		
		$ans = array( 'Details' => array(), 'Scores' => array(), 'Params' => array() );
		
		$this->load->model('AttributeScores');
		$this->load->model('RequestScores');
		
		$params = $this->AttributeScores->get_investigate_params_by_RID($RID, true, -1, -1);
		
		
		foreach($params as $param) {
			
			if($param->att_id == '19') {
				continue;
			}
			if($param->att_id == '7' && $param->att_source == 'H') {
				$hostname = $param->data;
			}
			if($param->att_id == '16') { // Authorization
				if(strlen($param->data) > 10) {
					$param->data = substr($param->data, 0, 10) . '...';
				}
			}

			$ans['Params'][] = array(
				'id'  => intval($param->att_id),
				'td0' => htmlspecialchars($param->att_name),
				'td1' => htmlspecialchars($param->data),
				'td2' => intval($param->attribute_score_normal),
				'td3' => $param->att_source,
				'alias' => $param->att_alias,
				'noisy' => intval($param->noisy),
				'mask'  => intval($param->mask),
				'structure' => $param->structure
			);
			
		}
		
		
		$details = $this->RequestScores->get_investigate_details_by_RID($RID, true, -1, -1);
		
		if($details && !empty($details)) {
			$details = $details[0];
			
			$ans['Details'][] = array (
				'td0' => 'Method',
				'td1' => $details->method . ' ' . $details->status_code
			);
			
			if($details->subdomain_name && $details->subdomain_name != '') {
			
				$hostname = $details->subdomain_name;
			
			} else {
			
				if($details->app_domain && $details->app_domain != '') {
					$hostname = $details->app_domain;
				}
			
			}
			
			if($details->page_id != 0 && isset($hostname) && $details->display_path && $details->display_path != '') {
			
				$ans['Details'][] = array (
					'td0' => 'URL',
					'td1' => strtolower($details->protocol) . '://' . $hostname . $details->display_path
				);
				
			} else {
			
				if($details->page_id == 0) {
					
					$page_name = "";
					
					foreach($params as $param) {
						if($param->att_id == '19') {
							$page_name = $param->data;
						}
					}
					
					if($page_name == "") {
						$page_name = $details->display_path;
					}
					if($page_name == "") {
						$page_name = "/404 not found";
					}
					
					$ans['Details'][] = array(
						'td0' => 'URL',
						'td1' => ($hostname ? (strtolower($details->protocol) . '://' . $hostname) : '') . $page_name
					);
				
				}
			
			}
			
			if(isset($details->title) && $details->title != '') {
				$ans['Details'][] = array(
					'td0' => 'Title',
					'td1' => $details->title
				);
			}			
			
			$ans['Details'][] = array(
				'td0' => 'Date',
				'td1' => $details->time
			);
			$ans['Details'][] = array(
				'td0' => 'IP',
				'td1' => $details->user_ip
			);
			if(isset($hostname)) {
				$ans['Details'][] = array(
					'td0' => 'Application',
					'td1' => $hostname
				);
			}
			$ans['Details'][] = array(
				'td0' => 'Location',
				'td1' => $details->city . ', ' . $details->country
			);	
			if($details->user_id && $details->user_id != 0) {
				$ans['Details'][] = array(
					'td0' => 'User',
					'td1' => $details->user
				);	
			}

			$ans['Scores'] = array(
				array('key' => 'Query', 'value' => intval(floatval($details->query_score) * 100)),
			//	array('key' => 'Request', 'value' => intval(floatval($details->request_score) * 100)),
				array('key' => 'Flow', 'value' => intval(floatval($details->flow_score) * 100)),
				array('key' => 'Speed', 'value' => intval(floatval($details->landing_normal) * 100)),
				array('key' => 'Geo', 'value' => intval(floatval($details->geo_normal) * 100)),
				array('key' => 'Average', 'value' => intval(floatval($details->avg_score) * 100))
			);
			
		}
		
		return $ans;
	
	}
	
	function get_params_for_request() {
	
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$ans = array( 'Details' => array(), 'Scores' => array(), 'Params' => array() );
	
		// Parse Post Values
		$RID  = $this->input->post('RID', true);
		$date = $this->input->post('date', true);
		if(!$RID || !$date) {
			return_json($ans);
		}
		
		$ans = $this->_get_params_for_request($RID, $date);
		
		return_json($ans);
	
	}
	
}