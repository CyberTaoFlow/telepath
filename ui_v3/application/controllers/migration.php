<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Migration extends CI_Controller
{

    public function __construct()
    {

        parent::__construct();
        require 'vendor/autoload.php';

    }

    public function index()
    {
        $this->load->view('maintenence');
    }

    public function migrate()
    {

        $host_mapping = array();

        $atms_file = '/opt/telepath/conf/atms.conf';
        $atms_conf = parse_ini_file($atms_file);
        $client = new Elasticsearch\Client();

        // Build config
        $config['hostname'] = $atms_conf['database_address'];
        $config['username'] = $atms_conf['username'];
        $config['password'] = $atms_conf['password'];
        //$config['port']     = $db_port;

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

        $this->load->model('M_Requests', '', $config);

        //$latest_req = $this->M_Requests->get_latest();

        $this->load->model('M_Maintenence', '', $config);
        for ($x = 0; $x < 10000000; $x++) {

            if ($x % 100 == 0) {
                echo $x . "\n";
            }

            $doc = $this->M_Maintenence->buildDocument($x);
            if ($doc) {

                $host = $doc['id.resp_h'];
                if (isset($host_mapping[$host])) {
                    $doc['id.resp_h'] = $host_mapping[$host];
                } else {
                    $host_mapping[$host] = gethostbyname($host);
                    $doc['id.resp_h'] = $host_mapping[$doc['id.resp_h']];
                }

                $params = array();
                $params['body'] = $doc;
                $params['index'] = 'telepath_migration';
                $params['type'] = 'http';
                $ret = $client->index($params);
            }
        }

    }

    public function check()
    {

        $client = new Elasticsearch\Client();

        // Checks ELASTIC / MYSQL Connectivity
        $op = isset($_REQUEST['op']) ? $_REQUEST['op'] : 'default';
        $atms_file = '/opt/telepath/conf/atms.conf';
        echo 'Reading ATMS config .. ';
        $atms_conf = parse_ini_file($atms_file);
        echo ' done.<br>';

        echo 'Attempting MYSQL connection .. ';

        $con = mysqli_connect($atms_conf['database_address'], $atms_conf['username'], $atms_conf['password'], "telepath");

        // Check connection
        if (mysqli_connect_errno()) {
            echo "Failed to connect to MySQL: " . mysqli_connect_error();
        } else {
            echo "MYSQL success!<br>";

            $res = mysqli_query($con, "SELECT count(*) as count from request_scores_merge");
            if ($res->num_rows > 0) {
                $row = mysqli_fetch_assoc($res);
                $count = $row['count'];
                echo 'Have ' . $count . ' records in requests table ..<br>';
            }

            echo 'Checking ELASTIC<br>';


            $res = $client->indices()->getMapping();
            echo 'Have following indices : <br>';
            echo implode(', ', array_keys($res));

            echo '<br><br>READY!<br>';

            echo 'By clicking start you are about to wipe ElasticSearch DB and populate it with data from MYSQL<br>';

            echo '<a style="cursor: pointer; text-decoration: underline; color: blue;" onclick="migrate_start()">Start Migration</a>';

        }

        mysqli_close($con);

    }

}
