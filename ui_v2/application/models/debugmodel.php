<?php

class DebugModel extends CI_Model {
	
	function __construct()
	{
		parent::__construct();
	}
	
	function debug() {
		$this->debug = $this->load->database('test', TRUE);
		return $this->debug->get('pages')->result();
	}
		
}

?>