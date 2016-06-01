<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Parameters extends Tele_Controller
{

    function __construct()
    {
        parent::__construct();
        // Init Model
        $this->load->model('M_Params');

    }

    function get_global_headers()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $this->load->model('M_Globalheaders');
        $headers = $this->M_Globalheaders->get_global_headers();

        $ans = array('items' => array(), 'success' => true);

        foreach ($headers as $header) {
            $ans['items'][] = $header;
        }
        $ans['items'][] = 'user-agent';
        sort($ans['items']);
        return_json($ans);

    }

    function set_parameter_alias()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $this->load->model('M_Params');

        $att_id = $this->input->post('att_id', true);
        $att_alias = $this->input->post('att_alias', true);
        $this->M_Params->att_update($att_id, array('att_alias' => $att_alias));
        $param = $this->M_Params->att_get($att_id);
        return_json(array('success' => true, 'param' => $param[0]));

    }

    function set_parameter_config()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $this->load->model('M_Params');

        $att_id = $this->input->post('att_id', true);
        $att_mask = $this->input->post('att_mask', true) == '0' ? '0' : '1';
        $this->M_Params->att_update($att_id, array('mask' => $att_mask));
        $param = $this->M_Params->att_get($att_id);
        return_json(array('success' => true, 'param' => $param[0]));

    }

}