<?php
	
class M_Dashboard extends CI_Model {
	
	function __construct() {
		parent::__construct();
		
//		$params = array();
//		$params['hosts'] = array ('127.0.0.1:9200');
		
		$this->elasticClient = new Elasticsearch\Client();
	}
	
	
	
//	public function get_alerts($range, $apps = array(), $sort = 'count', $dir = 'ASC') {
//
//		$this->load->model('M_Alerts');
//		return $this->M_Alerts->get_alerts(false, false, $sort, $dir, 0, 5, false, $range, $apps);
//
//	}
	
//	public function get_cases($range, $apps = array()) {
//
//		$params['body'] = array(
//			'size'  => 0,
//			'aggs'  => array(
//				'cases' => array(
//					"terms" => array(
//						"field" => "cases_name",
//						"size" => 200
//					),
//					"aggs" => [
//						"sid" => [ "cardinality" => [ "field" => "sid", "precision_threshold" => 200] ],
//						'date'=>  ['max'=>['field'=>'ts']]
//					]
//				)
//			),
//			'query' => array(
//				'bool' => array(
//					'must' => array(
//						array(
//							'range' => array(
//							  'ts' => array(
//								'gte' => intval($range['start']),
//								'lte' => intval($range['end'])
//							  )
//							)
//						)
//					)
//				)
//			)
//		);
//
//		$params = append_application_query($params, $apps);
//		$params = append_access_query($params);
//
//		$results = $this->elasticClient->search($params);
//		$ans = array();
//
//		if(!empty($results) && isset($results['aggregations'])) {
//
//			foreach($results['aggregations']['cases']['buckets'] as $bucket) {
//
//				$this->load->model('M_Cases');
//				$case_data = $this->M_Cases->get_case_data($bucket['key']);
//
//				if (isset($case_data['empty']) && $case_data['empty'] == false)
//				{
//					$ans[] = array('name' => $bucket['key'], 'count' => $bucket['sid']['value'], 'last_time'=>$bucket['date']['value'], 'checkable' => false, 'case_data' => $case_data);
//				}
//
//			}
//			return $ans;
//
//		}
//
//		return $ans;
//
//	}
	
	public function get_gap_alerts($interval, $range, $apps = array()) {

		$params['index']='telepath-20*';
		$params['type']='http';
		$params['body'] = array(
			'size'  => 0,
			'aggs'  => array(
				'sid' => array(
					"histogram" => [
						"field" => "ts",
						"interval" => $interval,
						"min_doc_count" => 0,
						"extended_bounds" => ["min"=>intval($range['start']),"max"=>intval($range['end'])]
					]
				)
			),
			'query' => [
				'bool' => [
					'must' => [
						[ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ],
//						[ 'range' => [ 'alerts_count' => [ 'gte' => 1 ] ] ],
						[ 'exists' => [ 'field' => 'alerts' ] ],

					]
				]
			]
		);
		
		$params = append_application_query($params, $apps);
		$params = append_access_query($params);
		
		$results   = $this->elasticClient->search($params);
		$data = array();
		if(!empty($results) && isset($results['aggregations']['sid']))
		{
			//$result['alerts'][]   = array($time['start'] * 1000, $this->get_gap_alerts($time, $apps));
			foreach ($results['aggregations']['sid']['buckets'] as $bucket)
			{
				$data[] = array($bucket['key'] * 1000, $bucket['doc_count']);
			}
			return $data;
		}

		return $data;

	}

