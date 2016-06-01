<?php

class M_Cases extends CI_Model {
	
	function __construct() {
		parent::__construct();
		
		// Connect elastic
		//$params = array('hosts' => array('127.0.0.1:9200'));
#$params['logging'] = true;
#$params['logPath'] = '/tmp/elasticsearch.log';
		$this->elasticClient = new Elasticsearch\Client();
	}

	public function old_get_case_data($cid) {
		
		$params['index'] = 'telepath-config';	
		$params['body'] = [
			'size' => 100,
			'query' => [ 'term' => [ '_type' => 'cases' ] ]
		];
		
		$result = get_elastic_results($this->elasticClient->search($params));

		$results = array();
		foreach($result as $row) {
			$results = array_merge($results, $row['All_Cases']);
		}
		
		if($cid == 'all') {
			return $results;
		}

		foreach($results as $row) {
			if(strtolower($row['case_name']) == strtolower($cid)) {
				$row['empty'] = false;
				return $row;
			}
		}
		


	}

	public function get_case_data($cid)
	{

		if ($cid == 'all') {
			$response = $this->elasticClient->search(['index' => 'telepath-cases','type'=>'case']);
			$cases = get_source($response);
			return $cases;

		}

		$params = [
			'index' => 'telepath-cases',
			'type' => 'case',
			'id' => $cid,
		];

		if ($this->elasticClient->exists($params)) {
			$response = $this->elasticClient->get($params);
			$response['_source']['empty'] = false;
			return $response['_source'];
		}

		return array(array('case_name' => $cid, 'details' => array(), 'empty' => true));


	}

	public function get($limit = 100, $range = false, $apps = array(), $search=null) {

		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = array(
			'size'  => 0,
			'aggs'  => array(
				'cases' => array(
					"terms" => array(
						"field" => "cases_name",
						"size" => $limit
					),
					"aggs" => [
						"sid" => [ "cardinality" => [ "field" => "sid", "precision_threshold" => 200 ] ],
						'date'=>  ['max'=>['field'=>'ts']]
					]
				)
			),
			'query' => array(
				'bool' => array(
					'must' => array(
						array(
							'range' => array(
							  'ts' => array(
								'gte' => intval($range['start']),
								'lte' => intval($range['end'])
							  )
							)
						),
						array(
							'exists' => [ 'field' => 'cases_name' ]
						)
					)
				)
			)
		);
//		if($search && strlen($search) > 1) {
//			$params['body']['query']['bool']['must'][] = [ 'query_string' => [ "query" => $search, "default_operator" => 'AND'  ] ];
//		}
		$params = append_application_query($params, $apps);
		$params = append_access_query($params);
		$results = $this->elasticClient->search($params);
		$ans = array();
		
		if(!empty($results) && isset($results['aggregations'])) {
			
			foreach($results['aggregations']['cases']['buckets'] as $bucket) {
				
				$ans[] = array('name' => $bucket['key'], 'count' => $bucket['sid']['value'], 'last_time'=>$bucket['date']['value'], 'checkable' => false, 'case_data' => $this->get_case_data($bucket['key']));
				
			}
			return $ans;
			
		} 
		
		return $ans;
		
	}
	
	public function get_case_sessions($limit = 100, $cid, $range = array(), $apps = array(), $sort  = 'date', $sortorder = 'desc' ) {

		
		switch($sort) {
			case 'date':
				$sortfield = 'date';
			break;
			case 'count':
				$sortfield = '_count'; // we need to get a specific case, so the count is not the sum of the "cases_count" elastic field
			break;
			case 'score':
				$sortfield = 'last_score';
			break;
//			case 'type':
//				$sortfield = 'alerts_count';
//			break;
			
		}

		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['_source']=false;
		$params['body'] = [
			'size' => 100,
			"aggs" => [
				"sid" => [ 
				
					"terms" => [ "field" => "sid", "size" => 100, "order" => [ $sortfield => $sortorder ] ],
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
						"score" => [
							"avg" => [ "field" => "alerts.score" ]
						],
						"alerts_names" => [
							"terms" => [ "field" => "alerts.name", "size" => 100 ]
						],
						"date" => [
							"min" => [ "field" => "ts" ]
						],
						"user" => [
							"terms" => ["field" => "username",
								"order" => ["_term" => "desc"],
								"size" => 1]
						],
					],
				
				],
				"sid_count" => [
					"cardinality" => [ "field" => "sid", "precision_threshold" => 200 ],
				]
			],
/*
			'query' => [
				'bool' => [
					'must' => [
						[ 'term' => [ '_type' => 'http' ] ],
					]
				],
			],
*/
		];

