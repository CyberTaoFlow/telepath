<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Setup extends CI_Controller
{

    function __construct()
    {
        parent::__construct();
    }

    public function cli($db_host, $db_port, $db_user, $db_pass)
    {

        if ($this->input->is_cli_request()) {

            echo "Checking database .. \n";

            // Build config
            $config['hostname'] = $db_host;
            $config['username'] = $db_user;
            $config['password'] = $db_pass;
            $config['port'] = $db_port;

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
            $this->M_Maintenence->work();
            $this->M_Maintenence->patch();

        }

    }

}
