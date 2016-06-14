// TODO:: Figure which ones are irrelevant and remove

var sys;
var node_selected;
var group_nodes_array=[];//this holds a sorted,directed list of nodes by their appearance in the group
var w_pages_add_mode;//this is the flag to tell whether we're on edit mode
var node_menu_horizontal;//,node_menu_vertical;//create the nodes menu
var last_selected_extensions; //an array that holds all the extensions chosen
var extension_store;  
var w_blink_interval; //blink interval for the record sign
var tool_container; //the container that holds the tooltip for abbreviated nodes on the graph

// END TODO::

var current_app_id = false;
var current_flow_id = false;
var selected_app = false;
var recordNode = false;

var positioning_states = [];
var last_expanded = [];

function getColorPercent(value) {

	var end   = 360;
	var start = 240;
	var a = value/100,
     b = end*a;
     c = b+start;

    //Return a CSS HSL string
    return 'hsl('+c+',100%,50%)';
	
}

var hex2Rgb = function(hex){
  var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})|([a-f\d]{1})([a-f\d]{1})([a-f\d]{1})$/i.exec(hex);
  return result ? {        
    r: parseInt(hex.length <= 4 ? result[4]+result[4] : result[1], 16),
    g: parseInt(hex.length <= 4 ? result[5]+result[5] : result[2], 16),
    b: parseInt(hex.length <= 4 ? result[6]+result[6] : result[3], 16),
    toString: function() {
      var arr = [];
      arr.push(this.r);
      arr.push(this.g);
      arr.push(this.b);
      return "rgb(" + arr.join(",") + ")";
    }
  } : null;
};

function RadialGradient(element, color1, color2, flag) {
	
	color1 = hex2Rgb(color1);
	color2 = hex2Rgb(color2);
	
	$(element).css('background-color', color1);
	
	/*var browsers = ['', '-o-', '-ms-', '-moz-', '-webkit-'];
	$.each(browsers, function(i, browser) {
		if(flag) {
			$(element).css('background', browser + 'radial-gradient(center , circle cover , ' + color2  + ' 10% , ' + color1 + ' 70%)');
		} else {
			$(element).css('background', browser + 'radial-gradient(center , circle cover , ' + color2 + ' 10% , ' + color1 + ' 70%)');
		}
	});*/
	
}

var myColours = [
	'#0000FF',
	'#0000CC',
	'#000099',
	'#000066',
	'#330066',
	'#660033',
	'#660000',
	'#990000',
	'#CC0000',
	'#FF0000',
];

function colorPercent(x, col) {
	if(x == 100) x = 99;
    return col[Math.floor(x / 10)];
}