		$params['body']['query']['bool']['must'][] = [ 'term' => [ "cases_name" => $cid ] ];

		if(!empty($range)) {
			$params['body']['query']['bool']['must'][] = [ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ];
		}

		$params = append_application_query($params, $apps);
		$params = append_access_query($params);
		$result = $this->elasticClient->search($params);
		
		$results = array('items' => array());
		
		if(isset($result["aggregations"]) &&
		   isset($result["aggregations"]["sid"]) &&
		   isset($result["aggregations"]["sid"]["buckets"]) &&
		   !empty($result["aggregations"]["sid"]["buckets"])) {

				$sid_buckets = $result["aggregations"]["sid"]["buckets"];
				foreach($sid_buckets as $sid) {

					$results['items'][] = array(
						"sid"     => $sid['key'],
						"city"    => $sid['city']['buckets'][0]['key'],
						"alerts_count"  => $sid['alerts_count']['value'],
						"alerts_names"  => $sid['alerts_names']['buckets'],
						"country" => strtoupper($sid['country_code']['buckets'][0]['key']),
						"ip_orig" => long2ip($sid['ip_orig']['buckets'][0]['key']),
						"host"    => $sid['host']['buckets'],
						"count"   => $sid['doc_count'],
						"score"  => $sid['score']['value'],
						"date"  => $sid['date']['value'],
						"user" => $sid['user']['buckets'][0]['key']
					);
			}
		}

		$results['success'] = true;
		$results['query']   = $params;
		$results['count']   = intval($result["aggregations"]['sid_count']['value']);

