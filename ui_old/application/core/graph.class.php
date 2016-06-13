<?php

// Workflow Graph Class

class Graph {

	public	$vertices    = array();
	public	$edges       = array();
	public  $frequencies = array();
	
	public function addNode($node) {
		
		$id = $node->id;

		if(isset($this->vertices[$id])) {
			return false;
		}

		$this->vertices[$id] = $node;
		
	}

	public function addEdge($src, $dst, $frequency) {
		
		if(!isset($this->vertices[$src->id])) {
			echo "addEdge:: source {$src->id} not in vertices";
			return false;
		}
		if(!isset($this->vertices[$dst->id])) {
			$this->addNode($dst);
		}
		
		if(!isset($this->edges[$src->id])) {
			$this->edges[$src->id] = array();
		}
		
		if(!isset($this->frequencies[$src->id])) {
			$this->frequencies[$src->id] = array();
		}

		// UPD EDGE
		$this->vertices[$src->id]->set_expanded(1);
		$this->edges[$src->id][] = $dst;
		
		// UPD FREQ
		$this->frequencies[$src->id][$dst->id] = $frequency;

	}
	
	public function getNode($id) {
		if(isset($this->vertices[$id])) {
			return $this->vertices[$id];
		} else {
			return false;
		}
	}

}

?>