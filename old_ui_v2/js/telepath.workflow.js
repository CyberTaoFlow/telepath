var workflowControllerPath = 'index.php/workflow/';
var useOldController 	   = false;

//main varibale -- the network object for the graph
var sys;
var node_selected;
var group_nodes_array=[];//this holds a sorted,directed list of nodes by their appearance in the group
var w_pages_add_mode;//this is the flag to tell whether we're on edit mode
var node_menu_horizontal;//,node_menu_vertical;//create the nodes menu
var last_selected_extensions; //an array that holds all the extensions chosen
var extension_store;  
var w_blink_interval; //blink interval for the record sign
var tool_container; //the container that holds the tooltip for abbreviated nodes on the graph

var current_app_id = false;
var current_flow_id = false;
var selected_app = false;
var recordNode = false;
var recording_active = false;
var w_recording_name = '';

telepath.workflow = {
	stopTracking: function () {
	
		clearInterval(w_recording_interval);
		clearTimeout(w_recording_interval);
		recording_active = false;
		
	},
	init: function () {
	
		Ext.define('MyApp.view.MultiSelectAppTreePanel', {
			extend: 'MyApp.view.AppTreePanel',
			initComponent: function() {
				var me = this;
				Ext.applyIf(me, {
					selModel: Ext.create('Ext.selection.RowModel', {
						mode: 'MULTI'
					})
				});
				me.callParent(arguments);
			}
		});
		
		telepath.workflow.apptree = Ext.create('MyApp.view.MultiSelectAppTreePanel');
		telepath.workflow.apptreeCTL = new AppTreePanel('Workflow', telepath.workflow.apptree);
		Ext.getCmp('w_config').add(telepath.workflow.apptree);
		
		// Start HalfViz
		// $('#w_diagram-innerCt').append('<div id="halfviz" width="100%" height="100%" style="height:100%;width:100%;"><canvas id="viewport" width="800" height="600"></canvas></div>');
		// var mcp = HalfViz("#halfviz");
		// this.hoverMenuInit();
		
	}

}

/*
 * Function: render_record_url
 * 
 * Invoked:
 * 	When we open the record flow window. This generates a clickable url for the user to click on so we can track him;
 * 	
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<start_blinking_record_sign>
 * 
 */

function render_record_url() {
	
	var app = selected_app.raw.app_domain;
	var ssl = selected_app.raw.ssl_flag;
	
	var val = parseInt(Math.random()*1000);
	var url = (ssl==1?"https":"http")+"://"+app+"/?hybridrecord="+val;
	
	Ext.getCmp('w_record_url').setValue(url);
	w_value_to_track = val;
	
	$('input', Ext.getCmp('w_record_url').getEl().dom).hover(function () {
		$(this).css({ cursor: 'pointer', textDecoration: 'underline' });
	}, function () {
		$(this).css({ cursor: 'default', textDecoration: 'none' });
	}).click(function () {
	
		var url = Ext.getCmp('w_record_url').getValue();
		window.open(url, '_blank');
		//window.open(this.href,'_self');
		
	});
	
	Ext.getCmp('w_record_start').getEl().addListener('click', function () {
	
		Ext.getCmp('w_record_name').validate();
		if(Ext.getCmp('w_record_name').isValid()) {
			w_recording_name = Ext.getCmp('w_record_name').getValue();
			recording_active = true;
			track_session();
		}
		
	});
	
}

var w_rightclick_menu = Ext.create('Ext.menu.Menu', { 
	floating: true,
	hidden: true,
	id: 'w_rightclick_menu',
	hideMode:'display',//crucial to fix black squares bug in chrome
	//style: 'visibility: hidden; background-color:orange; display: none!important;',
	items: [
		{
			xtype: 'menuitem',
			id: 'w_rightclick_menu1',
			text: 'Add alias'
		},
		{
			xtype: 'menuitem',
			id: 'w_rightclick_menu4',
			text: 'Remove alias'
		},
		{
			xtype: 'menuitem',
			id: 'w_rightclick_menu2',
			text: 'Show full name',
			hidden:true
		},
		{
			xtype: 'menuitem',
			id: 'w_rightclick_menu3',
			text: 'Hide full name',
			hidden:true
		}
	]
});

