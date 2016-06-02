telepath.alert = {
	
	grabNames: function(arr) {
	
		var result = '';
		$.each(arr, function(i, x) {
			result = result + x.key + ', ';		
		});
		if(result.length > 0) { result = result.substr(0, result.length - 2); }
		return result;
		
	},
	rowFormatter: function(row,mode) {
		
		if(row._source) { row = row._source }

		//Do not show "city" in the Dashboard
		if (mode == 'dashboard'){
			var identification = (row.user!='' ? {key: 'user', value: row.user } : { key: 'IP', value: row.ip_orig } );
			return {
				raw: row,
				itemID: row.sid,
				icon: 'alert',
				checkable: row.checkable,
				count: row.alerts_count,
				progbar: true,
				progbarValue: row.ip_score,
				time: row.date,
				title:  telepath.alert.grabNames(row.alerts_names),
				details: [
					identification,
					{ key: 'country', value: row.country },
					{ key: 'host', value: telepath.alert.grabNames(row.host) },
				]
			}
		}else{
			return {
				raw: row,
				itemID: row.sid,
				icon: 'alert',
				checkable: row.checkable,
				count: row.alerts_count,
				progbar: true,
				progbarValue: row.ip_score,
				time: row.date,
				title:  telepath.alert.grabNames(row.alerts_names),
				details: [ 
					{ key: 'IP', value: row.ip_orig },
					{ key: 'country', value: row.country },
					{ key: 'city', value: row.city },
					{ key: 'host', value: telepath.alert.grabNames(row.host) },
					{ key: 'actions', value: row.actions_count },
					{ key: 'cases', value: row.cases_count },
					{ key: 'user', value: row.user }
				]
			}
		};	
	},
	filters: {
		H: true,
		P: true,
		G: true,
		J: true,
		X: true,
	},
	getSeverity: function(numeric_score) {
		
		var result = $('<div>').addClass('tele-severity');
		
		if(numeric_score > 0 && numeric_score < 70) {
			result.addClass('tele-severity-low').text('Low');
		}
		if(numeric_score >= 70 && numeric_score < 90) {
			result.addClass('tele-severity-medium').text('Medium');
		}
		if(numeric_score >= 90 && numeric_score < 100) {
			result.addClass('tele-severity-critical').text('Critical');
		}
		
		result.append('&nbsp;<span>(' + numeric_score + ')</span>');
		
		return result;
		
	},
	lookupAction: function(business_id) { // Action name for business_id
	
		for(x in telepath.actionList) {
			if(telepath.actionList[x].id = business_id) {
				return telepath.actionList[x].name;
			}
		}
		return 'Browsing';
		
	},
	lookupRequest: function(RID) { // Request for RID or false if not found
		for(x in this.data.requests) {
			var req = this.data.requests[x];
			if(req.RID == RID) {
				return req;
			}
		}
		return false;
	},
	lookupAlert: function(RID) { // Alert for RID or first Alert for SID
		for(x in this.data.alerts) {
			var alert = this.data.alerts[x];
			if(alert.RID == RID) {
				return alert;
			}
		}
		return this.data.alerts[0];
	},
	data: [],
	updateSelected: function() {
	
		$('.selected', this.actionsContainer).removeClass('selected');
		var newSelected = $( "li:nth-child(" + (this.selectedIndex + 1) + ")", this.actionsContainer);
		$('.tele-listitem-inner', newSelected).addClass('selected');
		var dataID = newSelected.listitem('option', 'dataID');
		this.expandRequest(dataID);
		$(this.actionsContainer).mCustomScrollbar("scrollTo", "li:nth-child(" + (this.selectedIndex + 1) + ")");
		
	},
	scrollUp: function() {
		if(this.selectedIndex > 0) {
			this.selectedIndex--;
		}
		this.updateSelected();
	},
	scrollDown: function() {
		if(this.selectedIndex < this.data.requests.length) {
			this.selectedIndex++;
		}
		this.updateSelected();
	},
	keyDown: function(e) {
		// Up/Down Navigation
		if(e.keyCode == '38') { e.preventDefault(); telepath.alert.scrollUp(); }
		if(e.keyCode == '40') { e.preventDefault(); telepath.alert.scrollDown(); }
		
	},
	destroy: function() {
		// Unbind
		$(document).unbind('overlay_destroy', telepath.alert.destroy);
		$(document).unbind('keydown', telepath.alert.keyDown);
	},
	initTools: function() {
		
		// Archive Alert
		this.archiveAlert = $('<div>').btn({ icon: 'archive', text: 'Archive Alert', callback: function () {
			
		}});
		this.overlay.headerEl.append(this.archiveAlert);
		
		this.overlay.headerEl.append('<div class="tele-navsep"></div>'); // Sep
		
		// Move To
		this.moveTo = $('<div>').dropdown({ icon: 'moveTo', value: 'Move To', callback: function () {
			
		}});
		this.overlay.headerEl.append(this.moveTo);
		
		this.overlay.headerEl.append('<div class="tele-navsep"></div>'); // Sep
		
		// Edit Rule
		this.editRule = $('<div>').btn({ icon: 'edit', text: 'Edit Rule', callback: function () {
			
		}});
		this.overlay.headerEl.append(this.editRule);
		
	},
	initSingle: function(RID) {
		
		// Rebind
		$(document).unbind('keydown', telepath.alert.keyDown);
		$(document).bind('keydown', telepath.alert.keyDown);
		$(document).bind('overlay_destroy', telepath.alert.destroy);
		
		// Show loading
		telepath.overlay.init('alerts', 'Loading ...');
		this.overlay = telepath.overlay;
		
		// Show tools
		this.initTools();
		
		// Load the alert
		telepath.ds.get('/alerts/get_alert', { RID: RID }, function (data) {
			telepath.alert.showAlert(data.items);
		});

	},
	init: function(alertID, index) {
		
		// Rebind
		$(document).unbind('keydown', telepath.alert.keyDown);
		$(document).bind('keydown', telepath.alert.keyDown);
		$(document).bind('overlay_destroy', telepath.alert.destroy);
		
		// Show loading
		telepath.overlay.init('alerts', 'Loading ...');
		this.overlay = telepath.overlay;
		this.loadAlert(alertID);
		
		// Show pagination
		this.pagination = $('<div>').pagination({ 
			current: index, 
			count: telepath.alerts.data.alerts.length, 
			name: 'Alert', 
			callback: function (itemIndex) {
				
				var alertID = telepath.alerts.data.alerts[itemIndex].id;
				telepath.alert.loadAlert(alertID);
				
			} 
		});
		
		this.overlay.headerEl.append(this.pagination);
		this.overlay.headerEl.append('<div class="tele-navsep"></div>'); // Sep
		
		// Show tools
		this.initTools();

	},
	loadAlert: function(alertID) {
		
		this.alertID = alertID;
		this.overlay.titleEl.html('Loading alert #' + this.alertID);
		this.overlay.contentEl.empty().append(telepath.loader);
		
		var alertItem = false;
		for(x in telepath.alerts.data.alerts) {
			if(telepath.alerts.data.alerts[x].id == alertID) {
				alertItem = telepath.alerts.data.alerts[x];
				break;
			}
		}
		if(!alertItem) {
			return;
		}
		
		telepath.ds.get('/alerts/get_alert', { SID: alertItem.SID, epoch: alertItem.date }, function (data) {
			telepath.alert.showAlert(data.items);
		});
		
	},
	formatData: function(item) {
		
		var that = this;
		
		var result = {};
		
		result.title = item.title != '' ? item.title : item.display_path;
		result.progbar = true;
		result.time    = item.date;
		result.timeFormat = 'h:i A';
		result.offset = 20;
		result.icon   = 'suspect';
		result.dataID = item.sid;
	
		$.each(telepath.alert.data.alerts, function (i, alert) {
			if(alert.RID == item.RID) {
				result.icon   = 'suspect_red';
			}
		});
		
		/*var avg = parseInt(
			(parseInt(item.flow_score) + 
			 parseInt(item.geo_normal) + 
			 parseInt(item.landing_normal) + 
			 parseInt(item.query_score)) / 4);
		
		result.progbarValue = avg;*/

		result.progbarValue=item.ip_score;
		result.callback = function (widget, el) {
			$('.selected', that.actionsContainer).removeClass('selected');
			$('.tele-listitem-inner', widget.element).addClass('selected');
			that.expandRequest(widget.options.dataID);
		};
		
		return result;
		
	},
	showAlert: function(data) {
		
		// Clear 
		var container = this.overlay.contentEl;
		container.empty();
		
		// Init Params
		var that = this;
		this.selectedIndex = 0;
		telepath.alert.data = data;
		
		// Build Layout
		this.alertLeft  = $('<div>').addClass('tele-alert-left');
		this.alertMid   = $('<div>').addClass('tele-alert-mid');
		this.alertRight = $('<div>').addClass('tele-alert-right');
		
		container.append(this.alertLeft).append(this.alertMid).append(this.alertRight);
		
		//this.overlay.contentEl.empty().append(JSON.stringify(data));
		this.overlay.titleEl.html('Alert #' + this.alertID);
		
		// Sort out stats
		var reqStats = {
			'All': data.requests.length,
			'Alerts': data.alerts.length,
			'Actions': 0
		}
		// Find requests with completed business action
		$.each(data.requests, function(i, request) {
			if(request.business_id && 
			   request.business_status &&
			   request.business_status == '2') {
			   reqStats['Actions']++;
			}
		});
		
		// Print stats
		var statsEl = $('<div>').addClass('tele-alert-stats');
		$.each(reqStats, function(key, stat) {
			var statEl  = $('<div>').addClass('tele-alert-stat');
			var statKey = $('<div>').addClass('tele-alert-stat-key').html(key);
			var statValue = $('<div>').addClass('tele-alert-stat-value').html(stat);
			statEl.append(statKey).append(statValue);
			statsEl.append(statEl);
		});
		this.alertLeft.append(statsEl);
		
		// Print duration
		var durationEl = $('<div>').addClass('tele-alert-duration');
		var durationStartEl = $('<div>').addClass('tele-alert-duration-start');
		var durationStartLbl = $('<div>').addClass('tele-alert-duration-lbl').html('Started:');
		var durationStartVal = $('<div>').addClass('tele-alert-duration-val').html(date_format('d/m/y | h:i A', data.requests[0].date));
		var durationEndEl = $('<div>').addClass('tele-alert-duration-end');
		var durationEndLbl = $('<div>').addClass('tele-alert-duration-lbl').html('Ended:');
		var durationEndVal = $('<div>').addClass('tele-alert-duration-val').html(date_format('d/m/y | h:i A', data.requests[data.requests.length - 1].date));
		
		durationStartEl.append(durationStartLbl).append(durationStartVal);
		durationEndEl.append(durationEndLbl).append(durationEndVal);
		durationEl.append(durationStartEl).append(durationEndEl);
		this.alertLeft.append(durationEl);
		
		// Sort out list
		this.actionsContainer = $('<div>').addClass('tele-alert-actions');
		this.alertLeft.append(this.actionsContainer);
		
		// Request Details
		this.requestDetails = $('<div>').addClass('tele-alert-details').addClass('tele-popup');
		this.alertMid.prepend(this.requestDetails);
		
		// Print Requests
		this.lastAction = -1;
		this.printed = 0;
		
		for(x in data.requests) {
			
			var req = data.requests[x];
			
			if(req.business_id != this.lastAction) {
				
				this.actionContainer      = $('<div>').addClass('tele-alert-action');
				this.actionContainerIcon  = $('<div>').addClass('tele-alert-action-icon').addClass('tele-icon').addClass('tele-icon-suspect');
				
				this.actionContainerTitle = $('<div>').addClass('tele-alert-action-title')
													  .text(this.lookupAction(req.business_id));

				this.newList  = $('<ul class="tele-list">');
				
				this.actionContainer.append(this.actionContainerIcon)
									.append(this.actionContainerTitle)
									.append(this.newList);
				
				$(this.actionsContainer).append(this.actionContainer);
				
				this.lastAction = req.business_id;
				
			}
			
			var item = this.formatData(req);
			var newListItem = $('<li>').attr('id', 'alert-item-' + x);
			this.newList.append(newListItem);
			newListItem.listitem(item);
			//if(this.printed > 9) { break; }
			this.printed++;
			
		}
		
		
		// Infinitely load additional alerts
		$(this.actionsContainer).mCustomScrollbar({
			scrollButtons:{	enable: false },
			scrollInertia: 150,
			callbacks: {
			onTotalScrollOffset: 50,
			onTotalScroll: function () {
				
				return;
				// 10 items at a time
				var newMax = that.printed + 9;
				var offset = 0;
				
				for(x in data.requests) {
					
					if(offset < that.printed + 1) { offset++; continue; }
					
					var req = that.formatData(data.requests[x]);
					
					var newListItem = $('<li>').attr('id', 'alert-item-' + x);
					$(that.newList).append(newListItem);
					newListItem.listitem(req);
					if(that.printed > newMax) { break; }
					that.printed++;
					
				}
				
				that.updateSelected();
				
				$(that.actionsContainer).mCustomScrollbar("update"); //update scrollbar according to newly appended content

			}
			}
		});
		
		this.updateSelected();

	},
	expandRequest: function(RID) {
		
		var that = this;
	
		this.requestDetails.empty();
		this.requestDetails.append(RID);
		this.requestDetails.append(telepath.loader);
		
		var req = this.lookupRequest(RID);
		if(req) {
			this.requestInfo = req;
		} else {
			// console.log('RID ' + RID + ' was not found in requests');
			return;
		}
		
		telepath.ds.get('/similarities/', { param_type: 'request', param_id: RID }, function(data) {
		
			// console.log(data);
			
			that.similaritiesList = $('<div>');
			
			that.similaritiesList.teleList({
			title: 'Similar Requests',
			titleCallback: function () {
				
			},
			data: data.items,
			formatter: function(item) {
				
				return { 
					icon: 'suspect', 
					time: item.date, 
					itemID: item.RID,
					progbar: true, 
					progbarValue: parseInt(item.ip_score * 100),
					details: [
						{ key: 'country', value: item.country },
						{ key: 'IP', value: item.IP }
					]
				};
				
			},
			callbacks: {
				click: function (widget) {
				
					// console.log(widget.options);
					
				},
				hover_in: function(el, item) {
					
					// console.log(item);
						
				},
				hover_out: function(el, item) {
				
					// console.log(item);
				}
			}}).appendTo(that.requestDetails);
			
			
		});
		
		telepath.ds.get('/alerts/get_session_flow_params', 
			{ 
				sid: req.SID, 
				rids: RID, 
				epoch: req.date 
			}, function(data) {
			
			that.expandRequestData(data);
			
		});
		
	}, 
	expandRequestData: function(data) {
		$('.loader', this.requestDetails).remove();
		this.requestData = data.items;
		this.printParams();
	},
	printParams: function(container) {
		var container = this.requestDetails;
		container.empty();
		this.printParamsFilters(container);
		this.printAlertDetails(container);
		this.printParamsTable(container);
		this.printAlertInfo(this.alertRight);
	},
	printParamsFilters: function(container) {
		
		var that = this;
		this.filtersContainer = $('<div>').addClass('tele-alert-filters');
		container.append(this.filtersContainer);
		
		var filters = [
			{ type: 'H', label: 'Headers' },
			{ type: 'G', label: 'Get' },
			{ type: 'P', label: 'Post' },
			{ type: 'J', label: 'Json' },
			{ type: 'X', label: 'XML' }
		];
		
		$.each(filters, function (i, filter) {
			var filter_el = $('<div>').teleCheckbox({ 
				checked: telepath.alert.filters[filter.type], 
				label: filter.label, 
				inputFirst: true,
				callback: function (widget) {
					telepath.alert.filters[filter.type] = widget.options.checked;
					that.printParams();
					// console.log(telepath.alert.filters);
			}});
			that.filtersContainer.append(filter_el);
		});
		
	},
	printAlertDetails: function(container) {
	
		// Container
		this.alertDetails = $('<div>').addClass('tele-alert-details-info');
		container.append(this.alertDetails);

		// Title
		this.alertDetailsTitle = $('<div>').addClass('tele-alert-details-info-title').text(this.lookupAction(this.requestInfo.business_id));
		
		// Time
		this.alertDetailsTimeWrap   = $('<div>').addClass('tele-alert-details-info-time-wrap');
		this.alertDetailsTimeLabel  = $('<div>').addClass('tele-alert-details-info-time-label').text('Request time:');
		this.alertDetailsTime  		= $('<div>').addClass('tele-alert-details-info-time').text(date_format('d/m/y | H:i:s', this.requestInfo.date));
		this.alertDetailsTimeWrap.append(this.alertDetailsTimeLabel).append(this.alertDetailsTime);
		
		// Severity
		var severityPercent         = parseInt(this.requestData.avg_score * 100) + '%';
		this.alertSeverityWrap      = $('<div>').addClass('tele-alert-severity-wrap');
		this.alertSeverityLabel     = $('<div>').addClass('tele-alert-severity-label').text('Severity');
		this.alertSeverityPercent   = $('<div>').addClass('tele-alert-severity-percent').text(severityPercent);
		this.alertSeverityProgBar   = $('<div>').addClass('tele-alert-severity-progbar');
		this.alertSeverityProgValue = $('<div>').addClass('tele-alert-severity-progbar-value').css({ width: severityPercent });
		
		this.alertSeverityWrap.append(this.alertSeverityLabel).append(this.alertSeverityPercent).append(this.alertSeverityProgBar);
		this.alertSeverityProgBar.append(this.alertSeverityProgValue);
		
		this.alertDetails.append(this.alertSeverityWrap).append(this.alertDetailsTitle).append(this.alertDetailsTimeWrap);
		
		//console.log(this.requestInfo);
		//console.log('Request Data');
		//console.log(this.requestData);
		
		
	},
	printAlertInfo: function(container) {
		
		container.empty();
		var alert = this.lookupAlert(this.requestInfo.RID);
		
		// Setup containers
		this.alertInfo      = $('<div>').addClass('tele-alert-info');
		this.alertInfoTitle = $('<div>').addClass('tele-alert-info-title').text('Alert Info');
		this.alertInfoLabel = $('<div>').addClass('tele-alert-info-label').text('Description:');
		this.alertInfoDesc  = $('<div>').addClass('tele-alert-info-description').text(alert.description);
		this.alertInfoTable = $('<div>').addClass('tele-alert-info-table');
	
		function getRow(lbl, data) {
			var row = $('<tr>');
			var td_1 = $('<td>').html(lbl).addClass('tele-alert-info-key');
			var td_2 = $('<td>').html(data).addClass('tele-alert-info-value');
			row.append(td_1).append(td_2);
			return row;
		}
		
		// Fill table data
		var table = this.alertInfoTable;
		
		table.append(getRow('Time:', date_format('d/m/y | H:i:s', alert.date)));
		table.append(getRow('Severity:', this.getSeverity(alert.numeric_score)));
		table.append(getRow('Applications:', this.requestInfo.app_domain));
		table.append(getRow('IP:', alert.ip));
		table.append(getRow('Location:', '<span class="flag flag-' + this.requestInfo.country + '"></span>' + 
							'<span class="tele-country">' + telepath.countries.a2n(this.requestInfo.country) + '</span>'));
							
		if(alert.user && alert.user != '') {
			table.append(getRow('User:', alert.user));
		}
		
				
		// Append All
		this.alertInfo.append(this.alertInfoTitle)
					  .append(this.alertInfoLabel)
					  .append(this.alertInfoDesc)
					  .append(this.alertInfoTable);
					  
		container.append(this.alertInfo);
		
	
	},
	printParamsTable: function(container) {
		
		var tableWrap = $('<div>').addClass('tele-alert-params-table-wrap');
		var table 	  = $('<table>').addClass('tele-alert-params-table');
		
		$.each(this.requestData.params, function(i, param) {

			var param_display = telepath.alert.filters[param.att_source];
			if(param_display) {
			
				var row = $('<tr>');
				var param_name    = param.att_alias != '' ? param.att_alias : param.name;
				var col_name  = $('<td>').addClass('tele-param-name').html(param.att_name);
				var col_data  = $('<td>').addClass('tele-param-data').html(param.data);
				var col_score = $('<td>').addClass('tele-param-score').html(parseInt(param.attribute_score_normal) + '%');
				
				if(parseInt(param.attribute_score_normal)) {
					col_score.addClass('severe');
					col_data.addClass('severe');
				}
				
				row.append(col_name).append(col_data).append(col_score);
				table.append(row);
				
			}
			
		});
		
		container.append(tableWrap);
		tableWrap.append(table);
		
		$(tableWrap).mCustomScrollbar();
		
	}
	
};