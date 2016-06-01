/* ND -- For clarity moved workflow record code here */

function get_pages_by_ip() {

	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/record",
		params:{
			mode:'record',
			ip:w_ip_to_record,
			start_time:w_recording_start_time
		},
		success:function(response){
			try{
				var json = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #6', 'Error while trying to get the pages from Telepath system.');
				return;
			}

			// RecordTree -- Create Mode
			load_workflow_tree(json);
			
			// TODO:: RecordTree -- Append mode
			// update_workflow_tree(json);

		}
	})
}

/*
 * Function: get_pages_by_sid
 * 
 * Invoked:
 * 	Privately by start_recording. This function gets all the pages visited by the user from a certain time
 * 	and loads all the pages to the graph and to the graph.
 * 	
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<start_recording>
 * 
 */
function get_pages_by_sid() {

	Ext.Ajax.request({
		url: telepath.controllerPath + "/workflow/record",
		params:{
			mode:'record',
			sid:w_sid_to_record,
			start_time:w_recording_start_time
		},
		success:function(response){
			try{
				var json = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #6', 'Error while trying to get the pages from Telepath system.');
				return;
			}

			// RecordTree -- Create Mode
			load_workflow_tree(json);
			
			// TODO:: RecordTree -- Append mode
			// update_workflow_tree(json);

		}
	})
}

function load_workflow_tree(json) {
	
	// Sample Output
	
	var start_index = recordNode.childNodes.length;
	// Parse RECORD JSON to TREESTORE data

	var rootnode = {'expanded': true, children: [] };

	if(!json.nodes || !json.nodes[0]) {

		rootnode.expanded = false;
		rootnode.expandable = false;
		
	} else {

		$.each(json.nodes, function(i, v) { 
			
			
				var pagenode = { 'text': v.page_name, 
							     'type': 'flow_page', 
								 'expanded': false, 
								 'expandable': true, 
								 'children': [], 
								 'iconCls': 'tree-icon-page', 
								 'id': 'flow_page_' + v.page_id + '_' + (start_index + i),
								 'value': v.page_id
								};
				if(!v.params && !v.params[0]) {
					
					pagenode.expanded = false;
					pagenode.expandable = false;
				
				} else {
				
					$.each(v.params, function(x, z) {
						
						var paramtype = 'attribute';
						
						var paramnode = { 
										  'text': decodeURIComponent(z.att_name), 
										  'type': paramtype, 
										  'value': z.data, 
										  'expandable': false, 
										  'expanded': false,
										  'id': 'record_' + v.page_id + '_param_' + z.att_id + '_' + (start_index + i) + '_' + x,
										  'iconCls': 'tree-icon-attribute' + z.att_source 
										};
						
						pagenode.children.push(paramnode);
						
					});			
					
				}
				
				var newnode = recordNode.appendChild(pagenode);
				
				// Exclude params by default;
				v.include = false;
				
				newnode.raw.data = v;
			
				// Advance the recording time start so we only get new nodes from the server
				if(v.date > w_recording_start_time) {
					w_recording_start_time = v.date;
				}
			
		});
		
	}
	
	// Init RF 
	if(!window.recordflow) {
		window.recordflow = new RecordFlow();
	}
	
	// Refresh RF
	window.recordflow.refresh(recordNode);
	
}

/* 021013 -- New Record Flow */

function RecordFlow() {
	
	this.itemWidth = 330;

}

RecordFlow.prototype.clear  = function () {
	
	$('.flowContainer', this.container).remove();
	$('#halfviz').show();
	
}

RecordFlow.prototype.refresh = function (recordNode) {
	
	// Clear outer
	this.container = $('#w_diagram-innerCt');
	
	var ScrollTop = $('.flowContainer', this.container).scrollTop();
	
	$('.flowContainer', this.container).remove();
	$('#halfviz').hide();
	
	// Clear inner
	this.flowContainer = $('<div>').addClass('flowContainer').addClass('record');
	this.container.append(this.flowContainer);
	
	var i;
	
	if(recordNode.data.type == 'flow') {
		i = 0;
	} else {
		i = 1;
	}
	
	var totalPrinted = 0;
	var totalLimit   = recordNode.childNodes.length;
	
	for(i; i < totalLimit; i++) { // Skip stop node (1)
		if(recordNode.childNodes[i].raw.data) {
			
			var requestTable = this.getRequestTable(recordNode.childNodes[i]);
			this.flowContainer.append(requestTable);
			totalPrinted++;
			
		}
	}
	
	$(this.flowContainer).append('<div class="clear"></div>');
	
	$('.flowContainer', this.container).scrollTop(ScrollTop);

}

