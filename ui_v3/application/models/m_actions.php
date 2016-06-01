<?php

class M_Actions extends CI_Model {
	
	function __construct() {
		parent::__construct();
		
		// Connect elastic
		//$params = array('hosts' => array('127.0.0.1:9200'));
		$this->elasticClient = new Elasticsearch\Client();
	}

	function get_actions($host) {

		$params['body'] = [
			'size'   => 999,
				'query' => [ "bool" => [ "must" => [
					[ 'term' => [ "domain" => $host ] ],
					[ 'term' => [ '_type' => 'actions' ] ]
				] ]	]
		];
		
		$results = $this->elasticClient->search($params);
		return $results['hits']['hits'];
	
	}

	function set_delete_action($uid){

		$deleteParams = array();
		$deleteParams['index'] = 'telepath-actions';
		$deleteParams['type'] = 'actions';
		$deleteParams['id'] = $uid;
		$retDelete = $this->client->delete($deleteParams);

		$this->client->indices()->refresh(array('index' => 'telepath-actions'));

	}

	function get_action_autocomplete($text){


		$params['index'] = 'telepath-actions';
		$params['type'] = 'actions';
		$params['body']['size'] = 999;
		$params['_source_include'] = ["application", "action_name"];
		$params['body'] = [
			'size' => 9999,
			'query' => ["bool" => ["must" => ["query_string" => ["fields" => ["application", "action_name"], "query" => '*' . $text . '*']]]],
		];

		$results = $this->client->search($params);

		$ans = [];
		if (!empty($results['hits']['hits'])) {
			foreach ($results['hits']['hits'] as $hit) {
				$fields = $hit['_source'];
				$ans[] = array('key' => $fields['application'] . ' :: ' . $fields['action_name'], 'raw' => $fields);
			}
		}
		return_success($ans);
	}

	function search_actions($text){


		$params['index'] = 'telepath-actions';
		$params['type'] = 'actions';
		$params['body']['size'] = 999;
		$params['sort'] = ['application'];
		$params['body'] = [
			'size' => 9999,
			'query' => ["bool" => ["must" => ["query_string" => ["fields" => ["action_name"], "query" => '*' . $text . '*',"lowercase_expanded_terms"=>false]]]],
		];

		$results = $this->elasticClient->search($params);

		$results=get_app_source($results);

//		$results = array_map(function($result) {
//			return array(
//				'host' => $result['application'],
//				'action_name' => $result['action_name']
//			);
//		}, $results);



		return $results;

	}

//	function get_app_with_actions($search){
//
//		$params['index'] = 'telepath-actions,telepath-applications';
//		$params['type'] = 'actions,application';
//		$params['body']['size'] = 9999;
//		$params['body'] = [
//			'partial_fields' => [
//				"_src" => [
//					"include" => ["application", "action_name", "host"]
//				],
//			],
//			'size' => 9999,
//			'query' => ["bool" => ["must" => ["query_string" => ["fields" => ["application", "action_name","host"], "query" => '*' . $search . '*']]]],
//		];
//
//		$result = $this->client->search($params);
//
//		return $result['hits']['hits'];
//	}

	function set_clear_actions(){

		$params['index'] = 'telepath-actions';
		$params['type'] = 'actions';
		$params['body']['query']['match']['domain'] = '192.168.1.111';
                delete_by_query($this->client, $params);
	}

	function get_app_actions($host){


		$params['index'] = 'telepath-actions';
		$params['type'] = 'actions';
		$params['body']['size'] = 999;
		$params['body']['query']['match']['application'] = $host;

		return get_elastic_results($this->client->search($params));

	}

	function set_flow($name,$app,$data){

		$new_json = array('action_name' => $name, 'application' => $app, 'business' => $data);
		// Make sure we have an index
		$indexParams['index'] = 'telepath-actions';
		// Create index if it does not exists only (Yuli)
		$settings = $this->client->indices()->getSettings($indexParams);
		if (!$settings) {
			$this->client->indices()->create($indexParams);
		}
		// Delete old
		$params['index'] = 'telepath-actions';
		$params['type'] = 'actions';
		$params['body']['query']['bool']['must'][] = ['term' => ['action_name' => $name]];
		$params['body']['query']['bool']['must'][] = ['term' => ['application' => $app]];
		#$res = $this->client->deleteByQuery($params);
                delete_by_query($this->client, $params);

		// Insert new
		$params = ['body' => $new_json, 'index' => 'telepath-actions', 'type' => 'actions'];
		$this->client->index($params);
		$this->client->indices()->refresh(array('index' => 'telepath-actions'));
	}

