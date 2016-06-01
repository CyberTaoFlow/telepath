<?php

class M_Suspects extends CI_Model {
	
	function __construct() {
	
		parent::__construct();
		
		// Connect elastic
		//$params = array('hosts' => array('127.0.0.1:9200'));
#$params = array();
#$params['logging'] = true;
#$params['logPath'] = '/tmp/elasticsearch.log';
		$this->elasticClient = new Elasticsearch\Client();
		
	}

	public function get_threshold()
	{

//		$params['index'] = 'telepath-20*';
//		$params['type'] = 'http';
//		$params['body'] = [
//			'size' => 0,
//			"aggs" => [
//				"grades_stats" => [
//					"extended_stats" => ["field" => "score_average", "sigma" => 3]
//				]
//			],
//			'query' => [
//				'bool' => [
//					'must_not' => [
//						["match" => ['operation_mode' => '1']]
//					]
//				]
//			]
//		];
//
//		$result = $this->elasticClient->search($params);
//
//		if (isset($result["aggregations"]) &&
//			isset($result["aggregations"]["grades_stats"]) &&
//			isset($result["aggregations"]["grades_stats"]["std_deviation_bounds"]) &&
//			isset($result["aggregations"]["grades_stats"]["std_deviation_bounds"]["upper"]) &&
//			!empty($result["aggregations"]["grades_stats"]["std_deviation_bounds"]["upper"])
//		) {
//			return $result["aggregations"]["grades_stats"]["std_deviation_bounds"]["upper"];
//		} else {
//			return 0.85;
//		}
		return 0.80;

	}
	
