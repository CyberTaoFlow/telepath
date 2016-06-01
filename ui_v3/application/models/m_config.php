<?php

class M_Config extends CI_Model
{

    private $tableName = 'config';
    private $agentsRegexTableName = 'agents_regex';
    private $whitelistIPsTableName = 'whitelist_ip';

    function __construct()
    {

        parent::__construct();


    }

    public function changed(){
        $params = [
            'index' => 'telepath-config',
            'type' => 'config',
            'id' => 'config_was_changed_id',
            'body' => [
                'doc' => [
                    "value"=>"1"
                ]
            ]
        ];

        $this->elasticClient->update($params);
    }

    public function sql_whitelist_get_ips()
    {

        $this->db->select('user_ip');
        $this->db->distinct();
        $this->db->from($this->whitelistIPsTableName);
        $query = $this->db->get();
        $result = $query->result();
        $ans = array();
        foreach ($result as $row) {
            $ans[] = $row->user_ip;
        }
        // Sort array of ips as a numeric values, Yuli
        sort($ans, SORT_NUMERIC);
        return $ans;

    }

    public function whitelist_get_ips()
    {

//        /telepath-config/ips/whitelist_id/
        $params = [
            'index' => 'telepath-config',
            'type' => 'ips',
            'id' => 'whitelist_id'
        ];

        $result = $this->elasticClient->get($params);

        return $result['_source']['ips'];
    }

    public function sql_whitelist_delete_ip($ip)
    {
        $isValid = filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4);
        if (!$isValid) {
            # check for IP range
            $ip2 = explode('-', $ip);
            if (!empty($ip2) && !empty($ip2[0]) && !empty($ip2[1])) {
                if (filter_var($ip2[0], FILTER_VALIDATE_IP, FILTER_FLAG_IPV4) &&
                    filter_var($ip2[1], FILTER_VALIDATE_IP, FILTER_FLAG_IPV4)
                ) {
                    $isValid = 1;
                    $ip = $ip2[0] . '-' . $ip2[1];
                }
            }
        }
        if (!$isValid) {
            return 0;
        }


