<?php

class M_Rules extends CI_Model {
	
	function __construct() {
		parent::__construct();
		
		// Connect elastic
		//$params = array('hosts' => array('127.0.0.1:9200'));
#$params['logging'] = true;
#$params['logPath'] = '/tmp/elasticsearch.log';
		$this->elasticClient = new Elasticsearch\Client();
	}
	
	public function del_rule($name, $category) {
		
		$ret = array();
	
		$params['index'] = 'telepath-rules';
		$params['type']  = 'rules';
		
		$params['body']['query']['bool']['must'][] = ['match' => ['name' => $name]];
		$params['body']['query']['bool']['must'][] = ['match' => ['category' => $category]];

		#$results = $this->elasticClient->deleteByQuery($params);
		delete_by_query($this->elasticClient, $params, 1);

		// Generic cases query
		$params = [];
		$params['index'] = 'telepath-config';
		$params['type']  = 'cases';
		$params['id'] = 'cases_id';

			// Gather data
		$results = $this->elasticClient->get($params);

		$result=$results['_source'];

			// No data to delete from?
			if(empty($result) || !isset($result['All_Cases'])) {
				return;
			}

			// Collect new case data, excluding deleted cases
			$param= $category . '::' . $name;
			$new_data = array();
			foreach($result['All_Cases'] as $cases) {
				$new_case = array('case_name'=>$cases['case_name'],'created'=>$cases['created']);
				$found = false;
				foreach ($cases['details'] as $case) {
					if ($case['value'] == $param ) {
						continue;
					}
					elseif (substr_count($case['value'],  $param)  ) {
						$new_value = '';
						$explode = explode(',', $case['value']);
						foreach ($explode as $c) {
							if ($c != $param ) {
								$new_value == '' ? $new_value = $new_value . $c : $new_value = $new_value . ',' . $c;
							}
						}
						$case['value'] = $new_value;
					}
						$new_case['details'][] = $case;

					}
				if ($new_case['details']!=null){
					$new_data[]=$new_case;
				}
			}
			// Update our document

		$params['body'] = array('All_Cases' => $new_data);
			$this->elasticClient->index($params);
			$this->elasticClient->indices()->refresh(array('index' => 'telepath-config'));



		return $results;


	}
	
	public function add_rule($data) {
		
		foreach($data['criteria'] as $i => $val) {
			$data['criteria'][$i] = json_decode($val, true);
		}
		
		$params = ['body' => $data, 'index' => 'telepath-rules', 'type' => 'rules'];
		$this->elasticClient->index($params);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-rules'));
		return $data;
		
	}
	
