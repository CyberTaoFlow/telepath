<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Setup extends CI_Controller {
	
	function __construct()
	{
		parent::__construct();
	}
	
	public function cli($db_host, $db_port, $db_user, $db_pass) {
		
		if($this->input->is_cli_request()) {
			
			echo "Checking database .. \n";
			
			// Build config
			$config['hostname'] = $db_host;
			$config['username'] = $db_user;
			$config['password'] = $db_pass;
			$config['port']     = $db_port;
			
			// More config
			$config['database'] = "telepath";
			$config['dbdriver'] = "mysqli";
			$config['dbprefix'] = "";
			$config['pconnect'] = FALSE;
			$config['db_debug'] = TRUE;
			
			// Try connecting
			$this->load->database($config, FALSE, TRUE);
			$this->db->query('USE telepath');
			
			// Start process and keep updating update.json in files dir
			
			// No Limit
			set_time_limit(0);
			
			$this->load->model('M_Maintenence', '', $config);
			$this->M_Maintenence->work($logfile);
			$this->M_Maintenence->patch();
			
		}
	
	}
	
	public function manual() {
		
		$logfile = realpath('.') . '/files/update.log';
		$this->load->model('M_Maintenence');
		$this->M_Maintenence->work(false);
	
	}
	
	public function index() {
		
		$action  = $this->input->post('action');
		$logfile = realpath('.') . '/files/update.log';
		
		if(!$action) {
			$action  = $this->input->get('action');
		}
		if(!$action) { $this->load->view('setup'); }
		
		switch($action) {
			
			case 'update_db':
				
				// Reset update log
				file_put_contents($logfile, 'Started .. (0%)');
				
				$db_host = $this->input->post('db_host');
				$db_port = $this->input->post('db_port');
				$db_user = $this->input->post('db_user');
				$db_pass = $this->input->post('db_pass');
				
				// Build config
				$config['hostname'] = $db_host;
				$config['username'] = $db_user;
				$config['password'] = $db_pass;
				$config['port']     = $db_port;
				
				// More config
				$config['database'] = "telepath";
				$config['dbdriver'] = "mysqli";
				$config['dbprefix'] = "";
				$config['pconnect'] = FALSE;
				$config['db_debug'] = TRUE;
				
				// Try connecting
				$this->load->database($config, FALSE, TRUE);
				$this->db->query('USE telepath');
				
				// Start process and keep updating update.json in files dir
				
				// No Limit
				set_time_limit(0);
				
				$this->load->model('M_Maintenence', '', $config);
				$this->M_Maintenence->work($logfile);
				
			
			break;
			
			case 'status':
			
				header('Content-Type: application/json');
				$data = explode("\n", file_get_contents($logfile));
				$ans  = array();
				
				$offset = intval($this->input->post('offset'));
				
				foreach($data as $i => $row) {
					if($i >= $offset) {
						$ans[] = $row;
					}
				}
				
				echo json_encode($ans);
				die;
			
			break;
					
		}
				
		
	}
		
}