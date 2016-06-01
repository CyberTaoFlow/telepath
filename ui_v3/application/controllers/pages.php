<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Pages extends CI_Controller
{

    function __construct()
    {
        parent::__construct();
    }

    function set_page_alias()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $this->load->model('PagesModel');

        $page_id = $this->input->post('page_id', true);
        $page_alias = $this->input->post('page_alias', true);
        $this->PagesModel->page_update($page_id, array('title' => $page_alias));
        $page = $this->PagesModel->page_get($page_id);

        $page_name = $page->display_path;
        $page_name = $page_name != ' ' ? extract_page_name($page_name) : $page_id;

        $ans = array();
        $ans['success'] = true;
        $ans['page'] = array('text' => $page_name, 'page_name' => $page_name, 'page_id' => $page_id, 'title' => $page->title, 'path' => $page->display_path);

        return_json($ans);

    }


}