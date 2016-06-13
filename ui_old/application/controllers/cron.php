<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

ini_set('memory_limit','256M');

class Cron extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function debug() {
		
		$mail_config = $this->_compile_mail_config();
		$this->cron_alerts($mail_config);
		
	}
	
	public function cron_alerts($mail_config) {
		
		$this->load->model('AlertsModel');
		$this->load->model('AttributeScores');
		
		$data = $this->AlertsModel->get_report_alerts();
		
		
		
		print_r($data);
		
	}
	
	public function cron_reports($mail_config) {
		
		$this->load->model('AlertsModel');
		$this->load->model('AttributeScores');
		
	}
	
	
	public function testmail() {
		
		$smtp   = $this->input->post('smtp', TRUE);
		$port   = $this->input->post('smtp_port', TRUE);
		$user   = $this->input->post('smtp_user', TRUE);
		$pass   = $this->input->post('smtp_pass', TRUE);
		$target = $this->input->post('test_mail', TRUE);
	
		$test_config =  array(
		  'protocol' => 'sendmail',
		  'smtp_host' => $smtp,
		  'smtp_port' => $port,
		  'smtp_user' => $user,
		  'smtp_pass' => $pass,
		  'mailtype' => 'html',
		  'charset' => 'iso-8859-1',
		  'wordwrap' => TRUE
		);
				
		$config['mailtype'] = 'html';
		$this->load->library('email', $test_config);
		$this->email->set_newline("\r\n");
		$this->email->from('telepath@hybridsec.com'); // change it to yours
		$this->email->to($target);// change it to yours
		$this->email->subject('Telepath test mail');
		$this->email->set_mailtype("html");
		$this->email->message('Telepath test mail');
		
		if($this->email->send()) {
			return_success();
		} else {
			return_fail($this->email->print_debugger());
		}
		
	}
	
	public function _process_merge() {
		
		// Recreate Merge Tables
		$split_tables = array('request_scores', 'alerts', 'attribute_scores');
		
		foreach($split_tables as $base) {
			
			$o = array($base);
			// Cleanup
			$this->db->query('DROP TABLE IF EXISTS ' . $base . '_merge');
			// Build list
			$q = $this->db->query('SHOW TABLES LIKE "' . $base . '_from_%"');
			foreach($q->result_array() as $k => $v) { $o = array_merge($o, array_values($v)); }
								
			// Create Merge
			// $this->db->query('CREATE TABLE ' . $base . '_merge LIKE ' . $base);
			// Method #2
			$res = $this->db->query('SHOW CREATE TABLE ' . $base)->result();
			$res = (array) $res[0];
			$res = $res['Create Table'];
			$res = explode("\n", $res);
			foreach($res as $i => $v) {
				if(strpos($v, 'FULLTEXT') > -1) {
					unset($res[$i]);
					// If last index drop trailing comma from previous line
					if($i == count($res) - 1) {	$res[$i - 1] = substr($res[$i - 1], 0, -1);	}
				}
			}
			$res = implode("\n", $res);
			$res = str_replace($base, $base . '_merge', $res);
			$this->db->query($res);
			
			// UNION
			$this->db->query('ALTER TABLE ' . $base . '_merge ENGINE=MERGE, UNION=(' . implode(',', $o) . ')');
			
		}

    }
	
	public function index() {
		
		if(!$this->input->is_cli_request() && !ENVIRONMENT == 'development') {
			echo 'This script can be run via CLI only';
			die;
		}
		
		// DB
		$this->load->database();
		
		// Maint Task 0 -- Re-Create MERGE tables in the database from split data
		$this->_process_merge();
		
		// Prep Config
		$this->load->model('ConfigModel');
		$config = $this->ConfigModel->get();

		// Task 1 -- Update known bad IP's and Tor exit nodes
		$this->_update($config);
		
		// Task 2 -- Process reports (mail / remote syslog)
		$this->_process_reports($config);
	
	}
	
	public function _update($config) {
		
		$base_path = 'http://www.hybridsec.com/updates/';
		
		$proxy = $config['proxy_flag'] == '1' && 
				 $config['proxy_ip'] != '' && 
				 $config['proxy_port'] != '' ? 
				 $config['proxy_ip'] . ':' . $config['proxy_port'] : false;
		
		// Deprecated code
		// $bot_list = $this->_simple_curl($base_path . 'Known-Bot.txt', $proxy);
		// $tor_list = $this->_simple_curl($base_path . 'Tor_ip_list_EXIT.txt', $proxy);
		// $bot_list = explode("\n", $bot_list);
		// $tor_list = explode("\n", $tor_list);
		
		// New code
		
		$bot_list = array();
		$tor_list = array();
		
		$updates_data = gzuncompress($this->_simple_curl($base_path . 'updates.gz', $proxy));
		$mode = '';
		$updates_data = explode("\n", $updates_data);
		
		foreach($updates_data as $row) {
			if($row == "BAD_IPS >>>")  { $mode = 'bad_ip';   continue; }
			if($row == "TOR_EXIT >>>") { $mode = 'tor_exit'; continue; }
			if($mode == 'bad_ip') 	   { $bot_list[] = $row; }
			if($mode == 'tor_exit')    { $tor_list[] = $row; }
		}
		
		$bots = 0;
		$tors = 0;
		
		$insert_data = array();
		
		foreach($bot_list as $line) {
			preg_match( '/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/', $line, $m );
			if(!empty($m)) {
				$insert_data[] = array('ip_addr' => $m[0], 'name' => 'KB');
				$bots++;
			}
		}
		foreach($tor_list as $line) {
			preg_match( '/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/', $line, $m );
			if(!empty($m)) {
				$insert_data[] = array('ip_addr' => $m[0], 'name' => 'Tor');
				$tors++;
			}
		}
		
		echo 'Telepath Update :: KB :: ' . $bots . ' :: TOR :: ' . $tors . "\n";
		
		echo 'Updating DB Start';
		$this->db->query('TRUNCATE TABLE bad_ips');
		$this->db->insert_batch('bad_ips', $insert_data);
		$this->db->query("UPDATE config SET value='1' WHERE action_code=71");
		echo 'Updating DB End';

	}
	
	public function _process_reports($config) {
		
		ini_set('display_errors',1);
		ini_set('display_startup_errors',1);
		error_reporting(-1);
		
		$this->load->model('AlertsModel');
		$this->load->model('AttributeScores');
		
		echo 'Reports Enter';
		
		$syslog = false;
		
		// Setup Syslog
		if($config['remote_syslog_ip'] != '') {
		
			$this->load->library('Syslog');
			$syslog = new Syslog();
			
			$syslog_addr = $config['remote_syslog_ip'];
			$syslog_addr = explode(':', $syslog_addr);
			
			if(count($syslog_addr) == 2) {
			
				$syslog->SetServer($syslog_addr[0]);
				$syslog->SetPort($syslog_addr[1]);
				
			} else {
			
				$syslog->SetServer($syslog_addr[0]);
				
				
			}
			
			$syslog->SetFacility(3);
		    $syslog->SetSeverity(7);
			$syslog->SetHostname('telepath');
			
		}
		
		$data = $this->AlertsModel->get_report_alerts();
		
		$columns = array(
			array('row_id' => 'id', 'title' => 'Alert ID'),
			array('row_id' => 'date', 'title' => 'Time'),
			array('row_id' => 'IP', 'title' => 'Source IP'),
			array('row_id' => 'name', 'title' => 'Alert Name'),
			array('row_id' => 'description', 'title' => 'Alert Description'),
			array('row_id' => 'display_path', 'title' => 'Page'),
			array('row_id' => 'app_domain', 'title' => 'Domain'),
			array('row_id' => 'average_score', 'title' => 'Score'),
			array('row_id' => 'alert_param_ids', 'title' => 'Parameters'),
		);
		
		$html = '<table border="1"><thead><tr>';
		foreach($columns as $col) {
			$html .= '<td>' . $col['title'] . '</td>';
		}
		$html .= '</tr></thead><tbody>%TBODY%</tbody></table>';
		
		$recepients_data = array();

		foreach($data as $row) {
			
			$row_html    = '<tr>';
			$row_syslog  = '';
			
			foreach($columns as $col) {
				
				$col_html = '<td style="padding: 3px 5px">';
				$col_data = '';
				
				switch($col['row_id']) {
					
					case 'date':
						// Date Formatter
						$col_data .= date('d/m/y H:i:s', $row[$col['row_id']]);
						// %a, %d %b %Y %H:%M:%S
						
					break;
					
					case 'alert_param_ids':
						// Extract Parameters
						$params = $this->AttributeScores->get_specified_param_by_RIDs($row[$col['row_id']], $row['RID'], true, $row['date'] - 60, $row['date'] + 60);
						foreach($params as $param) {
							
							$att_name = $param->att_alias != '' ? $param->att_alias : $param->att_name;
							$col_data .= $att_name . '&nbsp;:&nbsp;' . $param->data . "\n";
							
						}
						
					break;
					
					default:
					
						// Normal output
						$col_data .= $row[$col['row_id']];
						
					break;
				
				}
				
				$col_html .= $col_data . '</td>';
				$row_html .= $col_html;
				//echo $row_html;
				
				$row_syslog .= $col_data . ' | ';
				
			}
			
			$row_html .= '</tr>';
			
			if($row['email_recipient'] != '') {
			
				$recepients = explode(',', $row['email_recipient']);
				
				foreach($recepients as $rcv) {
					
					if(!isset($recepients_data[$rcv])) {
						$recepients_data[$rcv] = array();
					}
					
					$recepients_data[$rcv][] = $row_html;
					
				}

			}
			if($row['syslog'] && $syslog) {
				// Dispatch syslog event
				$syslog->SetContent($row_syslog);
				$syslog->Send();
			}
			
		}
		//print_r($recepients_data);
		if(!empty($recepients_data)) {
			foreach($recepients_data as $email => $rows) {
				
				$rows_html = '';
				foreach($rows as $row) {
					$rows_html .= $row;
				}
				
				// Dispatch email
				$this->_send_mail($email, 'info@hybridsec.com', 'Hybrid Telepath Report', str_replace('%TBODY%', $rows_html, $html));
				
			}
		}
		
		$this->AlertsModel->mark_report_alerts();
		echo 'Reports Done';
		
	}
	
	function _send_mail($to, $from, $subject, $body) {
		
		$config = get_smtp_config();
		$config['protocol'] = 'sendmail';
		$config['mailtype'] = 'html';
		
		$message = '';
		$this->load->library('email', $config);
		$this->email->set_newline("\r\n");
		$this->email->from($from); // change it to yours
		$this->email->to($to);// change it to yours
		$this->email->subject($subject);
		$this->email->set_mailtype("html");
		$this->email->message($body);
		
		if($this->email->send()) {
			echo 'Sent email to ' . $to . "\n";
		} else {
			show_error($this->email->print_debugger());
		}

	}
	
	public function _simple_curl($url, $proxy = false) {
		
		// is cURL installed yet?
		if (!function_exists('curl_init')) {
			die('Sorry cURL is not installed!');
		}
	 
		$ch = curl_init();
		
		if($proxy) {
			curl_setopt($ch, CURLOPT_HTTPPROXYTUNNEL, 0);
			curl_setopt($ch, CURLOPT_PROXY, $proxy);
		}
		
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_TIMEOUT, 30);
		
		$output = curl_exec($ch);
		
		if(curl_errno($ch)) {
			echo 'error:' . curl_error($ch) . "\n";
			return '';
		}
		
		curl_close($ch);
		return $output;
	
	}
	
	
	private function _compile_mail_config() {
		
		$mail_config = array();
		
		// 1. load all groups
		$this->load->model('Ion_auth_model');
		$acl = $this->Ion_auth_model;
				
		$all_groups = $acl->groups()->result();
		$all_users  = $acl->users()->result();
		
		foreach($all_users as $user) {
		
			// Check if user has email defined
			
			if(!isset($user->email) || $user->email == '') {
				continue; // Nothing to do then, break the loop.
			} else {
				$mail_config[$user->email] = array();
			}
			
			// Check if user explicity configured mail settings
			
			if(isset($user->extradata) && $user->extradata != '') {
			
				$tmp = json_decode($user->extradata);
				
				if(isset($tmp->apps)) {
					$mail_config[$user->email]['explicit_apps'] = $tmp->apps;
				}

				if(isset($tmp->extra)) {
				
					if($tmp->extra->mail_reports == '0') {
						$mail_config[$user->email]['explicit_reports'] = false;
					}
					if($tmp->extra->mail_reports == '1') {
						$mail_config[$user->email]['explicit_reports'] = true;
					}
					if($tmp->extra->mail_alerts == '0') {
						$mail_config[$user->email]['explicit_alerts'] = false;
					}
					if($tmp->extra->mail_alerts == '1') {
						$mail_config[$user->email]['explicit_alerts'] = true;
					}
					
				}
				
			}
			
			$mail_config[$user->email]['implicit_apps'] = array();
			
			// Check if one of user groups implicitly configured mail settings
			$user_groups = $acl->get_users_groups($user->user_id)->result();
			
			foreach($user_groups as $group) {
				
				if(isset($group->extradata) && $group->extradata != '') {
			
					$tmp = json_decode($group->extradata);
					
					if(isset($tmp->apps)) {
						$mail_config[$user->email]['implicit_apps'] = array_unique(array_merge($mail_config[$user->email]['implicit_apps'], $tmp->apps));
					}

					if(isset($tmp->extra)) {
				
						if($tmp->extra->mail_reports == '1') {
							$mail_config[$user->email]['implicit_reports'] = true;
						}
						if($tmp->extra->mail_alerts == '1') {
							$mail_config[$user->email]['implicit_alerts'] = true;
						}
					
					}
					
				
				} else {
					// No specific applications / mailer settings
					//continue;
				}
			
			}
						
		}
		
		return $mail_config;
		
	}
	
}