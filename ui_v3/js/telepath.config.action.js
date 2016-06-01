telepath.action = {};


telepath.action.tracker = {
	
	requests: [],
	
	min_ts: 0,
	key: false,
	value: false,
	timer: false,
	tick_value: 3000,
	init: function(key, value, ts) {
		
		// Clear the timer
		if(this.timer) {
			clearTimeout(this.timer);
			this.timer = false;
		}
		// Set parameters
		
		this.key    = key;
		this.value  = value;
		this.min_ts = ts;
		
	},
	tick: function() {
		
		switch(key) {
		
			case 'param':
			
			break;
			
			case 'sid':
				
			break;
			
			case 'ip':
				
			break;
			
		}
		
	}
	
	
}

telepath.action.recorder = {
	
	new_flow: true,
	requests: [],
	paused: false,
	ts_offset: 0,
	initRecordingTools: function(action_name) {
		
		var that = this;
		this.recordTools = $('<div>').addClass('tele-record-tools');
		
		if(!action_name) {
			action_name = '';
		}
		
		// Name
		this.actionName = $('<div>').teleInput({ value: action_name, label: 'Name' });
		this.recordTools.append(this.actionName);
		
		// Controls
		var controls = [ 'record', 'stop', 'pause' ];
		$.each(controls, function(i, control) {
			var controlEl = $('<div>').addClass('tele-control').addClass('tele-control-' + control).html('<div></div>').click(function () {
			
				switch(control) {
					
					case 'stop':
					
						clearTimeout(that.timer);
						$('.tele-control', that.recordTools).remove();
					
					break;
					
					case 'pause':
						
						that.paused = !that.paused;
					
					break;
				
				}
			
			});
			
			that.recordTools.append(controlEl);
		});

		this.container.parent().prepend(this.recordTools);
		
	},
	processRequests: function(requests, offset) {
		
		var that = this;
		
		if($('.tele-record-tools').size() == 0) {
			this.initRecordingTools();
		}
		
		if(requests.length > 0) {
			$.each(requests, function(i, request) {
				
				// If paused, don't add the request, we want to continue getting requests so we know the last timestamp
				if(!that.paused) {
				
					request.el = $('<div>').teleRequest({ data: request });
					$('.mCSB_container', that.container).append(request.el);
					that.requests.push(request);
				
				}
				
				// Set max time to rcv subsequent
				if(request.ts > that.ts_offset) {
					that.ts_offset = request.ts;
				}
				
			});
		}
		
		// Restart timer, this time using timestamp of last request to receive only subsequent requests
		// for now..
		this.timer = setTimeout(function () {
			telepath.ds.get('/actions/get_requests', { mode: that.recordType, value: that.recordValue, host: telepath.action.currentApp, offset: (parseInt(that.ts_offset) || 0) + 1 }, function(data) {
				// Process even if no items to reset timer iteration
				that.processRequests(data.items);
			});
		}, 3000);
		
	},
	startRecording: function(type) {
		
		var that = this;
		this.timerValue = 0;
		this.recordType = type;
		clearInterval(this.timer);
		that.progbarInner.css({ width: 0 }); 
		
		var value = $('input', this.input).val();
		$('input', this.input).css({ borderColor: '#999' });
		
		if(type == 'URL') {
			value = $('a', this.input).attr('href');
			value = value.split('=')[1]; // ssl://sub.domain?hybridrecord = VALUE
		}
		
		if((type == 'IP' || type == 'user' || type == 'SID') && value == '') {
			$('input', this.input).css({ borderColor: 'red' });
			telepath.dialog({ type: 'alert', title: 'Business Action', msg: 'Missing value' });
			return;
		}
		
		this.recordValue = value;
		
		if(telepath.debug) {
			// console.log('Tracking ' + type + ' - ' + value);
		}
		
		function tick() {
			
			that.timerValue += 5;
			that.progbarInner.css({ width: that.timerValue + '%' }); 
			
			// Request (No time being sent, we don't know when was the last request we're actually tracking)
			// Lockon = true, only get latest TS
			telepath.ds.get('/actions/get_requests', { mode: that.recordType, value: that.recordValue, host: telepath.action.currentApp, lockon: true }, function(data) {
				if(data.total > 0) {
				
					// Have requests matching recording parameters, initialize recording.
					that.ts_offset = data.items.ts;
					
					if(that.recordType == 'URL') {
						that.recordType  = 'SID';
						that.recordValue = data.items.sid;
					}
					
					// Clearup
					that.container.empty();
					
					// Another inner container for padding
					var tmp = $('<div>').addClass('tele-action-container');
					that.container.append(tmp);
					that.container = tmp;
					
					clearInterval(that.timer);
					that.processRequests(data.items);
					
					$('.tele-action-container').css({ padding: 0, height: 400 }).mCustomScrollbar({
						scrollButtons:{	enable: false },
						scrollInertia: 150,
						advanced: {
							updateOnContentResize: true
						} 
					});
					
					that.showSaveLoad = telepath.config.action.showSaveLoad;
					that.showSaveLoad();
					
					
				}
			});
				
			
			if(that.timerValue > 99) {
				clearInterval(that.timer);
				telepath.dialog({ type: 'alert', title: 'Business Action', msg: 'No matching requests, tracking timed out.' });
			}
		
		}
		
		this.timer = setInterval(function () { tick(); }, 3000);
		tick();

	},
	init: function () {
		
		$('.popover').remove();
		
		// Trackers
		this.trackers = [
			{ type: 'URL', title: 'URL', desc: 'Record all activity from URL' },
			{ type: 'user', title: 'Username', desc: 'Record all activity of a speciﬁc user' },
			{ type: 'IP', title: 'IP', desc: 'Record all activity of a speciﬁc device' },
			// Remove Session recording by SID for now, Yuli
			//{ type: 'SID', title: 'Session', desc: 'Record all activity of a speciﬁc session' }
		];
		
		// Containers
		this.container = telepath.config.actions.contentRight;
		this.toolbar   = telepath.config.actions.barRight;
		this.container.addClass('tele-workflow-recorder');
		
		// Cleanup
		this.container.empty();
		this.toolbar.empty();
		
		// Title 
		this.barTitle  = $('<div>').addClass('tele-panel-subtitle-text').html('Record New Business Action');
		this.toolbar.append(this.barTitle);
		
		// Trackers Title
		this.trackersTitle  = $('<div>').addClass('tele-title-1').html('Select recording source:');
		this.container.append(this.trackersTitle);
		
		//this.initDebugger();
		//return;
		
		// Printout
		this.printTrackers();
		this.container.mCustomScrollbar({ advanced:{ updateOnContentResize: true } });

	},
	initDebugger: function () {
		// console.log('Actions debugger online');
		// console.log('Current host  : ' + telepath.action.currentApp);
		this.container.append('<div>Active sessions in last 5 minutes ::</div>');
		var that = this;
		telepath.ds.get('/actions/get_top_active_sessions', { host: telepath.action.currentApp.trim() }, function(data) {
			// console.log(data.items);
			if(data.items) {
				$.each(data.items, function(i, item) { 
					// console.log(item);
					var item_el = $('<div>');
					item_el.append('<span>' + new Date(item.ts * 1000).toString() + '<span>');
					item_el.append('<span>' + item.sid + '</span>');
					
					item_el.click(function () {
						
						$(this).css({ paddingBottom: 20 });
						// console.log('Tracking ' + item.sid + ' starting stamp ' + item.ts);
						
						
						if(telepath.action.recorder.timer) {
							clearInterval(telepath.action.recorder.timer);
						}
						
						telepath.action.recorder.timer = setInterval(function () {
							telepath.ds.get('/actions/track_session_by_sid', { sid: item.sid, time: item.ts }, function(data) {
								// console.log(data);
								if(data.items && data.items.length > 0) {
									item_el.append($('<div>').html(JSON.stringify(data)));
								}
								// Process even if no items to reset timer iteration
								// that.processRequests(data.items);
							});
						}, 3000);

					});
					
					that.container.append(item_el);
					
				});
			}
		});
		
	},
	trackerClick: function(el, type) {
		
		var that = this;
		
		var trackerInfo = $('<div>').addClass('tele-workflow-tracker-info').addClass('tele-rounded-box').hide();
	
		//var trackerTitle = $('<div>').addClass('tele-title-2').html('Its recommended that you clear browser cache before you start recording');
		//trackerInfo.append(trackerTitle);
		
		var application_ssl       = false;
		var application_subdomain = ''
		var application_domain    = telepath.action.currentApp;
		var hybrid_record         = 'hybridrecord';
		var hybrid_record_id      = Math.floor(Math.random() * (999999 - 100000 + 1)) + 99999; // 99-999K random seed
		
		// Build link
		var hybrid_link           = (application_ssl ? 'https://' : 'http://') +  // SSL
									(application_subdomain != '' ? application_subdomain + '.' : '') + // Subdomain
									application_domain + '/?' + hybrid_record + '=' + hybrid_record_id; // Domain + Link
		
		switch(type) {
				
				case 'IP':
					var input = $('<div>').teleInput({ label: 'Target IP:' });
					trackerInfo.append(input);
				break;
				case 'user':
					var input = $('<div>').teleInput({ label: 'Target Username:' });
					trackerInfo.append(input);
				break;
				case 'SID':
					var input = $('<div>').teleInput({ label: 'Target Session:' });
					trackerInfo.append(input);
				break;
				case 'URL':
					var input = $('<div>').teleInput({ label: 'Target URL:', value: hybrid_link, link: true });
					trackerInfo.append(input);
				break;
			
		}
		
		input.addClass('tele-workflow-tracker-input-' + type);
		
		if(type !== 'URL') {
		
			$('.tele-input-input', input).autocomplete({ 
				autoFill: true,
				source: function(request, response) {
					telepath.ds.get('/actions/get_suggest', { mode: type, host: application_domain }, function(data) {
						if(data.items) {
							response(data.items);
						}
					});
				},
				minLength: 0
			}).focus(function () {
				$(this).autocomplete('search', $(this).val());
			});
		
		}
		
		/*
		var controls = [ 'record', 'stop', 'pause' ];
		$.each(controls, function(i, control) {
			var controlEl = $('<div>').addClass('tele-control').addClass('tele-control-' + control).html('<div></i');
			trackerInfo.append(controlEl);
		});
		*/
		
		// Record Button
		
		that.input = input;
		var record_cmd = $('<div>').addClass('tele-control-record-cmd').hover(function () {
			$('*', this).addClass('hover');
		}, function () {
			$('*', this).removeClass('hover');
		}).click(function () {
			that.startRecording(type, $('input', that.input).val());
		});
		
		var record_lbl = $('<div>').addClass('tele-control-record-cmd-lbl').html('Start Recording');
		var record_ico = $('<div>').addClass('tele-control').addClass('tele-control-record');
		var record_div = $('<div>');
		
		// Button
		record_ico.append(record_div);
		record_cmd.append(record_ico).append(record_lbl);

		// Progbar
		var progbar 	  = $('<div>');
		progbar.addClass('tele-listitem-bigprogbar');
		this.progbarInner = $('<div>').addClass('tele-listitem-progbar-inner').css({ width: 0 });
		progbar.append(this.progbarInner);
		
		// Append		
		trackerInfo.append(record_cmd);
		trackerInfo.append(progbar);
		
		// Fade In
		$(el).animate({ marginBottom: 100 }, 'fast', function () {
			$(this).append(trackerInfo);
			trackerInfo.fadeIn();
		});
		
	},
	printTrackers: function() {
		
		var that = this;
		var container = this.container;
		
		$.each(this.trackers, function (i, tracker) {
			
			//console.log(tracker);
			
			var trackerEl = $('<div>').addClass('tele-workflow-tracker').addClass('tele-rounded-box');
			var trackerTitle = $('<div>').addClass('tele-title-1').html(tracker.title);
			var trackerDesc  = $('<div>').addClass('tele-text').html(tracker.desc);
			
			trackerEl.append(trackerTitle).append(trackerDesc);
			
			trackerEl.hover(function () {
				$(this).addClass('hover');
			}, function () {
				$(this).removeClass('hover');
			}).click(function () {
				
				// Already active
				if($(this).hasClass('selected')) {
					return;
				}
				
				// Cleanup
				$('.tele-workflow-tracker.tele-rounded-box.selected').each(function () {
					$(this).animate({ marginBottom: 10 }, 'fast', function () {
					}).removeClass('selected');
					$('.tele-workflow-tracker-info', this).fadeOut('fast', function () { $(this).remove(); });
				});
				
				// Activate
				$(this).addClass('selected');
				that.trackerClick(this, tracker.type);
				
			});
			
			container.append(trackerEl);
		
		});
	
	},
	
};