Ext.getCmp('w_rightclick_menu1').addListener(
	'click'	,
	function(){
		var node_rightclicked = sys.renderer.node_rightclicked;
		if(!node_rightclicked) return;
		Ext.create('MyApp.view.workflow_rename_page').show();
		Ext.getCmp('workflow_rename_page_field').setValue(node_rightclicked.data.full_label);
		sys.renderer.ctx.save();
		// Use the identity matrix while clearing the canvas
		sys.renderer.ctx.setTransform(1, 0, 0, 1, 0, 0);
		sys.renderer.ctx.clearRect(0, 0, sys.renderer.canvas.width, sys.renderer.canvas.height);
		// Restore the transform
		sys.renderer.ctx.restore();
		sys.renderer.draw_node(node_rightclicked,node_rightclicked._p,sys.renderer.ctx,sys.renderer.gfx)
		
		
	}

);

Ext.getCmp('w_rightclick_menu2').addListener(
	'click'	,
	function(){
		var node_rightclicked = sys.renderer.node_rightclicked;
		if(!node_rightclicked) return;
		node_rightclicked.data.label = node_rightclicked.data.full_label;
		sys.renderer.ctx.save();
		// Use the identity matrix while clearing the canvas
		sys.renderer.ctx.setTransform(1, 0, 0, 1, 0, 0);
		sys.renderer.ctx.clearRect(0, 0, sys.renderer.canvas.width, sys.renderer.canvas.height);
		// Restore the transform
		sys.renderer.ctx.restore();
		sys.renderer.draw_node(node_rightclicked,node_rightclicked._p,sys.renderer.ctx,sys.renderer.gfx)		
	}

);

Ext.getCmp('w_rightclick_menu3').addListener(
	'click'	,
	function(){
		var node_rightclicked = sys.renderer.node_rightclicked;
		if(!node_rightclicked) return;
		var name = node_rightclicked.data.full_label
		node_rightclicked.data.label = "..."+name.substring(name.length-20,name.length)
	}

);

Ext.getCmp('w_rightclick_menu4').addListener(
	'click'	,
	function(){
		w_rename_page("")
	}

);


function w_rename_page(new_name){
	var node_rightclicked = sys.renderer.node_rightclicked;
	if(!node_rightclicked) return;
	
	telepath.util.request("/workflow/rename_page", { page_id:node_rightclicked.data.id, name:new_name, mode:"rename_page" }, function(data) {
		node_rightclicked.data = data.nodes[node_rightclicked.data.id];
	}, 'Error renaming page.');
	
}

/*
 * Function: clear_all_configuration
 * 
 * Invoked:
 * 	Resets the application combobox,the groups combobox, and the extensions list
 * 
 * 
 * Parameters:
 *
 *
 * 
 * Returns:
 * 	undefined	
 * 
 */
function clear_all_configuration(){
	clean_graph();
	Ext.getCmp('w_extensions').getStore().removeAll();
}

