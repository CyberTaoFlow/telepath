<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Maintenence extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
		
	}
	
	function index() {
		
		ini_set('display_errors',1);
		ini_set('display_startup_errors',1);
		error_reporting(-1);
		
		$config['hostname'] = "192.168.1.25";
		$config['username'] = "root";
		$config['password'] = "";
		$config['database'] = "telepath";
		$config['dbdriver'] = "mysqli";
		$config['dbprefix'] = "";
		$config['pconnect'] = FALSE;
		$config['db_debug'] = TRUE;

		$this->load->model('M_Maintenence', '', $config);
		$this->M_Maintenence->check();
			
	}
	
	
}
