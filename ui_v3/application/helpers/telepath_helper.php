<?php
	
	function cmp_name($a, $b) {
		return strcasecmp (strtolower($a["name"]), strtolower($b["name"]));
	}
	
	function append_application_query($base, $apps) {
	
		if (empty($apps))
		{
			return $base;
		}
	
		if(count($apps) == 0) {
			return $base;
		}
		
		if(isset($base['body']) && isset($base['body']['query']) && isset($base['body']['query']['bool'])) {
			$base['body']['query']['bool']['must'][] = [ "terms" => [ "host" => $apps ] ];
		}
		
		return $base;
		
	}
	
	function append_access_query($base, $debug = false) {
		

	
		$context = &get_instance();

		// Admins == unlimited
		if($context->input->is_cli_request() || $context->ion_auth->is_admin() ) {
			return $base;
		}
		
		$apps   = $context->acl->allowed_apps;
		$ranges = $context->acl->allowed_ranges;
		
		// Not defined == unlimited
		if(empty($apps) && empty($ranges)) {
			return $base;
		}
				
		$limit_conditions = array();
		
		if(isset($base['body']) && isset($base['body']['query']) && isset($base['body']['query']['bool'])) {
				
			// Applications
			if(!empty($apps)) {
				
				foreach($apps as $app) {
					$limit_conditions[] = [ "term" => [ "host" =>  $app ] ];
				}
			
			}
			
			// Single or Range of IP
			if(!empty($ranges)) {
				
				foreach($ranges as $range) {
				
					if(strpos($range, '-') !== false) {
						
						// IP Range
						
						$range = explode('-', $range);
						$limit_conditions[] = [ "range" => [ "ip_resp" => [ "gte" => $range[0], "lte" => $range[1] ] ] ];
						
						
					
					} else {
						
						// Single address
						
						$limit_conditions[] = [ "term" => [ "ip_resp" => $range ] ];
											
					
					}
				
				}
			
			}
			
			if(!empty($limit_conditions)) {
				$base['body']['query']['bool']['should'] = $limit_conditions;
				$base['body']['query']['bool']['minimum_should_match'] = 1;
			}
			
			
			
		}
		return $base;
		
		
	}
	
	function get_elastic_results($results) {
		$result  = array();
		if(!empty($results) && isset($results['hits']) && isset($results['hits']['hits'])) {
			foreach($results['hits']['hits'] as $row) {
				$new_row  = $row['_source'];
				if(isset($row['_id'])) { $new_row['uid'] = $row['_id']; }
				if(isset($row['_score']) && !isset($new_row['score'])) { $new_row['score'] = $row['_score']; }
				if(isset($row['_index'])) { $new_row['index'] = $row['_index']; }
				if(isset($row['_type'])) { $new_row['type'] = $row['_type']; }
				$result[] = $new_row;
			}
		}		
		return $result;
	}

function delete_by_query($client, $params, $max = 0)
{
	#$results = $this->elasticClient->deleteByQuery($params);
	$results   = get_elastic_results($client->search($params));
	if (!$results)
	{
		return;
	}
	if (count($results) == 1 or $max == 1)
	{
                foreach ($results as $res){
                        $params=[];
                        $params['index'] = $res['index'];
                        $params['type'] = $res['type'];
                        $params['id']=$res['uid'];
                        $client->delete($params);
			return;
                }
		return;
        }
	foreach ($results as $res){
		$params=[];
		$params['index'] = $res['index'];
		$params['type'] = $res['type'];
		$params['id']=$res['uid'];
		$client->delete($params);
	}
}


function prepare_elastic_results($results) {
	$result  = array();
	if(!empty($results) && isset($results['hits']) && isset($results['hits']['hits'])) {
		foreach($results['hits']['hits'] as $row) {
			$result[] = $row['fields']['_src'][0];
		}
	}
	return $result;
}