/*
 * Function: clean_graph
 * 
 * Invoked:
 * 	When we want clean the graph from all the nodes
 * 
 * 
 * Parameters:
 * 
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
function clean_graph(){
	var empty_graph = {
		'edges':{},
		'nodes':{}
	}
	
	if(window.recordflow) {
	window.recordflow.clear();
	}
	
	//sys.merge(empty_graph)
}

/*
 * Function: update_graph_by_app
 * 
 * Invoked:
 * 	When we want to load the workflow for a the application selected in the combobox
 * 
 * 
 * Parameters:
 * 
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
function update_graph_by_app() {
	
	if(window.recordflow) {
		window.recordflow.clear();
	}
	
	/*
	telepath.util.request('/workflow/get_workflow_graph', {
		app_id: selected_app.raw.app_id,
		app_name: '',
		depth: 0,
		extensions:'Show All',
		frequency:0
	}, function(data){ sys.merge(data.items) }, 'Error.');
	*/
	
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
 * Function: update_graph_by_depth
 * 
 * Invoked:
 * 	When the 'page depth' field was changed. Will reload the graph for the selected application with nodes in depth of up to the selected depth
 * 
 * 
 * Parameters:
 * 
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
function update_graph_by_depth(){
	
	/*node_menu_horizontal.hide();*/
	
	var extensions=""
	var ext_selected = Ext.getCmp('w_extensions').getSelectionModel().getSelection();
	if (ext_selected.length==0)
		extensions = "Show All";
	else{
		for (var i=0;i<ext_selected.length;i++){
			extensions+=ext_selected[i].get('ext')+",";
		}
	}	

	if (!selected_app) {
		Ext.Msg.alert('Error', 'Please select an application to display pages');
		return;
	}
	
	var mask = Ext.getCmp('workflow_mask');
	mask.show();
	
	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/get_workflow_graph",
		params:{
			mode:"get_workflow_graph",
			app_id: selected_app.raw.app_id,
			app_name: '',
			depth:0,
			extensions:extensions,
			frequency:0
		},
		success:function(response){
			mask.hide()
			try{
				var json = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #9', 'Error while loading graph');
				return;							
			}
			
			if(window.recordflow) {
				window.recordflow.clear();
			}
			
			//sys.merge(json)

			var store = Ext.getCmp('w_extensions').getStore();
			store.loadData(json.extensions);
			
		},
		failure: function(response, opts) {
			mask.hide()
			console.log('workflow : server-side failure with status code ' + response.status);
		}
	})	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 * Function: update_graph_by_extensions
 * 
 * Invoked:
 * 	When the 'Show' button of the extensions table was clicked. 
 * 	Will reload the graph for the selected application with nodes in depth of up to the selected depth
 * 
 * 
 * Parameters:
 * 
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
function update_graph_by_extensions(){
	
	/*node_menu_horizontal.hide()*/
	
	var extensions=""
	var ext_selected = Ext.getCmp('w_extensions').getSelectionModel().getSelection()
	if (ext_selected.length==0)
		extensions = "Show All"
	else{
		for (var i=0;i<ext_selected.length;i++){
			extensions+=ext_selected[i].get('ext')+","
		}
	}	

	if (!selected_app){
		Ext.Msg.alert('Error', 'Please select an application to display pages');
		return;
	}
	
	var mask = Ext.getCmp('workflow_mask')
	mask.show()
	
	Ext.Ajax.request({
		url: url,
		params:{
			mode: telepath.controllerPath + "/workflow/get_workflow_graph",			
			app_id: selected_app.raw.app_id,
			app_name: '',
			depth:0,
			extensions:extensions,
			frequency:0
		},
		success:function(response){
			mask.hide()
			try{
				var json = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #10', 'Error while loading graph');
				return;							
			}
			
			if(window.recordflow) {
				window.recordflow.clear();
			}
			
			//sys.merge(json)
			
		},
		failure: function(response, opts) {
			mask.hide()
			console.log('workflow : server-side failure with status code ' + response.status);
		}
	})	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Function: search_page
 * 
 * Invoked:
 * 	When the 'Search' button of the search button was clicked (or enter was pressed)
 * 	Will reload the graph with nodes that hold that name as a substring
 * 
 * 
 * Parameters:
 * 
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
function search_page(){
	
	/*node_menu_horizontal.hide()*/
	
	//reset nodes depth to zero
	/*Ext.getCmp('w_depth').suspendEvents()
	Ext.getCmp('w_depth').setValue(0)
	Ext.getCmp('w_depth').resumeEvents()*/
	var page_field = Ext.getCmp('w_search_page')

	if (!page_field.isValid()){
		Ext.Msg.alert('Error', 'Please enter a name');
		return
	}

	if (!selected_app) {
		Ext.Msg.alert('Error', 'Please select an application to display pages');
		return
	}
	
	var mask = Ext.getCmp('workflow_mask')
	mask.show()
	
	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/search_page",
		params:{
			mode:"search_page",			
			app_id: selected_app.raw.app_id,
			app_name: '',
			depth:0,
			frequency:0,
			page_name:page_field.getValue(),
			search_mode:'Contains'
		},
		success:function(response){
			mask.hide()
			try{
				var json = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #11', 'Error while loading graph');
				return;							
			}

			if(window.recordflow) {
				window.recordflow.clear();
			}
			//change_graph_definition()
			//sys.merge(json)			
		}
	})
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////

function clear_search(){
	Ext.getCmp('w_search_page').reset()
	Ext.getCmp('w_search_page_mode').reset()
	update_graph_by_app()
}



