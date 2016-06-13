<?php

class LogmodeModel extends CI_Model {
	
	function __construct()
	{
		parent::__construct();
	}
	
	// Get all groups by APP ID
	function session_cleanup($session_id) {
	
		$this->db->where('user', $session_id);
		$this->db->delete('logmode_files_que'); 
		return array('ok' => 1);
		
	}
	
	// Get all groups by APP ID
	function session_file_status($session_id) {
	
		$this->db->select('processed');
		$this->db->where('user', $session_id);
		$this->db->from('logmode_files_que');
		
		$query = $this->db->get();
		
		return $query->result();
		
	}
	
	// Group Create
	function session_file_insert($session_id, $file, $logtype, $app_id) {
		
		//echo 'Updating Session :: ' . $session_id . ' :: File :: ' . $file . ' :: LOGTYPE :: ' . $logtype . ' :: APPID :: ' . $app_id;
		
		$file_data = array(
			'user' => $session_id,
			'file' => $file,
			'processed' => 0,
			'type' => $logtype,
			'app_id' => intval($app_id)
		);

		if($this->acl->all_apps() || in_array($app_id, $this->acl->allowed_apps)) {
			$this->db->insert('logmode_files_que', $file_data); 
			return $this->db->insert_id();
		}
		
		return false;
		
	}

}

?>