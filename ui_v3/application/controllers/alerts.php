<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Alerts extends Tele_Controller
{

    function __construct()
    {
        parent::__construct();
    }

    public function index()
    {

        $this->load->model('M_Alerts');

        $sort = $this->input->post('sort');
        $dir = $this->input->post('dir') == 'true' ? 'ASC' : 'DESC';
        $search = $this->input->post('search');
        $filters = $this->input->post('filters');
        if ($search && substr($search, -1) != '*' && $search[0]!='"' && substr($search, -1) != '"' )
        {
            $search = str_replace('OR*','OR',str_replace('AND*','AND',str_replace(' ','* ',$search))) . '*';

        }
        $offset = intval($this->input->post('offset')) > 0 ? intval($this->input->post('offset')) : 0;

        if (!$sort || !in_array($sort, array('date', 'name', 'count', 'score'))) {
            $sort = 'date';
        }

        $range = $this->_get_range();
        $apps = $this->_get_apps();

        // Prep filter
//        $filter = array();

        // Alerts Data
        $alerts = $this->M_Alerts->get_alerts(/*false, false,*/ $sort, $dir, $offset, 15, /*$filter*/ $range, $apps, $search, $filters);

        if ($offset > 0) {
            // We need just the alert items
            return_json($alerts);
        }

        $time_chart = $this->M_Alerts->get_time_chart($range, $apps, $search, $filters);
        $distribution_chart = $this->M_Alerts->get_distribution_chart($range, $apps, $search);
        $action_distribution_chart = $this->M_Alerts->get_action_distribution_chart($range, $apps, $search, $filters);


        if ($filters){
            foreach($distribution_chart as $key => $dis){
                if (!in_array($dis['label'], $filters)){
                    $distribution_chart[$key]['data']=0;
                }
            }
        }


        return_success(
            array(
                'alerts' => $alerts,
                'time_chart' => $time_chart,
                'distribution_chart' => $distribution_chart,
                'action_distribution_chart' => $action_distribution_chart
            )
        );

    }

}
