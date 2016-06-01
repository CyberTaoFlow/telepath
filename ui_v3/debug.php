<?php

include_once('debug.inc.php');
require 'vendor/autoload.php';
$client = new Elasticsearch\Client();


$db     = new ElasticTelepath($client);
$res = $client->indices()->getMapping();
echo '1';
$res = array_keys($res);
print_r($res);
die;

$bro = array();

foreach($res as $key) {
	$key = explode('-', $key);
	if(count($key) == 2 && $key[0] == 'bro') {
		$ts = date_create_from_format('YmdHi', intval($key[1]));
		$bro[$ts->getTimestamp()] = implode('-', $key);
	}
}
asort($bro);
print_r($bro);

$first = array_shift($bro);
print_r($first);

$query = array(); // Set size to 0 when need to aggregate
$db->apply_query_host($query, '*.ac.il', true);
print_r($query);

$query = array('body' => array(
                'query' => array(
                        'filtered' => array(
                                'query' => array(
                                        'bool' => array(
                                                'must_not' => array(
													array(
														'query_string' => array(
															"default_field" => "http.host",
															"query" => '*.ac.il'
														)
													)
                                                )
                                        )
                                ),
                                'filter' => array(),
                                'sort' => array()
                        )
                )
			));

foreach($bro as $index) {
	
	$query['index'] = $index;
	$r = $client->deleteByQuery($query);
	print_r($r);
	
}

die;


$query = array('size'  => 0); // Set size to 0 when need to aggregate
$db->apply_query_aggregate($query, 'id.orig_h', 100);
$db->apply_query_host($query, '*.ac.il');
$db->apply_query_range($query, 0, 999999999999999999999);
$res = $db->execute_query($query);

$ans = array();
if(!empty($res) && isset($res['aggregations'])) {
	foreach($res['aggregations']['id.orig_h']['buckets'] as $bucket) {
		$ans[$bucket['key']] = $bucket['doc_count'];
	}
}

print_r($ans);
die;

$query = array('size'  => 10);
$db->apply_query_host($query, '*.ac.il', true);
print_r($query);
print_r(json_encode($query));
$res = $db->execute_query($query);

print_r($res);

die;