	public function set_rule($data) {
		foreach($data['criteria'] as $i => $val) {
			$data['criteria'][$i] = json_decode($val, true);
		}

		$query = ['body' => [ 'query' => [ 'term' => [ '_id' => $data['uid'] ] ] ], 'index' => 'telepath-rules' ];
		$result = $this->elasticClient->search($query);
		if (isset($result['hits']['hits'][0]['_source']['builtin_rule']))
		{
			if ($result['hits']['hits'][0]['_source']['builtin_rule'])
			{
				$r = $result['hits']['hits'][0]['_source'];
				$data['builtin_rule'] = $r['builtin_rule'];
				// fix enable
				foreach ($r['criteria'] as $i => $val)
				{
					$enable = $val['enable'];
					foreach ($data['criteria'] as $i2 => $val2)
					{
						if ($val["kind"] == $val2["kind"])
						{
							$enable = $val2["enable"];
						}
					}
					$r['criteria'][$i]['enable'] = $enable;
				}
				$data['criteria'] = $r['criteria'];
				$data['name'] = $r['name'];
			}
		}
		// var_dump(json_encode($data));
		//return;
		#$res = $this->elasticClient->deleteByQuery($query);
                delete_by_query($this->elasticClient, $query );
		
		$params = ['body' => $data, 'index' => 'telepath-rules', 'type' => 'rules'];
		$this->elasticClient->index($params);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-rules'));


		$params = [];
		$params['index'] = 'telepath-config';
		$params['type']  = 'cases';
		$params['id'] = 'cases_id';

		$old= $data['category'] . '::' . $result['hits']['hits'][0]['_source']['name'];

		$new=$data['category'] . '::' . $data['name'];
		// Gather data
		$results = $this->elasticClient->get($params);

		$result=$results['_source'];
		// No data to delete from?
		if(empty($result) || !isset($result['All_Cases'])) {
			return;
		}

		// Collect new case data, excluding deleted cases

		$new_data = array();
		foreach($result['All_Cases'] as $cases) {
			$new_case = array('case_name'=>$cases['case_name'],'created'=>$cases['created']);
			$found = false;
			foreach ($cases['details'] as $case) {
				if ($case['value'] == $old ) {
					$case['value'] = $new;
				}
				elseif (substr_count($case['value'],  $old)  ) {
					$new_value = '';
					$explode = explode(',', $case['value']);
					foreach ($explode as $c) {
						if ($c != $old ) {
							$new_value == '' ? $new_value = $new_value . $c : $new_value = $new_value . ',' . $c;
						}
						else
						{
							$new_value == '' ? $new_value = $new_value . $new : $new_value = $new_value . ',' . $new;
						}
					}
					$case['value'] = $new_value;
				}
				$new_case['details'][] = $case;

			}
			if ($new_case['details']!=null){
				$new_data[]=$new_case;
			}
		}
		// Update our document

		$params['body'] = array('All_Cases' => $new_data);
		$this->elasticClient->index($params);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-config'));

		return $data;
	
	}
	
	public function get_rules($category = false) {
		
		$ret = array();
	
		$params['index'] = 'telepath-rules';
		$params['type']  = 'rules';
		$params['body']['size'] = 999;
		
		if($category) {
			$params['body']['query']['match']['category'] = $category;
		}

		$results   = get_elastic_results($this->elasticClient->search($params));
		usort($results, "cmp_name");
		return $results;
			
	}
	
	public function get_rule($name, $category=false) {
		
		$ret = array();
	
		$params['index'] = 'telepath-rules';
		$params['type']  = 'rules';
		
		$params['body']['query']['bool']['must'][] = ['match' => ['name' => $name]];
		if($category){
			$params['body']['query']['bool']['must'][] = ['match' => ['category' => $category]];
		}

		$results   = get_elastic_results($this->elasticClient->search($params));
		
		return $results;
	
	}
	
	public function add_category($cat) {
		
		$cats = $this->__get_categories();
		$cats[] = $cat;		
		$this->__set_categories($cats);
	
	}
	
	public function del_category($cat) {
		
		// Delete rules assosiated with the category
		$query = ['body' => [ 'query' => [ 'term' => [ 'category' => $cat ] ] ], 'index' => 'telepath-rules' ];
		#$this->elasticClient->deleteByQuery($query);
                delete_by_query($this->elasticClient, $query, 1);
		
		// Clear category from category index
		$old_cats = $this->__get_categories();
		$new_cats = [];
		
		foreach($old_cats as $old_cat) {
			if($cat != $old_cat) {
				$new_cats[] = $old_cat;
			}
		}
		
		$this->__set_categories($new_cats);

		$params = [];
		$params['index'] = 'telepath-config';
		$params['type']  = 'cases';
		$params['id'] = 'cases_id';

		// Gather data
		$results = $this->elasticClient->get($params);

		$result=$results['_source'];;

		// No data to delete from?
		if(empty($result) || !isset($result['All_Cases'])) {
			return;
		}

		// Collect new case data, excluding deleted cases
		$param= $cat;
		$new_data = array();
		foreach($result['All_Cases'] as $cases) {
			$new_case = array('case_name'=>$cases['case_name'],'created'=>$cases['created']);
			$found = false;
			foreach ($cases['details'] as $case) {
				if ((explode('::', $case['value'])[0]) == $param ) {
					continue;
				}
				elseif (substr_count($case['value'],  $param)  ) {
					$new_value = '';
					$explode = explode(',', $case['value']);
					foreach ($explode as $c) {
						$test=explode('::',$c);
						if ($test[0]!= $param) {
							$new_value == '' ? $new_value = $new_value . $c : $new_value = $new_value . ',' . $c;
						}
					}
					$case['value'] = $new_value;
				}
				$new_case['details'][] = $case;

			}
			if ($new_case['details']!=null){
				$new_data[]=$new_case;
			}
		}
		// Update our document

		$params['body'] = array('All_Cases' => $new_data);
		$this->elasticClient->index($params);
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-config'));



	}
	
