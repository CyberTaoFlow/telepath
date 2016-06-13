<?php

// Workflow Node Class
class paramNode {
	
	public $id;
	public $name;
	public $type;
	public $has_icon	= 0;
	public $icon_src	= '';
	
	function __construct($id, $name, $value, $type) {
		
		$this->id     	 = $id;
		$this->name   	 = $name . ' = ' . $value;
		$this->type   	 = $type;
		
		if(in_array($this->type, array('json', 'xml', 'web_service', 'get', 'post', 'header'))) {
			$this->has_icon = 1;
			$this->icon_src = '../../images/' . $this->type . '.png';			
		}
		
	}
	
	public function get_visual_interface() {
	
		return array (
			'label' 		=> $this->name,
			'color' 		=> '#FFFFFF',
			'menu_visible'	=> 0,
			'url'  			=> $this->name,
			'id'  			=> $this->id,
		 	'type'  		=> $this->type,
			'hovered'  		=> 0,
			'selected' 	 	=> 0,
			'expandable' 	=> 0,
			'expanded' 		=> 1,
			'indices_in_group' => array(),
			'icon_src'  		=> $this->icon_src,
			'has_icon' 			=> $this->has_icon,
			'name_abbreviated' 	=> 0,
			'full_label' 		=> $this->name,
			'has_alias' 		=> 0
		);

	}

}

class Node {
	
	public $id;
	public $name;
	public $url;
	public $app_id;
	public $type;
	public $duplicate	= false;
	public $has_icon	= 0;
	public $icon_src	= '';
	public $has_alias	= 0;
	public $depth 		= 0;
	public $color 		= 'WHITE';
	public $discovered_time = 0;
	public $finished_time   = 0;
	public $time		= 0;
	public $parent		= NULL;
	public $is_root		= true;
	public $expanded	= 0;
	public $expandable	= 0;
	public $dist_from_root  = PHP_INT_MAX;

	public $json_encoded;
	
	function __construct($controller, $name,$id,$app_id,$type="page",$duplicate=false) {

		$wf_model = $controller;

		// Init Node PROP
		$this->id     	 = $id;
		$this->app_id 	 = $app_id;
		$this->name   	 = $name;
		$this->type   	 = $type; // page|json|xml|web_service
		$this->duplicate = $duplicate;

		// Sort out rest of the stuff..		

		// Set Name		
		
		$query = $wf_model->page_get_name($id);
		if($query && $query[0]->title != '' && $query[0]->title != ' ' && $query[0]->title != 'None') {
			$this->name = $query[0]->title;
		}
		
		// Get Page Frequency
		$this->frequency = $wf_model->page_get_freq($id);
				
		// Set URL	
		// TODO:: Do this in single query.. (appcount+appdomain)
		if(intval($app_id) == -1) {
			$this->url = "Application Deleted";
		} else {
			
			if($wf_model->app_get_count($app_id) != 1) {
				$this->url = "No Hostname found";				
			} else {
				$query     = $wf_model->app_get_domain($app_id);
				$this->url = 'http://' . $query[0]->app_domain . '/' . $this->name;
			}

		}
		
		// Check webservice
		if($wf_model->page_is_web_service($id)) {
			$this->type = 'web_service';
		}
		
			// Assign an icon to everything that isnt a page
		if(in_array($this->type, array('json', 'xml', 'web_service', 'get', 'post', 'header'))) {
			$this->has_icon = 1;
			$this->icon_src = '../../images/' . $this->type . '.png';			
		}
		
		// Check is Expandable
		switch($this->type) {
			
			case 'web_service':
				$this->expandable = 1;
			break;

			case 'page':
				$this->expandable = $wf_model->count_flow_diagram_page_from($id, $app_id) +
						    $wf_model->count_flow_diagram_json_page($id) > 0 ? 1 : 0;
			break;

			case 'json':
				$this->expandable = $wf_model->count_flow_diagram_json_j_from($id) > 0 ? 1 : 0;	
			break;

			default:
			case 'xml':
				$this->expandable = 0;
			break;

		}

	}

	/*
	public function get_visual_interface() {
		
		return array (
			'label' 		=> $this->name,
			'url'  			=> $this->url,
			'id'  			=> $this->id,
		 	'type'  		=> $this->type,
			'expandable' 	=> $this->expandable,
			'expanded' 		=> $this->expanded,
			'frequency'     => $this->frequency
		);

	}
	*/
	public function get_visual_interface() {
		
		return array (
			'label' 			=> $this->name,
			'color' 			=> '#FFFFFF',
			'menu_visible'		=> 0,
			'url'  				=> $this->url,
			'id'  				=> $this->id,
		 	'type'  			=> $this->type,
			'hovered'  			=> 0,
			'selected' 	 		=> 0,
			'expandable' 		=> $this->expandable,
			'expanded' 			=> $this->expanded,
			'indices_in_group' 	=> array(),
			'icon_src'  		=> $this->icon_src,
			'has_icon' 			=> $this->has_icon,
			'name_abbreviated' 	=> 0,
			'full_label' 		=> $this->name,
			'has_alias' 		=> $this->has_alias
		);

	}

	// TODO:: Drop this useless function
	public function set_expanded($mode) {
		$this->expanded = $mode;
	}

}

?>