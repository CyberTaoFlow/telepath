<?php

class M_Cases extends CI_Model {
	
	private $tableName = 'cases';
	
	function __construct() {
		parent::__construct();
	}

	public function case_alerts_count($cid) {
		
		// Connect to case alerts
		$this->db->from('case_alerts');
		$this->db->where('case_id', $cid);
		$q_count = $this->db->count_all_results();
		return $q_count;
	
	}
	
	public function get($conditions = array(), $limit = 100, $sort = 'count', $dir = 'asc', $range = false) {
		
		if(!empty($conditions)) {
			foreach($conditions as $condition => $value) {
				$this->db->where($condition, $value);
			}
		}
		
		$this->db->select('id, name, case_data, created, favorite');
		$this->db->select('COUNT(DISTINCT SID) as count', false);
		$this->db->from('cases');
		$this->db->join('case_requests', 'case_requests.case_id = cases.id', 'left');
		$this->db->join('request_scores_merge', 'request_scores_merge.RID = case_requests.RID', 'left');
		
		if($range) {
			$this->db->where('date >=', intval($range['start']));
			$this->db->where('date <=', intval($range['end']));
		}
		
		$this->db->group_by('id');
				
		$this->db->limit($limit);
		$this->db->order_by($sort, $dir);

		$q = $this->db->get();
		
		return $q->result();
		
	}
	
	public function get_requests($range = false, $apps = array(), $cid = false) {
		
		$this->db->from('case_requests');
		$this->db->join('request_scores_merge', 'request_scores_merge.RID = case_requests.RID', 'left');
		$this->db->join('pages', 'request_scores_merge.page_id=pages.page_id');
		$this->db->join('subdomains', 'pages.subdomain_id=subdomains.subdomain_id');
		
		$this->db->select('SQL_CALC_FOUND_ROWS request_scores_merge.RID, SID, seq_index, subdomain_name as app_domain', false);
		$this->db->select('(select ssl_flag from applications where app_id=hostname) as ssl_flag', false);
		$this->db->select('pages.page_id, pages.display_path, pages.title');
		$this->db->select('CAST(query_score*100 AS SIGNED) as query_score', false);
		$this->db->select('CAST(flow_score*100 AS SIGNED) as flow_score', false);
		$this->db->select('CAST(landing_normal*100 AS SIGNED) as landing_normal', false);
		$this->db->select('CAST(geo_normal*100 AS SIGNED) as geo_normal', false);
		$this->db->select('CAST(avg_score*100 AS SIGNED) as avg_score', false);
		$this->db->select('user_ip, date, hostname, country, city');
		
		// Range
		if($range) {
			$this->db->where('date >=', intval($range['start']));
			$this->db->where('date <=', intval($range['end']));
		}
		
		// Apps
		if(!empty($apps)) {	$this->db->where_in('hostname', $apps); }
		if(!$this->acl->all_apps()) { $this->db->where_in('hostname', $this->acl->allowed_apps); }
		
		// CID
		if($cid) {
			$this->db->where('case_id', $cid);
		}
		
		$this->db->order_by('request_scores_merge.RID DESC');
		$this->db->group_by('request_scores_merge.SID');
		
		$this->db->limit(100);
		
		$items = $this->db->get()->result();
		$total = $this->db->query('SELECT FOUND_ROWS() as total;')->row_array();
		$total = $total['total'];
		
		return array('items' => $items, 'total' => $total);
			
	}
	
	public function get_requests_chart($range, $apps, $cid = false) {
	
	
		$times = $this->get_range_gaps($range);
		$result = array();

		foreach($times as $time) {
			$result[]		 = array($time['start'] * 1000, $this->get_gap_case($time, $apps, $cid));
		}
		
		return $result;
	
	
	}
	
	public function get_gap_case($range, $apps = array(), $cid) {
		
		$this->db->from('case_requests');
		$this->db->join('request_scores_merge', 'request_scores_merge.RID = case_requests.RID', 'left');
		
		// Range
		$this->db->where('date >=', intval($range['start']));
		$this->db->where('date <=', intval($range['end']));
		
		// Apps
		if(!empty($apps)) {	$this->db->where_in('hostname', $apps); }
		if(!$this->acl->all_apps()) { $this->db->where_in('hostname', $this->acl->allowed_apps); }
		
		// CID
		$this->db->where('case_requests.case_id', $cid);
		
		return $this->db->count_all_results();
	
	}
	
	
	public function delete($cids) {
		
		$this->db->where_in('id', $cids);
		$this->db->delete($this->tableName);
		return true;
		
	}
	
	public function create($data) {
		
		$data['created'] = time();
		$this->db->insert($this->tableName, $data);
		return $this->db->insert_id();
		
	}
	
	public function update($cid, $data) {
		
		$data['created'] = time();
		$this->db->where('id', $cid);
		$this->db->update($this->tableName, $data);
		return true;
		
	}
	