	public function get($range, $apps, $sort, $sortorder, $start = 0, $limit = 100, $suspect_threshold, $search = '') {
		
		//$suspect_threshold = $this->get_threshold();
		//ROI CHECK THIS OUT (Yuli)
//		$suspect_threshold = 0.75;
		
		$sortfield = 'count';
		
		// fix undefined variabale (Yuli)
		$count = 0;
		
		switch($sort) {
			case 'date':
				$sortfield = 'date';
			break;
			case 'count':
//			case 'score':
				$sortfield = '_count';
			break;
//			case 'alerts':
//				$sortfield = 'alerts_count';
//			break;
		}
		
		//die;
		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = [
			'size' => 0,
			"aggs" => [
				"sid" => [ 
					"terms" => [ "field" => "sid", "size" => intval($limit) * 10, "order" => [ $sortfield => strtolower($sortorder) ] ], // Lists can scroll up to 10 times


// Disabe internal aggregation, Yuli
/*
					"aggs" => [
						"country_code" => [ 
							"terms" => [ "field" => "country_code", "size" => 1 ] 
						],
						"city" => [ 
							"terms" => [ "field" => "city" , "size" => 1 ] 
						],
						"id" => [ 
							"terms" => [ "field" => "_id" , "size" => 1 ] 
						],
						"ip_orig" => [
							"min" => [ "field" => "ip_orig" ]
						],
						"host" => [
							"terms" => [ "field" => "host" , "size" => 100 ]
						],
						"alerts_count" => [
							"sum" => [ "field" => "alerts_count" ]
						],
						"score" => [
							"avg" => [ "field" => "score_average" ]
						],
						"date" => [
							"max" => [ "field" => "ts" ]
						],
					],
*/
				
				],
				"sid_count" => [
					"cardinality" => [ "field" => "sid" ],
				]
			],
		];

		if ($sortfield == "date")
		{
			$params['body']["aggs"]["sid"]["aggs"] = [ "date" => [ "max" => [ "field" => "ts" ] ] ];
		}
//		else if ($sortfield == "alerts_count")
//		{
//			$params['body']["aggs"]["sid"]["aggs"] = [ "alerts_count" => [ "sum" => [ "field" => "alerts_count" ] ] ];
//		}
//		$params['body']["aggs"]["sid"]["aggs"] = [ "cases_count" => [ "sum" => [ "field" => "cases_count" ] ] ];

		$params['body']['query']['bool']['must'][] = [ 'range' => [ 'score_average' => [ 'gte' => $suspect_threshold ] ] ];
		$params['body']['query']['bool']['must_not'][] =  [ 'exists' => [ 'field' => 'alerts' ] ];
		$params['body']['query']['bool']['must_not'][] =  [ 'match' => [ 'operation_mode' => '1' ] ];

		if(!empty($range)) {
			$params['body']['query']['bool']['must'][] = [ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ];
		}
		if($search && strlen($search) > 1) {
			$params['body']['query']['bool']['must'][] = [ 'query_string' => [ "query" => $search, "default_operator" => 'AND'  ] ];
		}
		
		#print_r(json_encode($params));
		// die;
		
		$params = append_application_query($params, $apps);
		$params = append_access_query($params);
		#var_dump($params);

		$result = $this->elasticClient->search($params);
				
		$count_offset = 0;
		$count_insert = 0;
		
		if(isset($result["aggregations"]) && 
		   isset($result["aggregations"]["sid"]) && 
		   isset($result["aggregations"]["sid"]["buckets"]) && 
		   !empty($result["aggregations"]["sid"]["buckets"])) {
		   
				$sid_buckets = $result["aggregations"]["sid"]["buckets"];
				
				
				foreach($sid_buckets as $sid) {
				
					if($count_offset >= $start) {
						// Create subrequest for agregated data

						$sid_key = $sid['key'];
						$doc_count = $sid['doc_count'];

						$params2 = array();
						$params2['index'] = 'telepath-20*';
						$params2['type'] = 'http';
						$params2['body'] = [
								'size' => 0,
								"aggs" => [
									"country_code" => [
										"terms" => [ "field" => "country_code", "size" => 1 ]
									],
									"city" => [
										"terms" => [ "field" => "city" , "size" => 1 ]
									],
									"id" => [
										"terms" => [ "field" => "_id" , "size" => 1 ]
									],
									"ip_orig" => [
										"min" => [ "field" => "ip_orig" ]
									],
									"host" => [
										"terms" => [ "field" => "host" , "size" => 100 ]
									],
									"alerts_count" => [
										"sum" => [ "field" => "alerts_count" ]
									],
									"cases_count" => [
										"sum" => [ "field" => "cases_count" ]
									],
									"score" => [
										"avg" => [ "field" => "score_average" ]
									],
									"business_action" => [
										"terms" => [ "field" => "business_action", "size"=> 100 ]
									],
									"date" => [
										"max" => [ "field" => "ts" ]
									],
									"user" => [
										"terms" => [ "field" => "username",
											"order" => ["_term" => "desc" ],
											"size" => 1 ]
									],
									"last_score" => [
										"terms" => [
												"field" => "ip_score",
											"order"=>["max_ts" => "desc" ]
											],
										'aggs'=>[
											'max_ts'=>[
													"max" => [ "field" => "ts"]
											]
										]
									]
								]
							];

						$params2['body']['query']['bool']['must'][] = [ 'term' => ['sid' => $sid['key'] ] ];
						$params2['body']['query']['bool']['must'][] = [ 'range' => [ 'score_average' => [ 'gte' => $suspect_threshold ] ] ];
						$params2['body']['query']['bool']['must_not'][] =  [ 'exists' => [ 'field' => 'alerts' ] ];
						$params2['body']['query']['bool']['must_not'][] =  [ 'match' => [ 'operation_mode' => '1' ] ];

						if(!empty($range)) {	
							$params2["body"]["query"]["bool"]["must"][] = [ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ];
						}
						$result2 = $this->elasticClient->search($params2);
						$sid = $result2['aggregations'];
						if (count($sid['city']['buckets']) > 0)
						{
							$results['items'][] = array(
								"sid"     => $sid_key,
								"city"    => $sid['city']['buckets'][0]['key'], 
								"alerts_count"  => $sid['alerts_count']['value'],
								"cases_count"=> $sid['cases_count']['value'],
								"country" => strtoupper($sid['country_code']['buckets'][0]['key']),
								"ip_orig" => long2ip($sid['ip_orig']['value']),
								"host"    => $sid['host']['buckets'],
								"count"   => $doc_count,
								"business_action" => $sid['business_action']['buckets'],
								"date"  => $sid['date']['value'],
								"ip_score" =>$sid['last_score']['buckets'][0]['key'],
								"user" => $sid['user']['buckets'][0]['key']
							);
							$count_insert++;
							if($count_insert >= $limit) {
								break;
							}
						} else {
							// can not return record details here !!!
						}

					} else {
						$count_offset++;
					}

				}

			$count = $result["aggregations"]["sid_count"]["value"];

		}

		$results['success'] = true;
		$results['count']   = $count;
		$results['query']   = $params;
		$results['std']     = $suspect_threshold;
		return $results;



	}
	
	// public function get_suspect_score($key, $value) {
	
	// }
	
}

?>
