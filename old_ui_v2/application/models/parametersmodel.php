<?php

class ParametersModel extends CI_Model {
	
	private $tableName = 'attributes';
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function att_update($att_id, $data) {
		
		$check = $this->att_get($att_id);
		if(!empty($check)) {
			$this->db->where('att_id', $att_id);
			$this->db->update($this->tableName, $data); 
		} else {
			return_fail('Cound not access parameter');
		}
				
	}
	
	public function att_get($att_id) {
	
		$this->db->select('attributes.*');

		if(!$this->acl->all_apps()) {
			$this->db->join('pages', $this->tableName . '.page_id=pages.page_id');
			$this->db->where_in('pages.app_id', $this->acl->allowed_apps);
		}
		$this->db->where('att_id', $att_id);
		$this->db->from($this->tableName);
		$query = $this->db->get();
		
		return $query->result();
		
	}
	
	public function search_parameter($app_id, $param) {
		
		$this->db->select('app_id')
			 ->from('pages')
			 ->where('path REGEXP', "^/[a-zA-Z0-9_.,-/]*" . $param . "[a-zA-Z0-9_.-]*$");
		
		$this->db->distinct();
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$query = $this->db->get();
		$result = $query->result();
		
		if(isset($result[0])) {
			$app_ids = array();
			foreach($result as $row) {
			
				$app_ids[] = $row->app_id;
				
				$this->db->select('app_id, app_domain')
						 ->from('applications')
						 ->where_in('app_id', $app_ids);
				
				$query = $this->db->get();
				$result = $query->result();
				
				return $result;
							
			}
		} else {
			return array();
		}

	}
	

		
}

?>