RecordFlow.prototype.getRequestTable = function(pageNode) {
	
	var request    = pageNode.raw.data;
	var recordNode = pageNode.parentNode;
	
	var table = $('<table>').addClass('record-flow-table');

	function getCell(html, className) {
		var result = $('<td>').html(html);
		if(className) {
			result.addClass(className);
		}
		return result;
	}
	
	var row = $('<tr>');

	if(request.date) {
		
		var date = new Date(request.date*1000);
		var hours = date.getHours();
		var minutes = date.getMinutes();
		var seconds = date.getSeconds();
		
		row.clone().append(getCell('Time'))
						.append(getCell(hours + ':' + minutes + ':' + seconds))
						.appendTo(table);
	
	}
	
	// PARAMETERS
	if(request.params && request.params.length > 0) {
		
		for (var i=0; i < request.params.length;i++) {
			
			var z = request.params[i];
			var cell = $('<td>');
			
			// Our magic attribute
			if(z['att_id'] == '18' || z['att_name'] == 'hybridrecord') {
				continue;
			}
			
			var link = $('<a>').attr('href', '#').text(decodeURIComponent(z['att_name'])).attr('rel', z['att_id']);
			
			var valueCell = $('<td>');
			var valueEdit = $('<span>').addClass('icon-edit').attr('rel', z['att_id']);
			var valueLink = $('<span>').attr('href', '#').text(decodeURIComponent(z['data'])).attr('rel', z['att_id']);
			
			//if(!z.include) {
			//	valueLink.addClass('excluded');
			//}
			
			link.click(function () {
				
				var id = $(this).attr('rel');
				$.each(pageNode.childNodes, function(i, record) {
					var recordID = record.data.id.split('_')[2];
					if(id == recordID) {
						pageNode.expand();
						telepath.workflow.apptree.getSelectionModel().select(record);
					}
				});
				
			});
			
			valueEdit.click(function () {
				
				var id = parseInt($(this).attr('rel'));
				
				$(this).parent().find('span').hide();
				
				var valueInput = $('<input>').val($(this).parent().find('span:last').text()).attr('rel', id);
				valueInput.blur(function () {
					
					var id    = parseInt($(this).attr('rel'));
					var value = $(this).val();
					if(value == '') value = '*';
					
					$(this).parent().find('span').show();
					$(this).parent().find('span:last').text(value).removeClass('excluded');
					$(this).remove();
					
					$.each(request.params, function (i, param) {
						if(param.att_id == id) {
							param.data = value;
							param.include = true;
						}
					});
					
				});
				
				$(this).parent().append(valueInput);
				valueInput.focus();
				
			}).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover') });
			
			cell.append(link);
			valueCell.append(valueEdit).append(valueLink);
			
			row.clone().append(cell)
				   .append(valueCell)
				   .appendTo(table);
				   
		}
	
	}
	

	// Look, a Zebra!
	$('tr:even', table).addClass('even');
	$('tr:odd', table).addClass('odd');
	
	var app = recordNode.parentNode.raw.app_domain;
	var ssl = recordNode.parentNode.raw.ssl_flag;
	
	var url = (ssl==1?"https":"http")+"://"+ app + request.path;
	var link = $('<a>').addClass('icon-link').text('').attr('href', url).attr('target', '_blank').hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover') });
	var span = $('<span>').text(request.page_name != "" ? request.page_name : request.path).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover') }).css('text-decoration', 'underline');
	
	span.click(function () {
		pageNode.expand();
		telepath.workflow.apptree.getSelectionModel().select(pageNode);
	});
	
	var wrap       = $('<div>').addClass('record-flow-wrap');
	var inner_wrap = $('<div>').addClass('record-flow-inner-wrap');
	var table_wrap = $('<div>').addClass('record-flow-table-wrap');
	var title_wrap = $('<div>').addClass('record-flow-title-wrap');
	
	title_wrap.append(span).append(link);
	table_wrap.append(table);
	inner_wrap.append(title_wrap).append(table_wrap);
	wrap.append(inner_wrap);

	return wrap;

}