		if(!empty($result['hits']['hits'])){
			foreach ($result['hits']['hits'] as $key=>$val){
				unset($result['hits']['hits'][$key]['_score']);
			}
		}
		$results['requests']= $result['hits']['hits'];
		return $results;

	}

	public function get_similar_sessions($requests, $cid)
	{
		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = [
			'query' => [
				'bool' => [
					'must' => [
						["more_like_this" => [
							"docs" => $requests,
							"max_query_terms" => 25,
							"min_term_freq" => 0,
							"min_doc_freq" => 20,
//							"max_doc_freq" => 0,
							"min_word_length" => 0,
//							"max_word_length" => 0,
							"minimum_should_match" => "30%",
//							"percent_terms_to_match" => 0.5
							]
						]
					],
					'must_not' => [
						['term' => ["cases_name" => $cid]]
					]
				]
			],
			"aggs" => [
				"sid" => [

					"terms" => ["field" => "sid", "size" => 100,
//						"order" => [ 'score' => 'desc' ]
					],
					"aggs" => [
						"country_code" => [
							"terms" => ["field" => "country_code", "size" => 1]
						],
						"city" => [
							"terms" => ["field" => "city", "size" => 1]
						],
						"id" => [
							"terms" => ["field" => "_id", "size" => 1]
						],
						"ip_orig" => [
							"terms" => ["field" => "ip_orig", "size" => 1]
						],
						"host" => [
							"terms" => ["field" => "host", "size" => 100]
						],
						"alerts_count" => [
							"sum" => ["field" => "alerts_count"]
						],
						"score" => [
							"avg" => ["field" => "alerts.score"]
						],
						"alerts_names" => [
							"terms" => ["field" => "alerts.name", "size" => 100]
						],
						"date" => [
							"min" => ["field" => "ts"]
						],
						"user" => [
							"terms" => ["field" => "username",
								"order" => ["_term" => "desc"],
								"size" => 1
							]
						],
//						"similarity" => [
//							"max" => [ "script" => "_score" ]
//						],
					],

				],
				"sid_count" => [
					"cardinality" => ["field" => "sid", "precision_threshold" => 200],
				]
			]
		];
//		if(!empty($range)) {
//			$params['body']['query']['bool']['must'][] = [ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ];
//		}
//
//		$params = append_application_query($params, $apps);
		$params = append_access_query($params);

		$result = $this->elasticClient->search($params);
		$results = array('items' => array());

		if (isset($result["aggregations"]) &&
			isset($result["aggregations"]["sid"]) &&
			isset($result["aggregations"]["sid"]["buckets"]) &&
			!empty($result["aggregations"]["sid"]["buckets"])
		) {

			$sid_buckets = $result["aggregations"]["sid"]["buckets"];
			foreach ($sid_buckets as $sid) {

				$results['items'][] = array(
					"sid" => $sid['key'],
					"city" => $sid['city']['buckets'][0]['key'],
					"alerts_count" => $sid['alerts_count']['value'],
					"alerts_names" => $sid['alerts_names']['buckets'],
					"country" => strtoupper($sid['country_code']['buckets'][0]['key']),
					"ip_orig" => long2ip($sid['ip_orig']['buckets'][0]['key']),
					"host" => $sid['host']['buckets'],
					"count" => $sid['doc_count'],
					"score" => $sid['score']['value'],
					"date" => $sid['date']['value'],
					"user" => $sid['user']['buckets'][0]['key']
				);
			}
		}

		$results['count'] = intval($result["aggregations"]['sid_count']['value']);

		return $results;
	}

	public function store_similar_case_sessions($similars, $cid)
	{
		$params = [
			'index' => 'telepath-cases',
			'type' => 'case',
			'id' => $cid,
			'body' => [
				'doc'=>[
					'similars' => $similars
				]
			]
		];

		$this->elasticClient->update($params);
	}

	public function get_similar_case_sessions($cid)
	{
		$params = [
			'index' => 'telepath-cases',
			'type' => 'case',
			'id' => $cid
		];

		if ($this->elasticClient->exists($params)) {
			$response = $this->elasticClient->get($params);
		}

		if (isset($response['_source']['similars']))
			return $response['_source']['similars'];
		else
			return [];
	}

	public function delete_similar_case_sessions($cid)
	{
		$params = [
			'index' => 'telepath-cases',
			'type' => 'case',
			'id' => $cid
		];

		if ($this->elasticClient->exists($params)) {
			$this->elasticClient->delete($params);
		}
	}


	// Get the time of the last flagged requests by cases
