<?php
	
class RequestScores extends CI_Model {
	
	private $base_table_name = 'request_scores_merge';
		
	function __construct()
	{
		parent::__construct();
		require 'vendor/autoload.php';
		$this->elasticClient = new Elasticsearch\Client();
	}
	
	function get_by_uid($uid) {
		
		$params['body'] = array(
			'size'  => 1,
			'query' => array(
				'bool' => array(
					'must' => array(
						array('query_string' => array('default_field' => 'http.uid', 'query' => $uid)),
					)
				)
			)
		);
		
		$results = $this->elasticClient->search($params);
		$results = get_elastic_results($results);
		if(!empty($results)) {
			return $results[0];
		}
		return array();
	
	}
	
}
?>