        $this->db->where('user_ip', $ip);
        $this->db->from($this->whitelistIPsTableName);
        $this->db->delete();
    }


    public function sql_whitelist_add_ip($ip)
    {
        $isValid = filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4);
        if (!$isValid) {
            # check for IP range
            $ip2 = explode('-', $ip);
            if (!empty($ip2) && !empty($ip2[0]) && !empty($ip2[1])) {
                if (filter_var($ip2[0], FILTER_VALIDATE_IP, FILTER_FLAG_IPV4) &&
                    filter_var($ip2[1], FILTER_VALIDATE_IP, FILTER_FLAG_IPV4)
                ) {
                    $isValid = 1;
                    $ip = $ip2[0] . '-' . $ip2[1];
                }
            }
        }
        if (!$isValid) {
            return 0;
        }
        $this->db->insert($this->whitelistIPsTableName, array('user_ip' => $ip));
        return $this->db->insert_id();

    }

    public function whitelist_set_ips($ips)

    {

        usort($ips, [$this, 'compare_from']);

        $params = [
            'index' => 'telepath-config',
            'type' => 'ips',
            'id' => 'whitelist_id',
            'body' => [
                'doc' => [
                    'ips' => $ips
                ]
            ]
        ];

        $this->elasticClient->update($params);
    }

    public function get_ip_balances()
    {

        $params = [
            'index' => 'telepath-config',
            'type' => 'ips',
            'id' => 'loadbalancerips_id'
        ];

        $result = $this->elasticClient->get($params);

        return  $result['_source']['ips'];

    }

    public function get_header_balances()
    {

        $params = [
            'index' => 'telepath-config',
            'type' => 'headers',
            'id' => 'loadbalancerheaders_id'
        ];

        $result = $this->elasticClient->get($params);

        return $result['_source']['headers'];

    }

    function compare_from($a, $b)
    {
        return strnatcmp($a['from'], $b['from']);
    }

    
    public function set_ip_balances($ips)
    {

        usort($ips, [$this, 'compare_from']);

        $params = [
            'index' => 'telepath-config',
            'type' => 'ips',
            'id' => 'loadbalancerips_id',
            'body' => [
                'doc' => [
                        'ips' => $ips
                ]
            ]
        ];

        $this->elasticClient->update($params);



    }

    public function set_header_balances($headers)
    {


        $params = [
            'index' => 'telepath-config',
            'type' => 'headers',
            'id' => 'loadbalancerheaders_id',
            'body' => [
                'doc' => [
                        'headers' => $headers
                ]
            ]
        ];

        $this->elasticClient->update($params);

    }

    public function get()
    {
        /*        GET /telepath-config/config/_search?pretty=true
        {
            "query" : {
            "match_all" : {}
            },
            "size": 98
        }*/

        /*        $params = [
                    'index' => 'telepath-config',
                    'type' => 'config',
                    'body' => [
                        'query' => [
                            'match_all' => [

                            ]
                        ]
                    ]
                ];*/


//        $result = $this->elasticClient->search($params);

        $params = [
            'index' => 'telepath-config',
            'type' => 'config',
            'body' => [
                'query' => [
                    'match_all' => [
                    ],


                ],
                "size" => '97'
            ]
        ];


        $result = $this->elasticClient->search($params);


        $results = [];
        foreach ($result['hits']['hits'] as $value) {

            $results[$value['_id']] = $value['_source']['value'];

        }
        return $results;
    }


    public function sql_get($key = false)
    {
        $this->db->select('name, value');
        $this->db->from($this->tableName);

        if ($key) {
            if (!is_array($key)) {
                $key = array($key);
            }
            $this->db->where_in('name', $key);
        }

        $query = $this->db->get();
        $config = $query->result();

        $ans = array();
        if (is_array($config) && !empty($config)) {
            foreach ($config as $conf) {
                $ans[$conf->name] = $conf->value;
            }
        } else {
            return false;
        }

        return $ans;
    }

    public function chek_config()
    {

        $values = $this->sql_get();

        $write = '';
        foreach ($values as $key => $value) {

            $write .= 'name: ' . $key . "\n value: " . $value . "  status:   translate: no   by name: \n\n\n";

        }
        file_put_contents("sql_config.txt", $write);


    }


    public function insert_to_config()
    {
        $params = $this->get();

        foreach ($params as $key => $value) {
            $par = [
                'index' => 'telepath-config',
                'type' => 'config',
                'id' => $key,
                'body' => [
                    "value" => $value
                ]
            ];
            $this->elasticClient->index($par);
        }
    }

    public function update($key, $value, $doc_as_upsert=false)
    {

        $params = [
            'index' => 'telepath-config',
            'type' => 'config',
            'id' => $key,
            'body' => [
                'doc' => [
                    'value' => $value
                ],
                'doc_as_upsert'=>$doc_as_upsert
            ]
        ];

        $this->elasticClient->update($params);

    }

    public function get_key($key)
    {
        $params = [
            'index' => 'telepath-config',
            'type' => 'config',
            'id' => $key,
        ];
        if ($this->elasticClient->exists($params)) {
            $response = $this->elasticClient->get($params);
            if (isset($response['_source']['value']) && !empty($response['_source']['value']))
                return $response['_source']['value'];
        }
        return false;
    }


    public function sql_update($key, $value)
    {



        /*$this->db->where('name', $key);
        $this->db->update($this->tableName, array('value' => $value));*/

    }

    public function insert($key, $value)
    {

        $this->db->insert($this->tableName, array('name' => $key, 'value' => $value));

    }

    public function sql_set_agents($agents)
    {

        // Empty, clear all records
        if (empty($agents)) {

            $this->db->where('1=1', null, false);
            $this->db->delete('agents');
            return;

        } else {

            $current_list = $this->get_agents();

            foreach ($agents as $id => $agent) {
                if ($agent['idx'] == NULL || empty($agent['idx'])) { // No IDX == NEW
                    unset($agent['idx']);
                    $this->db->insert('agents', $agent);
                    unset($agents[$id]);
                }
            }

            // Has items, compare to current
            foreach ($current_list as $current_agent) {

                foreach ($agents as $agent) {
                    $found = false;
                    if ($agent['idx'] == $current_agent->idx) {
                        // Update Task
                        $found = true;
                        $this->db->where('idx', $agent['idx']);
                        unset($agent['idx']);
                        $this->db->update('agents', $agent);
                    }
                    if (!$found) {
                        // Not found -- Remove Task
                        $this->db->where('idx', $current_agent->idx);
                        $this->db->delete('agents');
                    }

                }

            }

        }

    }

    public function sql_get_agents()
    {
        $this->db->select('idx, agent_id, FilterExpression, NetworkInterface');
        $this->db->from('agents');
        $query = $this->db->get();
        $result = $query->result();
        return $result;
    }

    public function set_agents($value)
    {


        $params = [
            'index' => 'telepath-config',
            'type' => 'interfaces',
            'id' => 'interface_id',
            'body' => [
                'doc' => [
                    'interfaces' => $value
                ]
            ]
        ];

        $this->elasticClient->update($params);
    }

    public function get_agents()
    {

        $params = [
            'index' => 'telepath-config',
            'type' => 'interfaces',
            'id' => 'interface_id'
        ];

        $result = $this->elasticClient->get($params);

        return $result['_source']['interfaces'];
    }


    public function sql_get_regex()
    {

        //    http://localhost:9200/telepath-config/filter_extensions/extensions_id

        $ans = array();
        $this->db->select('header_name, regex');
        $this->db->from('agents_regex');
        $query = $this->db->get();
        $result = $query->result();
        foreach ($result as $row) {
            $ans[$row->header_name] = $row->regex;
        }
        return $ans;
    }

    public function get_regex()
    {


        $params = [
            'index' => 'telepath-config',
            'type' => 'filter_extensions',
            'id' => 'extensions_id'
        ];


        $result = $this->elasticClient->get($params);

        return $result['_source']['filter_extensions'];

    }


    public function sql_set_regex($value)
    {
        $this->db->where('header_name', 'URL');
        $this->db->update('agents_regex', array('regex' => $value));
    }

    public function set_regex($value)
    {

        $params = [
            'index' => 'telepath-config',
            'type' => 'filter_extensions',
            'id' => 'extensions_id',
            'body' => [
                'doc' => [
                    'filter_extensions' => $value
                ]
            ]
        ];

        $this->elasticClient->update($params);

    }


    public function sql_get_scheduler($mode)
    {

        // Sanity Check
        $table = false;
        if ($mode == 'scheduler') {
            $table = 'scheduler';
        }
        if ($mode == 'report_scheduler') {
            $table = 'report_scheduler';
        }
        if (!$table) return;


        $this->db->from($table);
        $query = $this->db->get();
        $result = $query->result();
        foreach ($result as $row) {
            $row = (array)$row;
            $key = array_shift($row);
            $ans[$key] = $row;
        }

        return $ans;

    }

