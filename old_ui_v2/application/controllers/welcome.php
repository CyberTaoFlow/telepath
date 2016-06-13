<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Welcome extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function index() {
		header('Location: index.html');
		exit;
	}
}