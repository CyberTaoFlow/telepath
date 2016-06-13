<?php
	
	function get_smtp_config($ci_instance = false) {
		
		if(!$ci_instance) { $ci_instance = &get_instance(); }
		$ci_instance->load->model('ConfigModel');
		$config = $ci_instance->ConfigModel->get();

		return array(
		  'protocol' => 'smtp',
		  'smtp_host' => $config['smtp'],
		  'smtp_port' => $config['smtp_port'],
		  'smtp_user' => $config['rep_user'],
		  'smtp_pass' => $config['rep_pass'],
		  'mailtype' => 'html',
		  'charset' => 'iso-8859-1',
		  'wordwrap' => TRUE
		);

	}
	
	function telepath_log($class, $function, $context, $request) {
		
		// LOG Request
		$logfile = '/tmp/telepath_ui_log';
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
		$context->acl->init_current_acl();
		
		telepath_log($class, $function, $context, $_REQUEST);
		
		// Rewrites (caution)
		
		$rewrite_list = array(
			
			// TODO:: change function names and re-route
			array('from_class' => 'Workflow', 'from_function' => 'record', 'to_class' => 'Workflow', 'to_function' => 'add_record'),
			array('from_class' => 'Workflow', 'from_function' => 'group_delete', 'to_class' => 'Workflow', 'to_function' => 'del_group'),
			array('from_class' => 'Workflow', 'from_function' => 'group_save', 'to_class' => 'Workflow', 'to_function' => 'set_group'),
			array('from_class' => 'Workflow', 'from_function' => 'groups_get_general_cb', 'to_class' => 'Workflow', 'to_function' => 'get_groups'),
			
			// for simplicity:: Access to pages == Access to application
			array('from_class' => 'Pages', 'from_function' => 'set_page_alias', 'to_class' => 'Applications', 'to_function' => 'set_application'),
			
			// for simplicity:: Access to parameters == Access to application
			array('from_class' => 'Parameters', 'from_function' => 'get_global_headers', 'to_class' => 'Dashboard', 'to_function' => 'get_dashboard'),
			array('from_class' => 'Parameters', 'from_function' => 'set_parameter_alias', 'to_class' => 'Applications', 'to_function' => 'set_application'),
			array('from_class' => 'Parameters', 'from_function' => 'set_parameter_config', 'to_class' => 'Applications', 'to_function' => 'set_application'),
			
			// Engine Status/Start/Stop == Get / Set Telepath
			array('from_class' => 'Telepath', 'from_function' => 'status', 'to_class' => 'Telepath', 'to_function' => 'get_telepath'),
			array('from_class' => 'Telepath', 'from_function' => 'start', 'to_class' => 'Telepath', 'to_function' => 'set_telepath'),
			array('from_class' => 'Telepath', 'from_function' => 'stop', 'to_class' => 'Telepath', 'to_function' => 'set_telepath'),
			
			// Basic user functions for viewing and updating self user info == Get Telepath
			array('from_class' => 'Users', 'from_function' => 'get_self', 'to_class' => 'Telepath', 'to_function' => 'get_telepath'),
			array('from_class' => 'Users', 'from_function' => 'set_self', 'to_class' => 'Telepath', 'to_function' => 'get_telepath'),
			
		);
		
		foreach($rewrite_list as $rewrite_item) {
			if($class == $rewrite_item['from_class'] && $function == $rewrite_item['from_function']) {
				$class    = $rewrite_item['to_class'];
				$function = $rewrite_item['to_function'];
			}
		}
		
		// END Rewrites
		
		// Validate
		
		if($class == 'Telepath' && $function == 'index') {

			// telepath_get:: validation is within index.php view file
		
		} else {
		
			// Passthru if Admin
			
			if(!$context->ion_auth->is_admin()) {
				
				// Fail if not logged in.
				if(!$context->ion_auth->logged_in()) {
					return_json(array('success' => false, 'logout' => true, 'error' => 'Not logged in.'));
				}
				
				// Initialize access list.
				$context->acl->init_current_acl();
				
				// Assume fail.
				$allowed = false;
				
				// Convention for CRUD access model
				$function_split = explode('_', $function);
				$short_array    = array('get', 'set', 'del', 'add');
				
				if(in_array($function_split[0], $short_array)) {
					$allowed = $context->acl->has_perm($class, $function_split[0]); // Check CRUD
				} else {
					$allowed = $context->acl->has_perm($class, $function); // Check Full Notation
				}
				
				// Fail
				if(!$allowed) {
					return_json(array('success' => false, 'debug' => 'Auth declined access for ' . $class . ' function ' . $function));
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
		return_json(array('success' => true, 'items' => $items, 'total' => count($items)));
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
	
?>