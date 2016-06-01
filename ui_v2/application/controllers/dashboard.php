<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Dashboard extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function get_realtime() {
		
	
		$m = new Memcached();
		$m->addServer('localhost', 11211);
	
		// We need to read suricata stats file
		
		$out = array(
			'alerts' 	     => $this->db->count_all_results('alerts_merge'),
			'case_alerts' 	 => $this->db->count_all_results('case_alerts') ,
			'applications'   => $this->db->count_all_results('applications'),
			'requests'		 => $this->db->count_all_results('request_scores_merge'),
			'pages' 		 => $this->db->count_all_results('pages'),
		);
		
		// We need to read suricata stats file
		$lines      = `tail -100 /opt/telepath/suricata/logs/stats.log`;
		$lines      = explode("\n", $lines);
		$need       = array('capture.kernel_packets', 'capture.kernel_drops');
		$suricata   = array();
		
		foreach($lines as $i => $line) {
			 $line = explode("|", str_replace(" ","", $line));
			 //if(in_array($line[0], $need)) {
				$suricata[$line[0]] = $line[2];
			 //}
		}
		$out = array_merge($out, array_shift($m->getStats()), $suricata);
		
		echo json_encode($out);
		die;
		
	}
	
	public function set_dashboard() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$date1 = $this->input->post('date1', true);
		$date2 = $this->input->post('date2', true);
		$refresh = $this->input->post('refresh', true);
		$trendsNum = $this->input->post('trendsNum', true);
		$suspects = $this->input->post('suspects', true);
		
		$this->load->model('DashboardModel');
		$this->DashboardModel->setSettings($date1, $date2, $refresh, $trendsNum, $suspects);
		
		return_json(array('success' => true));
		
	}
	
	public function set_dashboard_order() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$order = $this->input->post('order', true);
		$order = explode(',', $order);
		
		$panels = array('d_mapPanel', 'd_anomalyPanel', 'd_countriesPanel', 'd_trendsPanel');
		
		// Check its 4 panels
		if(count($panels) != count($order)) {
			return_json(array('success' => false));
		}
		// Check its the same 4 panels
		foreach($panels as $panel) {
			if(!in_array($panel, $order)) {
				return_json(array('success' => false));
			}
		}
		
		$this->load->model('DashboardModel');
		$this->DashboardModel->setDashboardOrder($order);
		
		return_json(array('success' => true));

	}
	
	public function get_dashboard() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		$scale  = $this->input->post('scale' , true);
		$limit  = $this->input->post('limit' , true);
		$order  = $this->input->post('order' , true);
		$gap    = $this->input->post('gap'   , true);
		$items  = $this->input->post('items' , true);
		
		$limitSuspects = 15;
		$limitTrends = 7;
		
		// Defaults
		if(!$gap) {	$gap = '6monthsToYear';	}
		if(!$scale) { $scale = 'Year'; }
		if(!$items) {
			$items  = $this->input->get('items' , true);
			if(!$items) {
				$items = 'settings,anomalies,suspects,bad_points,clusters,trends,sessions,dashboard_order,eta,status,limits';
			}
		}
		
		$items = explode(',', $items);
		
		// Start model
		$this->load->model('DashboardModel');
		$range = $this->DashboardModel->getRange($scale);
		
		// Prep result
		$ans = array('success' => true);
		$benchmarks = array();		
		
		
		// Sort out selected apps
		$selected_apps = $this->input->post('selected_apps' , true);
		if($selected_apps) {
			$selected_apps = json_decode($selected_apps, true);
		}
		$all_apps = false;
		$freetext_app = '';
		
		// Pass 1, see if ALL was selected
		if(isset($selected_apps) && !empty($selected_apps)) {
			foreach($selected_apps as $app) {
				if($app['id'] == '-1' || $app['sub_id'] == '-1') {
					$all_apps = true;
					if($app['text'] != 'All') {
						$freetext_app = $app['text'];
					}
				}
			}
		}
		
		if($all_apps) {
			unset($selected_apps);
		}
		
		$got_apps = false;
		$got_subs = false;
		
		// Pass 2, see what types of selection we have
		if(isset($selected_apps) && !empty($selected_apps)) {
			foreach($selected_apps as $app) {
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
		if(isset($selected_apps) && !empty($selected_apps)) {
			foreach($selected_apps as $app) {
			
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

		// Collect data
		foreach($items as $item) {
			
			$time = -microtime(true); 
			
			switch($item) {
			
				case 'settings':
					$ans['settings']   		= $this->_getSettings(true);
				break;
				case 'anomalies':
					$ans['anomalies']  		= $this->_getAnomalies($range, $filter_apps, $filter_subs);
				break;
				case 'suspects':
					$ans['suspects']   		= $this->_getSuspects($range, $filter_apps, $filter_subs, $limitSuspects);
				break;
				case 'bad_points':
					$ans['bad_points'] 		= $this->_getBadPoints($range, $filter_apps, $filter_subs);
				break;
				case 'clusters':
					$ans['clusters']   		= $this->_getClusters($range, $filter_apps, $filter_subs);
				break;
				case 'countries':
					$ans['countries']  		= $this->_getCountries($range, $filter_apps, $filter_subs);
				break;
				case 'trends':
					$ans['trends']     		= $this->_getTrends($range, $filter_apps, $filter_subs, $gap, $scale, $limitTrends);
				break;
				case 'sessions':
					$ans['sessions']   		= $this->_getSessions($range, $filter_apps, $filter_subs, $gap, $scale);
				break;
				case 'dashboard_order':
					$ans['dashboard_order'] = $this->_getDashboardOrder();
				break;
				case 'eta':
					$ans['eta'] 			= $this->_getETA();
				break;
				case 'limits':
					$ans['limits']   		= $this->_getLimits();
				break;
				case 'map':
					$ans['map']   		= $this->_getMap($range, $filter_apps, $filter_subs);
				break;
				
			}
			
			$time += microtime(true);
			$benchmarks[$item] = $time;
			
		}
	
		$ans['benchmarks'] = $benchmarks;
	
		return_json($ans);
		
	}
	
	private function _getSettings() {
		
		$this->load->model('ConfigModel');
		$config = $this->ConfigModel->get();
		$fields = array('operation_mode' => 1, 'time_start' => 1, 'time_end' => 1, 'timerInterval' => 1, 'limitAlertTrends' => 1, 'limitTopSuspects' => 1);
		$config = array_intersect_key($config, $fields);
		
		// BAD CODE, BAD.
		$limits = $this->_getLimits();
		if(isset($limits) && !empty($limits) && isset($limits[0])) {
			
			$config['date_min'] = $limits[0]->date_min;
			$config['date_max'] = $limits[0]->date_max;
			
			if(intval($config['date_min']) == 0) {
				$config['time_start'] = strtotime('-1 day');
			}
			if(intval($config['date_max']) == 0) {
				$config['time_start'] = strtotime('-1 day');
			}
			
			// We ended up having bad or no defaults
			if(intval($config['time_start']) == 0) {
				$config['time_start'] = strtotime('-7 days');
				$config['time_end']   = time();
			}
						
			if(intval($config['time_start']) > intval($config['date_min'])) {
				$config['time_start'] = $config['date_min'];
			}
			if(intval($config['time_end']) > intval($config['date_max'])) {
				$config['time_end'] = $config['date_max'];
			}
		}
		
		return $config;
		
	}

	private function _getAnomalies($range, $filter_apps, $filter_subs) {
		
		$anomalies = $this->DashboardModel->getAnomalies($range, $filter_apps, $filter_subs);
		
		if(empty($anomalies)) {
			return array(array("data" => 0, "name" => 'No Alerts', "percentage" => '0%', "id" => -1));
		} else {
			$total = 0;
			foreach($anomalies as $anomaly) {
				$total += $anomaly['data'];
			}
			foreach($anomalies as $key => $anomaly) {
			
				if ($total == 0) {
					$anomalies[$key]['percentage'] = '0%';
				} else {
					$anomalies[$key]['percentage'] = number_format(($anomaly['data'] / $total) * 100, 2) . '%';
				}
					
			}
			
			return array_slice($anomalies, 0, 10);
		}
		
	}
	
	private function _getSuspects($range, $filter_apps, $filter_subs, $limit) {
		
		$suspects = $this->DashboardModel->getSuspects($range, $filter_apps, $filter_subs, $limit);
		$ans 	  = array();
		
		foreach($suspects as $suspect) {
			$ans[] = array(
				'td0' => $suspect->date,
				'td1' => $suspect->user_id,
				'td2' => $suspect->IP,
				'td3' => $suspect->city,
				'td4' => $suspect->country,
				'td5' => number_format($suspect->query_score * 100, 2) . '%', // QUERY
				'td7' => number_format($suspect->flow_score * 100, 2) . '%',  // FLOW (PATH)
				'td8' => number_format($suspect->landing_normal * 100, 2) . '%',  // SPEED
				'td9' => number_format($suspect->geo_normal * 100, 2) . '%',  // LOCATION
				'td12' => number_format($suspect->score * 100, 2) . '%',
				'td13' => $suspect->SID,
				'td24' => '',
			);
		}
		
		// Connect to users, attach user reputation
		$user_ids = array();
		foreach($ans as $id => $item) {
			if(intval($item['td1']) > 0) {
				$user_ids[] = $item['td1'];
			} else {
				$ans[$id]['td1'] = ''; // Remove the default 0
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
					
					foreach($ans as $id => $item) {
						if(intval($item['td1']) == intval($row->user_id)) {
							$item['td1'] = $row->user;
							$item['td24'] = $row->score;
							$ans[$id] = $item;
						}
					}
					
				}
			}
		}
		
		return $ans;
		
	}
	
	private function _getBadPoints($range, $filter_apps, $filter_subs) {
	
		$bad_points = $this->DashboardModel->getBadPoints($range, $filter_apps, $filter_subs);
		return $bad_points;
		
	}
	
	private function _getMap($range, $filter_apps, $filter_subs) {
	
		$map    = array();
		$mapRaw = $this->DashboardModel->get_map($range, $filter_apps, $filter_subs);
		
		foreach($mapRaw as $row) {
			$map[strtolower($row->country)] = intval($row->sum_count);
		}
		
		return $map;
		
	}
	
	private function _getClusters($range, $filter_apps, $filter_subs) {
	
		$clusters = $this->DashboardModel->getClusters($range, $filter_apps, $filter_subs);
		return $clusters;
		
	}
	
	private function _getCountries($range, $filter_apps, $filter_subs) {
		
		$countries = $this->DashboardModel->getCountries($range, $filter_apps, $filter_subs);
		if(empty($countries)) {
			return array(array("name" => "No Alerts","data" => 0, "percentage" => '0%', "id" => -1));
		} else {
			$ans = array();
			$total = 0;
			foreach($countries as $country) {
				$total += $country->sum;
			}
			foreach($countries as $country) {
				$ans[] = array(
					'name' => $country->country,
					'data' => $country->sum,
					'percentage' => number_format(($country->sum / $total) * 100, 2) . '%'
				);
			}
			return $ans;
		}
		
	}
	
	private function _getTrends($range, $filter_apps, $filter_subs, $gap, $scale, $limit) {
		
		$trends = $this->DashboardModel->getTrends($range, $filter_apps, $filter_subs, $gap, $scale);
		$counters = array();
		
		if(is_array($trends) && !empty($trends)) {
			
			foreach($trends as $trend => $values) {
				
				$counters[$trend] = 0;
				
				if(is_array($values) && !empty($values)) {
					
					foreach($values as $val) {
						$counters[$trend] += intval($val[0]);
					}
					
				}
			
			}
		
		}
		
		arsort($counters);
			
		$result = array();
		$i = 0;
		foreach($counters as $key => $value) {
			
			$i++;
			
			if($i > $limit) {
				break;
			}
		
			$result[$key] = $trends[$key];

		}
			
		return $result;
		
	}
	
	private function _getSessions($range, $filter_apps, $filter_subs, $gap, $scale) {
		
		$sessions = $this->DashboardModel->getSessions($range, $filter_apps, $filter_subs, $gap, $scale);
		return $sessions;
	
	}
	
	private function _getDashboardOrder() {
	
		$dashboard_order = $this->DashboardModel->getDashboardOrder();
		$ans = array();
		foreach($dashboard_order as $row) {
			$ans[] = $row->panelId;
		}
		return $ans;
		
	}
	
	private function _getETA() {
	
		$eta = $this->DashboardModel->getETA();
		$eta = $eta[0]->value;
		return $eta;
		
	}
	
	private function _getLimits() {
	
		$limits = $this->DashboardModel->getLimits();
		return $limits;
		
	}

}
