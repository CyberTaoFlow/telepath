<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Groups extends CI_Controller
{

    function __construct()
    {
        parent::__construct();
    }

    function get_list()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $groups = $this->ion_auth->groups()->result();
        $ans = array();
        foreach ($groups as $group) {
            $ans[] = array('id' => $group->id,
                'name' => $group->name,
                'description' => $group->description);
        }

        return_success($ans);

    }

    function get_group()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $id = $this->input->post('id');
        if (intval($id) == 0) {
            return_fail('Bad group_id');
        }

        $group = $this->ion_auth->group($id)->result();
        if (!$group || empty($group)) {
            return_fail('Bad group_id');
        }

        $group = $group[0];

        $extradata = json_decode($group->extradata, true);
        $group_apps = isset($extradata['apps']) ? $extradata['apps'] : array();
        $group_ranges = isset($extradata['ranges']) ? $extradata['ranges'] : array();

        $group = array('id' => $group->id,
            'name' => $group->name,
            'description' => $group->description);

        $users_raw = $this->ion_auth->users($id)->result();
        $users_clean = array();

        foreach ($users_raw as $user) {
            $users_clean[] = array('id' => $user->id);
        }

        $perm = $this->acl->get_group_perm(array($id));

        return_success(array('group' => $group, 'users' => $users_clean, 'perm' => $perm, 'apps' => $group_apps, 'ranges' => $group_ranges));

    }

    function add_group()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $post = $this->input->post();

        $data = $post['items']['group'];
        $group_id = $data['id'];
        $users_new = isset($post['items']['users']) ? $post['items']['users'] : array();
        $perm_new = isset($post['items']['perm']) ? $post['items']['perm'] : array();

        $apps = isset($post['items']['apps']) ? $post['items']['apps'] : array();
        $ranges = isset($post['items']['ranges']) ? $post['items']['ranges'] : array();


        unset($data['id']);

        if ($group_id == 'new') {

            if (!isset($data['description'])) {
                $data['description'] = '';
            }

            $group_id = $this->ion_auth->create_group($data['name'], $data['description'], array('extradata' => json_encode(array('apps' => $apps, 'ranges' => $ranges))));

            if (!$group_id) {
                return_fail($this->ion_auth->errors());
            }

        }

        if ($users_new && !empty($users_new)) {
            foreach ($users_new as $user_id) {
                $this->ion_auth->add_to_group($group_id, $user_id);
            }
        }

        $this->acl->clear_group_perm($group_id);

        if ($perm_new && !empty($perm_new)) {
            foreach ($perm_new as $perm_id) {
                $this->acl->set_group_perm($group_id, $perm_id);
            }
        }

        return_success(array('group_id' => $group_id));

    }

    function set_group()
    {

        telepath_auth(__CLASS__, __FUNCTION__);

        $post = $this->input->post();

        $data = $post['items']['group'];
        $group_id = $data['id'];
        //$users_new  = isset($post['items']['users']) ? $post['items']['users'] : array();
        $perm_new = isset($post['items']['perm']) ? $post['items']['perm'] : array();

        $apps = isset($post['items']['apps']) ? $post['items']['apps'] : array();
        $ranges = isset($post['items']['ranges']) ? $post['items']['ranges'] : array();

        unset($data['id']);

        if ($group_id != 'new') {

            // Update group details
            $group = $this->ion_auth->update_group($group_id, $data['name'], array('description' => $data['description'], 'extradata' => json_encode(array('apps' => $apps, 'ranges' => $ranges))));
            if (!$group) {
                return_fail($this->ion_auth->errors());
            }

            /*
            $users_raw   = $this->ion_auth->users($group_id)->result();
            $users_old = array();

            foreach($users_raw as $user) {
                $found = false;
                foreach($users_new as $new_user) {
                    if($user == $new_user) {
                        $found = true;
                    }
                }
                if(!$found) {
                    $this->ion_auth->remove_from_group($group_id, $user->id);
                }
            }
            */

        }

        /*if($users_new && !empty($users_new)) {
            foreach($users_new as $user_id) {
                $this->ion_auth->add_to_group($group_id, $user_id);
            }
        }*/

        $this->acl->clear_group_perm($group_id);

        if ($perm_new && !empty($perm_new)) {
            foreach ($perm_new as $perm_id) {
                $this->acl->set_group_perm($group_id, $perm_id);
            }
        }

        return_success(array('group_id' => $group_id));

    }

    public function del_group()
    {

        $id = $this->input->post('id');
        $this->ion_auth->delete_group($id);

        return_success();

    }

}
