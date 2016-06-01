<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Realtime extends Tele_Controller
{

    public function __construct()
    {

        parent::__construct();
        // Connect elastic
       // $params = array('hosts' => array('127.0.0.1:9200'));
        $this->elasticClient = new Elasticsearch\Client();

    }

    public function index()
    {

        $m = new Memcached();
        $m->addServer('localhost', 11211);

        // We need to read suricata stats file
        /*
        $out = array(
            'alerts' 	     => $this->db->count_all_results('alerts_merge'),
            'case_alerts' 	 => $this->db->count_all_results('case_alerts') ,
            'applications'   => $this->db->count_all_results('applications'),
            'requests'		 => $this->db->count_all_results('request_scores_merge'),
            'pages' 		 => $this->db->count_all_results('pages'),
        );
        */
        $out = array();

        // We need to read suricata stats file
        $lines = `tail -100 /opt/telepath/suricata/logs/stats.log`;
        $lines = explode("\n", $lines);
        $need = array('capture.kernel_packets', 'capture.kernel_drops');
        $suricata = array();


        foreach ($lines as $i => $line) {
            $line = explode("|", str_replace(" ", "", $line));
            if (count($line) > 2 && in_array($line[0], $need)) {
                $suricata[$line[0]] = $line[2];
            }
        }
        $out = array_merge($out, array_shift($m->getStats()), $suricata);

        echo json_encode($out);

        //return_json(array('capture.kernel_packets' => rand(0,100), 'capture.kernel_drops' => rand(0,100), 'cmd_set' => rand(0,100)));

    }

}