telepath.workflow = {

	init: function () {

		telepath.util.getAppTree(function () {
		
			telepath.workflow.apptree = Ext.create('MyApp.view.AppTreePanel');
			telepath.workflow.apptreeCTL = new AppTreePanel('Workflow', telepath.workflow.apptree);
			
			telepath.workflow.apptree.setHeight($(window).height() - 50);
			telepath.workflow.apptree.setWidth(630);
			telepath.workflow.apptree.render(Ext.getDom('w_config-innerCt'));
						
		});
		
		/*
		var vis = new SimpleVis("#w_diagram-innerCt", [12,33,20,40,60,10,25,44,13,23,14,25,8]);
		$("#w_diagram-innerCt").append('<a id="move">MOVE</a>');
		window.vis = vis;
		$("#move").on("click", function(e) {
		vis.bin.range(vis.bin.range().reverse());
		vis.force.resume();
		return false;
		});
		*/
		
		this.diagram.initStates();
		
	},
	
	diagram: {
		
		reorder: function () {
		
			this.springyInit();
			this.container.empty();
			var nodes = this.drawNodes(this.data, true);
			this.initPlumb();			
			this.connectEdges(this.data.edges, nodes);
			
		},
		initStates: function () {
			$.getJSON(telepath.controllerPath + '/workflow/load_states', function (data) {
				positioning_states = data;
			}, 'json');
		},
		applyState: function(state, nodeDOM) {
			var node_id = nodeDOM.data('node').id;
			$.each(state.state, function(i, state_node) {
				if(state_node.id == node_id) {
					nodeDOM.css({ top: state_node.y, left: state_node.x });
				}
			});
		},
		findState: function(app_id, node_id) {
			
			var result = false;
			
			$.each(positioning_states, function(i, state) {
				if(state.app == parseInt(app_id) && state.expanded == node_id) {
					result = state;
					return false;
				}
			});
			
			return result;
			
		},
		updateState: function(new_state) {
		
			// Find if we stored this state before
			// Insert new state
			var found = false;
			
			$.each(positioning_states, function(i, state) {
				if(state.app == new_state.app && state.expanded == new_state.expanded) {
					positioning_states[i] = new_state;
					found = true;
					return false;
				}
			});
			
			if(!found) {
				positioning_states.push(new_state);
			}
			
			$.post(telepath.controllerPath + '/workflow/save_states', { states: JSON.stringify(positioning_states) }, function (data) {
				if(!data || !data.success) {
					console.log('Error Saving Positioning States');
				}
			}, 'json');
			
		},
		expand: function(node) {
			
			last_expanded.push(node);
			
			Ext.Ajax.request({
				url: telepath.controllerPath + "/workflow/expand_node",
				params:{
					page_id:node.id,
					app_id: selected_app.raw.app_id,
					extensions: "Show All",
					frequency: 0,
					page_type: node.type
				},
				success:function(response){	
					
					try{
						var json = Ext.decode(response.responseText)
					}
					catch(err){
						Ext.Msg.alert('Error #12', 'Error while loading graph');
						return;							
					}
					
					telepath.workflow.diagram.load(json);
					
				},
				failure: function(response, opts) {
					console.log('workflow : server-side failure with status code ' + response.status);
				}
			});
		
		},
		springyInit: function () {
			
			var that = this;
			
			that.springyGraph = new Springy.Graph();
			that.springyLayout = new Springy.Layout.ForceDirected(that.springyGraph, 100, 10, 1);
			that.springyRenderer = new Springy.Renderer(that.springyLayout,

                function clear() {
				
                },
                function drawEdge(edge, p1, p2) {
				
                },
                function drawNode(node, p) {
					
					try {
					
						var position = that.springyToScreen(p);
						
						window.position = position;
						
						node.data.container.css({
							left: position[0], 
							top: position[1],
							position: 'absolute'
						});
						
						jsPlumb.repaint(node.data.container);
					
					} catch(e) {}
					
                }
				
			);
			
			// Limit 3 seconds for solution
			setTimeout(function () {
				that.springyRenderer.stop();
			}, 5000);
			
		},
		springyToScreen: function (p, containerWidth, containerHeight) {
			
			var that = telepath.workflow.diagram;
			// calculate bounding box of graph layout.. with ease-in
			var currentBB = that.springyLayout.getBoundingBox();
			
			window.currentBB = currentBB;
			
			var screen    = that.springyScreenSize();
			
			var xOffset = (containerWidth / 2) || 0;
			var yOffset = (containerHeight / 2) || 0;
			// convert to/from screen coordinates
			var size = currentBB.topright.subtract(currentBB.bottomleft);
			var sx = (p.subtract(currentBB.bottomleft).divide(size.x).x * screen.x);
			var sy = (p.subtract(currentBB.bottomleft).divide(size.y).y * screen.y);
			return [sx, sy];
			
		},
		springyFromScreen: function(s) {
			
			var that = telepath.workflow.diagram;
			
			var screen    = that.springyScreenSize();
			var currentBB = that.springyLayout.getBoundingBox();

			var size = currentBB.topright.subtract(currentBB.bottomleft);
					
			var px = (s.x / screen.x) * size.x + currentBB.bottomleft.x;
			var py = (s.y / screen.y) * size.y + currentBB.bottomleft.y;
			return new Vector(px, py);
				
		},
		springyScreenSize: function () {
			
			var that = telepath.workflow.diagram;
		
			return {
				x: that.container.width() - 200, 
				y: that.container.height() - 100
			}
			
		},
		container: '',
		createNode: function(node, min_frequency, max_frequency) {
			
			var that = this;
			
			var nodeDOM = $('<span>');
			nodeDOM.data('node', node);
			
			nodeDOM.addClass('node').addClass('initial');
			
			var icon = $('<span>').addClass('node-' + node.type);
			
			var label     = $('<span>').addClass('label');
			var bg		  = $('<div>').addClass('bg');
			
			label.text(node.label);
			
			//nodeDOM.append(node.id);

			var percent = ((node.frequency - min_frequency) / max_frequency) * 100;
			nodeDOM.css('borderColor', colorPercent(percent, myColours));
			
			nodeDOM.css('padding', percent / 10);
			nodeDOM.css('border-radius' , 10);
			bg.css('border-radius' , 10);
			
			RadialGradient(bg, colorPercent(percent, myColours), '#FFF');
			
			if(node.expandable) {
			
				var expandable = $('<span>').addClass('expand');
				nodeDOM.append(expandable);
				expandable.click(function () {
					
					if($(this).hasClass('open')) {
					
						// POP Current
						last_expanded.pop();
						// POP Previous
						var previous = last_expanded.pop();
						
						if(previous.id == 'application') {
							update_graph_by_app();
						} else {
							that.expand(previous);
						}
						
					} else {
						that.expand($(this).parent().data('node'));
					}
					
				});
				
				if(last_expanded[last_expanded.length - 1].id == node.id) {
					expandable.addClass('open');
				}
				
			}

			nodeDOM.append(bg);
			nodeDOM.append(label);
			//nodeDOM.append(icon);

			that.container.append(nodeDOM);

			return nodeDOM;
			
		},
		resizeContainer: function () {
			
			var screen_x = 0;
			var screen_y = 0;
			
			$('.node').each(function () {
			
				var node_y = $(this).offset().top + $(this).outerHeight();
				var node_x = $(this).offset().left + $(this).outerWidth();
				
				screen_y = Math.max(screen_y , node_y);
				screen_x = Math.max(screen_x , node_x);
				
			});
		
			if(screen_x > that.container.width()) {
				that.container.width(screen_x);
			}
			if(screen_y > that.container.height()) {
				that.container.height(screen_y);
			}
			
		},
		initPlumb: function() {
			
			var that = this;
			
			jsPlumb.importDefaults({
				Endpoint : ["Dot", {radius:8}],
				HoverPaintStyle : {strokeStyle:"#1e8151", lineWidth:2 },
				ConnectionOverlays : [
					[ "Arrow", { 
						location:1,
						id:"arrow",
	                    length:14,
	                    foldback:0.8
					} ]
				]
			});
 
			// make everything draggable
			jsPlumb.draggable($('.node'), { stop: function (e) { 
				
				var data = { app: parseInt(selected_app.raw.app_id), expanded: last_expanded[last_expanded.length - 1].id, state: [] };
				$('.node').each(function () { 
					data.state.push({ id: parseInt($(this).data('node').id), x: parseInt($(this).css('left')), y: parseInt($(this).css('top')) });
				});
				
				that.updateState(data);
				
			} });
		
		},
		drawNodes: function(data, ignore_state) {	
			
			var that = this;			
			var min_frequency = 100;
			var max_frequency = 0;
			var nodes = {};
			var state = that.findState(selected_app.raw.app_id, last_expanded[last_expanded.length -1].id);
			
			if(state && !ignore_state) {
				that.state = state;
			} else {
				that.state = false;
			}
			
			$.each(data.nodes, function(i, node) {
				min_frequency = Math.min(min_frequency, node.frequency);
				max_frequency = Math.max(max_frequency, node.frequency);
			});
			
			$.each(data.nodes, function(i, node) {
				
					var nodeDOM = that.createNode(node, min_frequency, max_frequency); 
					
					if(that.state) {
						that.applyState(that.state, nodeDOM);					
					} else {
						nodeDOM.springy = that.springyGraph.newNode({ container: nodeDOM });
					}

					nodes[node.id] = nodeDOM;

			});
			
			if(!that.state) {
			
				var offset = $('#w_diagram-innerCt').offset();

				$(".node.initial").each(function(i) {
				  var left = $(this).offset().left;
				  var top = $(this).offset().top;
				  
				  $(this).css({
					left: left - offset.left,
					top: top - offset.top
				  });
				});
				
			}
			
			$(".node").removeClass("initial");
			
			return nodes;
			
		},
		connectEdges: function(edges, nodes) {
			
			var that = this;
			
			// MIN / MAX EDGE FREQUENCY
			var min_edge_frequency = 100;
			var max_edge_frequency = 0;
			
			$.each(edges, function(from, to_array) {
				$.each(to_array, function(to, obj) {
					min_edge_frequency = Math.min(min_edge_frequency, obj.frequency);
					max_edge_frequency = Math.max(max_edge_frequency, obj.frequency);
				});
			});
				
			// Find Intersections (source -> target -> source)
			var intersects = [];
			for(from_1 in edges) {
				for(to_1 in edges[from_1]) {
					for(from_2 in edges) {
						for(to_2 in edges[from_2]) {
							if(to_2 == from_1 && from_2 == to_1) {
								intersects.push({ key: from_1 + '_' + to_1, frequency: edges[from_1][to_1].frequency + edges[from_2][to_2].frequency, printed: false });
							}
						}
					}
				}
			}
			
			$.each(edges, function(from, to_array) {

				$.each(to_array, function(to, obj) {

					var is_intersection = false;
					var intersection    = false;
					var percent = 100;
					var is_printed      = false;
					
					$.each(intersects, function(i, x) {
						if((x.key == from + '_' + to || x.key == to + '_' + from)) {
							is_intersection = true;
							if(max_edge_frequency > min_edge_frequency && max_edge_frequency - min_edge_frequency > 0) {
								percent = ((x.frequency - min_edge_frequency) / (max_edge_frequency - min_edge_frequency)) * 100;
							}
							if(percent > 100) {
								percent = 100;
							}
							is_printed = x.printed;
						}
					});
					
					if(is_printed) {
						return true;
					}
					
					if(!is_intersection) {
					
						if(max_edge_frequency > min_edge_frequency && !(obj.frequency - min_edge_frequency == 0 || max_edge_frequency - min_edge_frequency == 0)) {
							percent = ((obj.frequency - min_edge_frequency) / (max_edge_frequency - min_edge_frequency)) * 100;
						}
						
						jsPlumb.makeSource(nodes[from], {
							anchor:"Continuous",
							connector:[ "StateMachine", { curviness:20 } ],
							connectorStyle:{ strokeStyle: colorPercent(percent, myColours), lineWidth: 2, outlineColor:"transparent", outlineWidth:4 },
						});
					
						jsPlumb.connect({
							source:nodes[from], 
							target:nodes[to],
						});
						
					} else {
						
						jsPlumb.makeSource(nodes[from], {
							anchor:"Continuous",
							connector:[ "StateMachine", { curviness:20 } ],
							connectorStyle:{ strokeStyle: colorPercent(percent, myColours), lineWidth: 2, outlineColor:"transparent", outlineWidth:4 },
						});
											
						var arrowCommon = { foldback:0.7, fillStyle: colorPercent(percent, myColours), width:14 };
						var overlays = [
							[ "Arrow", { location:0.7 }, arrowCommon ],
							[ "Arrow", { location:0.3, direction:-1 }, arrowCommon ]
						];
					
						jsPlumb.connect({
							source:nodes[from], 
							target:nodes[to],
							overlays:overlays
						});
							
						// Mark as printed
						$.each(intersects, function(i, x) {
							if(x.key == from + '_' + to || x.key == to + '_' + from) {
								x.printed = true;
							}
						});
						
					}
					
					
					

					
					
				});
			
			});
			
		
		},
		load: function (data) {
			
			var that  = this;
			
			that.data = data;
			
			that.container = $("#w_diagram-innerCt");
			
			that.springyInit();
			that.container.empty();
			
			var nodes = that.drawNodes(data);
			
			that.initPlumb();			
			that.connectEdges(data.edges, nodes);
			
			// Resize container
			//that.resizeContainer();
		
		}
	
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
		track_session();
	});
	
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

	var mask = Ext.getCmp('workflow_mask');
	mask.show();
	
	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/get_workflow_graph",
		params: {
			mode:"get_workflow_graph", // Needed for old controller
			app_id: selected_app.raw.app_id,
			app_name: '',
			depth: 0,
			extensions:'Show All',
			frequency: 0
		},
		success:function(response){
		
			mask.hide();
			try{
				var json = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #8', 'Error while loading graph');
				return;							
			}

			last_expanded.push({ id: 'application' });
			telepath.workflow.diagram.load(json);
			
		},
		failure: function(response, opts) {
		
			mask.hide()
			console.log('server-side failure with status code ' + response.status);
			
		}
	})
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

