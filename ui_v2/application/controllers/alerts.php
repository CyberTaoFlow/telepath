<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Alerts extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function index() {
		
	}
	
	public function get_alerts_ips() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$ans = array('success' => true, 'items' => array());
		
		$this->load->model('AlertsModel');
		$ips = $this->AlertsModel->get_ips();
		
		foreach($ips as $ip) {
			$ans['items'][] = array ( 'IP' => $ip->IP );
		}
		
		$ans['total'] = count($ans['items']);
		
		return_json($ans);
	
	}
	
	public function get_session_flow_params() {
	
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$sid	 = $this->input->post('sid', true);
		$RIDS	 = $this->input->post('rids', true);
		
		$this->load->model('AlertsModel');
		
		$session_flow_params = $this->AlertsModel->get_session_flow_params($sid, $RIDS);
		
		return_success($session_flow_params);

	}
	
	public function get_session_flow() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$sid	 = $this->input->post('sid', true);
		$ip		 = $this->input->post('ip', true);
		$rule_id = $this->input->post('rule_id', true);
				
		$this->load->model('AlertsModel');
		
		$session_flow = $this->AlertsModel->get_session_flow($sid, $ip, $rule_id);
		
		return_success($session_flow);

	}
	
	public function get_alerts() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// Collect variables
		
		$start 		= intval($this->input->get('start', true));
		$limit 		= intval($this->input->get('limit', true));
		$sortorder  = $this->input->get('sortorder', true);
		$sortfield  = $this->input->get('sortfield', true);
		
		$val 		= $this->input->get('val', true);
		$variable	= $this->input->get('variable', true);
		
		$filter     = $this->input->get('filter', true);
		
		$page 		= intval($this->input->get('page', true));
		$sort 		= $this->input->get('sort', true);
		
		if($page) {
			$start = (intval($page) - 1) * intval($limit);
		}

		$sort = json_decode($sort, true);
		if($sort && is_array($sort) && !empty($sort)) {
			$sortfield = $sort[0]['property'];
			$sortorder = $sort[0]['direction'];
		}
		
		if($filter) {
			$filter = json_decode($filter, true);
		}
		
		
		
		$all_apps = false;
	
		// Pass 1, see if ALL was selected
		if(isset($filter['selected_apps']) && !empty($filter['selected_apps'])) {
			foreach($filter['selected_apps'] as $app) {
				if($app['id'] == '-1' || $app['sub_id'] == '-1') {
					$all_apps = true;
					if($app['text'] != 'All') {
						$filter['freetext_app'] = $app['text'];
					}
				}
			}
		}
		if($all_apps) {
			unset($filter['selected_apps']);
		}

		$got_apps = false;
		$got_subs = false;
		
		// Pass 2, see what types of selection we have
		if(isset($filter['selected_apps']) && !empty($filter['selected_apps'])) {
			foreach($filter['selected_apps'] as $app) {
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
		if(isset($filter['selected_apps']) && !empty($filter['selected_apps'])) {
			foreach($filter['selected_apps'] as $app) {
			
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
		
		$filter['apps'] = $filter_apps;
		$filter['subs'] = $filter_subs;

		// Translate indices to fields
		
		$sortfield_map = array (
			'td0' =>  'id',
			'td1' =>  'date',
			'td2' =>  'SID',
			'td3' =>  'user',
			'td4' =>  'INET_ATON(IP)',
			'td5' =>  'name',
			'td6' =>  'description',
			'td7' =>  'average_score',
			'td8' =>  'counter',
			'td9' =>  'country',
			'td10' => 'city',
			'td11' => 'hostname'
		);

		if(!in_array($sortfield, array_values($sortfield_map))) {
		
			if(isset($sortfield_map[$sortfield])) {
				$sortfield = $sortfield_map[$sortfield];
			} else {
				$sortfield = 'date';
			}
		
		}

		$this->load->model('AlertsModel');
		$alerts_arr = $this->AlertsModel->get_alerts($variable, $val, $sortfield, $sortorder, $start, $limit, $filter);
		
		$alerts = $alerts_arr['items'];
		$total  = $alerts_arr['total'];
		
		$params = $this->AlertsModel->get_alert_group_params();
		
		$ans = array();
		
		$param = array();
		
		foreach($alerts as $alert) {
			
			$ans[] = array(
				
				'td0'  => $alert->id,
				'td1'  => $alert->date,
				'td2'  => $alert->SID,
				'td3'  => $alert->user,
				'td4'  => $alert->IP,
				'td5'  => $alert->name,
				'td6'  => $alert->description,
				'td7'  => $alert->average_score . '%',
				'td8'  => $alert->counter,
				'td9'  => $alert->country,
				'td10' => $alert->city,
				'td11' => $alert->subdomain != '' ? $alert->subdomain : $alert->hostname,
				'td12' => $alert->rule_group,
				'td13' => isset($params[$alert->rule_group]) ? $this->AlertsModel->get_attributes_for_alert($alert->SID, $alert->rule_group, $params[$alert->rule_group]) : array()
				
			);
	
		}
		
		$ans = array('items' => $ans, 'total' => $total, 'success' => true);
		return_json($ans);

	}
	
	public function del_alerts() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// Prepare variables
		$ids_to_delete = $this->input->post('ids_to_remove', true);
		
		foreach($ids_to_delete as $key => $value) {
			$ids_to_delete[$key] = intval($value);
		}
		
		// Load model
		$this->load->model('AlertsModel');
		// Do Delete
		$this->AlertsModel->delete($ids_to_delete);
				
		// Return 
		return_json(array('success' => true));
		
	}
}
