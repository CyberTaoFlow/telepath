<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Workflow extends CI_Controller {
	
	public function __construct() {
            parent::__construct();
			
			// Node Class
			include_once('application/core/node.class.php');
			// Graph Class
			include_once('application/core/graph.class.php');
			// Helper Functions
			include_once('application/core/workflow.inc.php');
			// Init DB
			$this->load->database();
			// Init Model
			$this->load->model('WorkflowModel');
            $this->load->model('BusinessflowModel');
			
    }
	
	public function get_expand() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// Collect params
		$type 	 = $this->input->post('type', true);
		$id   	 = $this->input->post('id', true);
		
		switch($type) {
		
			case 'application':
				$this->_expand_workflow_app($id);
			break;
			case 'flow':
				$this->_expand_workflow_flow($id);
			break;
			
		}
		
	}
	
	
	// Workflow -- Expand APP
	private function _expand_workflow_app($app_id) {
	
		$flows   	= $this->BusinessflowModel->groups_by_app($app_id, true);
		$categories = $this->BusinessflowModel->categories_by_app($app_id, true);
		
		$ans    = array('success' => true, 'context' => 'workflow', 'app_id' => $app_id, 'flows' => array(), 'flow_categories' => array());
		
		foreach($categories as $category) {
			$ans['flow_categories'][] = array('name' => $category->name, 'id' => $category->id, 'type' => 'flow_category', 'expandable' => true);
		}
		
		foreach($flows as $flow) {
			
			$expandable = !(trim($flow->pages) == '');
			$ans['flows'][] = array('group' => $flow->group_name, 'id' => $flow->id, 'type' => 'flow', 'category_id' => $flow->category_id, 'expandable' => $expandable);
			
		}
		
		return_json($ans);
		
	}
	
	// Workflow -- Expand FLOW
	private function _expand_workflow_flow($group_id) {
		
		// Load Model
		$this->load->model('BusinessflowModel');
		$this->load->model('WorkflowModel');
		
		// Node Class
		include_once('application/core/node.class.php');
		// Graph Class
		include_once('application/core/graph.class.php');
		// Helper Functions
		include_once('application/core/workflow.inc.php');
		
		// Prepare return array
		$ans = array('success' => true, 'context' => 'workflow', 'success' => true, 'items' => array(), 'total' => 0);
		
		// Count pages for result
		$result_count = $this->BusinessflowModel->group_id_check($group_id);
		
		if($result_count > 0) {
		
			$results  = $this->BusinessflowModel->group_get_by_id($group_id);
			$ans['group_name'] = $results[0]->group_name;
			$ans['app_id']     = $results[0]->app_id;
			
			$pages = $results[0]->pages;
			if(trim($pages) == '' || trim($pages == '[]')) {
					// Nothing to do, no pages
			} else {
				// Collect pages data to graph
				if(substr($pages, 0, 1) == '[') {
				
					// NEW FORMAT
					$this->load->model('PagesModel');
					$this->load->model('Attributes');
					$pages = json_decode($pages);
					
					if($pages && is_array($pages) && !empty($pages)) {
						
						foreach($pages as $page) {
							
							$page_data = $this->PagesModel->page_get($page->page_id);
							if($page_data) {
							
								$page_name = $page_data->display_path;
								$page_name = $page_name != ' ' ? extract_page_name($page_name) : $page_id;
							
								$item      = array('page_name' => $page_name, 'page_id' => $page->page_id, 'alias' => $page_data->title, 'path' => $page_data->display_path);
								
								if(isset($page->params) && is_array($page->params) && !empty($page->params)) {
									$item['params'] = array();
									foreach($page->params as $param) {
										$attribute = $this->Attributes->get_att_by_id($param->att_id);
										if($attribute) {
											$item['params'][] = array('att_id' => $param->att_id, 'att_alias' => $attribute->att_alias, 'data' => $param->data, 'att_name' => $attribute->att_name, 'att_source' => $attribute->att_source);
										}
									}
								}
								
								$ans['items'][] = $item;
							
							}
							
						}
						
					}
				
				} else {
				
					// OLD FORMAT
					$pages = explode(',', $pages);
					foreach($pages as $page_id) {
					
						$ans['total']++;
						
						$this->load->model('PagesModel');
						$page_data = $this->PagesModel->page_get($page_id);
						
						$page_name = $page_data->display_path;
						$page_name = $page_name != ' ' ? extract_page_name($page_name) : $page_id;
				
						$ans['items'][] = array('page_name' => $page_name, 'page_id' => $page_id, 'alias' => $page_data->title, 'path' => $page_data->display_path);
						
					}
					
				}
				
			}
		}
		
		$ans['total'] = count($ans['items']);
		
		return_json($ans);
		
	}
	
	
	// Rename a category
	public function set_category_name() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// Params
		$category_id   = $this->input->post('category_id');
		$category_name = $this->input->post('category_name');
		
		// Rename
		$ok = $this->BusinessflowModel->category_rename($category_id, $category_name);
		
		// Return
		if($ok) {
			return_success(array('category_id' => $category_id, 'category_name' => $category_name));
		} else {
			return_fail('Category name already taken or DB error');
		}

	}
	
	// Create a category
	public function add_category() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// Params
		$application_id  = $this->input->post('app_id');
		$category_name  = $this->input->post('category_name');
		
		// Create
		$category_id = $this->BusinessflowModel->category_create($application_id, $category_name);
	
		// Return
		if($category_id) {
			return_success(array('category_id' => $category_id, 'category_name' => $category_name));
		} else {
			return_fail('Category name already taken or DB error');
		}
	
	}
	
	// Delete a category and all contained flows
	public function del_category() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// Param
		$category_id   = $this->input->post('category_id');
		
		// Delete
		$this->BusinessflowModel->category_delete($category_id);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(108, 1);
		
		// Return
		return_success(array('category_id' => $category_id));
		
	}
	
	// Add a flow to category
	public function set_category_flow() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		// Param
		$category_id = $this->input->post('category_id');
		$flow_id     = $this->input->post('flow_id');
		
		// Add flow to category
		$this->BusinessflowModel->category_add_flow($category_id, $flow_id);
		
		// Return
		return_success(array('category_id' => $category_id, 'flow_id' => $flow_id));
		
	}
	
	public function set_states() {
	
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$data = $this->input->post('states');
		if(!$data) {
			return_json(array('success' => false));
		}
		$this->load->model('ConfigModel');
		$states = $this->ConfigModel->get('workflow_states');
		if($states) {
			$this->ConfigModel->update('workflow_states', $data);
		} else {
			$this->ConfigModel->insert('workflow_states', $data);
		}
		return_json(array('success' => true));
		
	}
	
	public function get_states() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
	
		$this->load->model('ConfigModel');
		$states = $this->ConfigModel->get('workflow_states');
		if($states) {
			echo $states['workflow_states'];
		} else {
			echo '[]';
		}
	}

	public function record() {
	
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('RequestScores');
		$this->load->model('AttributeScores');
		
		$ans = array('nodes' => array());
		
		// Collect input
		$SID 	    = $this->input->post('sid', true);
		$IP 		= $this->input->post('ip', true);
		
		$start_time = $this->input->post('start_time', true);
		
		if($IP) {
			$pages = $this->RequestScores->get_pages_by_IP($IP, $start_time);
		} else {
			$pages = $this->RequestScores->get_pages_by_SID($SID, $start_time);
		}

		$last_node = false;
		
		foreach($pages as $page) {
			
			$page_id = $page->page_id;
			$RID	 = $page->RID;
			$date    = intval($page->date);
			$path    = $page->path;
			
			// Generate node
			$page_name = $this->WorkflowModel->page_get_display_name($page_id);
			$page_name = $page_name != ' ' ? extract_page_name($page_name) : $page_id; // Case of JSON
			
			// Attach parameters
			$params = $this->AttributeScores->get_params_by_RID($RID, true);
			
			foreach($params as $key => $value) {
				if($value->att_id == '18' || $value->att_name == 'hybridrecord') {
					unset($params[$key]);
				}
			}
			
			// Create TreeData
			$ans['nodes'][] = array('page_id' => $page_id, 'page_name' => $page_name, 'params' => $params, 'date' => $date, 'path' => $path, 'alias' => $page->title);

		}
		
		return_json($ans);
		
	}
	
	public function get_ip_by_att_value() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('AttributeScores');
		$this->load->model('RequestScores');
		
		$param_id  = 18; // Our magic "hybrid_record" param
		$att_value = $this->input->post('att_value', true);
		
		$RID = $this->AttributeScores->get_RID_by_param($param_id, $att_value);
		if($RID !== false) {
		
			$res = $this->RequestScores->get_IP_by_RID($RID);
			if(!$res) {
				return_json(array('success' => 'false', 'console' => 'no SID for request id ' . $RID ));
			}
			
			$IP = $res->user_ip;
			$time = $res->date;
			
			if($SID !== false) {
				return_json(array('success' => 'true', 'IP' => $IP, 'time' => $time));
			} else {
				return_json(array('success' => 'false'));
			}
		} else {
			return_json(array('success' => 'false'));
		}
		
	}
	
	public function get_sid_by_att_value() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('AttributeScores');
		$this->load->model('RequestScores');
		
		$param_id  = 18; // Our magic "hybrid_record" param
		$att_value = $this->input->post('att_value', true);
		
		$RID = $this->AttributeScores->get_RID_by_param($param_id, $att_value);
		if($RID !== false) {
		
			$res = $this->RequestScores->get_SID_by_RID($RID);
			if(!$res) {
				return_json(array('success' => 'false', 'console' => 'no SID for request id ' . $RID ));
			}
			
			$SID = $res->SID;
			$time = $res->date;
			
			if($SID !== false) {
				return_json(array('success' => 'true', 'SID' => $SID, 'time' => $time));
			} else {
				return_json(array('success' => 'false'));
			}
		} else {
			return_json(array('success' => 'false'));
		}
		
	}
	
	// Get all groups from all applications
	public function groups_get_general_cb() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('BusinessflowModel');
		$groups = $this->BusinessflowModel->groups_get_all();
		$ans	= array('success' => 'true', 'count' => count($groups), 'items' => array());
				
		foreach($groups as $group) {
			$ans['items'][] = array('group' => $group->group_name, 'id' => $group->id, 'app_id' => $group->app_id);
		}
		
		return_json($ans);
	
	}
	
	public function group_save() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$group_id  	 = $this->input->post('group_id', true);
		$group_name	 = $this->input->post('group_name', true);
		$json 		 = $this->input->post('json', true);
		
		$this->load->model('BusinessflowModel');
		
		$name_check = $this->BusinessflowModel->group_get_by_name($group_name, false, $group_id);
		$name_taken = isset($name_check[0]) ? true : false;
		
		if($name_taken) {
			return_json(array('success' => 'false', 'exists' => 'true'));
		}
		if(!$json) {
			$json = '[]';
		}
		
		// We need to keep the values html encoded for the engine
		$json = json_decode($json, true);
		foreach($json as $key => $value) {
			if(isset($value['params'])) {
				foreach($value['params'] as $p_key => $p_val) {
					if($p_val['data'] != '*') {
						$json[$key]['params'][$p_key]['data'] = urlencode($p_val['data']);
					}
				}
			}
		}
		$json = json_encode($json);		
		
		$group_id = $this->BusinessflowModel->group_update($group_name, $json, $group_id);
		
		// Update ATMS action
		$this->load->model('ConfigModel');
		$this->ConfigModel->update_action_code(108, 1);
		
		return_json(array('success' => 'true'));
		
	}
	
	public function group_delete() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$this->load->model('BusinessflowModel');
		
		$group_id = $this->input->post('group_id', true);
		if($group_id === false) {
			return_json(array('success' => 'false'));
		} else {
			$this->BusinessflowModel->group_delete($group_id);
			
			// Update ATMS action
			$this->load->model('ConfigModel');
			$this->ConfigModel->update_action_code(108, 1);
			
			return_json(array('success' => 'true'));

		}
		
	}
	
	public function group_create() {
		
		telepath_auth(__CLASS__, __FUNCTION__, $this);
		
		$app_id    	 = $this->input->post('app_id', true);
		$group_name	 = $this->input->post('group_name', true);
		$json 		 = $this->input->post('json', true);
		
		$this->load->model('BusinessflowModel');
		
		$name_check = $this->BusinessflowModel->group_get_by_name($group_name, $app_id);
		$name_taken = isset($name_check[0]) ? true : false;
		
		if($name_taken) {
			return_json(array('success' => 'false', 'exists' => 'true'));
		}
		if(!$json) {
			$json = '[]';
		}
		
		$group_id = $this->BusinessflowModel->group_create($group_name, $app_id, $json);
		if(!$group_id) {
			return_json(array('success' => 'false'));
		} else {
			
			// Update ATMS action
			$this->load->model('ConfigModel');
			$this->ConfigModel->update_action_code(108, 1);
			
			return_json(array('success' => true, 'group_id' => intval($group_id), 'group_name' => $group_name));
		}

	}
	
	public function search_page() {
		return_json(array());
	}

	public function rename_page() {
		return_json(array());
	}

	public function get_workflow_graph() {
		return_json(array());
	}

	public function expand_node() {
		return_json(array());
	}

	function get_json_by_page($ans, $root_node, $frequency) {
		return array();
	}

}
