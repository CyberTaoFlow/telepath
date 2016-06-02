<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

ini_set('memory_limit', '512M');

class Cron extends Tele_Controller
{

    public function reports()
    {

        // DB
        $this->load->database();

        // Prep Config
        $this->load->model('M_Config');
        $config = $this->M_Config->get();

        // Default Flag
        $syslog = false;

        // Check Config
        if ($config['write_to_syslog_id'] == '1' && $config['syslog_ip_id'] != '') {

            // Setup syslog library
            $this->load->library('Syslog');
            $syslog = new Syslog();

            $syslog_addr = $config['syslog_ip_id'];
            $syslog_addr = explode(':', $syslog_addr);

            if (count($syslog_addr) == 2) {
                $syslog->SetServer($syslog_addr[0]);
                $syslog->SetPort($syslog_addr[1]);
            } else {
                $syslog->SetServer($syslog_addr[0]);
            }

            $syslog->SetFacility(3);
            $syslog->SetSeverity(7);
            $syslog->SetHostname('telepath');

        } else {
            // Syslog is deactivated, nothing to do here
            return;
        }

        // Gather last minute alerts
        $client = new Elasticsearch\Client();

        $params['body'] = [
            "size" => 100,
            "sort" => ['ts' => 'desc'],
            "query" => ['bool' => ['must' => []]]
        ];
        $params['index'] = 'telepath-20*';

        $ts_start = intval(strtotime('-1 minute'));

        $params['body']['query']['bool']['must'][] = ['term' => ['_type' => 'http']];
        $params['body']['query']['bool']['must'][] = ['filtered' => ['filter' => ['exists' => ['field' => 'alerts']]]];
        $params['body']['query']['bool']['must'][] = ['range' => ['ts' => ['gte' => $ts_start]]];

        $alerts = $client->search($params);

        echo 'Since ' . date(DATE_RFC2822, $ts_start) . ' Till ' . date(DATE_RFC2822) . ' there are ' . $alerts['hits']['total'] . ' alerts.. ' . "\n";

        if (!empty($alerts['hits']['hits'])) {
            $alerts = $alerts['hits']['hits'];
        } else {
            return;
        }

        foreach ($alerts as $alert) {

            $alert = $alert['_source'];

            // new line for syslog
            $row_syslog = $alert['ts'] . '|' . $alert['ip_orig'] . '|' . $alert['ip_resp'] . '|';
            // Just in case..
            if (!empty($alert['alerts'])) {
                foreach ($alert['alerts'] as $a) {
                    $row_syslog .= $a['name'] . ' ';
                }
                $row_syslog = trim($row_syslog);
                $row_syslog .= '|';
            }

            $row_syslog .= $alert['uri'] . '|' . $alert['host'] . '|';

            if (!empty($alert['parameters'])) {

                // Concat interecting parameters
                foreach ($alert['parameters'] as $p) {
                    if (intval($p['score_data']) > 85) {
                        $row_syslog .= $p['name'] . '=' . $p['value'] . ',';
                    }
                }
                // Trailing comma
                $row_syslog = substr($row_syslog, 0, -1);
            }

            echo $row_syslog;

            $syslog->SetContent($row_syslog);
            $syslog->Send();

        }

    }

    public function index()
    {

        if (!$this->input->is_cli_request() && !ENVIRONMENT == 'development') {
            echo 'This script can be run via CLI only';
            die;
        }

        // DB
        $this->load->database();

        // Prep Config
        $this->load->model('M_Config');
        $config = $this->M_Config->get();

        // Task 1 -- Update known bad IP's and Tor exit nodes
        $this->_update($config);

    }

    public function __construct()
    {

        parent::__construct();
        require 'vendor/autoload.php';

    }

    public function debug()
    {

        $conf = $this->_compile_mail_config();

        print_r($conf);

    }