/*
 * Function: change_graph_definition
 * 
 * Invoked:
 * 	Whenever one of the graph physics definition was changed.
 * 	The function will take all the values from the scrollers and reload the graph
 * 
 * 
 * Parameters:
 * 
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
function change_graph_definition() {
	sys.parameters({
		repulsion:1500,
		stiffness:50,
		friction:(50/100.0).toFixed(1),
		gravity: false
	});
}



function debug_nodes(json){
	Ext.Object.each(json.nodes,function(key,value){
		console.log("id:"+key+" label:"+value.label)
	})
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Function: expand_node
 * 
 * Invoked:
 * 	Whenevr a 'node expand' button from the menu was clicked.
 * 	The function will load all of the node's children to the graph
 * 
 * 
 * Parameters:
 * 
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
function expand_node(){
	var mask = Ext.getCmp('workflow_mask')
	mask.show();
	var node = node_selected;
	var freq_field = 0;
	var extensions="";
	var ext_selected = Ext.getCmp('w_extensions').getSelectionModel().getSelection();
	if (ext_selected.length==0)
		extensions = "Show All";
	else{
		for (var i=0;i<ext_selected.length;i++){
			extensions+=ext_selected[i].get('ext')+",";
		}
	}
	
	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/expand_node",
		params:{
			mode:'expand_node', // Needed for old controller
			page_id:node.data.id,
			app_id: selected_app.raw.app_id,
			extensions:extensions,
			frequency: freq_field,
			page_type: node.data.type
		},
		success:function(response){	
			mask.hide()
			try{
				var json = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #12', 'Error while loading graph');
				return;							
			}
			sys.graft(json)
			node_selected.data.expanded = 1
			Ext.getCmp('w_node_menu_expand').hide()
			Ext.getCmp('w_node_menu_contract').show()
		},
		failure: function(response, opts) {
			mask.hide()
			console.log('workflow : server-side failure with status code ' + response.status);
		}
	})
}


/*
 * Function: contract_node
 * 
 * Invoked:
 * 	Whenevr a 'hide children' button from the menu was clicked.
 * 	The function will prune all of the kids and delete them from the graph
 * 
 * 
 * Parameters:
 * 
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<recursively_prune>
 * 
 * 
 */
function contract_node(){
	var node = node_selected;
	node.data.expanded = false
	Ext.getCmp('w_node_menu_expand').show()
	Ext.getCmp('w_node_menu_contract').hide()
	recursively_prune(node,node,false)
}
/*
 * Function: recursively_prune
 * 
 * Invoked:
 * 	Called privately from contract_node
 * 
 * 
 * Parameters:
 * 	node - the node of which all children will be deleted
 * 	original_node - the parent node of the external function - will not be deleted
 * 	delete_origin - boolean,delete or not delete original_node
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<contract_node>
 * 
 * 
 */
function recursively_prune(node,original_node,delete_origin){
	if (node==undefined)
		return
	var edges = sys.getEdgesFrom(node)
	var target;
	if (edges.length==0 && delete_origin){
		sys.pruneNode(node)
		return;
	}
	var edge;
	for (var i=0;i<edges.length;i++){
		target = edges[i].target
		edges[i].source.data.expanded = false
		if (edges[i].target==undefined || edges[i].source==undefined)
			return;
		sys.pruneEdge(edges[i]);	
		if (edges[i].target.name!=edges[i].source.name && edges[i].target.name!=original_node.name)
			recursively_prune(target,original_node,true)
	}
	if (delete_origin){
		sys.pruneNode(node)			
	}
}
/*
 * Function: go_to_url
 * 
 * Invoked:
 * 	Called privately from when we want to open a new tab with a url in the browser
 * 	When the user clicks the record link given.
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 * 
 */
function go_to_url(){
	console.log('Navigated..');
	var node = node_selected;
	window.open(node.data.url);
}


/*
 * Function: remove_node_from_table
 * 
 * Invoked:
 * 	Called when the 'x' was clicked on the pages table
 * 
 * 
 * Parameters:
 * 	value - the value of the page clicked
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 * 
 */
function remove_node_from_table(value){
	//first, check which table we are working on	
	var table;
	var table;
	if (w_pages_add_mode=='edit')
		table = Ext.getCmp('w_pages_edit')
	else if (w_pages_add_mode=='create')
		table = Ext.getCmp('w_pages_add')
	else 
		table = Ext.getCmp('w_pages')
	var store = table.getStore()
	var selected = table.getSelectionModel().getSelection()
	var row_number;
	var array_index;
	var node;
	var page_id;
	var i=1;
	//case clicked on an x
	if (selected.length){
		//save the page_id to get the node
		page_id = selected[0].get('page_id')
		//get the row number of the removed record
		store.each(function(record){
			if (record.getId()==selected[0].getId()){
				row_number=i
				return
			}
			i++;
		})
		table.getStore().remove(selected[0])		
	}
	//case of clicked remove from node, take the LAST one on the group with that value
	else{
		var to_remove;
		store.each(function(record){
			if (record.get('page_name')==value){
				row_number=i
				to_remove=record;
			}
			i++
		})
	}

	if (to_remove){
		page_id = to_remove.get('page_id')
		store.remove(to_remove)
	}
	node = sys.getNode(page_id)
	if (node!=undefined){
		//remove that row from the nodes indices
		node.data.indices_in_group.splice(node.data.indices_in_group.indexOf(row_number), 1);
		if (node.data.indices_in_group.length==0)
			node.data.selected = false
	}
	//now remove all the edges that 
	table.getView().refresh()
	refresh_nodes_indices(table)
}


