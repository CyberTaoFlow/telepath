<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Debug extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function index() {
		
		telepath_auth(__CLASS__, __FUNCTION__);
		
		//$acl = $this->acl->get_short_acl();
		//print_r($acl);
		
	}

}