	function _hybridrecord_to_sid($value, $host){

		$scope = 300; // in last 5 minutes

		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = [
			'size' => 100,
			'query' => ['bool' =>
				['must' => [
					['term' => ['parameters.name' => 'hybridrecord']],
					['range' => ['ts' => ['gte' => intval(time() - $scope)]]]
				],
				]]
		];

		$results = array();
		$result = get_elastic_results($this->client->search($params));
		if (!empty($result)) {

			foreach ($result as $row) {
				if (!empty($row['parameters'])) {
					foreach ($row['parameters'] as $param) {
						// We got our session, return its SID and offset timestamp
						if ($param['name'] == 'hybridrecord' && $param['value'] == $value) {
							return_success(array('sid' => $row['sid'], 'ts' => $row['ts']));
						}
					}
				}
			}

		} else {
			// Return empty array to UI, nothing found (yet)
			return_success();
		}

		// Something went wrong
		return_success();
	}

	function get_requests($mode, $value, $host, $offset, $lockon)
	{

		telepath_auth(__CLASS__, 'get_action');

		// empty array (Yuli)
		$params = array();
		// Base query
		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = [
			'size' => 100,
			'sort' => [["ts" => ["order" => "desc"]]]
		];

		if ($offset) {
			$params['body']['query']['bool']['must'][] = ['range' => ['ts' => ['gte' => $offset]]];
		}
		if ($host) {
			$params['body']['query']['bool']['must'][] = ['term' => ['host' => $host]];
		}

		// sanity check (Yuli)
		if ($mode == 'IP') {
			// we need to check if IP hass correct format
			// we will silently ignore this request returning empty result
			if (filter_var($value, FILTER_VALIDATE_IP) === false) {
				$empty_result = array();
				return_success($empty_result);
			}
		}

		switch ($mode) {

			case 'IP':

				$params['body']['query']['bool']['must'][] = ['term' => ['ip_orig' => $value]];

				break;

			case 'URL':

				$value = $this->_hybridrecord_to_sid($value, $host);

			// No break, continue as SID
			// break;

			case 'SID':

				$params['body']['query']['bool']['must'][] = ['term' => ['sid' => $value]];

				break;
		}

		$results = array();
		$result = get_elastic_results($this->client->search($params));

		// Strip headers
		$clean = array();
		if (!empty($result)) {

			if ($lockon) {

				$max_ts = 0;
				foreach ($result as $request) {
					if (intval($request['ts']) > $max_ts) {
						$max_ts = $request['ts'];
					}
				}
				return_success(array('ts' => $max_ts));

			} else {

				foreach ($result as $request) {

					// Copy aside, remove, initialize blank
					$r_params = $request['parameters'];
					unset($request['parameters']);
					$request['parameters'] = array();

					// Append only params of GET/POST
					if (!empty($r_params)) {
						foreach ($r_params as $param) {
							if ($param['type'] == 'P' || $param['type'] == 'G') {
								$request['parameters'][] = $param;
							}
						}
					}

					$clean[] = $request;

				}

			}
		}

		return_success($clean);

	}

	function __get_active_sessions($host)
	{

		$scope = 300; // in last 5 minutes

		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = [
			'size' => 0,
			"aggs" => [
				"sid" => [
					"terms" => ["field" => "sid", "size" => 100, "order" => ['date' => 'desc']],
					"aggs" => [
						"date" => ["max" => ["field" => "ts"]],
						"ip_orig" => [
							"min" => ["field" => "ip_orig"]
						],
					],
				],
			],
			'query' => ['bool' =>
				['must' => [
					['term' => ['host' => $host]],
					['range' => ['ts' => ['gte' => intval(time() - $scope)]]]
				],
				]]
		];

		$results = array();
		$result = $this->client->search($params);

		if (isset($result["aggregations"]) &&
			isset($result["aggregations"]["sid"]) &&
			isset($result["aggregations"]["sid"]["buckets"]) &&
			!empty($result["aggregations"]["sid"]["buckets"])
		) {

			$sid_buckets = $result["aggregations"]["sid"]["buckets"];
			foreach ($sid_buckets as $sid) {
				$results[] = array("sid" => $sid['key'], "ts" => $sid['date']['value'], "ip_orig" => long2ip($sid['ip_orig']['value']));
			}
		}

		return $results;

	}

}

?>
