<?php

class TableSplit extends CI_Model {
	
	private $table_cache = array();
	
	function __construct()
	{
		parent::__construct();
	}
	
	function get_table_split($table, $min_date = -1, $max_date = -1) {
		
		if(!isset($this->table_cache[$table])) {
			// Plain result of table array
			$this->table_cache[$table] = $this->_match_tables($this->_get_tables($table), $table, false);
		}
		
		if(!isset($this->table_cache[$table][0])) {
			echo 'Fatal Error in TableSplit.php, 0 matching tables';
			die;
		}
		
		// No range to check, return all tables
		if($min_date == -1 && $max_date == -1) {
		
			return $this->table_cache[$table];
			
		} else {
			
			// Prepare result
			$result = array();
			// Result consisting of from - to timestamps of range tables only
			$ranges = $this->_match_tables($this->table_cache[$table], $table, true);
			
			$max_range = 0;
			
			foreach($ranges as $table_range_name => $range) {
				
				if($range['to'] > $max_range) {
					$max_range = $range['to'];
				}
				
				$pass = false;

				if($min_date > -1 && $max_date > -1) { // Have BOTH
					if($min_date < $range['to'] && $max_date > $range['from']) { // Between MIN and MAX
						$pass = true;
					}
				} elseif($min_date > -1) { // Have MIN only
					if($min_date < $range['to']) { // Check MIN only
						$pass = true;
					}
				} elseif($max_date > -1) { // Have MAX only
					if($max_date > $range['from']) { // Check MAX only
						$pass = true;
					}
				}

				if($pass) {
					$result[] = $table_range_name;
				}
				
			}
			
			// If we have a current table and result set is empty or
			// If the requested maximum is more than any of the ranges in named tables, include head table
			if(in_array($table, $this->table_cache[$table]) && !isset($result[0]) || $max_date > $max_range) {
				$result[] = $table;
			}
			
			return $result;
			
		}

	}
		
	function _get_tables($pattern) {
		
		$this->db->select('table_name');
		$this->db->where('table_type', 'BASE TABLE');
		$this->db->where('table_schema', $this->db->database); // Main db name
		$this->db->where('table_name LIKE', '%' . $pattern . '%');
		
		$this->db->from('information_schema.TABLES');
		$this->db->order_by('table_name', 'asc');
		
		//$query = $this->db->query('show tables LIKE "%' . $pattern . '%"');
	
		$query = $this->db->get();
		
		$return = array();
		$result = $query->result();
		
		foreach($result as $row) {
			$return[] = $row->table_name;
		}

		return $return;
		
	}
	
	function _match_tables($tables, $tables_prefix, $get_ranges = false) {
		
		$result = array();
		$ranges = array();
		
		foreach($tables as $table) {
				
			// no match
			$match = false;

			// 1:1 match:: table_name
			$match_check    = $table == $tables_prefix;
			if($match_check) {
				$match = true;
			} else {
			
				// matching prefix:: table_name******
				$prefix_check   = substr($table, 0, strlen($tables_prefix)) == $tables_prefix;
				
				if($prefix_check) {
				
					// matching digit:: table_name0, table_name1
					$digit_check = is_numeric(substr($table, strlen($tables_prefix), 1));
					
					if($digit_check) {
					
						$match = true;
						
					} else {
						
						// matching range from:: ****_from_****
						$range_pos_from = strpos($table, '_from_');
						// matching range to:: ****_to_****
						$range_pos_to   = strpos($table, '_to_');
						
						if(strlen($tables_prefix) == $range_pos_from && $range_pos_to > $range_pos_from) {
							$match = true;
							if($get_ranges) {
								
								$range_from = substr($table, $range_pos_from + strlen('_from_'), $range_pos_to - $range_pos_from - strlen('_from_'));
								$range_to  	= substr($table, $range_pos_to + strlen('_to_'));
								
								$date_from  = DateTime::createFromFormat('Y_m_d_H_i_s', $range_from)->format('U');
								$date_to    = DateTime::createFromFormat('Y_m_d_H_i_s', $range_to)->format('U');

								$ranges[$table] = array('from' => $date_from, 'to' => $date_to);
							
							}
						}
						
					}
					
				}
				
			}
										
			if($match && !$get_ranges) {
				$result[] = $table;
			}
				
		}
		
		if($get_ranges) {
			return $ranges;
		} else {
			return $result;
		}

	}
	
}

?>