	// TODO:: Create global model
	
	public function getTimeDifference($date, $count, $mode) {
		
		$diff = $this->db->query('SELECT UNIX_TIMESTAMP(DATE_SUB(FROM_UNIXTIME(' . $date . ') ,INTERVAL "' . $count . '" ' . $mode . ')) as diff')->result();
		$result = $diff[0]->diff;
		return $result;
		
	}
	
	public function get_gap($range) {
	
		$difference	= $range['end'] - $range['start'];

		if ($difference>=0 & $difference<=60*60)
			return 'minutesToHour';
		else if ($difference>60*60 && $difference<=60*60*24)	
			return 'hoursToDay';
		else if ($difference>60*60*24 && $difference<=60*60*24*7)	
			return 'dayToWeek';
		else if($difference>60*60*24*7 && $difference<=60*60*24*30)
			return 'weekToMonth';
		else if($difference>60*60*24*30 && $difference<=60*60*24*365/2)
			return 'monthTo6months';
		else if($difference>60*60*24*30 && $difference<=60*60*24*365)
			return '6monthsToYear';
		else if ($difference>60*60*24*365)
			return 'aYearPlus';	
		
	}
	
	public function get_range_gaps($range) {
		
		$times = array();
		$gap   = $this->get_gap($range);
		
		switch($gap) {
			
			// MAX = 24 (hourly)
			case 'hoursToDay':
			
				$index = 1;
				while($index <= 24) {
				
					$start = $this->getTimeDifference($range['end'], $index, 'HOUR');
					$end   = $this->getTimeDifference($range['end'], $index - 1, 'HOUR');
					
					$index += 1;
					$times[] = array('start' => $start, 'end' => $end);
					
				}
				
			break;
			// MAX = 30 (daily)
			case 'dayToWeek':
				
				$index = 1;
				while($range['start'] <= $this->getTimeDifference($range['end'], $index, 'DAY')) {
				
					$start = $this->getTimeDifference($range['end'], $index, 'DAY');
					$end   = $this->getTimeDifference($range['end'], $index - 1, 'DAY');
					
					$index += 1;
					$times[] = array('start' => $start, 'end' => $end);
					
				}
				
			break;
			// MAX = 10 (every 3 days)
			case 'weekToMonth':
			
				$index = 3;
				while($range['start'] <= $this->getTimeDifference($range['end'], $index, 'DAY')) {
				
					$start = $this->getTimeDifference($range['end'], $index, 'DAY');
					$end   = $this->getTimeDifference($range['end'], $index - 3, 'DAY');
					
					$index += 3;
					$times[] = array('start' => $start, 'end' => $end);
					
				}
				
				$end = $this->getTimeDifference($range['end'], $index - 3, 'DAY');
				$times[] = array('start' => $range['start'], 'end' => $end);
				
				
			break;
			// MAX = 12 (every 2 weeks)
			case 'monthTo6months':
				$index = 2;
				while($range['start'] <= $this->getTimeDifference($range['end'], $index, 'WEEK')) {
				
					$start    = $this->getTimeDifference($range['end'], $index, 'WEEK');
					$end  = $this->getTimeDifference($range['end'], $index - 2, 'WEEK');
					
					$index += 2;
					$times[] = array('start' => $start, 'end' => $end);
					
				}
				
				$end = $this->getTimeDifference($range['end'], $index - 2, 'WEEK');
				$times[] = array('start' => $range['start'], 'end' => $end);
				
			break;
			// MAX = 12 (every month)
			case '6monthsToYear':
				$index = 1;
				while($range['start'] <= $this->getTimeDifference($range['end'], $index, 'MONTH')) {
				
					$start    = $this->getTimeDifference($range['end'], $index, 'MONTH');
					$end  = $this->getTimeDifference($range['end'], $index - 1, 'MONTH');
					
					$index += 1;
					$times[] = array('start' => $start, 'end' => $end);
					
				}
				
				$end = $this->getTimeDifference($range['end'], $index - 1, 'MONTH');
				$times[] = array('start' => $range['start'], 'end' => $end);
				
			break;
			// MAX = 36+ for 3 years (every 3 months)
			case 'aYearPlus':
				$index = 3;
				while($range['start'] <= $this->getTimeDifference($range['end'], $index, 'MONTH')) {
				
					$start    = $this->getTimeDifference($range['end'], $index, 'MONTH');
					$end  = $this->getTimeDifference($range['end'], $index - 3, 'MONTH');
					
					$index += 3;
					$times[] = array('start' => $start, 'end' => $end);
					
				}
				
				$end = $this->getTimeDifference($range['end'], $index - 3, 'MONTH');
				$times[] = array('start' => $range['start'], 'end' => $end);
				
			break;
			
		}
		
		return $times;
		
	}
	
}

?>