// retrieve only the _source field
function get_app_source($results,$learning_so_far=false)
{
	$result = array();
	if (!empty($results) && isset($results['hits']) && isset($results['hits']['hits'])) {
		foreach ($results['hits']['hits'] as $row) {
			// if the string was found in subdomains, we set the open parameter to true, to display the subdomains in the GUI
			if (isset ($row['_source']['subdomains']) && !empty ($row['_source']['subdomains']) && isset ($row['highlight']['subdomains'])) {
				$row['_source']['open'] = true;
			}
			if($learning_so_far){
				$row['_source']['learning_so_far']=thousandsCurrencyFormat($row['_source']['learning_so_far']);
			}

			$result[] = $row['_source'];
		}
	}
	return $result;
}

// retrieve only the _source field
function get_source($results)
{
	$result = array();
	if (!empty($results) && isset($results['hits']) && isset($results['hits']['hits'])) {
		foreach ($results['hits']['hits'] as $row) {
			$result[] = $row['_source'];
		}
	}
	return $result;
}


function get_gap($range) {
	
		$difference	= $range['end'] - $range['start'];

		if ($difference>=0 & $difference<=60*60)
			return 'minutesToHour';
		else if ($difference>60*60 && $difference<=60*60*24)	
			return 'hoursToDay';
		else if ($difference>60*60*24 && $difference<=60*60*24*7)	
			return 'dayToWeek';
		else if($difference>60*60*24*7 && $difference<=60*60*24*30)
			return 'weekToMonth';
		else if($difference>60*60*24*30 && $difference<=60*60*24*365/2)
			return 'monthTo6months';
		else if($difference>60*60*24*30 && $difference<=60*60*24*365)
			return '6monthsToYear';
		else if ($difference>60*60*24*365)
			return 'aYearPlus';	
		
	}
	
	function get_smtp_config($ci_instance = false) {
		
		/*if(!$ci_instance) { $ci_instance = &get_instance(); }
		$ci_instance->load->model('ConfigModel');
		$config = $ci_instance->ConfigModel->get();

		return array(
		  'protocol' => 'smtp_ip_id',
		  'smtp_host' => $config['smtp_ip_id'],
		  'smtp_port_id' => $config['smtp_port_id'],
		  'smtp_user' => $config['rep_user_id'],
		  'smtp_pass' => $config['rep_pass_id'],
		  'mailtype' => 'html',
		  'charset' => 'iso-8859-1',
		  'wordwrap' => TRUE
		);*/
		
		return array();

	}
	
	function telepath_log($class, $function, $context, $request) {


        
		// LOG Request
		$logfile = $context->config->item('telepath_ui_log');
		$log     = array('timestamp' => time() ,
						 'userdata' => $context->session->all_userdata(), 
						 'class' => $class, 
						 'function' => $function, 
						 'request' => $request
						);
						
		file_put_contents($logfile, json_encode($log) . "\n" , FILE_APPEND);
	
	}
	
	function telepath_auth($class, $function, $context = false) {
		
		if(!$context) { $context = &get_instance(); }
		
		// Initialize access list.
		if(!$context->input->is_cli_request()){
		$context->acl->init_current_acl();
		
		telepath_log($class, $function, $context, $_REQUEST);
		
		// Rewrites (caution)
		
		$rewrite_list = array(
//		Dashboard

			// Engine Status/Start/Stop == Get / Set Telepath
			array('from_class' => 'Dashboard', 'from_function' => 'index', 'to_class' => 'Dashboard', 'to_function' => 'get_dashboard'),
			array('from_class' => 'Cases', 'from_function' => 'index', 'to_class' => 'Cases', 'to_function' => 'get_cases'),
			array('from_class' => 'Suspects', 'from_function' => 'index', 'to_class' => 'Suspects', 'to_function' => 'get_suspects'),
			array('from_class' => 'Alerts', 'from_function' => 'index', 'to_class' => 'Alerts', 'to_function' => 'get_alerts'),

			// Basic user functions for viewing and updating self user info == Get Telepath
			array('from_class' => 'Users', 'from_function' => 'get_self', 'to_class' => 'Telepath', 'to_function' => 'get_telepath'),
			array('from_class' => 'Users', 'from_function' => 'set_self', 'to_class' => 'Telepath', 'to_function' => 'get_telepath'),
			
		);

		if ($class=='Search'){
			$function='get';
		}
		
		foreach($rewrite_list as $rewrite_item) {
			if($class == $rewrite_item['from_class'] && $function == $rewrite_item['from_function']) {
				$class    = $rewrite_item['to_class'];
				$function = $rewrite_item['to_function'];
			}
		}

		}
		
		// END Rewrites
		
		// Validate
		
		if($class == 'Telepath' && $function == 'index') {

			// telepath_get:: validation is within index.php view file
		
		} else {
		
			// Passthru if Admin
			
			if(!$context->input->is_cli_request() && !$context->ion_auth->is_admin()) {
				
				// Fail if not logged in.
				if(!$context->ion_auth->logged_in()) {
					return_json(array('success' => false, 'logout' => true, 'error' => 'Not logged in.'));
				}
				
				// Initialize access list.
				$context->acl->init_current_acl();
				
				// Assume fail.
				$allowed = false;
				
				// Convention for CRUD access model (later shortened into get / set)
				$function_split = explode('_', $function);
				if($function_split[0] == 'del' || $function_split[0] == 'add') {
					$function_split[0] == 'set';
				}
				$short_array    = array('get', 'set', 'del', 'add');
				
				if(in_array($function_split[0], $short_array)) {
					$allowed = $context->acl->has_perm($class, $function_split[0]); // Check CRUD
				} else {
					$allowed = $context->acl->has_perm($class, $function); // Check Full Notation
				}
				
				// Fail
				if($allowed) {
					return true;
				}

				else{
					return_json(array('success' => false, 'error' =>'you d`ont have permission to see '.$class, 'debug' => 'Auth declined access for ' . $class . ' function ' . $function));
				}
			}

		
		}
		
	}
	
	function return_json($array) {
		
		//function jsonRemoveUnicodeSequences($struct) {
		//	return preg_replace("/\\\\u([a-f0-9]{4})/e", "iconv('UCS-4LE','UTF-8',pack('V', hexdec('U$1')))", json_encode($struct));
		//}
		
		header('Content-Type: application/json; charset=utf-8');
		echo json_encode($array);
		die;
		
	}
	
	function return_success($items = array()) {
		return_json(array('success' => true, 'items' => $items, 'total' => count($items), 'ip' => $_SERVER['REMOTE_ADDR']));
	}
	function return_fail($error_msg = '') {
		return_json(array('success' => false, 'items' => array(), 'total' => 0, 'error' => $error_msg));
	}

	function extract_page_name($name) {
			return $name == '/' ? '/' : end(explode('/',$name));
	}
	
	function bin2text($bin_str) {
	
		$text_str = '';
		$chars = EXPLODE("\n", CHUNK_SPLIT(STR_REPLACE("\n", '', $bin_str), 8));
		$_I = COUNT($chars);
		FOR($i = 0; $i < $_I; $text_str .= CHR(BINDEC($chars[$i])), $i);
		RETURN $text_str;
		
	}

    function logger($message, $file_path = false)
    {
        $context = &get_instance();
        if (!$context->input->is_cli_request())
            return;

        if ($file_path) {
            file_put_contents($file_path, '');
        }

        echo date('Y-m-d H:i') . ' ' . $message . "\n";
    }

	function thousandsCurrencyFormat($num)
	{
		$x = round($num);
		$x_number_format = number_format($x);
		$x_array = explode(',', $x_number_format);
		$x_parts = array('K', 'M', 'B', 'T');
		$x_count_parts = count($x_array) - 1;
		$x_display = $x_array[0];
		if($x_count_parts>0) {
			$x_display .= ((int)$x_array[1][0] !== 0 ? '.' . $x_array[1][0] : '');
			$x_display .= $x_parts[$x_count_parts - 1];
		}
		return $x_display;
	}


	
?>
