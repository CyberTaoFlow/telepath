<?php

// Functions for the Workflow
	
function arr_check_ext($arr, $ext) {
	foreach($arr as $x) {
		if($x['ext'] == $ext) {
			return true;
		}
	}
	return false;
}

function BFS($src,$graph,$depth_allowed) {

	$src->color = 'GRAY';
	$src->dist_from_root = 0;
	$queue = array();
	array_unshift($queue, $src);

	while (sizeof($queue)) {
		$u = array_pop($queue);
		if(isset($graph->edges[$u->id])) {
			foreach ($graph->edges[$u->id] as $v) {
				if($v->color == 'WHITE') {

					$v->color = 'GRAY';
					$v->dist_from_root = $u->dist_from_root+1;
					$v->parent = $u;
					if($u->dist_from_root < $depth_allowed) {
						$u->set_expanded(1);
					} else {
						$u->set_expanded(0);
					}
					array_unshift($queue, $v);

				}
			}
		}
		$u->color = 'BLACK';
	}

}
	
function generate_graph_output($graph, $extensions, $ignore_depth = false, $depth = 0) {
		
		$extensions = explode(',', urldecode ($extensions));

		// Collect Nodes
		$ans = array(
			'edges' => array(), 
			'nodes' => array(), 
			'extensions' => array(array('ext' => 'Show All'))
		);

		foreach($graph->vertices as $node) {
			if($ignore_depth || $node->dist_from_root <= $depth || $node->dist_from_root == PHP_INT_MAX) {
				if($node->duplicate) {
					continue;
				}
				// Get extention
				$ext_split = explode('.',$node->name);
				$ext = count($ext_split) > 1 ? end($ext_split) : "";
				
				if((in_array('Show All', $extensions) || in_array($ext, $extensions)) && $ext != 'js') {
					
					$ans['nodes'][$node->id] = $node->get_visual_interface();
					if($ext!="" && !arr_check_ext($ans['extensions'], $ext)) {
						$ans['extensions'][] = array('ext' => $ext);
					}
					if($ignore_depth || $node->dist_from_root < $depth || $node->dist_from_root == PHP_INT_MAX) {
						
						if(isset($graph->edges[$node->id])) {
						
							if(!isset($ans['edges'][$node->id])) {
								$ans['edges'][$node->id] = array();
							}
						
							foreach($graph->edges[$node->id] as $child) {
								if($child->duplicate) {
									$ans['edges'][$node->id][$node->id]  = array('directed' => 1, 'loop' => 1, 'frequency' => isset($graph->frequencies[$node->id][$node->id]) ? $graph->frequencies[$node->id][$node->id] : 0);
								} else {
									$ans['edges'][$node->id][$child->id] = array('directed' => 1, 'loop' => 0, 'frequency' => isset($graph->frequencies[$node->id][$child->id]) ? $graph->frequencies[$node->id][$child->id] : 0);
								}
							}
						}
						
					}
				}
			}
		}

		return $ans;
		
}

function validate_edge($ans, $id) {
	
	if(!isset($ans['edges'][$id])) {
		$ans['edges'][$id] = array();
	}
	
}

function type_from_schema($schema) {
	
	switch($schema) {
		
		case 'X':
			return 'xml';
		break;
		case 'J':
			return 'json';
		break;
		case 'P':
			return 'post';
		break;
		case 'G':
			return 'get';
		break;
		case 'H':
			return 'header';
		break;
		
	}
	
	return false;
}

?>