	// Not used for now
	public function get_gap_score_per_time($interval, $range, $apps = array()) {

		$params['index']='telepath-20*';
		$params['type']='http';
		$params['body'] = array(
			'size'  => 0,
			'aggs'  => array(
				'sid' => array(
					"histogram" => [
						"field" => "ts",
						"interval" => $interval,
						"min_doc_count" => 0,
						"extended_bounds" => ["min"=>intval($range['start']),"max"=>intval($range['end'])]
					],
					'aggs' =>[
						"score" => [
							"avg" => [ "field" => "score_average" ]
						]
					]
				)
			),
			'query' => [
				'bool' => [
					'must' => [
						[ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ],
//						[ 'range' => [ 'alerts_count' => [ 'gte' => 1 ] ] ],
						[ 'exists' => [ 'field' => 'score_average' ] ],

					],
					'must_not' => [
						["match" => ['operation_mode' => '1']]
					]
				]
			]
		);

		$params = append_application_query($params, $apps);
		$params = append_access_query($params);

		$results   = $this->elasticClient->search($params);
		$data = array();
		if(!empty($results) && isset($results['aggregations']['sid']))
		{
			//$result['alerts'][]   = array($time['start'] * 1000, $this->get_gap_alerts($time, $apps));
			foreach ($results['aggregations']['sid']['buckets'] as $bucket)
			{
				if($bucket['score']['value']){
					$score=$bucket['score']['value'];
				}
				else{
					$score=0;
				}
				$data[] = array($bucket['key'] * 1000, $score);
			}
			return $data;
		}

		return $data;

	}

