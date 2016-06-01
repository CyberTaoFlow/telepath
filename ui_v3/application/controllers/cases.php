<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Cases extends Tele_Controller
{

    // Load Cases Model
    function __construct()
    {

        parent::__construct();
        $this->load->model('M_Cases');
        $this->load->model('M_Config');

    }

    // Get ALL
    public function get_cases()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $sort = $this->input->post('sort');
        $dir = $this->input->post('dir') == 'true' ? true : false;
        $search = $this->input->post('search');

        if (!$sort || !in_array($sort, array('date', 'name', 'count'))) {
            $sort = 'date';
        }

        $range = $this->_get_range();
        $apps = $this->_get_apps();

        telepath_auth(__CLASS__, __FUNCTION__);


        if (!$search) {
            $search = 'all';
        }
        $all = $this->M_Cases->get_case_data($search);

        if (empty($all)) {
            return_success();
        }

        $res0 = $this->M_Cases->get(100, $range, $apps, $search);
        $res = array();

        if (!isset($all[0])) {
            $all = array($all);
        }
        foreach ($all as $tmp) {

            $found = false;
            foreach ($res0 as $case) {
                if ($case['name'] == $tmp['case_name']) {
                    $found = true;
                    $res[] = $case;
                    break;
                }
            }

            if ($found == false) {
                $res[] = array('name' => $tmp['case_name'], 'count' => 0, 'case_data' => $tmp, 'checkable' => false, 'last_time' => false);
            }
        }

        // sort with php array function because an empty case is not within the result of the elastic query ($res0).
        switch ($sort) {
            case 'date':
                if ($dir) {
                    usort($res, function ($a, $b) {
                        return $a['last_time'] - $b['last_time'];
                    });
                } else {
                    usort($res, function ($a, $b) {
                        return $b['last_time'] - $a['last_time'];
                    });
                }
                break;
            case 'count':
                if ($dir) {
                    usort($res, function ($a, $b) {
                        return $a['count'] - $b['count'];
                    });
                } else {
                    usort($res, function ($a, $b) {
                        return $b['count'] - $a['count'];
                    });
                }
                break;
            case 'name':
                if ($dir) {
                    usort($res, function ($a, $b) {
                        return strcmp($a["name"], $b["name"]);
                    });
                } else {
                    usort($res, function ($a, $b) {
                        return strcmp($b["name"], $a["name"]);
                    });
                }
                break;

        }

        return_success($res);

    }

    public function get_case_stat()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

//        $now = $this->db->query('SELECT UNIX_TIMESTAMP() as ts')->row_array();
//        $now = $now['ts'];

        $cid = $this->input->post('cid');
        // 7 day range
//        $range = array(
//            'start' => $now - (3600 * 24 * 7),
//            'end' => $now
//        );

        $range = $this->_get_range();
        $apps = $this->_get_apps();

        $chart = $this->M_Cases->get_case_sessions_chart($range, $apps, $cid);
        return_success($chart);

    }

    // Get Specific
    public function get_case()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $this->load->model('M_Alerts');

        // Collect Input
        $cid = $this->input->post('cid');
        $sort = $this->input->post('sort');
        $dir = $this->input->post('dir') == 'true' ? 'ASC' : 'DESC';
       // $case_data= $this->input->post('case_data');

        if (!$sort || !in_array($sort, array('date', 'type', 'count'))) {
            $sort = 'date';
        }

        $range = $this->_get_range();
        $apps = $this->_get_apps();

        // Done

        // CASE DATA
        // $case = $this->M_Cases->get_case(array('id' => $cid));

        // Contains requests data to display
        $ans = array();

        $requests = $this->M_Cases->get_case_sessions(100, $cid, $range, $apps, $sort, $dir);
        $similars = $this->M_Cases->get_similar_case_sessions($cid);
       // $requests = $this->M_Cases->new_get_case_sessions(100, $range, $apps, $case_data);

        unset($requests['requests']);

        $ans = array_merge($ans, $requests);
        $ans['chart'] = $this->M_Cases->get_case_sessions_chart($range, $apps, $cid);
        $ans['case'] = array('case_data' => $this->M_Cases->get_case_data($cid));
        $ans['similars']=$similars;

        return_json($ans);

    }

    public function store_similar_case_sessions($cases = [])
    {
        telepath_auth(__CLASS__, __FUNCTION__);

        if (empty ($cases)) {
            logger('Start','/var/log/store_similar_case_sessions.log');
            $cases = [];
            $cases_data = $this->M_Cases->get_case_data('all');
            foreach ($cases_data as $case) {
                $cases[] = $case['case_name'];
            }
        }

        foreach ($cases as $cid) {
            $requests = $this->M_Cases->get_case_sessions(100, $cid);
            if (!empty($requests['requests'])) {
                $similars = $this->M_Cases->get_similar_sessions($requests['requests'], $cid);
                $this->M_Cases->store_similar_case_sessions($similars, $cid);
                logger('Finish to update similar sessions for case: '.$cid);
            }
        }

        logger('End of the process');

    }

    // Set (Update) Case favorite flag
    public function set_case_fav()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $cid = $this->input->post('cid');
        $fav = $this->input->post('favorite') == 'true';
        $this->M_Cases->update($cid,false,false,$fav);
       // $res = $this->M_Cases->get(array('id' => $cid));
        $res = $this->M_Cases->get_case_data($cid);
        return_success($res);

    }

    // Set (Update) Case
    public function set_case()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $range = $this->_get_range();
        $apps = $this->_get_apps();

        $name = $this->input->post('name');
        $case = $this->input->post('case_data');

        $this->M_Cases->update($name, json_decode($case, true));

        //not used
//        $this->M_Config->update('case_list_was_changed', 1);

        // Return
        return_success();

    }

    // Add (Create) Case
    public function add_case()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        // Collect info
        $name = $this->input->post('name');
        $case = $this->input->post('case_data');

        // Validate
        $tmp = $this->M_Cases->get_case_data($name);
        if (isset($tmp['empty']) && !$tmp['empty'] ) {
            return_success(array('existing' => true));
        }

        // Create
        $this->M_Cases->create($name, json_decode($case, true));
        //not used
//        $this->M_Config->update('case_list_was_changed', 1);

        // Return
        return_success();

    }

    // Delete Case
    public function del_cases()
    {

        telepath_auth(__CLASS__, __FUNCTION__);
        $cids = $this->input->post('cids');

        // Cast to array in case of 1 item
        if (!is_array($cids)) {
            $cids = array($cids);
        }

        $this->M_Cases->delete($cids);

        foreach ($cids as $cid) {
            $this->M_Cases->delete_similar_case_sessions($cid);
        }
        //not used
//        $this->M_Config->update('case_list_was_changed', 1);

        // Return updated cases list
        $this->get_cases();

    }

    public function flag_requests_by_cases(){

        if ($this->input->post('case')) {
            $cases = $this->input->post('case');
        } else {
            $cases = 'all';
        }

        if ($this->input->post('range')) {
            $range = $this->input->post('range')=='true';
        } else {
            $range = true;
        }

        if ($this->input->post('method')) {
            $method = $this->input->post('method');
        } else {
            $method = 'add';
        }

        if($cases!='all' && $method != 'delete' && !$range){
            register_shutdown_function([$this->M_Cases, 'remove_update_flag'],$cases[0]);
        }


        $this->M_Cases->flag_requests_by_cases($cases,$range, $method);

        if (!$range && $method!='delete'){
            $this->store_similar_case_sessions($cases);
        }

        if(!$range){
            return_success();
        }

    }

}
