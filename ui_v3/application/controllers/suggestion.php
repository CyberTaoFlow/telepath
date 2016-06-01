<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Suggestion extends Tele_Controller
{

    public function get()
    {
        print_r($this->db);
        $rid = $this->input->post('rid');
        if ($rid) {
            echo '1';
            $rid = intval($rid);
            $query = $this->db->query('CALL get_recommended_request_ids(' . $rid . ');');
            $result = $query->result();
            echo $this->db->last_query();
            print_r($result);
            return_json($result);
        }

    }

}
