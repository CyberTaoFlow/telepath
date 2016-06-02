<?php
	
class M_Requests extends CI_Model {
	
	function __construct() {
		parent::__construct();
		require 'vendor/autoload.php';
		$this->elasticClient = new Elasticsearch\Client();
	}
	
	function get_by_id($id) {
	
		$params['body'] = [
			'size'  => 1,
			'query' => [
				"ids" => [
					"type" => "http",
					"values" => [ $id ]
				]
			]
		];
		
		$results = $this->elasticClient->search($params);
		$results = get_elastic_results($results);
		if(!empty($results)) {
			return $results[0];
		}
		return array();
	
	}
	
	function get_latest($desc = true) {
		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		
		$params['body'] = array(
			'size'  => 1,
			'query' => array(
				'bool' => array(
					'must' => array(
						array('query_string' => array('default_field' => '_type', 'query' => 'http')),
					)
				)
			),
			"sort" => [	[ "ts" => [ "order" => $desc ? "desc" : "asc" ] ] ]
		);
		
		$results = $this->elasticClient->search($params);
		return $results;
	
	}

	function get_similar($id) {
		
		$req = $this->get_by_id($id);
		if(empty($req)) { return array(); }

		/*
		$params['body'] = [
		//	"explain" => true,
			"size"    => 100,
			"query"   => 
		];
		*/
		$params['index'] = 'telepath-20*';
		$params['type'] = 'http';
		$params['body'] = [
			'size' => 5,
			'query' => [
				'bool' => [
					'must' => [
						[ "more_like_this" => [ "ids" => [ $id ], "min_term_freq" => 0, "max_query_terms" => 100, /* "minimum_should_match" => '50%' */ ] ]
					],
					'must_not' => [
						[ "term" => [ "ip_orig" => $req['ip_orig'] ] ]
					]
				]
			]/*,
			'aggs'=>[
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
			]*/
		];
		
		$results = $this->elasticClient->search($params);

		foreach ($results['hits']['hits'] as $key =>$value) {

			$params['body'] = [
				'size' => 1,
				'sort' => [
					'ts' => ['order' => 'desc']],
				'query' => [
					'filtered' => ['filter' => ['term' => ['sid' => $value['_source']['sid']]]]]];

			$results2 = get_elastic_results($this->elasticClient->search($params));

			$results['hits']['hits'][$key]['_source']['ip_score'] = $results2[0]['ip_score'];


		}

		return $results;
		
	}	
	
	function get_by_query($body) {
		
		
		
	}
	
}
?>
