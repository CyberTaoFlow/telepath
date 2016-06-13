<?php

	//error_reporting(E_ALL);
	//ini_set('display_errors', 1);

	function mc_store($key, $data) {

		$key = uniqid();
		$mc = new Memcached();
		$mc->addServer("localhost", 11211);
		$mc->setOption(Memcached::OPT_COMPRESSION, false);
		$mc->set($key, json_encode($data));
		echo $key . "\n";

	}


	function process_request($data) {

		parse_str($data, $req);

		$new = array();

		$packet = array();
		if(isset($req['urid'])) {

			$packet = array('request'  => array('headers' => array()),
						'response' => array('headers' => array())
			);

			foreach($req as $key => $value) {

				switch($key) {

					case 'm':
					case 'mt':

						$packet['request']['timestamp'] = intval($value);

					break;
					case 'rh':
						$rh = explode("\n", $value);
						array_shift($rh);
						foreach($rh as $header) {
							$header = explode(':', $header, 2);
							if(count($header) == 2) {
								$packet['request']['headers'][$header[0]] = trim($header[1]);
							}
						}
					break;
					case 'reph':
						$reph = explode("\n", $value);
						foreach($reph as $header) {
							$header = explode(':', $header, 2);
							if(count($header) == 2) {
								$packet['response']['headers'][$header[0]] = trim($header[1]);
							}
						}

					break;
					case 'ip':
						$packet['request']['ip'] = $value;
					break;
					case 'rm':
						$packet['request']['method'] = $value;
					break;
					case 'op':
						$packet['request']['url'] = $value;
					break;
					case 'oq':
						parse_str($value, $oq);
						$packet['request']['query'] = $oq;
					break;
					case 'oh':
						$packet['request']['domain'] = $value;
					break;
					case 'sc':
						$packet['response']['code'] = $value;
					break;
					case 'dlt':
						$packet['response']['time'] = $value;
					break;
					case 'urid':
						$packet['request']['uuid'] = $value;
						$uuid = $value;
					break;
				}

			}
			mc_store($uuid, $packet);

		}
	}

	$post_full  = file_get_contents("php://input");
	$post_lines = explode("\n", $post_full);

	foreach($post_lines as $data) {
		process_request($data);
	}
?>