telepath.config.action = {
	new_flow: false,
	editAction: function(action_data) {
	
		// Containers
		this.container = telepath.config.actions.contentRight;
		this.toolbar   = telepath.config.actions.barRight;
		
		// Cleanup
		this.container.empty();
		this.toolbar.empty();
		
		// Tab Containers
		this.tabsEl   = $('<div>').addClass('tabs');
		this.tabsUl   = $('<ul>');
		this.tabsEl.append(this.tabsUl);
		
		telepath.action.currentApp = action_data.application;
		
		this.action_data = action_data;
		this.showAction();
		
	},
	showAction: function(action_name) {
		
		// Clearup
		var that = this;
		that.container.empty();
		$('.popover').remove();
		
		if(!action_name) {
			action_name = '';
		}

		// Name
		// this.recordTools = $('<div>').addClass('tele-record-tools');
		// this.actionName = $('<div>').teleInput({ value: action_name, label: 'Name' });
		// this.recordTools.append(this.actionName);
		// this.container.append(this.recordTools);
				
		
		if(that.action_data) {
			
			// Another inner container for padding
			var tmp = $('<div>').addClass('tele-action-container');
			that.container.append(tmp);
			that.container = tmp;
		
			$.each(that.action_data.business, function(i, request) {
				
				request.el = $('<div>').teleRequest({ data: request });
				that.container.append(request.el);
				
			});
			
			that.showSaveLoad();
			
		}
				
	},
	showSaveLoad: function() {
		
		var that = this;
		
		this.buttonsEl = $('<div>').addClass('tele-form-buttons');
		this.applyBtn  = $('<a href="#" class="tele-button tele-button-apply">Save</a>');
		this.cancelBtn = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');
		
		this.buttonsEl.append(this.applyBtn).append(this.cancelBtn);
		this.container.append(this.buttonsEl);
		
		// BIND Validate
		this.applyBtn.click(function () { 
			
			// Validate name
			// Iterate request widgets
			
			var actionData = [];
			$('.tele-action-container .tele-request').each(function () { 
				actionData.push($(this).data('tele-tele-request').options.data);
			});
			
			// Take just what we need
			
			var cleanData = [];
			
			// console.log(actionData);
			
			$.each(actionData, function(i, action) {
				
				var jsonNode = { pagename : action.uri ? action.uri : action.pagename };

				if(action.parameters && action.parameters.length > 0) {
					jsonNode.params = [];
					$.each(action.parameters, function (i, param) {
						jsonNode.params.push( { name: param.name, data: param.data } );
					});
				}
				
				cleanData.push(jsonNode);
			
			});
			
			// console.log(cleanData);
			
			// Either stored or from input
			var flow_name = that.action_data && that.action_data.action_name ?  that.action_data.action_name : $('input', telepath.action.recorder.actionName).val();
			
			if(!flow_name || flow_name == '') {
				telepath.dialog({ type: 'alert', title: 'Business Actions', msg: 'Missing action name' });
				return;
			}
			
			// Post to server
			telepath.ds.get('/actions/set_flow', {
				app: telepath.action.currentApp,
				flow_name: flow_name,
				json: JSON.stringify(cleanData)
			}, function(data) {
			
				telepath.config.actions.reload();
				
			});
			
			// Notify user
		});
		
		// BIND Cancel -- Clear all, show recorder
		this.cancelBtn.click(function () {
			// Clear timer when cancel is clicked (Yuli)
			clearInterval(that.timer);
			telepath.action.recorder.init();
		});
	
	}

}

