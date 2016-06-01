function SessionFlow(ssid, epoch, rule_id, ip){

	var that = this;

	this.ssid      = ssid;
	this.epoch     = epoch;
	this.position  = 0;
	this.container = $('.x-panel-body', Ext.getCmp('a_sessionDetails').getEl().dom);
	this.itemWidth = 270;
	this.controlsWidth = 160;
	this.debug     = false;
	this.rule_id   = rule_id;
	this.ip		   = ip;
	that.timer     = false;

	$( window ).resize(function() {
		that.timer = setTimeout(function() { if(window.sessionflow) { window.sessionflow.refresh(); } }, 500);
	});
 
	// Init Controls
	this.container.empty();

	this.flowContainer = $('<div>').addClass('flowContainer');

	this.scrollNextButton = $('<div>').addClass('cmd-flow-next');
	this.scrollPrevButton = $('<div>').addClass('cmd-flow-prev');

	this.ELstatus	     = $('<div>').addClass('flowStatusBar');
	this.ELcmdbar 		 = $('<div>').addClass('flow-cmd-bar');
	this.ELinnerbar      = $('<div>').addClass('flowInnerBar');
	this.ELtimebar       = $('<div>').addClass('flowTimeBar');
	
	this.mode = 'alerts';
	
	var modes = [
		{ id: 'all', 'text': 'All' },
		{ id: 'alerts', 'text': 'Alerts', checked: true },
		{ id: 'business', 'text': 'Business Flows' }
//		{ id: 'business_closed', 'text': 'Closed Business Flows' }
	];
	
	for(var i = 0; i < modes.length; i++) {
	
		var el_cb  = $('<input type="radio">').attr('name', 'flow_display_type').attr('value', modes[i].id);
		var el_lbl = $('<span>').text(modes[i].text);
		
		if(modes[i].checked) {
			el_cb.prop('checked', true);
		}
		
		el_cb.click(function () {
			that.mode     = $(this).val();
			that.position = 0;
			that.refresh();
		});
		
		this.ELcmdbar.append(el_cb).append(el_lbl);
	}
		 
	this.container.append(this.ELstatus)
		      .append(this.ELcmdbar)
			  .append(this.ELinnerbar)
			  .append(this.ELtimebar)
		      .append(this.scrollNextButton)
		      .append(this.scrollPrevButton)
		      .append(this.flowContainer);

	// Init Events
	this.scrollNextButton.click(function () {
		that.scrollNext();
	}).hover(function () { $(this).addClass('hover'); }, function() { $(this).removeClass('hover'); });
	
	this.scrollPrevButton.click(function() {
		that.scrollPrev();
	}).hover(function () { $(this).addClass('hover'); }, function() { $(this).removeClass('hover'); });
	
	// Request Data
	this.fetchData();

}

SessionFlow.prototype.fetchData = function () {
	
	// Do Ajax Calls
	var that = this;
	
	telepath.util.request('/alerts/get_session_flow', { sid: this.ssid, epoch: this.epoch, rule_id: this.rule_id, ip: this.ip }, function (data) { 

		that.all_nodes  = data.items.requests;
		that.alerts_obj = data.items.alerts;
		that.refresh();
			
	}, 'Error retreiving sessionflow.');

}


SessionFlow.prototype.getAlerts = function(request) {
	var alerts = [];
		
	for (var x = 0; x < this.alerts_obj.length; x++) {
		if (request.RID == this.alerts_obj[x].RID) {
			alerts.push(this.alerts_obj[x]);
		}
	}
	
	return alerts;
}

SessionFlow.prototype.refreshTimeBar = function() {
	
	// Update timebar
	if(this.printedArr.length > 0) {
	
		// Format First / Last timestamps
		var ELfirstRequest = $('<span>').addClass('first-request').text(date_format('d/m/y H:i:s', this.printedArr[0].request.date));
		var ELlastRequest  = $('<span>').addClass('last-request').text(date_format('d/m/y H:i:s', this.printedArr[this.printedArr.length - 1].request.date));
		
		// Append
		this.ELtimebar.empty().append(ELfirstRequest).append(ELlastRequest);
	
	}

}