	public function get_gap_score($learning=false) {

		$params['index']='telepath-20*';
		$params['type']='http';
		$params['body'] = array(
			'size'  => 0,
//			'aggs'  => array(
//				'score' => array(
//					"histogram" => [
//						"field" => "score_average",
//						"interval" => 0.1,
//						"min_doc_count" => 0,
//						"extended_bounds" => ["min"=>0,"max"=>1]
//					]
//				)
//			),
//			'aggs' =>[
//				"score" => [
//					"terms" => [ "field" => "score_average", "order"=>["_term"=>"asc"] ]
//				]
//			],
			'aggs'=> [
				"host" => [
					"terms" => [ "field" => "host" , "size" => 5 ],
					'aggs' => [
						"score" => [
							"range" => [
								"field" => "score_average",
								"ranges" => [
									["from" => 0, "to" => 0.01],
									["from" => 0.01, "to" => 0.02],
									["from" => 0.02, "to" => 0.03],
									["from" => 0.03, "to" => 0.04],
									["from" => 0.04, "to" => 0.05],
									["from" => 0.05, "to" => 0.06],
									["from" => 0.06, "to" => 0.07],
									["from" => 0.07, "to" => 0.08],
									["from" => 0.08, "to" => 0.09],
									["from" => 0.09, "to" => 0.10],
									["from" => 0.10, "to" => 0.11],
									["from" => 0.11, "to" => 0.12],
									["from" => 0.12, "to" => 0.13],
									["from" => 0.13, "to" => 0.14],
									["from" => 0.14, "to" => 0.15],
									["from" => 0.15, "to" => 0.16],
									["from" => 0.16, "to" => 0.17],
									["from" => 0.17, "to" => 0.18],
									["from" => 0.18, "to" => 0.19],
									["from" => 0.19, "to" => 0.20],
									["from" => 0.20, "to" => 0.21],
									["from" => 0.21, "to" => 0.22],
									["from" => 0.22, "to" => 0.23],
									["from" => 0.23, "to" => 0.24],
									["from" => 0.24, "to" => 0.25],
									["from" => 0.25, "to" => 0.26],
									["from" => 0.26, "to" => 0.27],
									["from" => 0.27, "to" => 0.28],
									["from" => 0.28, "to" => 0.29],
									["from" => 0.29, "to" => 0.30],
									["from" => 0.30, "to" => 0.31],
									["from" => 0.31, "to" => 0.32],
									["from" => 0.32, "to" => 0.33],
									["from" => 0.33, "to" => 0.34],
									["from" => 0.34, "to" => 0.35],
									["from" => 0.35, "to" => 0.36],
									["from" => 0.36, "to" => 0.37],
									["from" => 0.37, "to" => 0.38],
									["from" => 0.38, "to" => 0.39],
									["from" => 0.39, "to" => 0.40],
									["from" => 0.40, "to" => 0.41],
									["from" => 0.41, "to" => 0.42],
									["from" => 0.42, "to" => 0.43],
									["from" => 0.43, "to" => 0.44],
									["from" => 0.44, "to" => 0.45],
									["from" => 0.45, "to" => 0.46],
									["from" => 0.46, "to" => 0.47],
									["from" => 0.47, "to" => 0.48],
									["from" => 0.48, "to" => 0.49],
									["from" => 0.49, "to" => 0.50],
									["from" => 0.50, "to" => 0.51],
									["from" => 0.51, "to" => 0.52],
									["from" => 0.52, "to" => 0.53],
									["from" => 0.53, "to" => 0.54],
									["from" => 0.54, "to" => 0.55],
									["from" => 0.55, "to" => 0.56],
									["from" => 0.56, "to" => 0.57],
									["from" => 0.57, "to" => 0.58],
									["from" => 0.58, "to" => 0.59],
									["from" => 0.59, "to" => 0.60],
									["from" => 0.60, "to" => 0.61],
									["from" => 0.61, "to" => 0.62],
									["from" => 0.62, "to" => 0.63],
									["from" => 0.63, "to" => 0.64],
									["from" => 0.64, "to" => 0.65],
									["from" => 0.65, "to" => 0.66],
									["from" => 0.66, "to" => 0.67],
									["from" => 0.67, "to" => 0.68],
									["from" => 0.68, "to" => 0.69],
									["from" => 0.69, "to" => 0.70],
									["from" => 0.70, "to" => 0.71],
									["from" => 0.71, "to" => 0.72],
									["from" => 0.72, "to" => 0.73],
									["from" => 0.73, "to" => 0.74],
									["from" => 0.74, "to" => 0.75],
									["from" => 0.75, "to" => 0.76],
									["from" => 0.76, "to" => 0.77],
									["from" => 0.77, "to" => 0.78],
									["from" => 0.78, "to" => 0.79],
									["from" => 0.79, "to" => 0.80],
									["from" => 0.80, "to" => 0.81],
									["from" => 0.81, "to" => 0.82],
									["from" => 0.82, "to" => 0.83],
									["from" => 0.83, "to" => 0.84],
									["from" => 0.84, "to" => 0.85],
									["from" => 0.85, "to" => 0.86],
									["from" => 0.86, "to" => 0.87],
									["from" => 0.87, "to" => 0.88],
									["from" => 0.88, "to" => 0.89],
									["from" => 0.99, "to" => 0.90],
									["from" => 0.90, "to" => 0.91],
									["from" => 0.91, "to" => 0.92],
									["from" => 0.92, "to" => 0.93],
									["from" => 0.93, "to" => 0.94],
									["from" => 0.94, "to" => 0.95],
									["from" => 0.95, "to" => 0.96],
									["from" => 0.96, "to" => 0.97],
									["from" => 0.97, "to" => 0.98],
									["from" => 0.98, "to" => 0.99],
									["from" => 0.99, "to" => 1],
									["from" => 1]
								]
							]
						]
					],
				],
				"score" => [
						"range" => [
							"field" => "score_average",
							"ranges" => [
								["from" => 0, "to" => 0.01],
								["from" => 0.01, "to" => 0.02],
								["from" => 0.02, "to" => 0.03],
								["from" => 0.03, "to" => 0.04],
								["from" => 0.04, "to" => 0.05],
								["from" => 0.05, "to" => 0.06],
								["from" => 0.06, "to" => 0.07],
								["from" => 0.07, "to" => 0.08],
								["from" => 0.08, "to" => 0.09],
								["from" => 0.09, "to" => 0.10],
								["from" => 0.10, "to" => 0.11],
								["from" => 0.11, "to" => 0.12],
								["from" => 0.12, "to" => 0.13],
								["from" => 0.13, "to" => 0.14],
								["from" => 0.14, "to" => 0.15],
								["from" => 0.15, "to" => 0.16],
								["from" => 0.16, "to" => 0.17],
								["from" => 0.17, "to" => 0.18],
								["from" => 0.18, "to" => 0.19],
								["from" => 0.19, "to" => 0.20],
								["from" => 0.20, "to" => 0.21],
								["from" => 0.21, "to" => 0.22],
								["from" => 0.22, "to" => 0.23],
								["from" => 0.23, "to" => 0.24],
								["from" => 0.24, "to" => 0.25],
								["from" => 0.25, "to" => 0.26],
								["from" => 0.26, "to" => 0.27],
								["from" => 0.27, "to" => 0.28],
								["from" => 0.28, "to" => 0.29],
								["from" => 0.29, "to" => 0.30],
								["from" => 0.30, "to" => 0.31],
								["from" => 0.31, "to" => 0.32],
								["from" => 0.32, "to" => 0.33],
								["from" => 0.33, "to" => 0.34],
								["from" => 0.34, "to" => 0.35],
								["from" => 0.35, "to" => 0.36],
								["from" => 0.36, "to" => 0.37],
								["from" => 0.37, "to" => 0.38],
								["from" => 0.38, "to" => 0.39],
								["from" => 0.39, "to" => 0.40],
								["from" => 0.40, "to" => 0.41],
								["from" => 0.41, "to" => 0.42],
								["from" => 0.42, "to" => 0.43],
								["from" => 0.43, "to" => 0.44],
								["from" => 0.44, "to" => 0.45],
								["from" => 0.45, "to" => 0.46],
								["from" => 0.46, "to" => 0.47],
								["from" => 0.47, "to" => 0.48],
								["from" => 0.48, "to" => 0.49],
								["from" => 0.49, "to" => 0.50],
								["from" => 0.50, "to" => 0.51],
								["from" => 0.51, "to" => 0.52],
								["from" => 0.52, "to" => 0.53],
								["from" => 0.53, "to" => 0.54],
								["from" => 0.54, "to" => 0.55],
								["from" => 0.55, "to" => 0.56],
								["from" => 0.56, "to" => 0.57],
								["from" => 0.57, "to" => 0.58],
								["from" => 0.58, "to" => 0.59],
								["from" => 0.59, "to" => 0.60],
								["from" => 0.60, "to" => 0.61],
								["from" => 0.61, "to" => 0.62],
								["from" => 0.62, "to" => 0.63],
								["from" => 0.63, "to" => 0.64],
								["from" => 0.64, "to" => 0.65],
								["from" => 0.65, "to" => 0.66],
								["from" => 0.66, "to" => 0.67],
								["from" => 0.67, "to" => 0.68],
								["from" => 0.68, "to" => 0.69],
								["from" => 0.69, "to" => 0.70],
								["from" => 0.70, "to" => 0.71],
								["from" => 0.71, "to" => 0.72],
								["from" => 0.72, "to" => 0.73],
								["from" => 0.73, "to" => 0.74],
								["from" => 0.74, "to" => 0.75],
								["from" => 0.75, "to" => 0.76],
								["from" => 0.76, "to" => 0.77],
								["from" => 0.77, "to" => 0.78],
								["from" => 0.78, "to" => 0.79],
								["from" => 0.79, "to" => 0.80],
								["from" => 0.80, "to" => 0.81],
								["from" => 0.81, "to" => 0.82],
								["from" => 0.82, "to" => 0.83],
								["from" => 0.83, "to" => 0.84],
								["from" => 0.84, "to" => 0.85],
								["from" => 0.85, "to" => 0.86],
								["from" => 0.86, "to" => 0.87],
								["from" => 0.87, "to" => 0.88],
								["from" => 0.88, "to" => 0.89],
								["from" => 0.99, "to" => 0.90],
								["from" => 0.90, "to" => 0.91],
								["from" => 0.91, "to" => 0.92],
								["from" => 0.92, "to" => 0.93],
								["from" => 0.93, "to" => 0.94],
								["from" => 0.94, "to" => 0.95],
								["from" => 0.95, "to" => 0.96],
								["from" => 0.96, "to" => 0.97],
								["from" => 0.97, "to" => 0.98],
								["from" => 0.98, "to" => 0.99],
								["from" => 0.99, "to" => 1],
								["from" => 1]
							]
						]
					]

			],
			'query' => [
				'bool' => [
					'must' => [
//						[ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ],
						[ 'exists' => [ 'field' => 'score_average' ] ],

					]
				]
			]
		);

		if(!$learning){
			$params['body']['query']['bool']['must_not'][] =  [ 'match' => [ 'operation_mode' => '1' ] ];
		}

//		$params = append_application_query($params, $apps);
//		$params = append_access_query($params);

		$results   = $this->elasticClient->search($params);
//		$data = array();
		if (!empty($results) && isset($results['aggregations']) && isset($results['aggregations']['host']) && isset
			($results['aggregations']['host']['buckets']) && isset ($results['aggregations']['score'])
		) {
//		{
//			foreach( $results['aggregations']['score']['buckets'] as $bucket) {
//				$data[]=$bucket['doc_count'];
//			}
//		}
			$results['aggregations']['host']['buckets'][] = [
				'key' => 'All Applications',
				"score" => $results['aggregations']['score']
			];
		}
		return $results['aggregations']['host']['buckets'];

	}

