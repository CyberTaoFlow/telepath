<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

/**
 * @property  M_Config M_Config
 */
class Actions extends Tele_Controller
{

    public function __construct()
    {

        parent::__construct();
        require 'vendor/autoload.php';
        $this->load->model('M_Actions');
        //$this->client = new Elasticsearch\Client();
        //$params = array('hosts' => array('127.0.0.1:9200'));
#$params = array();
#$params['logging'] = true;
#$params['logPath'] = '/tmp/elasticsearch.log';
        $this->client = new Elasticsearch\Client();

    }

    public function set_delete_action()
    {

        // telepath_auth(__CLASS__, 'set_action');

        $host = $this->input->post('application');
        $action = $this->input->post('action');
        $uid = $this->input->post('uid');

       $this->M_Actions->set_delete_action($uid);

        $this->load->model('M_Config');
        $this->M_Config->update('business_flow_was_changed_id', '1');

        return_success($uid);

    }

    public function get_action_autocomplete()
    {

        telepath_auth(__CLASS__, 'get_action');

        $text = $this->input->post('text', true);
        $text = str_replace(":", "", $text);
        $text = trim($text);

        return $this->M_Actions->get_action_autocomplete($text);


    }

    public function set_clear_actions()
    {

        telepath_auth(__CLASS__, 'set_action');


    }

    public function get_app_actions()
    {

        telepath_auth(__CLASS__, 'get_action');

        $ret = array();
        $host = $this->input->post('host');
        $domain=$this->input->post('domain');

        $actions=$this->M_Actions->get_app_actions($host);


        if($domain=='root'){
        $this->load->model('M_Applications');
            $subdomains=$this->M_Applications->get_subdomains($host);
            }
        if(!isset($subdomains) || !$subdomains){
            $subdomains=[];
        }

        return_success(array('actions'=>$actions,'subdomains'=>$subdomains));
    }


    public function set_flow()
    {

        telepath_auth(__CLASS__, 'set_action');

        $app = $this->input->post('app');
        $name = $this->input->post('flow_name');
        $data = $this->input->post('json');
        $data = json_decode($data);

        $this->M_Actions->set_flow($name,$app,$data);

        $this->load->model('M_Config');
        $this->M_Config->update('business_flow_was_changed_id', '1');

        return_success();

    }

    public function _hybridrecord_to_sid($value, $host)
    {

        return $this->M_Actions->_hybridrecord_to_sid($value,$host);

    }

    public function get_requests()
    {

        telepath_auth(__CLASS__, 'get_action');

        // Mode, either IP, SID, PARAM or USER
        $mode = $this->input->post('mode');
        // The value for the mode
        $value = $this->input->post('value');
        // Host on which to track, can be black for cross host sessions
        $host = $this->input->post('host');
        // Offset timestamp - only return requests with timestamp greater than supplied (only new requests)
        $offset = $this->input->post('offset');
        // If no offset was provided assign 0 to keep query working

        // When this flag is set only return TS of last request
        $lockon = ($this->input->post('lockon') == 'true') ? true : false;

        return $this->M_Actions->get_requests($mode, $value, $host, $offset, $lockon);

    }

    public function get_suggest()
    {

        telepath_auth(__CLASS__, 'get_action');

        $host = $this->input->post('host');
        $mode = $this->input->post('mode');

        $sessions = $this->M_Actions->__get_active_sessions($host);
        $res = array();

        switch ($mode) {
            case 'IP':

                // De-Dupe with unique keys
                foreach ($sessions as $session) {
                    $res[$session['ip_orig']] = true;
                }
                // Get keys
                $res = array_keys($res);

                break;

            case 'SID':

                // Javascript doesnt like arrays with numeric keys, it thinks its a really large array
                foreach ($sessions as $session) {
                    $res[] = $session['sid'] . '';
                }

                break;

            case 'user':
                // TODO::
                break;
        }

        return_success($res);

    }

    // Track specific session
    public function track_session_by_sid()
    {

        $sid = $this->input->post('sid'); // at this sid
        $offset = $this->input->post('time'); // starting this stamp

        return_success($result);

    }

//    public function get_app_with_actions()
//    {
//
//        telepath_auth(__CLASS__, __FUNCTION__, $this);
//
//        $search = $this->input->post('search');
//
////        $res = $this->redisObj->get('cache_applications');
//
//        /*if (isset($res) && $res) {
//            $data = json_decode($res);
//            if ($data && !empty($data)) {
//                return_success($data);
//            }
//        }*/
//
//        if ($search){
//
//            $data=$this->M_Actions->get_app_with_actions($search);
//
//        }
//
//        else{
//
//            $data = $this->M_Applications->index($search);
//
//        }
//
//
//        $this->redisObj->set('cache_applications', json_encode($data), 600);
//
//        return_success($data);
//
//    }

}
