<?php

class M_Maintenence extends CI_Model {
	
	function __construct()
	{
		parent::__construct();
	}

	function log($str) {
		
		if($this->input->is_cli_request()) {
			$arr = json_decode($str, true);
			if($arr && isset($arr['msg'])) {
				if(isset($arr['index']) && isset($arr['total'])) {
					echo '(' . $arr['index'] . '/' . $arr['total'] . ') ' . $arr['msg'] . "\n";
				} else {
					echo $arr['msg'] . "\n";
				}
			}
		} else {
			file_put_contents($this->logfile, "\n" . $str, FILE_APPEND);
		}
	
	}
	
	function patch() {
		
		// Create all users from installer phase to CI users
		
		$new_users = $this->db->get('registered_users')->result();
		foreach($new_users as $user) {
			echo 'Creating ' . $user->user;
			$user_id = $this->ion_auth->register($user->user, $user->password, '', array(), array(1));
		}
		
		$this->db->from('registered_users')->where('1','1',false)->delete();
		
		$this->log('{"msg":"Done.", "stop": true }');
		
	}
	
	function work() {
		
		$tables = $this->db->list_tables();
		$mode   = 'install';
		
		// Check Installed
		if(!in_array('config', $tables)) {
		
			$this->log('{"msg":"Not yet installed, installing"}');
			
		} else {
			
			$this->db_version = $this->get_database_version();
			$this->log('{"msg":"Have telepath version ' . $this->db_version . ' installed, updating"}');
			$mode = 'update';
			
			//if(!$this->db_version) {
				// $this->log('{"msg":"Have unknown telepath version installed, aborting", "stop": true}');
				// die;
			//}
		}
		
		// Get engine version, update version in DB
		$this->engine_version = $this->get_engine_version();
		if(!$this->engine_version) {
			$this->log('{"msg":"Coult not determine telepath executable version, aborting", "stop": true}');
			die;
		}
		
		$base_path   = '/opt/telepath';
		$table_path  = $base_path . '/db/tables';
		$data_path   = $base_path . '/db/data';
		
		$table_files = scandir($table_path);
		$data_files  = scandir($data_path);
		
		$queries_tables[]    = array();
		$queries_alters[]    = array();
		
		$index = 0;
		
		foreach($table_files as $file) {
			
			if($file == '.svn' || $file == '.' || $file == '..') {
				continue;
			}
			
			$table_sql  = file_get_contents($table_path . '/' . $file);
			$table_name = trim(substr($file, 0, -4));
			
			$index++;
			$this->log('{"msg":"Checking table ' . $table_name . '", "index": ' . $index . ', "total": ' . (count($table_files) - 2) . ' }');
			$this->db->query($table_sql);
			usleep(10000);
			
			// Regular table
			$queries_tables[$table_name] = $table_sql;
			$queries_alters[$table_name] = $this->process_table_sql($table_sql);

		}
		
		$this->log('{"msg":"Checking ' . count($queries_tables) . ' tables.."}');
		
		$tableTotal = count($queries_tables);
		$alterTotal = 0;
		$keysTotal  = 0;
		$index      = 0;
		
		
		foreach($queries_alters as $table_name => $data) {
			$alterTotal += isset($data['alter_columns']) ? count($data['alter_columns']) : 0;		
			$keysTotal += isset($data['alter_columns']) ? count($data['alter_keys']) : 0;
		}
		
		$total = $alterTotal + $keysTotal;
		
		foreach($queries_alters as $table_name => $data) {
			
			if(!empty($data['alter_columns'])) {
				
				foreach($data['alter_columns'] as $col_index => $sql) {
					$index++;
					$this->log('{"msg":"Checking column ' . $col_index . '", "index": ' . $index . ', "total": ' . $total . ' }');
					$this->db->query($sql);
					usleep(10000);
				}
				
			}
			
		}
		
		foreach($queries_alters as $table_name => $data) {
			
			if(!empty($data['alter_keys'])) {
				
				foreach($data['alter_keys'] as $col_index => $sql) {
					$index++;
					$this->log('{"msg":"Checking index ' . $col_index . '", "index": ' . $index . ', "total": ' . $total . ' }');
					$this->db->query($sql);
					usleep(10000);
				}
				
			}
			
		}

		//$this->log('{"msg":"Checking for default configuration data.."}');
		
		$index = 0;
		foreach($data_files as $file) {
			
			if($file == '.svn' || $file == '.' || $file == '..') {
				continue;
			}
			
			$data_sql  = file_get_contents($data_path . '/' . $file);
			$table_name = trim(substr($file, 0, -4));
			
			$index++;
			$this->log('{"msg":"Checking data ' . $table_name . '", "index": ' . $index . ', "total": ' . (count($data_files) - 2) . ' }');
			
			if($this->db->count_all_results($table_name) == 0) {
				$this->db->query($data_sql);
			}

			usleep(10000);
			
		}
		
		$this->set_database_version($this->engine_version);
	
	}
	
	
	function filter_trim($string) {
		
		return trim(preg_replace("/[^a-zA-Z0-9_]+/", "", $string));
		
	}
	