/*
 * Function: add_node_to_group
 * 
 * Invoked:
 * 	Called when we double click a node on the graph.
 * 	The function adds the page to the table, and updates the graph.
 * 
 * 
 * Parameters:
 * 	node - the node object clicked
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 * 
 */
function add_node_to_group(node){
	node.data.selected = true
	var table;
	if (w_pages_add_mode=='edit')
		table = Ext.getCmp('w_pages_edit')
	else if (w_pages_add_mode=='create')
		table = Ext.getCmp('w_pages_add')
	else 
		table = Ext.getCmp('w_pages')
	var store = table.getStore()
	store.add({
		page_id:node.data.id,
		page_name:node.data.full_label
	})
	var index = store.count();
	node.data.indices_in_group.push(index);
	table.getView().refresh()
	refresh_nodes_indices(table)
}


function refresh_nodes_indices_array(page_ids){

	var node;
	var record;

	group_nodes_array=[];
	
	//first reset all the indices arrays of all the nodes
	sys.eachNode(function(node, pt) {
		//this is how to empty an array in javascript
		node.data.indices_in_group.length = 0;
		node.data.selected = false
	});
	
	//iterate on all entires in the table
	for (var i=0;i<page_ids.length;i++) {
	
			node = sys.getNode(page_ids[i]);
			if (node.data.indices_in_group.indexOf(i+1)==-1) {
				node.data.indices_in_group.push(i+1);
			}
			group_nodes_array.push(node);
			
	}
	
	//first, remove all the edges that were previously on the graph but only for the group
	sys.eachEdge(function(edge, pt1, pt2){
		if (edge.data.of_group){
			sys.pruneEdge(edge)
		}
	});
	
	//now the group_nodes_array is updated according to the table
	for (var i=0;i<group_nodes_array.length-1;i++){
		/*
		 *for each following entries in the array, 
		 *check if there's an edge, if so, mark as 'of_group' so we 
		 *can delete the edge in case we unmark one of the nodes	
		*/
		edges_arr = sys.getEdges(group_nodes_array[i],group_nodes_array[i+1])
		//no edge was found
		if (!edges_arr.length){
			sys.addEdge(group_nodes_array[i],group_nodes_array[i+1], {
				directed:true,
				of_group:true,
				loop:(group_nodes_array[i].name==group_nodes_array[i+1].name)
			})			
		}
		//mark the node as selected
		
	}
	for (var i=0;i<group_nodes_array.length;i++){
		group_nodes_array[i].data.selected = true
		sys.renderer.draw_node(group_nodes_array[i],group_nodes_array[i]._p,sys.renderer.ctx,sys.renderer.gfx)
	}
	//iterate on all the nodes in the graph,if the node is in group_nodes_array, mark it in red, else, mark in default color
	
	change_graph_definition()
	
}


/*
 * Function: refresh_nodes_indices
 * 
 * Invoked:
 * 	Called everytime we want to mark in red and give numbers to the nodes in the graph according to a page table.
 * 	The function takes the pages in the given table and creates red edges between two following pages in the table.
 * 	It also adds numbers on top of each node.
 * 	In general, we invoke that function every time the table was changed,pages were reordered or a page was removed.
 * 
 * Parameters:
 * 	table - a gridpanel extjs object by which its pages we want to update the graph
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 * 
 */

