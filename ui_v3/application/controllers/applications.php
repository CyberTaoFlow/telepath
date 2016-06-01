<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Applications extends Tele_Controller
{

    function __construct()
    {

        parent::__construct();
        $this->load->model('M_Applications');
        $this->load->model('M_Actions');

    }

    public function get_subdomain_autocomplete()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $text = $this->input->post('text', true);
        $this->load->model('M_Applications');
        return_success($this->M_Applications->get_subdomain_autocomplete($text));

    }


    //not_used
    public function get_index()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->load->model('M_Applications');
        return_success($this->M_Applications->get_index());
    }

    public function get_expand()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $search = $this->input->post('search');
        $actions = $this->input->post('actions');
        $learning_so_far = $this->input->post('learning_so_far');
        $sort = $this->input->post('sort');
        $dir = $this->input->post('dir') == 'true' ? 'asc' : 'desc';

        if (!$sort || !in_array($sort, array('name', 'count')) || $sort == 'name') {
            $sort = 'host';
            $dir = 'asc';
        }
        else{
            $sort="learning_so_far";
        }

//        $res = $this->redisObj->get('cache_applications');

//        if (isset($res) && $res) {
//            $data = json_decode($res);
//            if ($data && !empty($data)) {
//                return_success($data);
//            }
//        }

        // retrieve the apps
        $data = $this->M_Applications->index($search, $learning_so_far, $sort, $dir);

        // search in business actions
        if ($search && $actions){
           $actions = $this->M_Actions->search_actions($search);
            foreach($actions as $action){
                //TODO: add option to add an action to a subdomain
               //$root_domain= $this->M_Applications->get_root_domain($action['application']);
                // if the domain already exists, we add the action to this domain
                if($key=array_search($action['application'],array_column($data,'host'))){
                    $data[$key]['actions'][]=$action;
                }
//                elseif($key=array_search($root_domain,array_column($data,'subdomains'))){
//                    $data[$key]['subdomains']['actions'][]=$action['action_name'];
//                    $data[$key]['subdomains']['host']=$action['application'];
//                }
                else{
                    $data[]=['host'=>$action['application'], 'actions'=>[$action]];
                }

            }
        }

