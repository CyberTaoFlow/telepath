<?php

Class ElasticTelepath {

function __construct($client) {
	$this->ElasticClient = $client;
}

function execute_query($query) {
	return $this->ElasticClient->search(array('body' => $query));
}
function execute_delete_query($query) {
	return $this->ElasticClient->delete($query);
}

function validate_query(&$query) {
		
		if(!isset($query['query'])) 				    { $query['query'] = array(); }
//		if(!isset($query['filter'])) 				    { $query['filter'] = array(); }
		if(!isset($query['filter']['type'])) 			{ $query['filter']['type'] = array('value' => 'http'); }
		if(!isset($query['query']['bool'])) 		    { $query['query']['bool'] = array(); }
		if(!isset($query['query']['bool']['must']))     { $query['query']['bool']['must'] = array(); }
		if(!isset($query['query']['bool']['must_not'])) { $query['query']['bool']['must_not'] = array(); }
		
}

function apply_query_term(&$query, $key, $value, $not = false) {
	
	$this->validate_query($query);
	$query['query']['bool'][$not ? 'must_not' : 'must'][] = array(
		'term' => array(
			$key => $value,
		)
	);
	
}

function apply_query_range(&$query, $start, $end) {
	$this->validate_query($query);
	$query['query']['bool']['must'][] = array(
		'range' => array(
		  'ts' => array(
			'gte' => $start, //intval($range['start']) * 1000,
			'lte' => $end   // intval($range['end']) * 1000
		  )
		)
	);
}

function apply_query_host(&$query, $hostname, $not = false) {
	$this->validate_query($query);
	$query['query']['bool'][$not ? 'must_not' : 'must'][] = 
		array(
			'query_string' => array(
				"default_field" => "http.host",
				"query" => $hostname
			)
		);
}

function apply_query_aggregate(&$query, $field, $limit = 10) {
	if(!isset($query['aggs'])) { $query['aggs'] = array(); }
	$query['aggs'][$field] = array('terms' => array('field' => $field, 'size' => $limit));
}

}

error_reporting(E_ALL);
ini_set("display_errors", 1);

// SETUP APACHE TO OUTPUT DIRECTLY

// Turn off output buffering
ini_set('output_buffering', 'off');
// Turn off PHP output compression
ini_set('zlib.output_compression', false);
// Implicitly flush the buffer(s)
ini_set('implicit_flush', true);
ob_implicit_flush(true);
// Clear, and turn off output buffering
while (ob_get_level() > 0) {
	// Get the curent level
	$level = ob_get_level();
	// End the buffering
	ob_end_clean();
	// If the current level has not changed, abort
	if (ob_get_level() == $level) break;
}
// Disable apache output buffering/compression
if (function_exists('apache_setenv')) {
	apache_setenv('no-gzip', '1');
	apache_setenv('dont-vary', '1');
}

header("Content-Type: text/html");