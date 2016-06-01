<?php

class M_Nginx extends CI_Model {
	
	function __construct()
	{
		parent::__construct();
	}

	function gen_config() {
		
		$certs_dir  = '/opt/telepath/openresty/nginx/certs/';
		$fields     = 'app_id,app_domain,app_ips,certificate,private_key,ssl_flag,ssl_server_port';
		$apps_dirty = $this->db->select($fields)->limit(4096)->from('applications')->get()->result_array();
		$apps_clean = array();
		
		// Wipe clean the current certificates directory
		array_map('unlink', glob($certs_dir . "*"));
		
		foreach($apps_dirty as $app) {
			
			// If our domain name is somehow empty, we cant proxy it
			if($app['app_domain'] == '') {
				continue;
			}
			
			// Cant proxy application without destination IP
			if($app['app_ips'] == '') {
				continue;
			}
			
			// Double Check, Cant proxy application without destination IP
			$app_ips = explode(',', $app['app_ips']);
			if(count($app_ips) == 0 || (count($app_ips) == 1 && $app_ips[0] == '')) {
				continue;
			}
			
			// Copy back to array just the clean valid values
			$app['app_ips'] = [];
			foreach($app_ips as $ip) {
				if(filter_var($ip, FILTER_VALIDATE_IP)) {
					$app['app_ips'][] = $ip;
				}
			}
			
			// Just to make sure..
			if(count($app['app_ips']) == 0) {
				continue;
			}

			// If its SSL but we have no certificates, we can't proxy it
			if(intval($app['ssl_flag']) == 1 && ($app['certificate'] == '' || $app['private_key'] == '')) {
				continue;
			}
			
			// If its SSL but we have no port, default to 443
			if(intval($app['ssl_flag']) == 1 && intval($app['ssl_server_port']) == 0) {
				$app['ssl_server_port'] = '443';
			}
			
			// Attempt decoding certificates and store them into a directory
			if(intval($app['ssl_flag']) == 1) {
				
				$certificate  = base64_decode($app['certificate'], true);
				$private_key  = base64_decode($app['private_key'], true);
				
				// If decode failed, we can't proxy it...
				if(!$certificate || !$private_key) {
					continue;
				}
				
				// Save our certificates as files
				$key_file  = $certs_dir . 'application_' . $app['app_id'] . '_certificate.crt';
				$cert_file = $certs_dir . 'application_' . $app['app_id'] . '_private_key.key';
				
				file_put_contents($key_file,  $certificate);
				file_put_contents($cert_file, $private_key);
				
				// Validate write, under no circumstance we want nginx crashing due to bad config
				if(!file_exists($key_file) || !file_exists($cert_file)) {
					continue;
				}
				
			}
			
			// Cleanup the data array
			unset($app['certificate']);
			unset($app['private_key']);
			
			// All checks out, append to clean list
			$apps_clean[] = $app;
			
		}
		
		return $this->load->view('nginx', array('apps' => $apps_clean, 'certs_dir' => $certs_dir), true);
		
	}
	
}

?>