//        $this->redisObj->set('cache_applications', json_encode($data), 600);

        return_success($data);

    }

    public function get_search()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $search = $this->input->post('search');
        $mode = $this->input->post('mode');

        return_success($this->M_Applications->get_search($search,$mode));

    }

    public function get_page()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $host = $this->input->post('host');
        $path = $this->input->post('path');
        $mode = $this->input->post('mode');

        return_success($this->M_Applications->get_page($host, $path, $mode));

    }

    public function get_app_pages(){
        telepath_auth(__CLASS__, __FUNCTION__);

        $host = $this->input->post('host');

        return_success($this->M_Applications->get_app_pages($host));
    }

    public function get_app()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $host = $this->input->post('host', true);
        $context = $this->input->post('context', true);

        $app = $this->M_Applications->get($host);

        if (!$app) $app = array();



        if ($context == 'actions') {

            $this->load->model('M_Actions');
            $actions = $this->M_Actions->get_actions($host);
            $app['actions'] = $actions;

            if (!isset($app['action_categories'])) {
                $app['action_categories'] = array();
            }

        }

        return_success($app);

    }

    public function set_application_alias()
    {

        $app_id = $this->input->post('app_id', true);
        $app_alias = $this->input->post('app_alias', true);
        $this->Apps->app_update($app_id, array('display_name' => $app_alias));
        $app = $this->Apps->app_get($app_id);

        return_json(array('success' => true, 'app' => $app[0]));

    }


    // Updates Application
    public function set_app()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $data = $this->input->post();
        $data['host'] = str_replace(array('http://', 'https://'), array('', ''), strtolower($data['host']));
	if(empty($data['subdomains'])) {
		$data['subdomains']=[];
	}
        $this->M_Applications->set($data);

        $this->load->model('M_Config');
        $this->M_Config->update('app_list_was_changed_id',$data['host']);

        // REWRITE OUR NGINX.CONF
        $this->load->model('M_Nginx');
        $conf = $this->M_Nginx->gen_config();

        $logfile = $this->config->item('telepath_ui_log');

        file_put_contents($logfile, $conf);

        return_success();

    }

    public function get_next_id()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);
        return_json(array('success' => true, 'app_id' => $this->Apps->get_last_id() + 1));

    }

    public function set_ssl_certificate()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->_set_ssl('certificate');
    }

    public function set_ssl_private_key()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->_set_ssl('private_key');
    }

    private function _set_ssl($mode)
    {

        $app_id = $this->input->post('app_id', true);
        $waitMsg = $this->input->post('waitMsg', true);
        $file_name = $this->input->get('file_name', true);

        if ($mode != 'certificate' && $mode != 'private_key') {
            echo json_encode(array('success' => false, 'error' => 'No such certificate upload mode'));
            die;
        }
        if (!$app_id) {
            echo json_encode(array('success' => false, 'error' => 'No application was selected, please try again'));
            die;
        }
        if (!isset($_FILES) || !isset($_FILES['file'])) {
            echo json_encode(array('success' => false, 'error' => 'No file was uploaded'));
            die;
        }
        if ($_FILES['file']['error'] > 0) {
            echo json_encode(array('success' => false, 'error' => 'There was an error during upload'));
            die;
        }
        if ($_FILES['file']['size'] > 1048576) {
            echo json_encode(array('success' => false, 'error' => 'File size cannot exceed 1MB'));
            die;
        }

        $file_name = $_FILES['file']['name'];
        $file_data = file_get_contents($_FILES['file']['tmp_name']);

        $this->Apps->set_certificate($mode, $app_id, $file_name, $file_data);

        // Notice 'true', not true;
        return_json(array('success' => 'true', 'file' => $file_name));
    }

    public function get_ip_suggestion()
    {
        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $app_id = $this->input->post('app_id', true);

        if (!$app_id) {
            return_fail('No App ID specified');
        }
        $data = $this->M_Applications->get_ip_suggestion($app_id);
        return return_success($data);
    }

    public function get_cookie_suggestion()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $default = array('success' => true, 'items' => array());
        $default['items'][] = array('cookie' => 'PHPSESSID');
        $default['items'][] = array('cookie' => 'PHPSESSIONID');
        $default['items'][] = array('cookie' => 'JSESSIONID');
        $default['items'][] = array('cookie' => 'ASPSESSIONID');
        $default['items'][] = array('cookie' => 'ASP.NET_SessionId');
        $default['items'][] = array('cookie' => 'VisitorID');
        $default['items'][] = array('cookie' => 'SESS');
        $default['total'] = count($default['items']);

        $suggest = array('success' => true, 'items' => array());

        $app_id = $this->input->get('app_id', true);

        /*
                // THIS CODE IS NOT WORKING (Yuli)
                $cookies = $this->Apps->get_cookie_suggestion($app_id);
                if(!empty($cookies)) {
                    $cookies = $cookies[0]->cookie_suggestion;
                    if(strlen($cookies) > 0) {
                        $cookies = explode(',', $cookies);
                        foreach($cookies as $cookie) {
                            $suggest['items'][] = array('cookie' => $cookie);
                        }
                    }
                }
        */

        $suggest['total'] = count($suggest['items']);
        if ($suggest['total'] > 0) {
            return_json($suggest);
        } else {
            return_json($default);
        }

    }

    public function del_app()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $app_id = $this->input->post('app_id', true);

        if (!$app_id) {
            return_fail('No App ID specified');
        }

        // if(in_array($app_id, $this->acl->allowed_apps) || $this->acl->all_apps())
        //$result = $this->Apps->app_delete($app_id);

        // remove it from elastic search
        $this->M_Applications->delete($app_id);

        // REWRITE OUR NGINX.CONF
        $this->load->model('M_Nginx');
        $conf = $this->M_Nginx->gen_config();
        $logfile = $this->config->item('telepath_ui_log');

        file_put_contents($logfile, $conf);

        return_success();

    }

    // Just an alias
    public function get_list()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $apps = $this->Apps->index('app_domain', 'asc');
        $ans = array(array('id' => -1, 'domain' => 'All', 'display' => 'All'));

        foreach ($apps as $app) {
            $ans[] = array(
                'id' => intval($app->app_id),
                'domain' => $app->app_domain,
                'display' => $app->display_name
            );
        }

        return_success($ans);

    }

    public function get_apps_combobox_general()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->_get_apps_combobox();
    }

    public function get_apps_combobox_general_without_all()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->_get_apps_combobox(false);
    }

    private function _get_apps_combobox($with_all = true)
    {

        $sortorder = $this->input->get('sortorder', true);
        $sortfield = $this->input->get('sortfield', true);
        $sort = $this->input->get('sort', true);
        $sort = json_decode($sort, true);

        if ($sort && is_array($sort) && !empty($sort)) {
            $sortfield = $sort[0]['property'];
            $sortorder = $sort[0]['direction'];
        }

        $sortfield = $this->_i2c($sortfield);

        $this->load->model('Apps');

        if (!$sortfield) {
            $sortfield = 'app_domain';
        }
        if (!$sortorder) {
            $sortorder = 'asc';
        }

        $apps = $this->Apps->index($sortfield, $sortorder);

        $ans = array();

        if ($with_all) {
            $ans[] = array(
                'id' => '-1',
                'app' => 'All',
                'ssl' => 0
            );
        }
        foreach ($apps as $app) {
            $ans[] = array(
                'id' => intval($app->app_id),
                'app' => $app->app_domain,
                'ssl' => intval($app->ssl_flag)
            );
        }

        return_success($ans);

    }

    private function _i2c($i)
    {

        $i_to_c = array(
            "td0" => 'app_id',
            "td1" => 'app_domain',
            "td2" => 'display_name',
            "td3" => 'login_att_id',
            "td4" => 'logged_condition',
            "td5" => 'condition_value',
            "td6" => 'logout_page_id',
            "td7" => 'logout_att_id',
            "td8" => 'logout_att_value',
            "td9" => 'AppCookieName',
            "td10" => 'cpt_name',
            "td11" => 'cpt_val',
            "td12" => 'ntlm',
            "td13" => 'global_per_app',
            "td14" => 'exclude_group_headers',
            "td15" => 'global_pages',
            "td16" => 'certificate',
            "td17" => 'private_key',
            "td18" => 'ssl_flag',
            "td19" => 'ssl_server_port',
            "td20" => 'app_ips',
            "td21" => 'ssl_certificate_password',
            "td22" => 'cpt_injected_header_name',
            "td23" => 'basic_flag',
            "td24" => 'digest_flag',
            "td25" => 'form_flag',
            "td26" => 'form_param_id',
            "td27" => 'form_param_name',
            "td28" => 'form_authentication_flag',
            "td29" => 'form_authentication_cookie_flag',
            "td30" => 'form_authentication_redirect_flag',
            "td31" => 'form_authentication_redirect_page_id',
            "td32" => 'form_authentication_redirect_page_name',
            "td33" => 'form_authentication_redirect_response_range',
            "td34" => 'form_authentication_body_flag',
            "td35" => 'form_authentication_body_value',
            "td36" => 'form_authentication_cookie_name',
            "td37" => 'form_authentication_cookie_value',
            "td38" => 'cookie_suggestion',
            "td39" => 'certificate_path',
            "td40" => 'private_key_path'
        );

        if (!in_array($i, array_values($i_to_c))) {

            if (isset($i_to_c[$i])) {
                return $i_to_c[$i];
            } else {
                return 'app_id';
            }

        } else {

            return $i;

        }

    }

}