/* DEPR */

function RecordFlow(container) {
	
	this.itemWidth = 330;
	this.container = container;

}

RecordFlow.prototype.clear  = function () {
	
	$('.flowContainer', this.container).remove();
	
}

RecordFlow.prototype.refresh = function (data) {
	
	// Clear outer
	$('.flowContainer', this.container).remove();
	
	// Clear inner
	this.flowContainer = $('<div>').addClass('flowContainer');
	this.container.append(this.flowContainer);
	
	var totalPrinted = 0;
	var totalLimit   = data.length;
	
	for(var i=0; i < totalLimit; i++) { 
		var requestTable = this.getRequestTable(data[i]);
		this.flowContainer.append(requestTable);
		totalPrinted++;
	}
	
	$(this.flowContainer).append('<div class="clear"></div>');

}

RecordFlow.prototype.getRequestTable = function(request) {
	
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
			var valueLink = $('<span>').attr('href', '#').text(z['data']).attr('rel', z['att_id']);
			
			if(!z.include) {
				valueLink.addClass('excluded');
			}
			
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
	
	var app = '';//recordNode.parentNode.raw.app_domain;
	var ssl = '';//recordNode.parentNode.raw.ssl_flag;
	
	var url = (ssl==1?"https":"http")+"://"+ app + request.path;
	var link = $('<a>').addClass('icon-link').text('').attr('href', escapeHtml(url)).attr('target', '_blank').hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover') });
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