//	public function get_last_case_update()
//	{
//		$params = [
//			'index' => 'telepath-cases',
//			'type' => 'case',
//			'id' => 'last_update',
//		];
//		if ($this->elasticClient->exists($params)) {
//			$response = $this->elasticClient->get($params);
//			if (isset($response['_source']['last_update']) && !empty($response['_source']['last_update']))
//				return $response['_source']['last_update'];
//		}
//		return false;
//	}
//
//	// Set the time of the last flagged requests by cases
//	public function set_last_case_update($update_time)
//	{
//		$params = [
//			'index' => 'telepath-config',
//			'type' => 'config',
//			'id' => 'last_case_update_id',
//			'body' => [
//				'doc' => [
//					'last_update' => $update_time
//				],
//				'doc_as_upsert' =>true
//			]
//		];
//
//		$response = $this->elasticClient->update($params);
//		return $response;
//	}

	/**
	 * @param $cases_name array of strings (cases name) or "all"
	 * @param $range boolean - if true update only from the last update
	 * @param $method string - "add" ,"delete" or "update" case
     */

	public function flag_requests_by_cases($cases_name, $range, $method)
	{
		logger('Start','/var/log/flag_requests_by_cases.log');

		@set_time_limit(-1);

		ignore_user_abort(true);

		// If it's a script that always run, we take the time before the iterations
		if ($range){
			$update_time = time()-200;
			$this->load->model('M_Config');
		}

		$status = $this->elasticClient->indices()->stats(['index' => 'telepath-20*']);
		foreach ($status['indices'] as $index_name => $index_status) {
			logger('Start index: ' . $index_name );

			$params = [
//				"search_type" => "scan",    // use search_type=scan
				"scroll" => "1m",          // h ow long between scroll requests. should be small!
				"size" => 9999,               // how many results *per shard* you want back
				"index" => $index_name,
				"type" => 'http',
				"_source" => ['cases_name', 'cases_count']
			];

			// Delete all the flags of the cases, if method = delete or update
			if ($method != 'add') {

				foreach ($cases_name as $case) {
//					register_shutdown_function([$this, 'remove_update_flag'],$case,$method, $range);
					$params['body']['query']['bool']['must'][] = ['term' => ["cases_name" => $case]];
					$params['body']["sort"] = ["_doc"];
					$docs = $this->elasticClient->search($params);  // The response will contain the first batch of results and a _scroll_id

					$this->update_requests($docs['hits']['hits'], $case, true);

					$scroll_id = $docs['_scroll_id'];

					// Now we loop until the scroll "cursors" are exhausted
					while (\true) {

						// Execute a Scroll request
						$response = $this->elasticClient->scroll([
								"scroll_id" => $scroll_id,  //using our previously obtained _scroll_id
								"scroll" => "1m"          // and the same timeout window
							]
						);

						// Check to see if we got any search hits from the scroll
						if (count($response['hits']['hits']) > 0) {
							$this->update_requests($response['hits']['hits'], $case, true);
							// Get new scroll_id
							$scroll_id = $response['_scroll_id'];
						} else {
							// No results, scroll cursor is empty.  We've exported all the data
							break;
						}
					}

					logger('delete old case: ' . $case);

				}

			}

			// Add new flags if method = add or update
			if ($method != 'delete') {
				if ($cases_name == 'all')
					$cases = $this->get_case_data('all');
				else
					$cases = [$this->get_case_data($cases_name[0])];

				foreach ($cases as $case) {
//					register_shutdown_function([$this, 'remove_update_flag'],$case['case_name'],$method, $range);

					$params['body'] = [];
					foreach ($case['details'] as $condition) {

						if (!$condition['negate'])
							$appear = 'must';
						else
							$appear = 'must_not';

						$query_string='';
						switch ($condition['type']) {
							case "application":
								$term = "host";
								break;
							case "country":
								$term = "country_code";
								break;
							case "IP":
								$term = "ip_orig";
								$conditions=explode(',',$condition['value']);
								foreach ($conditions as $cond){
									if ($query_string!=''){
										$query_string.=' OR ';
									}
									if (strpos($cond,'-')){
										$query_string.='['.str_replace('-',' TO ',$cond).']';
									}
									else{
										$query_string.=$cond;
									}
								}
								break;
							case "rules":
								$term = "alerts.name";
								// the case.name contain only the rule name, so we need to delete the category rule name from the value string
								$query_string='"'.preg_replace('/,[\s\S]+?::/', '" OR "', substr($condition['value'], strpos($condition['value'], "::") + 2)).'"';
								break;
							case "parameter":
								$term = "parameters.name";
								break;
//						case "time":
//							$term= "ts";
//							break;
						}
						if($query_string==''){
							$query_string=str_replace(',', ' OR ', $condition['value']);
						}
						// The query to find the requests that match the case details
						$params['body']['query']['bool'][$appear][] = ['query_string' => ["default_field" => $term, "query" => $query_string ]];

					}

					// If the request has already this case name, we don't need to flag it
					$params['body']['query']['bool']['must_not'][] = ["term" => ["cases_name" => $case['case_name']]];
					$params['body']["sort"] = ["_doc"];

					// If it's a script that always run, we have to query only the latest requests
					if ($range && $last_update = $this->M_Config->get_key('last_case_update_id'))
						$params['body']['query']['bool']['must'][] = ['range' => ['ts' => ['gt' => $last_update]]];

					$docs = $this->elasticClient->search($params);  // The response will contain the first batch of results and a _scroll_id

					$this->update_requests($docs['hits']['hits'], $case['case_name'], false);


					$scroll_id = $docs['_scroll_id'];

					// Now we loop until the scroll "cursors" are exhausted
					while (\true) {

						// Execute a Scroll request
						$response = $this->elasticClient->scroll([
								"scroll_id" => $scroll_id,  // using our previously obtained _scroll_id
								"scroll" => "1m"         // and the same timeout window
							]
						);

						// Check to see if we got any search hits from the scroll
						if (count($response['hits']['hits']) > 0) {
							$this->update_requests($response['hits']['hits'], $case['case_name'], false);
							// Get new scroll_id
							$scroll_id = $response['_scroll_id'];
						} else {
							// No results, scroll cursor is empty.  We've exported all the data
							break;
						}
					}

					logger('Finish to '.$method . ' case: ' . $case['case_name']);

				}
			}

			$this->elasticClient->indices()->refresh(array('index' => $index_name));

		}

		if ($range) {
			$this->M_Config->update('last_case_update_id',$update_time,true);
			logger('Update the time to: '. $update_time);
			return;
		}


//		return_success();
	}


	// if it's not the script and we added or updated a case, we need to inform the user that the updating process is finish
	public function remove_update_flag($case_name)
	{
			$this->update($case_name, false, false);

	}


	/**
	 * @param $results
	 * @param $case_name
	 * @param $delete boolean - if true we delete the case, if false we add the case
	 */
	public function update_requests($results, $case_name, $delete)
	{

		foreach ($results as $result) {

			if (isset ($result['_source']['cases_name']) && !empty($result['_source']['cases_name']))
				$db_case_name = $result['_source']['cases_name'];
			else
				$db_case_name = [];

//			if (isset ($result['_source']['cases_count']) && !empty($result['_source']['cases_count']))
//				$db_case_count = $result['_source']['cases_count'];
//			else
//				$db_case_count = 0;

			$db_case_count=sizeof($db_case_name);

			// check if case_name already exists before adding it
			if (!$delete && !in_array($case_name, $db_case_name)) {
				array_push($db_case_name, $case_name);
				$db_case_count++;
			} // check that case_name exists before delete it
			elseif ($delete && in_array($case_name, $db_case_name)) {
				$db_case_name = array_diff($db_case_name, [$case_name]);
				$db_case_count--;
			}


			$params = [
				'index' => $result['_index'],
				'type' => 'http',
				'id' => $result['_id'],
				'body' => [
					'doc' => [
						'cases_name' => $db_case_name,
						'cases_count' => $db_case_count
					]
				]
			];

			$this->elasticClient->update($params);
		}
	}

	public function new_get_case_sessions($limit = 100, $range = array(), $apps, $case_data){
		$sort      = 'date';
		$sortorder = 'desc';

		switch($sort) {
			case 'date':
				$sortfield = 'date';
				break;
			case 'count':
				$sortfield = 'score';
				break;
			case 'type':
				$sortfield = 'alerts_count';
				break;

		}

		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = [
			'size' => 0,
			"aggs" => [
				"sid" => [

					"terms" => [ "field" => "sid", "size" => 100, "order" => [ $sortfield => $sortorder ] ],
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
						"score" => [
							"avg" => [ "field" => "alerts.score" ]
						],
						"alerts_names" => [
							"terms" => [ "field" => "alerts.name", "size" => 100 ]
						],
						"date" => [
							"min" => [ "field" => "ts" ]
						],
					],

				],
				"sid_count" => [
					"cardinality" => [ "field" => "sid", "precision_threshold" => 200 ],
				]
			]
		];

		foreach ($case_data['details'] as $condition){

				if ($condition['negate']=="false")
					$appear='must';
				else
					$appear='must_not';

				switch ($condition['type']){
					case "application":
						$term="http.host";
						break;
					case "country":
						$term="http.country_code";
						break;
					case "IP":
						$term="http.ip_orig";
						break;
				}
			$params['body']['query']['bool'][$appear][] = [ 'term' => [$term  => $condition['value'] ] ];

		}

		//$params['body']['query']['bool']['must'][] = [ 'term' => [ "http.cases_name" => $cid ] ];

		if(!empty($range)) {
			$params['body']['query']['bool']['must'][] = [ 'range' => [ 'ts' => [ 'gte' => intval($range['start']), 'lte' => intval($range['end']) ] ] ];
		}

		$params = append_application_query($params, $apps);
		$params = append_access_query($params);
		$result = $this->elasticClient->search($params);

		$results = array('items' => array());

		if(isset($result["aggregations"]) &&
			isset($result["aggregations"]["sid"]) &&
			isset($result["aggregations"]["sid"]["buckets"]) &&
			!empty($result["aggregations"]["sid"]["buckets"])) {

			$sid_buckets = $result["aggregations"]["sid"]["buckets"];
			foreach($sid_buckets as $sid) {

				$results['items'][] = array(
					"sid"     => $sid['key'],
					"city"    => $sid['city']['buckets'][0]['key'],
					"alerts_count"  => $sid['alerts_count']['value'],
					"alerts_names"  => $sid['alerts_names']['buckets'],
					"country" => strtoupper($sid['country_code']['buckets'][0]['key']),
					"ip_orig" => long2ip($sid['ip_orig']['buckets'][0]['key']),
					"host"    => $sid['host']['buckets'],
					"count"   => $sid['doc_count'],
					"score"  => $sid['score']['value'],
					"date"  => $sid['date']['value']
				);
			}
		}

		$results['success'] = true;
		$results['query']   = $params;
		$results['count']   = intval($result["aggregations"]['sid_count']['value']);
		return $results;
	}
	
	public function get_case_sessions_chart($range, $apps, $cid = false) {
		
		$dots  = 20;
		$step  = ($range['end'] - $range['start']) / $dots;

		$chart = array();
		
		for($x = 0; $x < $dots; $x++) {
			
			// LIM
			
			$scope_start = $range['start'] + ($x * $step);       // 4JS
			$scope_end   = $range['start'] + (($x + 1) * $step); // 4JS
				
			// QUERY

			$params['index'] = 'telepath-20*';
			$params['type'] = 'http';
			$params['body'] = array(
				'size'  => 0,
				'aggs'  => array(
					'sid' => array(
						"cardinality" => [
							"field" => "sid",
						]
					)
				),
				'query' => [
					'bool' => [
						'must' => [
							[ 'exists' => [ 'field' => 'cases_name' ] ]
						]
					]
				]
			);
			
			$params['body']['query']['bool']['must'][] = [ 'term' => [ "cases_name" => $cid ] ];
			$params['body']['query']['bool']['must'][] = [ 'range' => [ 'ts' => [ 'gte' => $scope_start, 'lte' => $scope_end ] ] ];
			
			
			$params = append_application_query($params, $apps);
			$params = append_access_query($params);
			
			$results   = $this->elasticClient->search($params);
			if(!empty($results) && isset($results['aggregations']['sid'])) {
				$val     = intval($results['aggregations']['sid']['value']);
				$chart[] = array($scope_end * 1000, $val);
			}
							
				
				
		}
		//print_r($params);
		//print_r($chart);
		
		return $chart;
		
	}
		
	
	public function old_delete($cids) {
		

		
		// Generic cases query
		$params = [];
		$params['body'] = [
			'size' => 100,
			'query' => [ 'term' => [ '_id' => 'cases_id' ] ]
		];
		
		// Gather data
		$result = get_elastic_results($this->elasticClient->search($params));
		
		// No data to delete from?
		if(empty($result) || !isset($result[0]['All_Cases'])) {
			return;
		}
		
		// Collect new case data, excluding deleted cases
		$new_data = array();
		foreach($result[0]['All_Cases'] as $case) {
			$found = false;
			foreach($cids as $cid) {
				if($case['case_name'] == $cid) {
					$found = true;
				}
			}
			if(!$found) {
				$new_data[] = $case;
			}
		}
		
		// Update our document
		$action_data = [
		   'index'       => 'telepath-config',
		   'type'        => 'cases',
		   'id'          => 'cases_id',
		];

		$action_data['body'] = array('All_Cases' => $new_data);
		$this->elasticClient->index($action_data);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-config'));


		
	}

	/**
	 * @param array $cids  cases ids
	 */
	public function delete($cids)
	{

		foreach ($cids as $cid) {
			$params = [
				'index' => 'telepath-cases',
				'type' => 'case',
				'id' => $cid
			];
			if ($this->elasticClient->exists($params)) {
				$this->elasticClient->delete($params);
			}
		}

		$this->elasticClient->indices()->refresh(array('index' => 'telepath-cases'));
	}
	
	public function old_create($name, $details) {
		
		$params = [];
		$params['body'] = [
			'size' => 100,
			'query' => [ 'term' => [ '_id' => 'cases_id' ] ]
		];

		$result = get_elastic_results($this->elasticClient->search($params));
		
		if(empty($result)) {
			die;
		}
		
		$action_data = [
		   'index'       => 'telepath-config',
		   'type'        => 'cases',
		   'id'          => 'cases_id',
		];

		$result[0]['All_Cases'][] = array('case_name' => $name, 'created' => time(), 'details' => $details, 'updating'=>true);
		
		$action_data['body'] = array('All_Cases' => $result[0]['All_Cases']);

		// Delete previous case data
		$deleteParams = array();
		$deleteParams['index'] = 'telepath-config';
		$deleteParams['type'] = 'cases';
		$deleteParams['id'] = 'cases_id';
		$retDelete = $this->elasticClient->delete($deleteParams);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-config'));

		// Index new data
		// Sample:: 
		// {"index":"telepath-config","type":"cases","id":"cases_id","body":{"All_Cases":[{"case_name":"Locals","details":[{"type":"country","negate":false,"value":"00"},{"type":"IP","negate":false,"value":"192.168.1.1-192.168.1.254"},{"type":"application","negate":true,"value":"www.hybridsec.com"}]},{"case_name":"Hackers","details":[{"type":"country","negate":false,"value":"CN"}]},{"case_name":"Americans","details":[{"type":"country","negate":false,"value":"US"}]},{"case_name":"Israelis","details":[{"type":"country","negate":false,"value":"IL"}]},{"case_name":"Trololo","created":1428320346,"details":[{"type":"rules","negate":false,"value":[{"type":"group","id":"Known Bad IP","category":"Hybrid"},{"type":"group","id":"ShellShock","category":"Hybrid"}]},{"type":"application","negate":false,"value":"ie.rempel.net"},{"type":"IP","negate":false,"value":"111.111.111.111,222.222.222.222,33.33.33.33-33.33.33.34"}]}]}}
		
		$this->elasticClient->index($action_data);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-config'));
		
	}

	/**
	 * @param string $name case name
	 * @param array $details details of the case
     */
	public function create($name, $details)
	{
		$params = [
			'index' => 'telepath-cases',
			'type' => 'case',
			'id' => $name,
			'body' => [
				'case_name' => $name,
				'created' => time(),
				'details' => $details,
				'updating' => true
			]
		];
		$this->elasticClient->index($params);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-cases'));
	}

	public function old_update($name, $data=false, $updating=true, $favorite=false) {
		
		$params = [];
		$params['body'] = [
			'size' => 100,
			'query' => [ 'term' => [ '_id' => 'cases_id' ] ]
		];
		
		$result = get_elastic_results($this->elasticClient->search($params));
		
		if(empty($result)) {
			die;
		}
		
		$action_data = [
		   'index'       => 'telepath-config',
		   'type'        => 'cases',
		   'id'          => 'cases_id',
		];



		foreach($result[0]['All_Cases'] as $key => $value) {
			if($result[0]['All_Cases'][$key]['case_name'] == $name) {
				if($data)
					$result[0]['All_Cases'][$key]['details'] = $data;
				// we need to flag the requests, the user has to know about this
				$result[0]['All_Cases'][$key]['updating']=$updating;
				$result[0]['All_Cases'][$key]['favorite']=$favorite;
			}
		}

		$action_data['body'] = array('All_Cases' => $result[0]['All_Cases']);
		
		$this->elasticClient->index($action_data);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-config'));
		
	}

	/**
	 * @param string $name name of the case
	 * @param bool $data the details of the case
	 * @param bool $updating flag to display to the user the update period
	 * @param bool $favorite favorite case, for dashboard display
     */
	public function update($name, $data = false, $updating = true, $favorite = false)
	{
		$params = [
			'index' => 'telepath-cases',
			'type' => 'case',
			'id' => $name,
			'body' => [
				'doc' => [
					'updating' => $updating,
					'favorite' => $favorite
				]
			]
		];
		if($data){
			$params['body']['doc']['details']=$data;
		}
		$this->elasticClient->update($params);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-cases'));
	}


}

?>
