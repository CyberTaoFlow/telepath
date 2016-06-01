<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Dashboard extends Tele_Controller
{

    function __construct()
    {
        parent::__construct();
        $this->load->model('M_Dashboard');
        $this->load->model('M_Suspects');

    }

    public function get_map()
    {

        telepath_auth(__CLASS__, __FUNCTION__);
        $range = $this->_get_range();
        $apps = $this->_get_apps();
        $map_mode = $this->M_Dashboard->get_map_mode(true);

        $map = $this->M_Dashboard->get_map($range, $apps, $map_mode);

        $data = array('map' => $map, 'map_mode'=>$map_mode);

        return_success($data);

    }

    public function get_chart()
    {

        telepath_auth(__CLASS__, __FUNCTION__);
        $range = $this->_get_range();
        $apps = $this->_get_apps();

        $suspect_threshold = $this->M_Suspects->get_threshold();

        $chart = $this->M_Dashboard->get_chart($range, $apps, $suspect_threshold);

        $data = array('chart' => $chart);

        return_success($data);

    }

    public function get_gap_score($learning=false)
    {
//        telepath_auth(__CLASS__, __FUNCTION__);
//        $range = $this->_get_range();
//        $apps = $this->_get_apps();
        if(isset ($learning) && $learning=='learning'){
            $learning=true;
        }
        else{
            $learning=false;
        }
        $data = $this->M_Dashboard->get_gap_score($learning);

        return_success($data);

    }



    public function get_suspects()
    {

        telepath_auth(__CLASS__, __FUNCTION__);
        $range = $this->_get_range();
        $apps = $this->_get_apps();

        $sort = $this->input->post('sort');
        $dir = $this->input->post('dir') == 'true' ? 'ASC' : 'DESC';

       // $suspects = $this->M_Dashboard->get_suspects($range, $apps, $sort, $dir);

        $suspect_threshold = $this->M_Suspects->get_threshold();
        $suspects = $this->M_Suspects->get($range, $apps, $sort, $dir, 0, 5, $suspect_threshold);



        $data = array('suspects' => $suspects);

        return_success($data);

    }

    public function get_alerts()
    {

        telepath_auth(__CLASS__, __FUNCTION__);
        $range = $this->_get_range();
        $apps = $this->_get_apps();

        $sort = $this->input->post('sort');
        $dir = $this->input->post('dir') == 'true' ? 'ASC' : 'DESC';

//        $alerts = $this->M_Dashboard->get_alerts($range, $apps, $sort, $dir);

        $this->load->model('M_Alerts');
        $alerts = $this->M_Alerts->get_alerts(/*false, false,*/ $sort, $dir, 0, 5, $range, $apps);

        $data = array('alerts' => $alerts);

        return_success($data);

    }

    public function get_cases()
    {

        telepath_auth(__CLASS__, __FUNCTION__);
        $range = $this->_get_range();
        $apps = $this->_get_apps();

//        $cases = $this->M_Dashboard->get_cases($range, $apps);

        $this->load->model('M_Cases');

        $all = $this->M_Cases->get_case_data('all');

        if(empty($all)){
            return_success();
        }

        $res0 = $this->M_Cases->get(100, $range, $apps);
        $cases = array();

        if (!isset($all[0])){
            $all=array($all);
        }
        foreach ($all as $tmp) {

            $found = false;
            foreach ($res0 as $case) {
                if ($case['name'] == $tmp['case_name']) {
                    $found = true;
                    $cases[] = $case;
                    break;
                }
            }

            if ($found == false) {
                $cases[] = array('name' => $tmp['case_name'], 'count' => 0, 'checkable' => false, 'case_data' => $tmp);
            }
        }
        # Make sure we have only 5 cases in the dashboard report, Yulli
        $result=[];
        foreach ($cases as $case){
            if (isset ($case['case_data']['favorite']) && $case['case_data']['favorite']){
                $result[]=$case;
            }
        }
        if (count($result)<5 && count($result)>0){
            foreach ($cases as $case){
                foreach ($result as $res) {
                    if ($case['name'] != $res['name'] && count($result) < 5) {
                        $result[] = $case;
                    }
                }
            }
        }
        elseif (count($result)==0){
            $result= array_slice($cases,0,5);
        }
        else{
            $result= array_slice($result,0,5);
        }


        $data = array('cases' => $result);

        return_success($data);

    }

    public function index()
    {

        // Mode is used for returning only map data

        $mode = $this->input->post('mode');
        if ($mode) {

            $range = $this->_get_range();
            $apps = $this->_get_apps();

            switch ($mode) {

                case 'map_alerts':

                    $this->M_Dashboard->set_map_mode('alerts', $range);
                    $map = $this->M_Dashboard->get_map($range, $apps, 'alerts');
                    return_success(array('map' => $map));

                    break;
                case 'map_traffic':

                    $this->M_Dashboard->set_map_mode('traffic',$range);
                    $traffic = $this->M_Dashboard->get_map($range, $apps, 'traffic');
                    return_success(array('traffic' => $traffic));

                    break;

            }

            return;
        }
        /*
        $hash1 = md5(serialize($range));
        $hash2 = md5(serialize($apps));
        $hash  = md5($hash1 . $hash2);

        $redisObj = new Redis();
        $redisObj->connect('localhost', '6379');
        $res = $redisObj->get('cache_dashboard_' . $hash);
        */
        /*
                if($res && $res != '') {
                    $data = json_decode($res);
                    if($data && !empty($data)) {
                        return_success($data);
                    }
                }
        */
        // Return everything ..

    }

}