function refresh_nodes_indices(table){
	var node;
	var record;
	var store = table.getStore()
	group_nodes_array=[]
	//first reset all the indices arrays of all the nodes
	sys.eachNode(function(node, pt){
		//this is how to empty an array in javascript
		node.data.indices_in_group.length = 0;
		node.data.selected = false
	})
	//iterate on all entires in the table
	for (var i=0;i<store.count();i++){
		record = store.getAt(i)		
		if (record){
			node = sys.getNode(record.get('page_id'))
			if (node.data.indices_in_group.indexOf(i+1)==-1)
				node.data.indices_in_group.push(i+1)
			group_nodes_array.push(node)
		}	
	}
	//first, remove all the edges that were previously on the graph but only for the group
	sys.eachEdge(function(edge, pt1, pt2){
		if (edge.data.of_group){
			sys.pruneEdge(edge)
		}
	})
	//now the group_nodes_array is updated according to the table
	for (var i=0;i<group_nodes_array.length-1;i++){
		/*
		 *for each following entries in the array, 
		 *check if there's an edge, if so, mark as 'of_group' so we 
		 *can delete the edge in case we unmark one of the nodes	
		*/
		edges_arr = sys.getEdges(group_nodes_array[i],group_nodes_array[i+1])
		//no edge was found
		if (!edges_arr.length){
			sys.addEdge(group_nodes_array[i],group_nodes_array[i+1], {
				directed:true,
				of_group:true,
				loop:(group_nodes_array[i].name==group_nodes_array[i+1].name)
			})			
		}
		//mark the node as selected
		
	}
	for (var i=0;i<group_nodes_array.length;i++){
		group_nodes_array[i].data.selected = true
		sys.renderer.draw_node(group_nodes_array[i],group_nodes_array[i]._p,sys.renderer.ctx,sys.renderer.gfx)
	}
	//iterate on all the nodes in the graph,if the node is in group_nodes_array, mark it in red, else, mark in default color
	
	change_graph_definition()
}

function save_edited_flow(flow_id, flow_name, pages, node, json) {
	
	pages = pages.substring(0, pages.length-1);
	
	Ext.Ajax.request({
		url:telepath.controllerPath + "/workflow/group_save",
		params:{
			mode:'save_group',
			group_id: flow_id,
			group_name: flow_name,
			pages:pages,
			json: JSON.stringify(json)
		},
		success:function(response){
			try{
				var data = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #13', 'Error while saving business action');
				return;							
			}
			if(eval(data['success'])){
				node.set('value', '<span style="color: green;">Saved</span>');
			}
			else if(eval(data['exists'])){
				node.set('value', '<span style="color: red;">Error</span>');
				Ext.Msg.alert('Error', 'Business action '+ flow_name + ' already exists. Please select a different name.');
			}
		}
	})
}

function delete_flow(flow_id, flow_name, node) {
		
	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/group_delete",
		params:{
			mode:'delete_group',
			group_id : flow_id,
		},
		success:function(response) {
			try{
				var ans = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #14', 'Error while deleting business action');
				return;			
			}
			if (eval(ans['success'])){
				node.remove();
			}
		}
	})
}


function save_new_flow(app_id, flow_name, pages, node, json) {
	
	pages = pages.substring(0,pages.length-1);
	
	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/group_create",
		params:{
			mode:'create_group',
			pages:pages,
			group_name: flow_name,
			app_id:app_id,
			json: JSON.stringify(json)
		},
		success:function(response) {
			
			try {
				var data = Ext.decode(response.responseText)
			}
			catch(err) {
				Ext.Msg.alert('Error #15', 'Error while saving business action');
				return;				
			}
			if (eval(data['success'])) {
				node.set('value', '<span style="color: green;">Saved</span>');
				node.set('id', 'flow_' + data['group_id']);
			}
			else if(eval(data['exists'])) {
				node.set('value', '<span style="color: red;">Error</span>');
			}
			
		}
	});
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// FLOW DETECTION WIZARD ////////////////////////////////////////////////////////////


var w_sid_to_record; 		//This is the sessions to track
var w_app_to_record;		//This is the app from which the pages will be recorded
var w_recording_start_time;	//This is the time the recording has started, only take requests taken later
var w_value_to_track;
var w_recording_interval;


/*
 * Function: start_recording
 * 
 * Invoked:
 * 	Called privately by track_session(). 
 * 	The function will start getting all the pages from the server by the user every defined interval
 * 	
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 * See Also:
 * 	<get_pages_by_sid>
 * 	<track_session>
 * 
 */
