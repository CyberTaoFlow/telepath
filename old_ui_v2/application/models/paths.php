<?php

class Paths extends CI_Model {
	
	private $tableName   	  = 'pages';
	private $field_page_id    = 'page_id';
	private $field_page_path  = 'path';
	private $field_page_title = 'title';
	private $delim   		  = '__TPFILES__';
	
	function __construct()
	{
		parent::__construct();
	}
	
	// Expand Application Directory::
	// Param:: 
	
	// - App_ID
	// - Level 0|1 (?)
	// - Folder Name (Start with dir .. )
	
	// Default Expand Application Root

	public function expand($subdomain_id, $folder_path = '/', $search_string = '', $offset = 0) {
		
		$level = count(explode('/', $folder_path));
		
		// Regex string
		if($level == 1) {
			$regexstring = '/[a-zA-Z0-9_\.,-\\//]*/[a-zA-Z0-9_\.,-\\//]*';
		}
		if($level > 1) {
			$regexstring = $folder_path . "[a-zA-Z0-9_\.,-\\//]*";
		}
		
		//$ans['debug'] = 'L: ' . $level . ' RGX: ' . $regexstring;
		
		$this->db->select($this->field_page_id . ',' . $this->field_page_path . ',' . $this->field_page_title); // app_id, path, title
		$this->db->from($this->tableName); // pages
		$this->db->where('subdomain_id', $subdomain_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		if($search_string != '') {
			$this->db->where($this->field_page_path . ' REGEXP', '^' . $regexstring);
			$this->db->where($this->field_page_path . ' LIKE', '%' . $search_string . '%');
		} else {
			$this->db->where($this->field_page_path . ' REGEXP', '^' . $regexstring);
		}
		
		// 512 dirs max -- TODO:: Show more
		$this->db->limit(999999, $offset);
		
		$query  = $this->db->get();

		$result = $query->result();
			
		return $this->parseResult($result, $level, $folder_path);
		
	}
	
	public function expandPages($subdomain_id, $folder_path = '/', $page_ids = array(), $offset = 0) {
		
		$level = count(explode('/', $folder_path));
		
		$this->db->select($this->field_page_id . ',' . $this->field_page_path . ',' . $this->field_page_title); // app_id, path, title
		$this->db->from($this->tableName); // pages
		$this->db->where('subdomain_id', $subdomain_id);
		
		if(!$this->acl->all_apps()) {
			$this->db->where_in('app_id', $this->acl->allowed_apps);
		}
		
		$this->db->where_in('page_id', $page_ids);
		
		$this->db->limit(999999, $offset);
		
		$query  = $this->db->get();
		
		$result = $query->result();
		
		return $this->parseResult($result, $level, $folder_path);
		
	}
	
	public function parseResult($result, $level, $folder_path) {
		
		$this->load->model('Params');
		
		// Create a multidimentional array of paths + files from query results
		$ans   = array();
		$array = array();
		foreach ($result as $row) {
		
		  $path = $row->path;
		  $path = trim($path, '/');
		  $list = explode('/', $path);
		  $n = count($list);
		  $arrayRef = &$array; // start from the root
		  for ($i = 0; $i < $n; $i++) {
			$key = $list[$i];
			
			if($i == $n - 1) {
				$arrayRef[$this->delim][] = array('path' => $row->path, 'page_id' => $row->page_id, 'title' => $row->title);
			} else {
				$arrayRef = &$arrayRef[$key]; // index into the next level
			}
			
		  }
		  
		}
		
		// TODO:: Set array pointer to current search dir, currently '/';
		if($level > 1) {
			$folder_route = explode('/', $folder_path);
			foreach($folder_route as $route) {
				if(isset($array[$route])) {
					$array = $array[$route];
				}
			}
		}
		
		foreach($array as $key => $value) {
			if($key == $this->delim) {
				foreach($value as $file_id => $file) {
					$fix_path = str_replace($folder_path, '', $file['path']);
					$fix_path = str_replace('/', '', $fix_path);
					
					$expandable = (count($this->Params->getParams($file['page_id'], '')) > 0);
					
					$ans[] = array('title' => $file['title'], 'id' => $file['page_id'], 'type' => 'page', 'text' => $fix_path, 'type_id' => 1, 'expandable' => $expandable);
				}
			} else {
				$ans[] = array('id' => $key, 'type' => 'directory', 'text' => $key, 'type_id' => 0);
			}
		}
	
		return $ans;
	
	}
	
	public function sortResult($ans) {

		// Array Multisort, first directories then files by text
		if(isset($ans[0])) {
		
			foreach ($ans as $key => $row) {
				$type[$key] = $row['type_id'];
				$text[$key] = $row['text'];
			}
		
			array_multisort($type, SORT_ASC, $text, SORT_ASC, $ans);
		
		}
		
		foreach($ans as $key => $row) {
			unset($ans[$key]['type_id']);
		}

		return $ans;
		
	}
		
	public function create($params) {
		
	}
	public function update($app_id, $params) {
	
	}

}

?>