    private function _compile_mail_config()
    {

        $mail_config = array();

        // 1. load all groups
        $this->load->model('Ion_auth_model');
        $acl = $this->Ion_auth_model;

        $all_groups = $acl->groups()->result();
        $all_users = $acl->users()->result();

        foreach ($all_users as $user) {

            // Check if user has email defined

            if (!isset($user->email) || $user->email == '') {
                continue; // Nothing to do then, break the loop.
            } else {
                $mail_config[$user->email] = array();
            }

            // Check if user explicity configured mail settings

            if (isset($user->extradata) && $user->extradata != '') {

                $tmp = json_decode($user->extradata);

                if (isset($tmp->apps)) {
                    $mail_config[$user->email]['explicit_apps'] = $tmp->apps;
                }

                if (isset($tmp->extra)) {

                    if ($tmp->extra->mail_reports == '0') {
                        $mail_config[$user->email]['explicit_reports'] = false;
                    }
                    if ($tmp->extra->mail_reports == '1') {
                        $mail_config[$user->email]['explicit_reports'] = true;
                    }
                    if ($tmp->extra->mail_alerts == '0') {
                        $mail_config[$user->email]['explicit_alerts'] = false;
                    }
                    if ($tmp->extra->mail_alerts == '1') {
                        $mail_config[$user->email]['explicit_alerts'] = true;
                    }

                }

            }

            $mail_config[$user->email]['implicit_apps'] = array();

            // Check if one of user groups implicitly configured mail settings
            $user_groups = $acl->get_users_groups($user->user_id)->result();

            foreach ($user_groups as $group) {

                if (isset($group->extradata) && $group->extradata != '') {

                    $tmp = json_decode($group->extradata);

                    if (isset($tmp->apps)) {
                        $mail_config[$user->email]['implicit_apps'] = array_unique(array_merge($mail_config[$user->email]['implicit_apps'], $tmp->apps));
                    }

                    if (isset($tmp->extra)) {

                        if ($tmp->extra->mail_reports == '1') {
                            $mail_config[$user->email]['implicit_reports'] = true;
                        }
                        if ($tmp->extra->mail_alerts == '1') {
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

    public function _update($config)
    {

        $base_path = 'http://www.hybridsec.com/updates/';

        $proxy = $config['proxy_mode_id'] == '1' &&
        $config['proxy_ip_id'] != '' &&
        $config['proxy_port_id'] != '' ?
            $config['proxy_ip_id'] . ':' . $config['proxy_port_id'] : false;

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

        foreach ($updates_data as $row) {
            if ($row == "BAD_IPS >>>") {
                $mode = 'bad_ip';
                continue;
            }
            if ($row == "TOR_EXIT >>>") {
                $mode = 'tor_exit';
                continue;
            }
            if ($mode == 'bad_ip') {
                $bot_list[] = $row;
            }
            if ($mode == 'tor_exit') {
                $tor_list[] = $row;
            }
        }

        $bots = 0;
        $tors = 0;

        // Delete existing index to save only relevant data
        $params = ['index' => 'telepath-bad-ips'];
        $this->elasticClient->indices()->delete($params);

        $params['type'] = 'bad';
        $params ['body'] = [];

        foreach ($bot_list as $line) {
            preg_match('/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/', $line, $m);
            if (!empty($m)) {
                $bots++;

                $params['body'][] = [
                    'index' => [
                        '_id' => $m[0]
                    ]
                ];

                $params['body'][] = [];

                // Every 1000 documents stop and send the bulk request
                if ($bots % 1000 == 0) {
                    $responses = $this->elasticClient->bulk($params);

                    // erase the old bulk request
                    $params ['body'] = [];

                    // unset the bulk response when you are done to save memory
                    unset($responses);
                }
            }
        }

        // Send the last batch if it exists
        if (!empty($params['body'])) {
            $this->elasticClient->bulk($params);
        }


        $params = ['index' => 'telepath-tor-ips'];
        $this->elasticClient->indices()->delete($params);

        $params['type'] = 'tor';
        $params ['body'] = [];

        foreach ($tor_list as $line) {
            preg_match('/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/', $line, $m);
            if (!empty($m)) {
                $tors++;

                $params['body'][] = [
                    'index' => [
                        '_id' => $m[0]
                    ]
                ];

                $params['body'][] = [];

                // Every 1000 documents stop and send the bulk request
                if ($tors % 1000 == 0) {
                    $responses = $this->elasticClient->bulk($params);

                    // erase the old bulk request
                    $params ['body'] = [];

                    // unset the bulk response when you are done to save memory
                    unset($responses);
                }
            }
        }

        // Send the last batch if it exists
        if (!empty($params['body'])) {
            $this->elasticClient->bulk($params);
        }


        echo 'Telepath Updated :: KB :: ' . $bots . ' :: TOR :: ' . $tors . "\n";

        //      $insert_data = array();

        /*foreach ($bot_list as $line) {
            preg_match('/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/', $line, $m);
            if (!empty($m)) {
                $insert_data[] = array('ip_addr' => $m[0], 'name' => 'KB');
                $bots++;
            }
        }
        foreach ($tor_list as $line) {
            preg_match('/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/', $line, $m);
            if (!empty($m)) {
                $insert_data[] = array('ip_addr' => $m[0], 'name' => 'Tor');
                $tors++;

            }
        }*/

//        foreach ($bot_list as $line) {
//            preg_match('/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/', $line, $m);
//            if (!empty($m)) {
//                $insert_data[] = array('from' => $m[0], 'to' => $m[1]);
//                $bots++;
//            }
//        }
//        foreach ($tor_list as $line) {
//            preg_match('/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/', $line, $m);
//            if (!empty($m)) {
//                $insert_data[] = array('from' => $m[0], 'to' => $m[1]);
//                $tors++;
//            }
//        }
//
//        echo 'Telepath Update :: KB :: ' . $bots . ' :: TOR :: ' . $tors . "\n";
//
//        echo 'Updating DB Start';
//
//        $params = [
//            'index' => 'telepath-config',
//            'type' => 'ips',
//            'id' => 'bad_ips',
//            'body' => [
//                'doc' => [
//                    'ips' => $insert_data
//                ],
//                'doc_as_upsert' => true
//            ]
//        ];
//
//
//        $this->elasticClient->update($params);
//       /* $this->db->query('TRUNCATE TABLE bad_ips');
//        $this->db->insert_batch('bad_ips', $insert_data);
//        $this->db->query("UPDATE config SET value='1' WHERE action_code=71");*/
//        echo 'Updating DB End';

    }

    public function _simple_curl($url, $proxy = false)
    {

        // is cURL installed yet?
        if (!function_exists('curl_init')) {
            die('Sorry cURL is not installed!');
        }

        $ch = curl_init();

        if ($proxy) {
            curl_setopt($ch, CURLOPT_HTTPPROXYTUNNEL, 0);
            curl_setopt($ch, CURLOPT_PROXY, $proxy);
        }

        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_TIMEOUT, 30);

        $output = curl_exec($ch);

        if (curl_errno($ch)) {
            echo 'error:' . curl_error($ch) . "\n";
            return '';
        }

        curl_close($ch);
        return $output;

    }

}
