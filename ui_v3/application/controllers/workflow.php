<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Workflow extends CI_Controller
{

    public function __construct()
    {

        parent::__construct();
        $this->load->model('WorkflowModel');
        $this->load->model('BusinessflowModel');
        $this->load->model('PagesModel');
        $this->load->model('Apps');

    }

    public function get_requests()
    {

        $key = $this->input->post('key', true);
        $value = $this->input->post('value', true);
        $time = $this->input->post('time', true);

        $clean = array();

        $data = $this->WorkflowModel->get_requests($key, $value, $time);

        if (!empty($data)) {
            foreach ($data as $row) {
                $row['page'] = $this->PagesModel->page_get($row['page_id']);
                $row['application'] = $this->Apps->app_get($row['hostname']);
                $clean[] = $row;
            }
        }

        return_success($clean);

    }

    public function get_suggest()
    {

        $mode = $this->input->post('mode', true);
        $clean = array();

        switch ($mode) {
            case 'IP':
                $data = $this->WorkflowModel->get_last_ips();
                break;
            case 'SID':
                $data = $this->WorkflowModel->get_last_sids();
                break;
            case 'user':
                $data = $this->WorkflowModel->get_last_users();
                break;
        }

        if (!empty($data)) {
            foreach ($data as $row) {
                $clean[] = $row->suggest;
            }
        }

        return_success($clean);

    }

    public function get_expand()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        // Collect params
        $type = $this->input->post('type', true);
        $id = $this->input->post('id', true);

        switch ($type) {

            case 'application':
                $this->_expand_workflow_app($id);
                break;
            case 'flow':
                $this->_expand_workflow_flow($id);
                break;

        }

    }


    // Workflow -- Expand APP
    private function _expand_workflow_app($app_id)
    {

        $flows = $this->BusinessflowModel->groups_by_app($app_id, true);
        $categories = $this->BusinessflowModel->categories_by_app($app_id, true);

        $ans = array('success' => true, 'context' => 'workflow', 'app_id' => $app_id, 'flows' => array(), 'flow_categories' => array());

        foreach ($categories as $category) {
            $ans['flow_categories'][] = array('name' => $category->name, 'id' => $category->id, 'type' => 'flow_category', 'expandable' => true);
        }

        foreach ($flows as $flow) {

            $expandable = !(trim($flow->pages) == '');
            $ans['flows'][] = array('group' => $flow->group_name, 'id' => $flow->id, 'type' => 'flow', 'category_id' => $flow->category_id, 'expandable' => $expandable);

        }

        return_json($ans);

    }

    // Workflow -- Expand FLOW
    private function _expand_workflow_flow($group_id)
    {

        // Load Model
        $this->load->model('BusinessflowModel');
        $this->load->model('WorkflowModel');

        // Prepare return array
        $ans = array('success' => true, 'context' => 'workflow', 'success' => true, 'items' => array(), 'total' => 0);

        // Count pages for result
        $result_count = $this->BusinessflowModel->group_id_check($group_id);

        if ($result_count > 0) {

            $results = $this->BusinessflowModel->group_get_by_id($group_id);
            $ans['group_name'] = $results[0]->group_name;
            $ans['app_id'] = $results[0]->app_id;

            $pages = $results[0]->pages;
            if (trim($pages) == '' || trim($pages == '[]')) {
                // Nothing to do, no pages
            } else {
                // Collect pages data to graph
                if (substr($pages, 0, 1) == '[') {

                    // NEW FORMAT
                    $this->load->model('PagesModel');
                    $this->load->model('Attributes');
                    $pages = json_decode($pages);

                    if ($pages && is_array($pages) && !empty($pages)) {

                        foreach ($pages as $page) {

                            $page_data = $this->PagesModel->page_get($page->page_id);

                            if ($page_data) {

                                $item = array('page' => $page_data, 'page_id' => $page->page_id);

                                if (isset($page->params) && is_array($page->params) && !empty($page->params)) {
                                    $item['params'] = array();
                                    foreach ($page->params as $param) {
                                        $attribute = $this->Attributes->get_att_by_id($param->att_id);
                                        if ($attribute) {
                                            $item['params'][] = array('att_id' => $param->att_id, 'att_alias' => $attribute->att_alias, 'data' => $param->data, 'att_name' => $attribute->att_name, 'att_source' => $attribute->att_source);
                                        }
                                    }
                                }

                                $ans['items'][] = $item;

                            }

                        }

                    }

                }

            }
        }

        $ans['total'] = count($ans['items']);

        return_json($ans);

    }


    // Rename a category
    public function set_category_name()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        // Params
        $category_id = $this->input->post('category_id');
        $category_name = $this->input->post('category_name');

        // Rename
        $ok = $this->BusinessflowModel->category_rename($category_id, $category_name);

        // Return
        if ($ok) {
            return_success(array('category_id' => $category_id, 'category_name' => $category_name));
        } else {
            return_fail('Category name already taken or DB error');
        }

    }

    // Create a category
    public function add_category()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        // Params
        $application_id = $this->input->post('app_id');
        $category_name = $this->input->post('category_name');

        // Create
        $category_id = $this->BusinessflowModel->category_create($application_id, $category_name);

        // Return
        if ($category_id) {
            return_success(array('category_id' => $category_id, 'category_name' => $category_name));
        } else {
            return_fail('Category name already taken or DB error');
        }

    }

    // Delete a category and all contained flows
    public function del_category()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        // Param
        $category_id = $this->input->post('category_id');

        // Delete
        $this->BusinessflowModel->category_delete($category_id);

        // Update ATMS action
        $this->load->model('ConfigModel');
        $this->ConfigModel->update_action_code(108, 1);

        // Return
        return_success(array('category_id' => $category_id));

    }

    // Add a flow to category
    public function set_category_flow()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        // Param
        $category_id = $this->input->post('category_id');
        $flow_id = $this->input->post('flow_id');

        // Add flow to category
        $this->BusinessflowModel->category_add_flow($category_id, $flow_id);

        // Return
        return_success(array('category_id' => $category_id, 'flow_id' => $flow_id));

    }

    public function set_states()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $data = $this->input->post('states');
        if (!$data) {
            return_json(array('success' => false));
        }
        $this->load->model('ConfigModel');
        $states = $this->ConfigModel->get('workflow_states');
        if ($states) {
            $this->ConfigModel->update('workflow_states', $data);
        } else {
            $this->ConfigModel->insert('workflow_states', $data);
        }
        return_json(array('success' => true));

    }

    public function get_states()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->load->model('ConfigModel');
        $states = $this->ConfigModel->get('workflow_states');
        if ($states) {
            echo $states['workflow_states'];
        } else {
            echo '[]';
        }
    }

    public function record()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->load->model('RequestScores');
        $this->load->model('AttributeScores');

        $ans = array('nodes' => array());

        // Collect input
        $SID = $this->input->post('sid', true);
        $start_time = $this->input->post('start_time', true);

        $pages = $this->RequestScores->get_pages_by_SID($SID, $start_time);

        $last_node = false;

        foreach ($pages as $page) {

            $page_id = $page->page_id;
            $RID = $page->RID;
            $date = intval($page->date);
            $path = $page->path;

            // Generate node
            $page_name = $this->WorkflowModel->page_get_display_name($page_id);
            $page_name = $page_name != ' ' ? extract_page_name($page_name) : $page_id; // Case of JSON

            // Attach parameters
            $params = $this->AttributeScores->get_params_by_RID($RID, true);

            foreach ($params as $key => $value) {
                if ($value->att_id == '18' || $value->att_name == 'hybridrecord') {
                    unset($params[$key]);
                }
            }

            // Create TreeData
            $ans['nodes'][] = array('page_id' => $page_id, 'page_name' => $page_name, 'params' => $params, 'date' => $date, 'path' => $path, 'alias' => $page->title);

        }

        return_json($ans);

    }

    public function get_sid_by_att_value()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->load->model('AttributeScores');
        $this->load->model('RequestScores');

        $param_id = 18; // Our magic "hybrid_record" param
        $att_value = $this->input->post('att_value', true);

        $RID = $this->AttributeScores->get_RID_by_param($param_id, $att_value);
        if ($RID !== false) {

            $res = $this->RequestScores->get_SID_by_RID($RID);
            if (!$res) {
                return_json(array('success' => 'false', 'console' => 'no SID for request id ' . $RID));
            }

            $SID = $res->SID;
            $time = $res->date;

            if ($SID !== false) {
                return_json(array('success' => 'true', 'SID' => $SID, 'time' => $time));
            } else {
                return_json(array('success' => 'false'));
            }
        } else {
            return_json(array('success' => 'false'));
        }

    }

    // Get all groups from all applications
    public function groups_get_general_cb()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->load->model('BusinessflowModel');
        $groups = $this->BusinessflowModel->groups_get_all();
        $ans = array('success' => 'true', 'count' => count($groups), 'items' => array());

        foreach ($groups as $group) {
            $ans['items'][] = array('name' => $group->group_name, 'id' => $group->id, 'app_id' => $group->app_id);
        }

        return_json($ans);

    }

    public function set_flow()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $group_id = $this->input->post('group_id', true);
        $group_name = $this->input->post('group_name', true);
        $json = $this->input->post('json', true);

        $this->load->model('BusinessflowModel');

        $name_check = $this->BusinessflowModel->group_get_by_name($group_name, false, $group_id);
        $name_taken = isset($name_check[0]) ? true : false;

        if ($name_taken) {
            return_json(array('success' => 'false', 'exists' => 'true'));
        }
        if (!$json) {
            $json = '[]';
        }

        $group_id = $this->BusinessflowModel->group_update($group_name, $json, $group_id);

        return_json(array('success' => 'true'));

    }

    public function group_delete()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->load->model('BusinessflowModel');

        $group_id = $this->input->post('group_id', true);
        if ($group_id === false) {
            return_json(array('success' => 'false'));
        } else {
            $this->BusinessflowModel->group_delete($group_id);

            // Update ATMS action
            $this->load->model('ConfigModel');
            $this->ConfigModel->update_action_code(108, 1);

            return_json(array('success' => 'true'));

        }

    }

    public function add_flow()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $app_id = $this->input->post('app_id', true);
        $group_name = $this->input->post('group_name', true);
        $json = $this->input->post('json', true);

        $this->load->model('BusinessflowModel');

        $name_check = $this->BusinessflowModel->group_get_by_name($group_name, $app_id);
        $name_taken = isset($name_check[0]) ? true : false;

        if ($name_taken) {
            return_json(array('success' => 'false', 'exists' => 'true'));
        }
        if (!$json) {
            $json = '[]';
        }

        $group_id = $this->BusinessflowModel->group_create($group_name, $app_id, $json);
        if (!$group_id) {
            return_json(array('success' => 'false'));
        } else {
            return_json(array('success' => true, 'group_id' => intval($group_id), 'group_name' => $group_name));
        }

    }

    public function search_page()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        $this->load->model('PagesModel');

        $app_id = $this->input->post('app_id', true);
        $page_name = $this->input->post('page_name', true);
        $search_mode = $this->input->post('search_mode', true);

        $ans = array('nodes' => array(), 'edges' => array());
        $graph = new Graph();

        $pages = $this->PagesModel->page_search($page_name, $app_id, $search_mode);

        // Add all results
        foreach ($pages as $page) {
            $page_id = $page->page_id;
            $page_name = extract_page_name($page->display_path);
            $page_node = new Node($this->WorkflowModel, $page_name, $page_id, $app_id, 'page');
            $graph->addNode($page_node);
        }

        // Match edges between any two nodes + build ans
        foreach ($graph->vertices as $node_1) {
            $ans['nodes'][$node_1->id] = $node_1->get_visual_interface();
            foreach ($graph->vertices as $node_2) {
                if ($this->PagesModel->page_check_edge($node_1->id, $node_2->id) > 0) {
                    validate_edge($ans, $node_1->id);
                    $ans['edges'][$node_1->id][$node_2->id] = array('directed' => '1', 'loop' => '0');
                }
            }
        }

        $ans['nodes_total'] = count($ans['nodes']);

        return_json($ans);

    }

    public function rename_page()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        // Gather Params
        $page_id = $this->input->post('page_id', true);
        $new_name = $this->input->post('name', true);

        if (!$new_name) {
            $new_name = '';
        }
        if (!$page_id) {
            return_json(array('success' => false));
        }

        // Update page name
        $this->WorkflowModel->page_set_name($page_id, $new_name);

        // Gather data for responce
        $page_data = $this->WorkflowModel->page_get_display_name($page_id, true); // TRUE flag for return app id
        $page_name = extract_page_name($page_data->display_path);
        $app_id = $page_data->app_id;

        // Construct node
        $page_node = new Node($this->WorkflowModel, $page_name, $page_id, $app_id, 'page');

        // Prepare return array
        $ans = array('success' => true, 'items' => array(), 'edges' => array(), 'nodes' => array(), 'nodes_total' => 1);
        $ans['nodes'][$page_id] = $page_node->get_visual_interface();

        // Return
        return_json($ans);

    }

    public function get_workflow_graph()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        // Gather Params
        $app_id = $this->input->post('app_id', true);
        $frequency = $this->input->post('frequency', true);
        $depth = $this->input->post('depth', true);
        $extensions = $this->input->post('extensions', true);

        // App access check
        if (!$this->acl->all_apps()) {
            if (!in_array($app_id, $this->acl->allowed_apps)) {
                return_fail('Access Denied');
            }
        }

        $graph = new Graph();

        $result = $this->WorkflowModel->get_workflow_graph($app_id, $frequency, $depth, $extensions);

        // INIT GRAPH
        foreach ($result as $row) {

            $dupe = false;
            $page_from_id = $row->page_from;
            $page_to_id = $row->page_to;

            $page_from_name = $row->display_path_from != NULL ? extract_page_name($row->display_path_from) : $page_from_id;
            $page_to_name = $row->display_path_to != NULL ? extract_page_name($row->display_path_to) : $page_to_id;
            $frequency = intval($row->frequency);

            $root = $row->seq;

            $to_add = new Node($this->WorkflowModel, $page_from_name, $page_from_id, $app_id);
            $graph->addNode($to_add);

            /* REPEATING CODE #1 */
            if (intval($page_from_id) == intval($page_to_id)) {
                $page_to_id = -1 * $page_to_id;
                $dupe = true;
            }
            $to_add = new Node($this->WorkflowModel, $page_to_name, $page_to_id, $app_id, 'page', $dupe);
            $graph->addNode($to_add);

            if (intval($page_from_id) !== intval($page_to_id)) {
                $graph->vertices[$page_to_id]->is_root = false;
            }

            $graph->addEdge($graph->vertices[$page_from_id], $graph->vertices[$page_to_id], $frequency);
            /* END REPEATING CODE # 1 */

        }

        // RUN BFS
        foreach ($graph->vertices as $node) {
            if ($node->color == 'WHITE' && $node->is_root) {
                BFS($node, $graph, $depth);
            }
        }

        // Dump our graph
        $ans = generate_graph_output($graph, $extensions, false, $depth);

        $ans['nodes_total'] = count($ans['nodes']);
        return_json($ans);

    }

    public function expand_node()
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        // Gather Params
        $app_id = $this->input->post('app_id', true);
        $frequency = $this->input->post('frequency', true);
        $page_id = $this->input->post('page_id', true);
        $extensions = $this->input->post('extensions', true);
        $node_type = $this->input->post('page_type', true);

        $graph = new Graph();

        // App access check
        if (!$this->acl->all_apps()) {
            if (!in_array($app_id, $this->acl->allowed_apps)) {
                return_fail('Access Denied');
            }
        }

        //echo "\nAPP $app_id FQ $frequency PAGE $page_id EXT $extensions TYPE $node_type\n\n";

        $node_type = trim($node_type);

        if ($node_type == 'page' || $node_type == 'web_service') {

            // Generate page from node
            $page_from_id = intval($page_id);
            $page_from_name = $this->WorkflowModel->page_get_display_name($page_from_id);
            $page_from_name = $page_from_name != ' ' ? extract_page_name($page_from_name) : $app_id;
            $page_from_node = new Node($this->WorkflowModel, $page_from_name, $page_from_id, $app_id, $node_type);
            $page_from_node->set_expanded(0);
            // Append to Graph
            $graph->addNode($page_from_node);

            $result = $this->WorkflowModel->get_workflow_by_page_from($app_id, $page_id, $frequency);

            // 1. Construct Graph
            foreach ($result as $row) {

                $dupe = false;
                $page_to_id = $row->page_to;
                $page_to_name = $row->display_path_to != NULL ? extract_page_name($row->display_path_to) : $page_to_id;
                $root = $row->seq;
                $freqency = intval($row->frequency);

                /* REPEATING CODE #1 */
                if (intval($page_from_id) == intval($page_to_id)) {
                    $page_to_id = -1 * $page_to_id;
                    $dupe = true;
                }
                $to_add = new Node($this->WorkflowModel, $page_to_name, $page_to_id, $app_id, 'page', $dupe);
                $graph->addNode($to_add);

                if (intval($page_from_id) !== intval($page_to_id)) {
                    $graph->vertices[$page_from_id]->is_root = false;
                }

                $graph->addEdge($graph->vertices[$page_from_id], $graph->vertices[$page_to_id], $freqency);
                /* END REPEATING CODE #1 */

            }

            // Dump our graph
            $ans = generate_graph_output($graph, $extensions, true);

            if ($node_type == 'web_service') {
                $ans = $this->get_json_by_page($ans, $page_from_node, $frequency);
            }

            $ans['nodes_total'] = count($ans['nodes']);

            return_json($ans);

        } elseif ($node_type == 'json' || $node_type == 'xml') {

            /*query = "SELECT j_to, (SELECT json,schema FROM json_diagram WHERE id=j_to),seq  FROM flow_diagram_json  WHERE frequency>="+freq_requested+" AND j_from='"+page_id+"' order by seq desc, j_from asc, frequency desc;"
            flow = s.run(query)
            ####################################    INIT GRAPH 	##############################
            if flow!=():
                page_from_id = page_id
                to_add = Node(str(page_from_id),page_from_id,app_id,'json')
                graph.addNode(to_add)
            for result in flow:
                page_to_id = int(result[0])
                page_to_name = str(result[1].encode('utf-8').strip())
                node_to_type = str(result[2])
                if node_to_type=='J':
                    node_to_type='json'
                elif node_to_type=='X':
                    node_to_type='xml'
                root =  int(result[3])
                if page_to_id not in graph.vertices:
                    to_add = Node(page_to_name,page_to_id,app_id,node_to_type)
                    graph.addNode(to_add)
                if page_from_id!=page_to_id:
                    graph.vertices[page_to_id].is_root = False
                    graph.addEdge(graph.vertices[page_from_id],graph.vertices[page_to_id])
            */

            // Dump our graph
            $ans = generate_graph_output($graph, $extensions, true);
            $ans['nodes_total'] = count($ans['nodes']);

            echo '111';
            print_r($ans);
            echo '222';

            return_json($ans);

        }

    }

    function get_json_by_page($ans, $root_node, $frequency)
    {

        telepath_auth(__CLASS__, __FUNCTION__, $this);

        // Reconstruct graph while appending JSON nodes
        $graph = new Graph();
        $app_id = $root_node->app_id;
        $graph->addNode($root_node);

        $result = $this->WorkflowModel->get_workflow_diagram_json($root_node->id, $frequency);

        foreach ($result as $row) {

            // LINK FROM NODES

            $json_from_id = $row->j_from;
            $json_from_name = $row->j_from_json != NULL ? $row->j_from_json : ' ';
            $json_from_schema = $row->j_from_structure;
            $frequency = $row->frequency;

            $json_from_type = type_from_schema($json_from_schema);

            $json_from_subnodes = explode('::', $json_from_name);
            $index = 0;

            $last_sub_node = $json_from_name;

            foreach ($json_from_subnodes as $sub_node) {

                $new_sub_id = $json_from_id . '_' . $index;
                $json_subnode = new Node($this->WorkflowModel, $sub_node, $new_sub_id, $app_id, $json_from_type);
                $graph->addNode($json_subnode);
                $and['nodes'][$new_sub_id] = $json_subnode->get_visual_interface();

                // First json child, add an edge from the root node to it
                if ($index == 0) {

                    validate_edge($ans, $root_node->id);

                    $graph->addEdge($root_node, $json_subnode, $frequency);
                    $ans['edges'][$root_node->id][$new_sub_id] = array('directed' => '1', 'loop' => '0');

                } else {

                    // Connect it to its parent json edge
                    $parent_subnode = $graph->getNode($json_from_id . '_' . ($index - 1));
                    // Just in case
                    if (!$parent_subnode) continue;

                    validate_edge($ans, $parent_subnode->id);

                    $graph->addEdge($parent_subnode, $json_subnode, $frequency);
                    $ans['edges'][$parent_subnode->id][$new_sub_id] = array('directed' => '1', 'loop' => '0');

                }

                $index++;
                $last_sub_node = $json_subnode;

            }

            // LINK TO NODES

            $json_to_id = $row->j_to;
            $json_to_name = $row->j_to_json != NULL ? $row->j_to_json : ' ';
            $json_to_schema = $row->j_to_structure;

            $json_to_type = type_from_schema($json_to_schema);

            $json_to_subnodes = explode('::', $json_to_name);
            $index = 0;

            foreach ($json_to_subnodes as $sub_node) {

                $new_sub_id = $json_from_id . '_' . $index;
                $json_subnode = new Node($this->WorkflowModel, $sub_node, $new_sub_id, $app_id, $json_to_type);
                $graph->addNode($json_subnode);
                $ans['nodes'][$new_sub_id] = $json_subnode->get_visual_interface();

                if ($index == 0) {

                    validate_edge($ans, $last_sub_node->id);

                    $graph->addEdge($last_sub_node, $json_subnode, 1);
                    $ans['edges'][$last_sub_node->id][$new_sub_id] = array('directed' => '1', 'loop' => '0');

                } else {

                    // Connect it to its parent json edge
                    $parent_subnode = $graph->getNode($json_to_id . '_' . ($index - 1));
                    // Just in case
                    if (!$parent_subnode) continue;

                    validate_edge($ans, $parent_subnode->id);

                    $graph->addEdge($parent_subnode, $json_subnode);
                    $ans['edges'][$parent_subnode->id][$new_sub_id] = array('directed' => '1', 'loop' => '0');

                }

                $index++;

            }

        }

        return $ans;
    }

}