function update_graph_by_depth() {
	update_graph_by_app();
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

	var mask = Ext.getCmp('workflow_mask');
	mask.show();
	var node = node_selected;
	
	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/expand_node",
		params:{
			mode:'expand_node', // Needed for old controller
			page_id:node.data.id,
			app_id: selected_app.raw.app_id,
			extensions: 'Show All',
			frequency: 0,
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
		},
		failure: function(response, opts) {
			mask.hide()
			console.log('workflow : server-side failure with status code ' + response.status);
		}
	})
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
				telepath.util.blinkError();
				return;		
			}
			if(eval(data['success'])){
				node.set('value', '<span style="color: green;">Saved</span>');
				telepath.util.blinkSaved();
			}
			else if(eval(data['exists'])){
				node.set('value', '<span style="color: red;">Error</span>');
				Ext.Msg.alert('Error', 'Flow '+ flow_name + ' already exists. Please select a different name.');
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
				telepath.util.blinkError();
				return;			
			}
			if (eval(ans['success'])){
				telepath.util.blinkSaved();
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
			
			try{
				var data = Ext.decode(response.responseText)
			}
			catch(err){
				telepath.util.blinkError();
				return;				
			}
			if (eval(data['success'])) {
				node.set('value', '<span style="color: green;">Saved</span>');
				node.set('id', 'flow_' + data['group_id']);
				telepath.util.blinkSaved();
			}
			else if(eval(data['exists'])) {
				node.set('value', '<span style="color: red;">Error</span>');
				Ext.Msg.alert('Error #15', 'Flow '+ flow_name + ' already exsists, choose a different name.');
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

	if (!w_sid_to_record || w_sid_to_record==[])
		return;
	
	//w_sid_to_record
	recordNode.set('text', w_sid_to_record);
	recordNode.set('leaf', false);
	recordNode.set('expandable', true);
	recordNode.set('expanded', true );
	
	recordNode.appendChild({ 'text': 'Stop Recording', 'id': 'stop_' + w_sid_to_record, 'type': 'record_stop', iconCls: 'tree-icon-stop', expandable: false, expanded: false, leaf: true });
	
	Ext.getCmp('w_expand_filter').disable();

	start_blinking_record_sign();
	
	get_pages_by_sid();
	
	w_recording_interval = setInterval(
			function(){
				get_pages_by_sid()
			},
			1000*5
	);
	
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

	Ext.WindowManager.get('w_workflow_record').setHeight(280);
	Ext.getCmp('w_record_start').disable();
	var p_bar=Ext.getCmp('w_detection_progress_bar');
	p_bar.show();
	start_progress_bar();
	
	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/get_sid_by_att_value",	
		params:{
			mode:'get_sid_by_att_value',
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
				w_sid_to_record = ans['SID'];
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
				setTimeout(function () { 
					track_session() 
				}, 3000);
			}	
		}
	})
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


function start_blinking_record_sign(){
	var win;
	win = Ext.getCmp('w_record_sign')
	win.show()
	var is_shown = true;
	w_blink_interval = setInterval(
	function(){
	    if (is_shown){
		win.hide()
		is_shown = !is_shown;
	    }
	    else{
		win.show()
		is_shown = !is_shown;					
	    }							
	},
	1000*1
	);	
}

function stop_blinking_record_sign(){
	clearInterval(w_blink_interval);
	Ext.get('w_record_sign').hide();
}