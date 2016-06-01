<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Permissions extends CI_Controller
{

    function __construct()
    {
        parent::__construct();
    }

    function get_list()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $master_list = $this->acl->get_master_list();
        $data = $this->db->get('ci_perm_data')->result();

        // Initialize master list
        if (count($data) < count($master_list)) {

            foreach ($master_list as $c => $m) {

                // Clear previous class method instances, if any
                $this->db->where('class', $c)->delete('ci_perm_data');

                // Insert new
                foreach ($m as $k => $d) {
                    $data = array('class' => $c, 'function' => $k, 'alias' => $c . '::' . $k, 'description' => $d);
                    $this->db->insert('ci_perm_data', $data);
                }
            }

            // Reload
            $data = $this->db->get('ci_perm_data')->result();

        }

        $ans = array();
        foreach ($data as $perm) {
            $perm = (array)$perm;
            $ans[] = array('id' => $perm['id'],
                'class' => $perm['class'],
                'function' => $perm['function'],
                'alias' => $perm['alias'],
                'description' => $perm['description'],
            );
        }

        return_success($ans);

    }

    function get_permission()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $id = $this->input->post('id');
        if (intval($id) == 0) {
            return_fail('Bad permission_id');
        }

        $data = $this->db->where('id', $id)->get('ci_perm_data')->result();
        $ans = array();

        if (!$data || empty($data)) {
            return_fail('Bad permission_id');
        }

        $perm = (array)$data[0];
        $perm = array('id' => $perm['id'],
            'class' => $perm['class'],
            'function' => $perm['function'],
            'alias' => $perm['alias'],
            'description' => $perm['description'],
            'params' => $perm['params']
        );

        if ($perm['params'] == '') {
            $perm['params'] = '{}';
        };

        // Extract applications
        $decode = json_decode($perm['params'], true);
        $applications = isset($decode['applications']) ? $decode['applications'] : array();
        if (isset($decode['applications'])) {
            unset($decode['applications']);
        }
        $perm['params'] = json_encode($decode);

        return_success(array('permission' => $perm, 'applications' => array()));

    }

    function set_permission()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $post = $this->input->post();

        $data_raw = $post['items']['permission'];
        $data = array();

        // Copy only actual


        $fields = array('id', 'class', 'function', 'alias', 'description', 'params');
        foreach ($fields as $field) {
            if (isset($data_raw[$field])) {
                $data[$field] = $data_raw[$field];
            }
        }

        $permission_id = $data['id'];
        unset($data['id']);

        if ($permission_id == 'new') { // Create

            $this->db->insert('ci_perm_data', $data);
            $permission_id = $this->db->insert_id();

        } else { // Update

            $this->db->where('id', $permission_id)->update('ci_perm_data', $data);

        }

        return_success(array('permission_id' => $permission_id));

    }

    public function del_permission()
    {

        $id = $this->input->post('id');

        // Clear perm set for groups
        $this->db->where('perm_id', $id)->delete('ci_group_perm');

        // Clear perm set for users
        $this->db->where('perm_id', $id)->delete('ci_user_perm');

        // Clear perm data
        $this->db->where('id', $id)->delete('ci_perm_data');

        return_success();

    }

}