SessionFlow.prototype.refreshBusinessClosed = function() {
	
	// Deviation from regular print
	// Collect closed flows with all alerts happening for this business_id in previous requests
	
	var width = this.container.width();
	var printArr = [];
	var processedRIDS = [];
	
	// Iterate requests
	for(var i = 0; i < this.all_nodes.length; i++) {
	
		var request = this.all_nodes[i];
		var alerts  = this.getAlerts(request);
		
		if(request.business_id && request.business_status == '2') {
			
			processedRIDS.push(request.RID);
			
			var flow = { request: request, alerts: [] };
			
			// Search previous requests for additional alerts
			var previous_alerts = [];
			
			for(x = i - 1; x > 0; x--) {
				
				var prev_request = this.all_nodes[x];
				
				// Skip over this request if already processed
				if($.inArray(prev_request.RID, processedRIDS) > -1) {
					continue;
				}
				
				if(prev_request.business_id && prev_request.business_id == request.business_id) {
					
					if(prev_request.business_status == '2') {
						
						// We've detected this same flow closes twice, meaning its another instance of it.
						break;
						
					} else {
						
						processedRIDS.push(prev_request.RID);
						// Prepend previous alerts to this flow
						var prev_alerts  = this.getAlerts(prev_request);
						
						if(prev_alerts.length > 0) {
							for(z = 0; z < prev_alerts.length; z++) {
								previous_alerts.unshift(prev_alerts[z]);
							}
						}
						
					}
					
				}
				
			}
			
			// Append previous alerts
			if(previous_alerts.length > 0) {
				for(z = 0; z < previous_alerts.length; z++) {
					flow.alerts.push(previous_alerts[z]);
				}
			}
			
			// Append closing request alerts
			if(alerts.length > 0) {
				for(z = 0; z < alerts.length; z++) {
					flow.alerts.push(alerts[z]);
				}
			}

			printArr.push(flow);
			
		}
		
	}
	
	for(var i = this.position; i < printArr.length && this.printedArr.length < this.itemsToPrint; i++) {
		
		// Generate HTML
		printArr[i].dom = this.getFlowTable(printArr[i]);
		
		// Add Arrow
		if(this.printedArr.length > 0) {
			this.flowContainer.append('<span class="flow-arrow"></span>');
		}
		
		// Append
		this.printedArr.push(printArr[i]);
		this.flowContainer.append(printArr[i].dom);				
			
	}
	
	// Center the container
	var offset = Math.floor((width - (this.printedArr.length * this.itemWidth)) / 2 - (this.controlsWidth / 2)) + 20; // 20 is for arrow
	this.flowContainer.css('padding-left', offset + 'px');

	// Show/Hide controls
	if(this.position == 0) {
		this.scrollPrevButton.hide();
	} else {
		this.scrollPrevButton.show();
	}
	
	if(this.position + this.printedArr.length + 1 > printArr.length) {
		this.scrollNextButton.hide();
	} else {
		this.scrollNextButton.show();
	}
	
	if(this.printedArr.length == 0 || printArr.length == 0) {
		this.ELtimebar.empty();
		this.ELstatus.html('Nothing to display.');
		return;
	}
	
	var alerts_count = 0;
	for(var i = 0 ; i < printArr.length ; i++) {
		alerts_count += printArr[i].alerts.length;
	}
	
	// Update status
	var html = 'Displaying from ' + (this.position + 1) + ' to ' + (this.position + this.printedArr.length) + ' in ' + printArr.length + ' closed flows and total of ' + alerts_count + ' alerts.';
	this.ELstatus.html(html);
	
	// Update timebar
	this.refreshTimeBar();
	
}