	private function __get_categories() {
		
		$results   = $this->elasticClient->search(['body' => [ 'query' => [ 'term' => [ '_type' => 'rule_categories' ] ] ], 'index' => 'telepath-rules' ]);
		if(isset($results['hits']['hits'][0])) {
			return $results['hits']['hits'][0]['_source']['rule_categories'];	
		} else {
			return array('Hybrid', 'Brute-Force');
		}
		
	}
	
	private function __set_categories($array) {
			
		// Cleanup
		$query = ['body' => [ 'query' => [ 'term' => [ '_type' => 'rule_categories' ] ] ], 'index' => 'telepath-rules' ];
		#$this->elasticClient->deleteByQuery($query);
                delete_by_query($this->elasticClient, $query, 1);
		
		// Set
		$params = ['body' => [ 'rule_categories' => $array ], 'index' => 'telepath-rules', 'type' => 'rule_categories','id' => 'categories_id'];
		$this->elasticClient->index($params);		
		
		// Refresh
		$this->elasticClient->indices()->refresh(array('index' => 'telepath-rules'));
		
	}
	
	public function clear_rules() {
		
		#$query = ['body' => [ 'query' => [ 'term' => [ '_type' => 'rules' ] ] ], 'index' => 'telepath-rules' ];
		#$this->elasticClient->deleteByQuery($query);
		$deleteParams['index'] = 'telepath-rules';
		$deleteParams['type'] = 'rules';
		$this->elasticClient->delete($deleteParams);
	}
	
	public function set_rules($obj) {
		
		$this->clear_rules();
		$params = ['body' => $obj, 'index' => 'telepath-rules', 'type' => 'rules'];
		$this->elasticClient->index($params);
		
	}
	
	
	public function get_json_object() {
		return $this->get_rules();
	}
	public function set_json_object($obj) {
		file_set_contents('rules.json', json_encode($obj));
		return true;
	}
	
	// New methods
	public function get_categories() {
		return $this->__get_categories();
	}

	function searchRules($search){

		$params['index']='telepath-rules';
		$params['type']=['rules'];
		$params['_source_include'] = ['category','name'];
		$params['body'] = [
			'size' => 9999,
			'query' => ["bool" => ["must" => ["query_string" => ["fields" => ['category', 'name'], "query" => '*' . $search . '*', "lowercase_expanded_terms"=>false]]]],

		];

		$result= $this->elasticClient->search($params);

		return $result['hits']['hits'];
	}

	// Method declaration
	// TODO:: implement using json
	public function get_rule_groups($assoc = false) {}
	public function get_group_by_id($group_id) {}
	public function get_rules_by_group_id($group_id) {}
	public function get_rule_by_name($name) {}
	public function get_category_by_name($name) {}
	public function get_category_by_id($id) {}
	public function create_category($category_name) {}
	public function update_category($category_id, $category_name) {}
	public function find_group_by_name($name) {}
	public function create_rule($rule_data) {}
	public function create_group($name, $description, $category_id, $action_log, $score_numeric ) {}
	public function toggle_rules_by_group($group_id, $value) {}
	public function get_groups() {}
	public function add_group_param($group_id, $param_id) {}
	public function update_group($group_id, $group_data) {}
	public function update_rule($rule_id, $rule_data) {}
	public function delete_rule($rule_id) {}
	public function delete_group($group_id) {}
	public function delete_category($category_id) {}
	public function create_new_rule($group_id) {}
	
	
}

?>