public function get_scheduler()
{

    $params = [
        'index' => 'telepath-scheduler',
        'type' => 'times',
        'body' => [
            'query' => [
                'match_all' => [

                ]
            ]
        ]
    ];

    $result = $this->elasticClient->search($params);

    return $result['hits']['hits'];
}

    public function set_scheduler($times)
    {


        $weekDay=array('Sunday'=>[],'Monday'=>[],'Tuesday'=>[],'Wednesday'=>[],'Thursday'=>[],'Friday'=>[],'Saturday'=>[]);

        $weekDay = array_merge($weekDay,$times);

        foreach ($weekDay as $day => $times) {

            $params = [
                'index' => 'telepath-scheduler',
                'type' => 'times',
                'id' => $day,
                'body' => [
                    'doc' => [
                        'times' => $times
                ]
            ]
            ];

            $this->elasticClient->update($params);

        }

    }
    public function sql_add_scheduler_event($mode, $event, $add = 1)
    {
        $table = false;
        if ($mode == 'scheduler') {
            $table = 'scheduler';
        }
        if ($mode == 'report_scheduler') {
            $table = 'report_scheduler';
        }
        if (!$table) return 1;

        if (!$event) return 2;
        $weekdays = array("Sat" => "Saturday", "Sun" => "Sunday", "Mon" => "Monday",
            "Tue" => "Tuesday", "Wed" => "Wednesday", "Thu" => "Thursday", "Fri" => "Friday");
        //$t = "Sun Jun 14 2015 16:00:00 GMT+0300 (Jerusalem Daylight Time)";
        $wday = $weekdays[substr($event, 0, 3)];
        $d = @date_parse($event);
        if (!$d) return 3;
        //$this->db->from($table);
        $this->db->where('day', $wday);
        $this->db->update($table, array(("n" . $d['hour']) => $add));
        return $event;
    }

    public function add_scheduler_event($mode, $event, $add = 1)
    {
        $table = false;
        if ($mode == 'scheduler') {
            $table = 'scheduler';
        }
        if ($mode == 'report_scheduler') {
            $table = 'report_scheduler';
        }
        if (!$table) return 1;

        if (!$event) return 2;
        $weekdays = array("Sat" => "Saturday", "Sun" => "Sunday", "Mon" => "Monday",
            "Tue" => "Tuesday", "Wed" => "Wednesday", "Thu" => "Thursday", "Fri" => "Friday");
        //$t = "Sun Jun 14 2015 16:00:00 GMT+0300 (Jerusalem Daylight Time)";
        $wday = $weekdays[substr($event, 0, 3)];
        $d = @date_parse($event);
        if (!$d) return 3;


        $this->db->from($table);
        $this->db->where('day', $wday);
        $this->db->update($table, array(("n" . $d['hour']) => $add));
        return $event;
    }

    public function sql_set_scheduler($mode, $data)
    {

        // Sanity Check
        $table = false;
        if ($mode == 'scheduler') {
            $table = 'scheduler';
        }
        if ($mode == 'report_scheduler') {
            $table = 'report_scheduler';
        }
        if (!$table) return;

        // VALIDATE DATA
        $weekday = array("Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday");
        $tmp = array();

        // Check All Days are present and no extra keys
        foreach ($data as $key => $value) {
            if (in_array($key, $weekday)) {
                $tmp[$key] = true;
            }
        }
        if (count($tmp) != 7) {
            return array('success' => false);
        }

        // Check All Hours are present
        foreach ($data as $key => $value) {
            if (count($value) != 24) {
                return array('success' => false);
            }
        }

        // Check All Hours are numeric
        foreach ($data as $key1 => $value1) {
            foreach ($value1 as $key2 => $value2) {
                if (intval($value) === 0 || intval($value) === 1) {
                    $data[$key1][$key2] = intval($value2);
                } else {
                    return array('success' => false);
                }
            }
        }

        // Done sanity check, lets do queries

        foreach ($data as $day => $schedule) { // SUN, MON..

            $this->db->where('day', $day);

            $updates = array();
            foreach ($schedule as $hour => $mode) { // 0 => 0, 1 => 0, 2 => 1 ...
                $updates[$hour] = $mode;
            }

            $this->db->update($table, $updates);

        }

        return array('success' => true);

    }

    public function check_file_loader_mode()
    {
        $params = [
            'index' => 'telepath-config',
            'type' => 'config',
            'id' => 'file_loader_mode_id'
        ];

        $result = $this->elasticClient->get($params);
        if ($result['_source']['value'] == "1")
            return true;
        else
            return false;
    }

}

?>