SessionFlow.prototype.refresh = function () {
	
	var width = this.container.width();
	
	// Cleanup 
	$('.hidden-source, .hidden-target, ._jsPlumb_overlay, ._jsPlumb_connector', this.container).remove();
	this.flowContainer.empty();
	this.itemsToPrint = Math.floor((width - this.controlsWidth) / this.itemWidth);
	this.printedArr   = [];
	
	// Variables
	var totalPrinted = 0;
	var printArr     = [];

	if(this.mode == 'business_closed') {
		this.refreshBusinessClosed();
		return;
	}
	
	// Print Domain Label	
	if(this.all_nodes[0] && !this.domainLabel) {
		this.domainLabel = $('<label>').html('<span>Domain:&nbsp;</span><b>' + this.all_nodes[0].subdomain_name + '</b>');
		this.ELinnerbar.append(this.domainLabel);
	}

	// Iterate requests
	for(var i = 0; i < this.all_nodes.length; i++) {
	
		var request = this.all_nodes[i];
		var alerts  = this.getAlerts(request);
		
		// Toggle print mode and collect print array
		switch(this.mode) {
			
			case 'all': // Print all
				printArr.push( { request: request, alerts: alerts } );
			break;
			
			case 'business': // Print Business flows only
				if(request.business_id) {
					printArr.push( { request: request, alerts: alerts } );
				}
			break;
			
			case 'alerts': // Print Alerts only
				if(alerts.length > 0) {
					printArr.push( { request: request, alerts: alerts } );
				}
			break;
		}
		
	}
	
	for(var i = this.position; i < printArr.length && this.printedArr.length < this.itemsToPrint; i++) {
		
		// Generate HTML
		// printArr[i].dom = this.getRequestTable(printArr[i]);
		
		// Add Arrow
		/*if(this.printedArr.length > 0) {
			this.flowContainer.append('<span class="flow-arrow"></span>');
		}*/
		
		// Append
		this.printedArr.push(printArr[i]);
		
		// this.flowContainer.append(printArr[i].dom);				
			
	}
	
	// Center the container
	var offset = Math.floor((width - (this.printedArr.length * this.itemWidth)) / 2 - (this.controlsWidth / 2)) + 20; // 20 is for arrow
	this.flowContainer.css('padding-left', offset + 'px');

	// Show/Hide controls
	if(this.position == 0) {
		this.scrollPrevButton.hide();
	} else {
		this.scrollPrevButton.show();
	}
	
	if(this.position + this.printedArr.length + 1 > printArr.length) {
		this.scrollNextButton.hide();
	} else {
		this.scrollNextButton.show();
	}
	
	if(this.printedArr.length == 0 || printArr.length == 0) {
		this.ELtimebar.empty();
		this.ELstatus.html('Nothing to display.');
		return;
	}
	
	var html = 'Displaying from ' + (this.position + 1) + ' to ' + (this.position + this.printedArr.length) + ' ';
	// Display counts
	switch(this.mode) {
			
			case 'all': // Print all
				
				var business_count = 0;
				var business_count_closed = 0;
				for(var i = 0; i < printArr.length; i++) {
					if(printArr[i].request.business_id) {
						if(printArr[i].request.business_status == '2') business_count_closed++;
						business_count++;
					}
					
				}
				
				var reqs = (printArr.length > 999) ? '1000+' : printArr.length;
				
				html += 'in ' + reqs + ' requests, ' + business_count + ' business flow requests (' + business_count_closed + ' closed) and total of ' + this.alerts_obj.length + ' alerts.';
				
			break;
			
			case 'business': // Print Business flows only
				
				var alerts_count = 0;
				for(var i = 0 ; i < printArr.length ; i++) {
					alerts_count += printArr[i].alerts.length;
				}
				
				var reqs = (printArr.length > 999) ? '1000+' : printArr.length;
				
				html += 'in ' + reqs + ' business flow requests which include ' + alerts_count + ' alerts.';
				
			break;
			
			case 'alerts': // Print Alerts only
			
				var alerts_count = 0;
				for(var i = 0 ; i < printArr.length ; i++) {
					alerts_count += printArr[i].alerts.length;
				}
				
				var reqs = (printArr.length > 999) ? '1000+' : printArr.length;
				
				html += 'in ' + reqs + ' requests which include a total of ' + alerts_count + ' alerts.';
				
			break;
	}
	
	this.ELstatus.html(html);

	// Update timebar
	this.refreshTimeBar();
	
	var that = this;
	
	var RIDS = [];
	for(var i = 0; i < this.printedArr.length; i++) {
		if(!that.printedArr[i].request.params) {
			RIDS.push(that.printedArr[i].request.RID);
		}
	}
	
	if(RIDS.length == 0) {
		that.printRequests();
	} else {
		telepath.util.request('/alerts/get_session_flow_params', { sid: this.ssid, rids: RIDS.join(','), epoch: this.epoch }, function (data) {
			
			for(var i = 0; i < that.printedArr.length; i++) {
				var RID = that.printedArr[i].request.RID;
				if(data.items[RID]) {
					that.printedArr[i].request.params = data.items[RID];
				}
			}
		
			that.printRequests();
			
		}, 'Error getting parameters for request');
	}
	
	
	
	
	
	
}

