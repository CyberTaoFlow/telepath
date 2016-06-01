<?php

class M_Search extends CI_Model {
	
	function __construct() {
		parent::__construct();
		
		//$params = array();
		//$params['hosts'] = array ('127.0.0.1:9200');
// $params['logging'] = true;
// $params['logPath'] = '/tmp/elasticsearch.log';

		
		$this->elasticClient = new Elasticsearch\Client();
	}

	function search($scope, $settings, $suspect_threshold = false)
	{
		
		$limit = 100;

		switch($settings['sort']) {

			case 'date':
				$sortfield = 'date';
				break;
			case 'count':
				if($scope=='alerts' || $scope == 'cases' ){
					$sortfield = $scope.'_count';
				}
				else{
					$sortfield = '_count';
				}

				break;
//			case 'score':
//				$sortfield = 'last_score';
//			break;
			default:
				$sortfield = 'date';
				break;
		}

		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = [
			'size' => 0,
			"aggs" => [
				"sid" => [ 
				
					"terms" => [ "field" => "sid", "size" => $limit, "order" => [ $sortfield => $settings['dir'] ] ],
					"aggs" => [
						/*"country_code" => [ 
							"terms" => [ "field" => "country_code", "size" => 1 ] 
						],
						"city" => [ 
							"terms" => [ "field" => "city" , "size" => 1 ] 
						],
						"id" => [ 
							"terms" => [ "field" => "_id" , "size" => 1 ] 
						],
						"ip_orig" => [
							"terms" => [ "field" => "ip_orig" , "size" => 1 ]
						],
						"host" => [
							"terms" => [ "field" => "host" , "size" => 100 ]
						],
						"alerts_count" => [
							"sum" => [ "field" => "alerts_count" ]
						],
						"alerts_names" => [
							"terms" => [ "field" => "alerts.name", "size" => 100 ]
						],
						"business_actions_count" => [
							"sum" => [ "field" => "business_actions_count" ]
						],
						"business_actions_names" => [
							"terms" => [ "field" => "business_actions.name", "size" => 100 ]
						],
						"score" => [
							"avg" => [ "field" => "score_average" ]
						],*/
						"date" => [
							"max" => [ "field" => "ts" ]
						],
					],
				
				],
				"sid_count" => [
					"cardinality" => [ "field" => "sid" ],
				]
			],
			'query' => [
				'bool' => [
					'must' => [
						[ 'range' => [ 'ts' => [ 'gte' => intval($settings['range']['start']), 'lte' => intval($settings['range']['end']) ] ] ],
						[
                            'query_string' => [
                                "query" => $settings['search'],
								"default_operator" => 'AND'
                            ] 
                        ]
					]
				],
			],
		];


		switch($scope) {
			case 'alerts':
				$params['body']['query']['bool']['must'][] =  [ 'exists' => [ 'field' => 'alerts' ] ];
				$params['body']["aggs"]["sid"]["aggs"]["alerts_count"] = [ "sum" => [ "field" => "alerts_count" ] ];
				//	$params['body']['query']['bool']['must'][]=[ 'range' => [ 'alerts_count' => [ 'gte' => 1 ] ] ];
				//$params2['body']['query']['bool']['must'][] = [ 'filtered' => [ 'filter' => [ 'exists' => [ 'field' => 'alerts' ] ] ] ];
			break;
			case 'cases':
				// Here we also need cases data
				$params['body']['query']['bool']['must'][] =  [ 'exists' => [ 'field' => 'cases_name' ] ];
				$params['body']["aggs"]["sid"]["aggs"]["cases_names"] = [ "terms" => [ "field" => "cases_name", "size" => 100 ] ];
				$params['body']["aggs"]["sid"]["aggs"]["cases_count"] = [ "sum" => [ "field" => "cases_count" ] ];
//				$params2['body']['query']['bool']['must'][] = [ 'filtered' => [ 'filter' => [ 'exists' => [ 'field' => 'cases_name' ] ] ] ];
//				$params2['body']["aggs"]["sid"]["aggs"]["cases_names"] = [ "terms" => [ "field" => "cases_name", "size" => 100 ] ];
//				$params2['body']["aggs"]["sid"]["aggs"]["cases_count"] = [ "sum" => [ "field" => "cases_count" ] ];
			break;
			case 'suspects':
				$params['body']['query']['bool']['must'][] = [ 'range' => [ 'score_average' => [ 'gte' => $suspect_threshold ] ] ];
				$params['body']['query']['bool']['must_not'][] =  [ 'exists' => [ 'field' => 'alerts' ] ];
//				$params['body']['query']['bool']['must_not'][] =  [ 'exists' => [ 'field' => 'cases_name' ] ];
//				$params2['body']['query']['bool']['must'][] = [ 'range' => [ 'score_average' => [ 'gte' => $suspect_threshold ] ] ];
//				$params2['body']['query']['bool']['must_not'][] = [ 'filtered' => [ 'filter' => [ 'exists' => [ 'field' => 'alerts' ] ] ] ];
			break;
			case 'requests':
				// old method
				$params['body']['query']['bool']['must'][] = [ 'range' => [ 'score_average' => [ 'lt' => $suspect_threshold ] ] ];
				$params['body']['query']['bool']['must_not'][] =  [ 'exists' => [ 'field' => 'alerts' ] ] ;
//				$params['body']['query']['bool']['must_not'][] =  [ 'exists' => [ 'field' => 'cases_name' ] ];
//				$params2['body']['query']['bool']['must'][] = [ 'range' => [ 'score_average' => [ 'lt' => $suspect_threshold ] ] ];
//				$params2['body']['query']['bool']['must_not'][] = [ 'filtered' => [ 'filter' => [ 'exists' => [ 'field' => 'alerts' ] ] ] ];
//				$params2['body']['query']['bool']['must_not'][] = [ 'filtered' => [ 'filter' => [ 'exists' => [ 'field' => 'cases_name' ] ] ] ];

				// new method !!!
/*
                                $params['body']['query']['filtered']['filter']['bool']['must'][] = [ 'range' => [ 'score_average' => [ 'lt' => $suspect_threshold ] ] ];
                                $params['body']['query']['filtered']['filter']['bool']['must_not'][] =  [ 'exists' => [ 'field' => 'alerts' ] ];
                                $params2['body']['query']['filtered']['filter']['bool']['must'][] = [ 'range' => [ 'score_average' => [ 'lt' => $suspect_threshold ] ] ];
                                $params2['body']['query']['filtered']['filter']['bool']['must_not'][] =  ['exists' =>  [   'field' => 'alerts'  ] ];
*/
			break;
		}



		//var_dump($settings['apps']);
		$params = append_application_query($params, $settings['apps']);	
		//var_dump(json_encode($params));
		$result = $this->elasticClient->search($params);
		$results = array('items' => array());

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
					"terms" => [ "field" => "ip_orig" , "size" => 1 ]
				],
				"host" => [
					"terms" => [ "field" => "host" , "size" => 100 ]
				],
				"alerts_count" => [
					"sum" => [ "field" => "alerts_count" ]
				],
				"alerts_names" => [
					"terms" => [ "field" => "alerts.name", "size" => 100 ]
				],
				"cases_count" => [
					"sum" => [ "field" => "cases_count" ]
				],
				"cases_names" => [
					"terms" => [ "field" => "cases_name", "size" => 100 ]
				],
				"business_actions_count" => [
					"sum" => [ "field" => "business_actions_count" ]
				],
				"business_actions_names" => [
					"terms" => [ "field" => "business_actions.name", "size" => 100 ]
				],
				"score" => [
					"avg" => [ "field" => "score_average" ]
				],
				"date" => [
					"max" => [ "field" => "ts" ]
				],
				"user" => [
					"terms" => [ "field" => "username",
						"size" => 1,
						"order" => [ "_term" => "desc" ]
					]
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

		$params2['body']['query']['bool']['must'][] = ['range' => ['ts' => ['gte' => intval($settings['range']['start']), 'lte' => intval($settings['range']['end'])]]];
		$params2['body']['query']['bool']['must'][] = ['query_string' => ["query" => $settings['search'],"default_operator" => 'AND']];

		if(isset($result["aggregations"]) && 
		   isset($result["aggregations"]["sid"]) && 
		   isset($result["aggregations"]["sid"]["buckets"]) && 
		   !empty($result["aggregations"]["sid"]["buckets"])) {
		   
				$sid_buckets = $result["aggregations"]["sid"]["buckets"];
				foreach($sid_buckets as $sid) {
		
					$sid_key = $sid['key'];
					$doc_count = $sid['doc_count'];

					$params3 = $params2;
					
					$params3['body']['query']['bool']['must'][] = [ 'term' => ['sid' => $sid['key'] ] ];

					$result2 = $this->elasticClient->search($params3);
					$sid = $result2['aggregations'];

					$item = array(
						"sid"     => $sid_key, 
						"city"    => $sid['city']['buckets'][0]['key'], 
						"alerts_count"  => $sid['alerts_count']['value'], 
						"alerts_names"  => $sid['alerts_names']['buckets'],
						"cases_count"  => $sid['cases_count']['value'],
						"cases_names"  => $sid['cases_names']['buckets'],
						"actions_count"  => $sid['business_actions_count']['value'], 
						"actions_names"  => $sid['business_actions_names']['buckets'],  
						"country" => strtoupper($sid['country_code']['buckets'][0]['key']),
						"ip_orig" => long2ip($sid['ip_orig']['buckets'][0]['key']),
						"host"    => $sid['host']['buckets'],
						"count"   => $doc_count,
						"score_average" => $sid['score']['value'],
						"date"  => $sid['date']['value'],
						"ip_score"=>$sid['last_score']['buckets'][0]['key'],
						"user" => $sid['user']['buckets'][0]['key']
					);

					// if one session ID includes alerts requests and regular requests, we don't display it
					if ($scope == 'requests' && ($item['alerts_count']>0 || $item['cases_count']>0) ){
						$result["aggregations"]["sid_count"]["value"]--;
						continue;
					}

					$results['items'][] = $item;
					
				}
				
				$results['total'] = $result["aggregations"]["sid_count"]["value"];
				
		}
				
		
		$results['success'] = true;
		$results['query']   = $params;
		return $results;
	
	}
	

}

?>
