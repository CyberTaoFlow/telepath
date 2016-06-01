<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Suspects extends Tele_Controller
{

    public function index()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $range = $this->_get_range();
        $apps = $this->_get_apps();

        $search = $this->input->post('search');
        if ($search && substr($search, -1) != '*' && $search[0]!='"' && substr($search, -1) != '"' )
        {
            $search = str_replace('OR*','OR',str_replace('AND*','AND',str_replace(' ','* ',$search))) . '*';

        }
        $sort = $this->input->post('sort');
        $dir = $this->input->post('dir') == 'true' ? 'ASC' : 'DESC';
        $offset = intval($this->input->post('offset')) > 0 ? intval($this->input->post('offset')) : 0;

        if (!$sort || !in_array($sort, array('date', 'count', 'alerts'))) {
            $sort = 'date';
        }

        $this->load->model('M_Suspects');
        $suspect_threshold = $this->M_Suspects->get_threshold();
        return_json($this->M_Suspects->get($range, $apps, $sort, $dir, $offset, 15, $suspect_threshold, $search));

    }

    public function get_avg_score()
    {

        $key = $this->input->post('key');
        $val = $this->input->post('value');

        switch ($key) {
            case 'SID':
            case 'user_ip':
            case 'RID':

                $this->load->model('RequestScores');
                $scores = $this->RequestScores->get_avg_scores($key, $val);
                return_success($scores);

            default:
                return_fail('Missing key');
                break;
        }


    }

}