SessionFlow.prototype.printRequests = function() { 

	this.flowContainer.empty(); // Fix, sometimes would print double otherwise
	
	for(var i = 0; i < this.printedArr.length; i++) {
	
		// Generate HTML
		this.printedArr[i].dom = this.getRequestTable(this.printedArr[i]);

		// Add Arrow
		if(i > 0) {
			this.flowContainer.append('<span class="flow-arrow"></span>');
		}
		
		this.flowContainer.append(this.printedArr[i].dom);				
		
	}
	
	// Detect Business Flows
	this.flows = this.detectFlows(this.printedArr);
	
	// Draw Business Flows
	this.linkFlows(this.flows, this.printedArr);	

}

SessionFlow.prototype.printBusinessFlows = function() { }

SessionFlow.prototype.detectFlows = function (printedArr) {
	
	var flows = [];
	
	function in_game(source) {
		
		if(flows) {
			
			for(var z = 0; z < flows.length; z++) {
				if(flows[z].open == source.RID || flows[z].close == source.RID) {
					return true;
				}
			}
		
		}
		
		
		return false;
	
	}
	function match_bid(target, source, status) {
		
		var logic_phase1 = source.business_id && target.business_id; // Both have BID
		var logic_phase2 = parseInt(source.business_id) == parseInt(target.business_id); // BID's match
		var logic_phase3 = target.business_status == status; // Status checks out
		var logic_phase4 = !in_game(target) && !in_game(source); // Not mapped in a flow yet
		
		return logic_phase1 && logic_phase2 && logic_phase3 && logic_phase4;
	
	}
	
	for(var i = 0; i < printedArr.length; i++) {

		if(printedArr[i].request.business_id) {

			// If we detected the flow before, don't do it again
			var found = in_game(printedArr[i].request);
			
			if(!found) {
				
				// Detect the flow
				
				var flow_open_RID   = false;
				var flow_close_RID  = false;   
				
				// LOGIC V2
				
				if(printedArr[i].request.business_status == '2') { // Search LEFT
					
					// Set close to current
					flow_close_RID = printedArr[i].request.RID;
					
					// In Visible
					for(var x = i; x > 0; x--) {
						
						if(match_bid(printedArr[x].request, printedArr[i].request, '1')) { // We have open BID visible on screen
						   arrow = 'left_visible';
						   flow_open_RID = printedArr[x].request.RID;
						   break;
						}
						
					}
					// In All
					if(!flow_open_RID) {
						
						// Step back from current to start
						for(var x = i; x >= 0; x--) {
							
							if(match_bid(this.all_nodes[x], printedArr[i].request, '1')) { // We have open BID non visible in previous requests
								arrow = 'left_hidden';
								flow_open_RID = this.all_nodes[x].RID;
								break;								
							}
							
						}
							
					}
					
					// Ouside Scope == One request flow || Not recorder opener status
					if(!flow_open_RID) {
						arrow = 'self';
					}
					
				}
				
				if(printedArr[i].request.business_status == '1') { // Search RIGHT
					
					// Set open to current
					flow_open_RID = printedArr[i].request.RID;
					
					// In Visible
					for(var x = i; x < printedArr.length; x++) {
						
						if(match_bid(printedArr[x].request, printedArr[i].request, '2')) { // We have close BID visible on screen
							arrow = 'right_visible';
							flow_close_RID = printedArr[x].request.RID;
							break;
						}
					
					}
					
					// In All
					if(!flow_close_RID) {

						for(var x = i; x < this.all_nodes.length; x++) {
						
							if(match_bid(this.all_nodes[x], printedArr[i].request, '2')) { // We have close BID non visible in next requests
								arrow = 'right_hidden';
								flow_close_RID    = this.all_nodes[x].RID;
								break;
							}
							
						}
							
					}
					
					// Outside Scope == Didnt record closing status yet
					if(!flow_close_RID) {
						arrow = 'right_infinity';
					}
					
				}

				// Append the flow
				flows.push({ open: flow_open_RID, close: flow_close_RID, business_id: printedArr[i].request.business_id, arrow: arrow });
					
			}

		}
		
	}
	
	return flows;

}