	public function get_gap_cases($interval, $range, $apps = array()) {
		
		$result = array('case' => 0, 'noncase' => 0);

		$params['index']='telepath-20*';
		$params['type']='http';
		$params['body'] = array(
			'size'  => 0,
			'aggs'  => array(
				'sid' => array(
					"histogram" => [
						"field" => "ts",
						"interval" => $interval,
						"min_doc_count" => 0,
						"extended_bounds" => ["min"=>intval($range['start']),"max"=>intval($range['end'])]
					]
				)
			),
			'query' => [
				'bool' => [
					'must' => [
						[ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ],
//						[ 'range' => [ 'cases_count' => [ 'gte' => 1 ] ] ],
						[ 'exists' => [ 'field' => 'cases_name' ] ],
					]
				]
			]
		);
		
		$params = append_application_query($params, $apps);
		$params = append_access_query($params);
		
		$results   = $this->elasticClient->search($params);
		$data = array();
		if(!empty($results) && isset($results['aggregations']['sid'])) {
			//$result['alerts'][]   = array($time['start'] * 1000, $this->get_gap_alerts($time, $apps));
			foreach ($results['aggregations']['sid']['buckets'] as $bucket)
			{
				$data[] = array($bucket['key'] * 1000, $bucket['doc_count']);
			}
			return $data;
		}
		
		return $data;
		
	}
	
//	public function get_suspects($range, $apps = array(), $sort = 'count', $dir = 'ASC', $limit = 5) {
//
//		$this->load->model('M_Suspects');
//		return $this->M_Suspects->get($range, $apps, $sort, $dir, 0, $limit);
//
//	}
	
