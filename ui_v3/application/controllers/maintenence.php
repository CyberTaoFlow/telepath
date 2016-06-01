<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Maintenence extends Tele_Controller
{

    public function __construct()
    {

        parent::__construct();

    }

    public function index()
    {
        $this->load->view('maintenence');
    }

    // Deletes all data in ElasticSearch
    public function clear()
    {

        $client = new Elasticsearch\Client();
        $res = $client->indices()->getMapping();

        foreach (array_keys($res) as $key) {
            $deleteParams = array('index' => $key);
            $client->indices()->delete($deleteParams);
        }

    }


}