function start_recording(){

	if (!w_sid_to_record && !w_ip_to_record)
		return;
	
	if(w_sid_to_record) {
		//recordNode.set('text', w_sid_to_record);
	}
	if(w_ip_to_record) {
		//recordNode.set('text', w_ip_to_record);
	}
	
	recordNode.set('text', w_recording_name);
	
	recordNode.set('leaf', false);
	recordNode.set('expandable', true);
	recordNode.set('expanded', true );
	
	recordNode.appendChild({ 'text': 'Stop Recording', 'id': 'stop_' + (w_sid_to_record ? w_sid_to_record : w_ip_to_record), 'type': 'record_stop', iconCls: 'tree-icon-stop', expandable: false, expanded: false, leaf: true });
	
	clear_all_configuration();
	
	Ext.getCmp('w_expand_filter').disable();

	start_blinking_record_sign();
	
	if(w_sid_to_record) {
		get_pages_by_sid();
		w_recording_interval = setInterval(function(){ get_pages_by_sid() }, 1000*5	);
		return;
	}
	if(w_ip_to_record) {
		get_pages_by_ip();
		w_recording_interval = setInterval(function(){ get_pages_by_ip() }, 1000*5	);
		return;
	}
	
}

function start_blinking_record_sign() {
	return;
}
function stop_blinking_record_sign() {
	return;
}

/*
 * Function: track_session
 * 
 * Invoked:
 * 	Called when the user clicks 'Start' on the workflow record button
 * 	The function will check on the server for the sid by trying to find the value that we told the user to insert
 * 	
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<get_pages_by_sid>
 * 	<track_session>
 * 
 */

 function track_session() {
	
	if(!recording_active) {
		return;
	}
	
	Ext.WindowManager.get('w_workflow_record').setHeight(280);
	Ext.getCmp('w_record_start').disable();
	var p_bar=Ext.getCmp('w_detection_progress_bar');
	p_bar.show();
	start_progress_bar();
	
	var mode = 'SID';
	var url = 'get_sid_by_att_value';
	
	w_ip_to_record  = false;
	w_sid_to_record = false;
	
	if(Ext.getCmp('w_ip_recording_mode').value) {
		mode = 'IP';
		url = 'get_ip_by_att_value';
	}
	
	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/" + url,	
		params:{
			att_value:w_value_to_track
		},
		success:function(response){
			try{
				var ans = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #2', 'Error while trying to get the session details from Telepath system.');
				Ext.getCmp('w_record_start').enable();
				return;
			}
			if (eval(ans['success'])){
				
				if(mode == 'IP') {
					w_ip_to_record = ans['IP'];
				} else {
					w_sid_to_record = ans['SID'];
				}
			
				w_app_to_record = selected_app.raw.app_id;
				Ext.getCmp('w_detection_progress_bar').reset();
				Ext.getCmp('w_detection_progress_bar').hide();
				Ext.getCmp('w_detection_progress_bar').updateProgress(1.0,"Hybrid Telepath has found your session. You may begin your recording");
				Ext.WindowManager.get('w_workflow_record').destroy();
				//we need to take the time on the server
				w_recording_start_time = ans['time']; //new Date(ans['time']).getTime()/1000;
				start_recording();
			}
			else {
				if(recording_active) {
					w_recording_interval = setTimeout(function () { 
						track_session();
					}, 3000);
				}
			}
		}
	});
}

function start_progress_bar(){
	var p_bar=Ext.getCmp('w_detection_progress_bar')
	p_bar.wait({
		interval: 500, //bar will move fast!
		duration: 20000000,
		increment: 100,
		text: 'Hybrid Telepath is tracking your session...',
		scope: this,
		fn: function(){
			start_progress_bar()
		}
	});
}


/*
 * Function: set_tooltip
 * 
 * Invoked:
 * 	Called when the user hovers a node with an abbreviated name such as "...page.ext"
 * 	the functions renders the tooltip and shows it with the nodes name
 * 	
 * 
 * Parameters:
 * 	x - the x location to place the tooltip
 * 	y - the y location to place the tooltip
 * 	title -  the string title for the tooltip
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */


function set_tooltip(x,y,title){
	//w_node_tooltip_obj.hide()
	tool_container.setPosition(x,y)
	tool_container.show()
	w_node_tooltip_obj.update(title)
	//w_node_tooltip_obj.show()
}


/*
 * Function: hide_tooltip
 * 
 * Invoked:
 * 	Called when the user hovers out of a node with an abbreviated name such as "...page.ext"
 * 	the functions hides the tooltip
 * 	
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function hide_tooltip(){
	if (tool_container){
		tool_container.setPosition(0,0)
		tool_container.hide()
	}
	
}