	// Dashboard Functionality
	function get_map($range, $apps, $map_mode) {

		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = [
			'size'  => 0,
			'aggs'  => [
				'country_code' => [
					"terms" => [
						"field" => "country_code",
						"size" => 200
					],
				]
			],
			'query' => [
				'bool' => [
					'must' => [
						[ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ],
					]
				],
			],
		];
		
		if($map_mode== 'alerts') {
			$params['body']['query']['bool']['must'][] = [ 'range' => [ 'alerts_count' => [ 'gte' => 1 ] ] ];
		}
		
		$params = append_application_query($params, $apps);
		$params = append_access_query($params);
		
		$results = $this->elasticClient->search($params);
		//print_r($results);
		if(!empty($results) && isset($results['aggregations'])) {
			
			$ans = array();
			foreach($results['aggregations']['country_code']['buckets'] as $bucket) {
				$ans[$bucket['key']] = $bucket['doc_count']; // $bucket['ip_orig']['value']
			}
			return $ans;
			
		} 
		return array();
		
	}
	
	public function get_gap_sessions($interval, $range, $apps = array(), $suspect_threshold, $suspects = false) {
		
		$result = array('case' => 0, 'noncase' => 0);

		$params['index']='telepath-20*';
		$params['type']='http';
		$params['body'] = array(
			'size'  => 0,
			'aggs'  => array(
				'sid' => array(
					"histogram" => [
						"field" => "ts",
						"interval" => $interval,
						"min_doc_count" => 0,
						"extended_bounds" => ["min"=>intval($range['start']),"max"=>intval($range['end'])]
					]
				)
			),
			'query' => [
				'bool' => [
					'must' => [
						[ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ],
						[ 'range' => [ 'score_average' => [ ($suspects ? 'gte' : 'lt') => $suspect_threshold ] ] ],
					],
					'must_not'=>[
						[ 'exists' => [ 'field' => 'alerts' ] ],
						[ 'exists' => [ 'field' => 'cases_name' ] ]
					]

				]
			]
		);


		$params = append_application_query($params, $apps);
		$params = append_access_query($params);
		$results   = $this->elasticClient->search($params);
		$data = array();
		if(!empty($results) && isset($results['aggregations']['sid'])) {
			//$result['alerts'][]   = array($time['start'] * 1000, $this->get_gap_alerts($time, $apps));
			foreach ($results['aggregations']['sid']['buckets'] as $bucket)
			{
				$data[] = array($bucket['key'] * 1000, $bucket['doc_count']);
			}
			return $data;
		}
		
		return $data;
				
	}
	
