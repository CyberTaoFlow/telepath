<?php
if (!defined('BASEPATH')) exit('No direct script access allowed');

class Notifications extends Tele_Controller
{

    function __construct()
    {
        parent::__construct();

//        $params = array();
//        $params['hosts'] = array('127.0.0.1:9200');

        $this->elasticClient = new Elasticsearch\Client();


    }

    public function get_gap_cases($range, $apps = array())
    {

        $result = array('case' => 0, 'noncase' => 0);

        $params['body'] = array(
            'size' => 0,
            'aggs' => array(
                'sid' => array(
                    "cardinality" => [
                        "field" => "sid",
                    ]
                )
            ),
            'query' => [
                'bool' => [
                    'must' => [
                        ['term' => ['_type' => 'http']],
                        ['range' => ['ts' => ['gte' => intval($range['start']), 'lte' => intval($range['end'])]]],
                        ['filtered' => ['filter' => ['exists' => ['field' => 'cases_name']]]],
                    ]
                ]
            ]
        );

        $results = $this->elasticClient->search($params);
        if (!empty($results) && isset($results['aggregations']['sid'])) {
            return $results['aggregations']['sid']['value'];
        }

        return $result;

    }

    public function db_time()
    {
        $result = $this->db->query('SELECT UNIX_TIMESTAMP() as time')->result();
        return_success($result);
    }

    public function get_indexes()
    {

        // Top Suspects
        $query = array('body' => ['size' => 0, 'query' => ['bool' => ['must' => [['term' => ['_type' => 'http']]]]]]);
        $res = $this->elasticClient->search($query);
        $requests = intval($res['hits']['total']);
        $query['body']['query']['bool']['must'][] = ['range' => ['alerts_count' => ['gte' => 1]]];
        $res = $this->elasticClient->search($query);
        $alerts = intval($res['hits']['total']);

        $result = array(
            'alerts' => $alerts,
            'requests' => $requests,
            /*
            'case_alerts' 	 => $this->db->count_all_results('case_alerts') + 0 ,
            'applications'   => $this->db->count_all_results('applications') + 0,
            'top_suspects'   => $top_suspects + 0,
            'pages' 		 => $this->db->count_all_results('pages') + 0,
            */
        );

        return_success($result);

    }

    public function get_syslog()
    {

        $redisObj = new Redis();
        $redisObj->connect('localhost', '6379');

        $i = 0;
        $ans = [];
        while (true) {

            $i++;
            $res = $redisObj->lpop('U');

            if (!$res || $i > 10) {
                break;
            }

            $ans[] = $res;

        }

        return_success($ans);

    }

    public function index()
    {

    }


}

?>