SessionFlow.prototype.linkFlows = function(flows, printedArr) {
	
	function find_el_by_rid(els, RID) {
		
		for(var i = 0; i < els.length; i++) {
			if(els[i].request.RID == RID) {
				return els[i];
			}
		}
		
		return false;
		
	}
	
	for(var i = 0; i < flows.length; i++) {
		
		var flow = flows[i];
		
		switch(flow.arrow) {
			
			case 'left_hidden':
				
				var target = find_el_by_rid(printedArr, flow.close);
				var hidden_source = $('<div>').css({ position: 'absolute', left: 50, top: (i * 20) + 300 }).addClass('hidden-source');
				
				this.container.append(hidden_source);
				var source = { dom: hidden_source };
				
				if(source && target) {
					this.connect(source, target, flow.business_id, i, flow.arrow);
				}
				
			break;
			
			case 'left_visible':
			
			break;
			
			case 'self':
				
				var source = find_el_by_rid(printedArr, flow.close);
				var target = find_el_by_rid(printedArr, flow.close);
				
				if(source && target) {
					this.connect(source, target, flow.business_id, i);
				}
				
			break;
			
			case 'right_visible':
			
				var source = find_el_by_rid(printedArr, flow.open);
				var target = find_el_by_rid(printedArr, flow.close);
				
				if(source && target) {
					this.connect(source, target, flow.business_id, i);
				}
				
			break;
			
			case 'right_hidden':
				
				var source = find_el_by_rid(printedArr, flow.open);
				var hidden_target = $('<div>').css({ position: 'absolute', right: 50, top: (i * 20) + 300 }).addClass('hidden-target');
				
				this.container.append(hidden_target);
				var target = { dom: hidden_target };
				
				if(source && target) {
					this.connect(source, target, flow.business_id, i, flow.arrow);
				}
				
			break;
			
			case 'right_infinity':
			
				Ext.Msg.alert('Business Flows Error', 'Detected a non closing business flow on RID:: ' + flow.open);
				
			break;
		
		}
			
	}

}

SessionFlow.prototype.connect = function(source, target, business_id, ident, arrow_type) {
	
	var label_location = source == target ? 0.5 : (ident + 1) * 30;
	var stub;
	
	switch(arrow_type) {
		case 'left_hidden':
			stub = [0, (ident + 1) * 20];
		break;
		case 'right_hidden':
			stub = [(ident + 1) * 20, 0];
		break;
		default:
			stub = [(ident + 1) * 20, (ident + 1) * 20];
		break;
	}
	
	jsPlumb.connect({
		source:$(source.dom), 
		target:$(target.dom), 			   	
		connector: [ "Flowchart", { stub: stub, gap:0, cornerRadius:5, alwaysRespectStubs:true } ],
		cssClass:"c1",
		anchors:["BottomCenter"],
		paintStyle:{ 
			lineWidth:2,
			strokeStyle:"#ccc",
			outlineWidth:1,
			outlineColor:"#666",
		},
		endpointStyle:{ radius: 1, fillStyle:"#eee" },
		overlays : [
			["Label", {													   					
				cssClass:"component label",
				label : Ext.getStore('general_flow_group_store').getById(business_id).data.group, 
				location: label_location,
				events:{
					"click":function(label, evt) {

					}
				}
			}]
		]
	}); 

}

SessionFlow.prototype.scrollPrev = function () {
	if(this.position > 0) {
		this.position--;		
	}
	this.refresh();
}

SessionFlow.prototype.scrollNext = function () {
	this.position++;
	this.refresh();
}

SessionFlow.prototype.getLapse = function(request) {
	
	var lapse = (request.date) - (this.printedArr.length > 0 ? this.printedArr[0].request.date : request.date);
	var hours = parseInt( lapse / 3600 ) % 24;
	var minutes = parseInt( lapse / 60 ) % 60;
	var seconds = Math.floor(lapse % 60);
	var lapseFormatted = '';
	
	if(hours > 0) {
		lapseFormatted += (hours < 10 ? "0" + hours : hours) + ':';
	}
	lapseFormatted += (minutes < 10 ? "0" + minutes : minutes) + ':' + (seconds  < 10 ? "0" + seconds : seconds);
	
	return lapseFormatted;
	
}