	function process_table_sql($sql, $table_name = false) {
		
		//$this->db->query($sql);
		$sql = explode("\n", $sql);
		
		$table_schema   = 'telepath';
		$columns = array();
		$keys   = array();
		
		$column_alter_queries = array();
		$key_alter_queries    = array();
		
		foreach($sql as $index => $line) {
		
			// Get table name
			if($index == 0) { // We might have a table split table name from ARG
				
				if(!$table_name) {
					$pos = strpos($line, '(');
					$tmp = explode(' ', substr($line, 0, $pos - 1));
					$table_name = $this->filter_trim(array_pop($tmp));
				}
				
			} else {
				
				// Skip over the last line
				if($index == count($sql) - 1 || strpos($line, 'ENGINE=') > -1) {
					continue;
				}
				
				$line = trim($line);
				if(substr($line, -1) == ',') {
					$line = substr($line, 0, -1);
				}
				
				// Collect keys and fields
				if(strpos($line, 'KEY') > -1) {
					$keys[] = $line;
				} else {
					$columns[] = $line;
				}
			
			}
			
		}
		
		foreach($columns as $column) {
			
			$column_parts       = explode(' ', $column, 2);
			$column_name       = $this->filter_trim($column_parts[0]);
			$column_definition = $column_parts[1];
			
			if($column_name == '') {
				continue;
			}

			$column_query  = "SELECT * FROM information_schema.columns WHERE table_schema = '$table_schema' AND table_name = '$table_name' AND column_name = '$column_name'";
			$column_result = $this->db->query($column_query)->result();
			
			if(empty($column_result)) {
				$column_alter_query = "ALTER TABLE $table_name ADD column `$column_name` $column_definition;";
			} else {
				$column_alter_query = "ALTER TABLE $table_name CHANGE `$column_name` `$column_name` $column_definition;";
			}
			
			$column_alter_queries[$table_name . "." . $column_name] = $column_alter_query;
			
		}
		
		foreach($keys as $key) {
			
			if(strpos(strtoupper($key), 'PRIMARY') === false) {
				
				$key_parts = explode(' ', $key);
				foreach($key_parts as $i => $part) {
					if($part == 'KEY') {
						$key_name = $this->filter_trim($key_parts[$i + 1]);
						break;
					}
				}
				
			} else {
				$key_name = 'PRIMARY';
			}
			
			$key_definition = strpos($key, '(');
			$key_definition = substr($key, $key_definition);

			$key_query  = "SELECT INDEX_NAME FROM INFORMATION_SCHEMA.STATISTICS WHERE table_schema = '$table_schema' AND table_name = '$table_name' AND index_name = '$key_name'";
			$key_result = $this->db->query($key_query)->result();
			
			if(empty($key_result)) {
			
				$key_alter_queries[$table_name . "." . $key_name] = "ALTER IGNORE TABLE $table_name ADD $key";
				
			}
			
		}
				
		return array('name' => $table_name, 'alter_columns' => $column_alter_queries, 'alter_keys' => $key_alter_queries);
	
	}
	
	function set_database_version($version) {
		
/*		$data = array('value' => $version);
		$this->db->where('name', 'engine_version');
		$this->db->update('config', $data);*/

		$params = [
			'index' => 'telepath-config',
			'type' => 'config',
			'id' => 'telepath_version_id',
			'body' => [
				'doc' => [
					'value' =>  $version
					]
			]
		];

		$this->elasticClient->update($params);

	}

	
	function get_engine_version() {
		
		// Run shell to find engine version
		$engine_shell  = '/opt/telepath/bin/engine -v';
		$engine_result = shell_exec($engine_shell);
		$engine_version = intval(array_pop(explode(':', $engine_result)));
	
		return $engine_version;
		
	}
	
	function get_database_version() {

		$params = [
			'index' => 'telepath-config',
			'type' => 'config',
			'id' => 'telepath_version_id',
		];

		$result = $this->elasticClient->get($params);

		return  $result['_source']['value'];
		
		/*// We have config, lets check its version and verify against the engine
		$row = $this->db->where('name', 'engine_version')->get('config')->row_array();
		// Serious problem here, stop
		if(!$row) {	return false; }
	
		return intval($row['value']);
	*/
	}
	
}

?>