	function get_chart($range, $apps, $suspect_threshold) {

		$time = $this->getRanges($range, false, false);


		$result = array(
			'alerts'   => $this->get_gap_alerts($time, $range, $apps),
			'sessions' => $this->get_gap_sessions($time, $range, $apps, $suspect_threshold),
			'cases'    => $this->get_gap_cases($time, $range, $apps),
			'suspects' => $this->get_gap_sessions($time, $range, $apps, $suspect_threshold, true),
			'score'=>  $this->get_gap_score_per_time($time, $range, $apps)
		);
		return $result;
	}
	
	public function getRanges($range, $gap, $scale) {

		$del = 30;
		$days = intval( ($range['end'] + 60 - $range['start']) / (60*60*24) );
		//var_dump("days $days");
		if ($days <= 7)
		{
			$del = 24;
		} else if ($days <= 31)
		{
			$del = $days;
		} else {
			$del = 30;
		}
		#var_dump("num $del, int: ". intval(($range['end'] - $range['start'] ) / $del));

		return intval(($range['end'] - $range['start'] ) / $del);

	}

	public function set_map_mode($status,$range){

		$this->user_id = $this->ion_auth->get_user_id();

		$parsed=array('time_range'=>$range,'status'=>$status);

		$this->ion_auth->update($this->user_id, array('extradata' => json_encode($parsed)));

	}

	public function get_map_mode($local=false)
	{
		$this->user_id = $this->ion_auth->get_user_id();

		if ($this->user_id) {
			$this->user = $this->ion_auth->user($this->user_id)->result();
			if (!isset($this->user[0])) {
				return;
			}
			$this->user = (array)$this->user[0];
		} else {
			return;
		}

		$parsed = $this->user['extradata'] != '' ? json_decode($this->user['extradata'], true) : false;

		if (isset ($parsed['status'])) {
			$map_mode = $parsed['status'];
		} else {
			$map_mode = "alerts";
		}

		if ($local)
			return $map_mode;

		return_success($map_mode);
	}
}

?>