SessionFlow.prototype.getFlowTable = function(item) {
	
	// TODO:: IS ALERT?
	var alert_name = "";
	var alert_value = "";
	var scoreRow = false;

	var request = item.request;
	var alerts  = item.alerts;
	
	var table = $('<table>').addClass('session-flow-table');

	var row = $('<tr>');

	var lapseFormatted = this.getLapse(request);
	
	// DATE
	row.clone().append(this.getCell('Lapse:'))
		   .append(this.getCell(lapseFormatted))
		   .appendTo(table);

	// ALERTS
	scoreRow = this.renderAlerts(table, alerts);
	
	// ZEBRA!
	$('tr:even', table).addClass('even');
	$('tr:odd', table).addClass('odd');

	// IP
	row.clone().append(this.getCell('IP:'))
		   .append(this.getCell(request.user_ip))
		   .appendTo(table);
	
	// Score
	if(scoreRow) {
		table.append(scoreRow);
	}
		
	var wrap       = $('<div>').addClass('session-flow-wrap');
	var inner_wrap = $('<div>').addClass('session-flow-inner-wrap');
	var table_wrap = $('<div>').addClass('session-flow-table-wrap');
	var title_wrap = $('<div>').addClass('session-flow-title-wrap');

	var span = $('<span>').text(Ext.getStore('general_flow_group_store').getById(request.business_id).data.group);
	
	title_wrap.append(span);
	table_wrap.append(table);
	
	inner_wrap.append(title_wrap).append(table_wrap);
	wrap.append(inner_wrap);

	return wrap;
	
}

SessionFlow.prototype.getRequestTable = function(item) {
	
	// TODO:: IS ALERT?
	var alert_name = "";
	var alert_value = "";
	var scoreRow = false;

	var request = item.request;
	var alerts  = item.alerts;
	
	var table = $('<table>').addClass('session-flow-table');

	var row = $('<tr>');

	var lapseFormatted = this.getLapse(request);
	
	// DATE
	row.clone().append(this.getCell('Elapsed Time:'))
		   .append(this.getCell(lapseFormatted))
		   .appendTo(table);

	if(request.user && request.user != '') {
		
		// User
		row.clone().append(this.getCell('User:'))
				   .append(this.getCell('<span title="User Reputation: ' + (request.user_score ? request.user_score : 0)  + '%">' + request.user + '</span>'))
				   .appendTo(table);
	
	}	   
		   
	// ALERTS
	scoreRow = this.renderAlerts(table, alerts);
		   
	// PARAMETERS
	if(request.params) {
		
		for (var i=0; i < request.params.length;i++)
		{
			var z = request.params[i];
			// Printed elsewhere ^^
			if (z['val'] == alert_value && 
				z['att_name'] == alert_name &&
				alert_value != "") {
				continue;
			}
			
			var att_title = decodeURIComponent(z['att_name']);
			
			if(z['att_alias'] != '') {
				att_title = '<span style="color:green;">' + z['att_alias'] + '</span>';
			}
			if(z['mask'] == '1') {
				att_title = att_title + '&nbsp;<span style="color:red;">*</span>';
			}

			var att_name_cell = this.getCell(att_title);
			
			this.bindRightClickHandler(att_name_cell, z);
			
			row.clone().append(att_name_cell)
				   .append(this.getCell(z['val']))
				   .appendTo(table);
		}
	
	}
	

	// ZEBRA!
	$('tr:even', table).addClass('even');
	$('tr:odd', table).addClass('odd');

	// IP
	row.clone().append(this.getCell('IP:'))
		   .append(this.getCell(request.user_ip))
		   .appendTo(table);
	
	// Score
	if(scoreRow) {
		table.append(scoreRow);
	}
	
	if(this.debug) {
		
		row.clone().append(this.getCell('RID:'))
		  .append(this.getCell(request.RID))
		  .appendTo(table);
	
		if(request.business_id) {
			
			row.clone().append(this.getCell('BID:'))
			   .append(this.getCell(request.business_id))
			   .appendTo(table);
			
		}
	
	}
	
	var wrap       = $('<div>').addClass('session-flow-wrap');
	var inner_wrap = $('<div>').addClass('session-flow-inner-wrap');
	var table_wrap = $('<div>').addClass('session-flow-table-wrap');
	var title_wrap = $('<div>').addClass('session-flow-title-wrap');

	var url  = telepath.util.renderUrl(request.subdomain_name, request.ssl_flag, request.display_path);
	var link = $('<a>').addClass('icon-link').text('').attr('href', url).attr('target', '_blank').hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover') });
	var page_title = request.title != '' ? request.title : request.display_path;
	var span = $('<span>').text(page_title).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover') }).css('text-decoration', 'underline');
	
	title_wrap.append(span).append(link);
	table_wrap.append(table);
	
	this.bindRightClickHandler(span, request);
	
	inner_wrap.append(title_wrap).append(table_wrap);
	wrap.append(inner_wrap);

	return wrap;

}

