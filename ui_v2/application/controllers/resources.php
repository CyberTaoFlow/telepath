<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Resources extends CI_Controller {
	
	private $app_config = '/app.js';
	
	function __construct()
	{
		parent::__construct();
	}
	
	function index() {
		
		header('Content-Type: application/javascript');
		
		$access = 'none';
		if($this->ion_auth->logged_in()) {
			$access = 'user';
		}
		if($this->ion_auth->is_admin()) {
			$access = 'admin';
		}
		
		switch($access) {
			
			case 'user':
				$this->_print_user_resources();
			break;
			case 'admin':
				$this->_print_user_resources();
				$this->_print_admin_resources();
			break;
			default:
			case 'none':
				// No data
			break;
		}
		
		// Handle ACCESS
		
		$panel_map = array(
			'dashboard' 	=> array('id' => 'dashboard', 	'title' => 'Dashboard',		   'ext' => 'MyApp.view.Dashboard', 'perm' => array('class' => 'Dashboard', 'function' => 'get')),
			'alerts' 		=> array('id' => 'alerts', 		'title' => 'Alerts', 		   'ext' => 'MyApp.view.Alerts', 'perm' => array('class' => 'Alerts', 'function' => 'get')),
			'investigate' 	=> array('id' => 'investigate', 'title' => 'Investigate', 	   'ext' => 'MyApp.view.Investigate', 'perm' => array('class' => 'Investigate', 'function' => 'get')),
			'workflow' 		=> array('id' => 'workflows', 	'title' => 'Business Actions', 'ext' => 'MyApp.view.Workflow', 'perm' => array('class' => 'Workflow', 'function' => 'get')),
			'behavior' 		=> array('id' => 'behavioral', 	'title' => 'Rules', 		   'ext' => 'MyApp.view.Behavior', 'perm' => array('class' => 'Rules', 'function' => 'get')),
			'configuration' => array('id' => 'config', 		'title' => 'Settings', 		   'ext' => 'MyApp.view.Configuration', 'perm' => array('class' => 'Config', 'function' => 'get')),
			'help' 			=> array('id' => 'help', 		'title' => 'Help', 			   'ext' => false, 'perm' => 'allow')
		);
		
		$panels_access = array();
		
		$is_admin = $this->ion_auth->is_admin();
		$perms    = array();
		
		if(!$is_admin) {
			$this->acl->init_current_acl();
			$perms = $this->acl->perms;
		}
				
		foreach($panel_map as $panel_key => $panel) {
			
			$allowed = false;
			
			$perm = $panel['perm'];
			unset($panel['perm']);
			
			if($is_admin) {
				$allowed = true;
			} else {
				if($perm == 'allow') {
					$allowed = true;
				} else {
					$allowed = $this->acl->has_perm($perm['class'], $perm['function']);
				}
			}
			
			if($allowed) {
				// First allowed panel is the active panel
				if(empty($panels_access)) {
					$panel['active'] = true;
				}
				$panels_access[$panel_key] = $panel;
				
			}
			
		}
	
		echo "\n";
		echo 'telepath.access = ' . json_encode(array('panels' => $panels_access, 'admin' => $is_admin, 'perm' => $perms)) . ';';
		echo "\n";

	}
	
	private function _print_user_resources() {
		
		// Main Project
		$base_path = realpath('.');
		$this->_parse_and_dump($base_path);

		// AppTree Project
		$base_path = realpath('.') . '/sencha/AppTree';
		$this->_parse_and_dump($base_path);
		
	}
	private function _print_admin_resources() {
		
		// Admin Project
		$base_path = realpath('.') . '/sencha/system';
		$this->_parse_and_dump($base_path);
		
	}
	
	// Utility Methods
	private function _parse_and_dump($path) {
	
		$app_data   = file_get_contents($path . $this->app_config);
		$app_json_object = $this->_decode_project_file($app_data);
		$this->_dump_project_files($app_json_object, $path);
		
	}

	private function _json_decode_nice($json, $assoc = FALSE) { 
	
		$json = str_replace(array("\n","\r"),"",$json); 
		$json = str_replace("'",'"',$json); 
		$json = preg_replace('/([{,]+)(\s*)([^"]+?)\s*:/','$1"$3":',$json); 
		return json_decode($json,$assoc); 
		
	}
	
	private function _decode_project_file($app_data) {
		
		$stores = 'stores: [';
		$views  = 'views: [';
		$suffix = ']';
		
		$stores_pos   = strpos($app_data, $stores);
		$suffix_1_pos = strpos($app_data, $suffix, $stores_pos);
		$stores_data  = substr($app_data, $stores_pos, $suffix_1_pos - $stores_pos + 1);
				
		$views_pos    = strpos($app_data, $views);
		$suffix_2_pos = strpos($app_data, $suffix, $views_pos);
		$views_data  = substr($app_data, $views_pos, $suffix_2_pos - $views_pos + 1);
		
		$app_json_string = '{' . $stores_data . ',' . $views_data . '}';
	
		return $this->_json_decode_nice($app_json_string,true);
		
	}
	
	private function _dump_project_files($app_json_object, $base_path) {
		
		foreach($app_json_object['stores'] as $file) {
			$fullfile =  $base_path . '/app/store/' . $file . '.js';
			if(is_file($fullfile)) {
				echo file_get_contents($fullfile);
			}
		}
		
		foreach($app_json_object['views'] as $file) {
			$fullfile = $base_path . '/app/view/' . $file . '.js';
			if(is_file($fullfile)) {
				echo file_get_contents($fullfile);
			}
		}
	
	}
	
	private function _debugger() {
		
		/*
		$f = scandir('/var/www/app/view/');
		$n = array();
		foreach($f as $i => $x) {
			if($x == '.' || $x == '..' || $x == '.svn') continue;
			$n[] = str_replace('.js', '', $x);
		}
		
		print_r($n);
		print_r(array_diff($n, $app_json_object['views']));
		print_r($app_json_object['stores']);
		die;
		*/
	
	}
	
}