SessionFlow.prototype.renderAlerts = function(table, alerts) {
	
	var row = $('<tr>');
	var scoreRow = false;
	
	// Print request alerts
	if(alerts.length > 0) {
	
		for(var i = 0; i < alerts.length; i++) {
			var alert = alerts[i];
			// Alert
			row.clone().append(this.getCell('Alert:', 'red'))
				   .append(this.getCell(alert.name,'red'))
				   .appendTo(table);
			
			var is_aspect = false;
			var label  = '';
			var arr_id = 0;

			switch(alert.aspect) {
				
				default:
				case '':
				case 'aspect':
								
					is_aspect = true;

					// Sort out Literal Score				
					var literal_score="";
		
							if( ((alert.numeric_score+"").length>0) && ((alert.literal_score+"").length==0) )
							{
									if(parseInt(alert.numeric_score)<40)
											literal_score="Low";
									else if(parseInt(alert.numeric_score)>=40 && parseInt(alert.numeric_score)<70)
											literal_score="Medium";
									else
											literal_score="High";
							}
							else
							{
									literal_score=alert.literal_score;
							}
					
					// Alert Has Name?
							if (alert.att_name)
							{
								
							var alert_value_escaped = $('<div>').text(alert.value).html();

							// Custom Name
							row.clone().append(this.getCell(alert.att_name + ':', 'red'))
								   .append(this.getCell(alert_value_escaped, 'red'))
								   .appendTo(table);		                	

							alert_name = alert.att_name;
							alert_value = alert_value_escaped;

							} else {
						// Score
						row.clone().append(this.getCell('Value:'))
							   .append(this.getCell(alert.value))
							   .appendTo(table);	                	

					}
					
					// Score
					scoreRow = row.clone().append(this.getCell('Score:'))
						   .append(this.getCell(literal_score + " (" + alert.numeric_score + ")"));
						   

				break;

				case 'query':
					label  = 'Query';
					arr_id = 'query_score';
				break;
				case 'request':
					label  = 'Request';
					arr_id = 'query_score';
				break;
				case 'flow':
					label  = 'Flow';
					arr_id = 'flow_score';
				break;
				case 'landing':
					label  = 'Landing';
					arr_id = 'landing_normal';
				break;
				case 'geo':
					label  = 'Geo';
					arr_id = 'geo_normal';
				break;
				case 'hour':
					label  = 'Hour';
					arr_id = 'date';
				break;
				case 'week_day':
					label  = 'WeekDay';
					arr_id = 'date';
				break;
				
			}
			try {
			if(!is_aspect) {
				row.clone().append(this.getCell(label + ':'))
					   .append(this.getCell(request[arr_id]))
					   .appendTo(table);
			}
			} catch(e) {}
			
		}
	}
	
	return scoreRow;

}

SessionFlow.prototype.getCell = function (html, className) {

	var result = $('<td>').html(html);
	if(className) {
		result.addClass(className);
	}
	return result;
	
}

SessionFlow.prototype.bindRightClickHandler = function(element, data) {
	
	var that = this;
	
	$(element).css('cursor', 'pointer');
	
	// Page -- Alias Add
	var menu_page_alias_add  = {
		text: 'Add Page Alias',
		iconCls: 'context-menu-aliasAdd',
		handler: function () {
			telepath.handlers.page.aliasAdd(that.selectedRecord.page_id, function (data) {
				that.fetchData();
			});
		}
	}
	// Page -- Alias Remove
	var menu_page_alias_remove  = {
		text: 'Remove Page Alias',
		iconCls: 'context-menu-aliasRemove',
		handler: function () {
			telepath.handlers.page.aliasRemove(that.selectedRecord.page_id, function (data) {
				that.fetchData();
			});
		}
	}
	// Page -- Alias Edit
	var menu_page_alias_edit  = {
		text: 'Edit Page Alias',
		iconCls: 'context-menu-aliasEdit',
		handler: function () {
			telepath.handlers.page.aliasEdit(that.selectedRecord.page_id, that.selectedRecord.title, function (data) {
				that.fetchData();
			});
		}
	}

	// Param -- Alias Add
	var menu_param_alias_add  = {
		text: 'Add Parameter Alias',
		iconCls: 'context-menu-aliasAdd',
		handler: function () {
			telepath.handlers.param.aliasAdd(that.selectedRecord.att_id, function (data) {
				that.fetchData();
			});
		}
	}
	// Param -- Alias Remove
	var menu_param_alias_remove  = {
		text: 'Remove Parameter Alias',
		iconCls: 'context-menu-aliasRemove',
		handler: function () {
			telepath.handlers.param.aliasRemove(that.selectedRecord.att_id, function (data) {
				that.fetchData();
			});
			
		}
	}
	// Param -- Alias Edit
	var menu_param_alias_edit  = {
		text: 'Edit Parameter Alias',
		iconCls: 'context-menu-aliasEdit',
		handler: function () {
			telepath.handlers.param.aliasEdit(that.selectedRecord.att_id, that.selectedRecord.att_alias, function (data) {
				that.fetchData();
			});
		}
	}
	
	// Context Param Mask
	var menu_param_mask  = {
		text: 'Mask Parameter',
		iconCls: 'context-menu-paramMask',
		handler: function () {
			telepath.handlers.param.maskAdd(that.selectedRecord.att_id, that.selectedRecord.att_alias != '' ? that.selectedRecord.att_alias : that.selectedRecord.att_name, function (data) {
				that.fetchData();
			});
		}
	}
	
	// Context Param Un-Mask
	var menu_param_unmask  = {
		text: 'Un-Mask Parameter',
		iconCls: 'context-menu-paramUnMask',
		handler: function () {
			telepath.handlers.param.maskRemove(that.selectedRecord.att_id, that.selectedRecord.att_alias != '' ? that.selectedRecord.att_alias : that.selectedRecord.att_name, function (data) {
				that.fetchData();
			});
		}
	}
		
	$(element).data('data', data);
	$(element).bind('contextmenu', function (e) {
		
		e.preventDefault();
		
		// Track mouse
		var posx = 0;
		var posy = 0;

		if (e.pageX || e.pageY) {
			posx = e.pageX;
			posy = e.pageY;
		} else if (e.clientX || e.clientY) {
			posx = e.clientX + document.body.scrollLeft + document.documentElement.scrollLeft;
			posy = e.clientY + document.body.scrollTop + document.documentElement.scrollTop;
		}
		
		// Figure menu
		
		var data = $(element).data('data');
		
		that.selectedRecord = data;
		
		if(data.page_id) { data.type = 'page';	}
		if(data.att_id)  { data.type = 'parameter'; }
		
		var menuItems = [];

		switch(data.type) {
			
			default:
			case 'page':
				
				if(data.title == '') {
					menuItems.push(menu_page_alias_add);
				} else {
					menuItems.push(menu_page_alias_edit);
					menuItems.push(menu_page_alias_remove);
				}
				
			break;
			case 'parameter':
				
				if(data.mask == '0') {
					menuItems.push(menu_param_mask);
				} else {
					menuItems.push(menu_param_unmask);
				}

				menuItems.push({ xtype: 'menuseparator' });
				
				if(data.att_alias == '') {
					menuItems.push(menu_param_alias_add);
				} else {
					menuItems.push(menu_param_alias_edit);
					menuItems.push(menu_param_alias_remove);
				}
			
			break;
			case 'alert':
			break;
			case 'rule':
			break;
			
		}
		
		// Create and Show
		var contextMenu = new Ext.menu.Menu({ items : menuItems });
		contextMenu.showAt({ x: posx, y: posy });
		
	});
	
}