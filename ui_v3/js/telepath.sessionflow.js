telepath.sessionflow = {
	
	RID: 0,
	RIDS: [],
	list: false,
	searchkey: '',
	selectedIndex: 0,
	similarities: [],
	currentUID: -1,
	reloadFlag: Date.now(),
	alertsList: false,
	session: { requests: [], alerts: [], flows: [] },
	resizeMid: function() {
		
		var height 	   = $(this.overlay.contentEl).height();
		var items  	   = $('.tele-request-details', this.boxMid).size();
		
		height = height - 100;
		if (height < 300)
		{
			height = 300;
		}

		if(items == 2) {
		
			height = (height / 2) - 120;
			$('.tele-alert-params-table-wrap .mCustomScrollBox').css({ maxHeight: height });
		
		}
		
		$('.tele-alert-params-table-wrap .mCustomScrollBox').css({ maxHeight: (height - 100) });
		
		$('.tele-alert-params-table-wrap').each(function () { 
			$(this).mCustomScrollbar('update'); 
		});
		
	
	},
	printRequestScores: function(data, container) {
		
		this.requestScoreEl = $('<div>').anomalyScore({ request: data });
		
		container.append(this.requestScoreEl);
		
	},
	showSimilarities: function(data) {
		
		var that = this;
		
		this.similarities = data;
		//$('<div>').addClass('tele-title-1').html('Similar Requests').css({ cssFloat: 'none' }).appendTo(this.boxRight);
		this.similaritiesList = $('<div>').appendTo(this.boxRight).addClass('tele-similarities-list');
		
		var height = this.overlay.contentEl.height() - this.requestScoreEl.outerHeight() - $('.tele-alert-info-table', this.overlay.contentEl).outerHeight() - 2*20 - 75;
		//console.log(height);
		
		this.similaritiesList.css({ width: 380, 'height': height /* + " !important" */ }); // Weird :)

		this.similaritiesList.on('teleList.afterUpdate', function (){

			$(this).trigger('resize');
		});

		this.similaritiesList.teleList({
			data: data.hits.hits,
			title: 'Similar Requests', 
			'height': height,
			formatter: function(row) {
				return telepath.sessionflow.formatData(row);
			},
			callbacks: {
				click: function (widget) { 
					
					that.requestData = widget.options.raw;
					
					$(widget.element).parent().find('.tele-listitem-inner.selected').removeClass('selected');
					$('.tele-listitem-inner', widget.element).addClass('selected');
					
					if(that.similarityDetails) {
						that.similarityDetails.remove();
					}
					$('.tele-similarity-details').remove();
					that.similarityDetails = $('<div>').addClass('tele-request-details tele-similarity-details').addClass('tele-popup-2').css({ marginTop: 20 });
					that.boxMid.append(that.similarityDetails);

					var wrap = $('<div>').addClass('tele-alert-details-info');
					
					that.requestData.score_average = parseFloat(that.requestData.score_average);
					
					//if(that.requestData.score > that.requestData.score_average) { that.requestData.score_average = that.requestData.score }
					if(that.requestData.score_average > 0 && that.requestData.score_average <= 1) {
						that.requestData.score_average = that.requestData.score_average * 100;
					}
					
					// Severity
					var severityPercent         = parseInt(that.requestData.score_average) + '%';
					that.alertSeverityWrap      = $('<div>').addClass('tele-alert-severity-wrap');
					that.alertSeverityLabel     = $('<div>').addClass('tele-alert-severity-label').text('Severity');
					that.alertSeverityPercent   = $('<div>').addClass('tele-alert-severity-percent').text(severityPercent);
					that.alertSeverityProgBar   = $('<div>').addClass('tele-alert-severity-progbar');
					that.alertSeverityProgValue = $('<div>').addClass('tele-alert-severity-progbar-value').css({ width: severityPercent });
					
					that.alertSeverityWrap.append(that.alertSeverityLabel).append(that.alertSeverityPercent).append(that.alertSeverityProgBar);
					that.alertSeverityProgBar.append(that.alertSeverityProgValue);
					
					wrap.append(that.alertSeverityWrap);
					
					
					// Title
					var title = $('<div>').addClass('tele-alert-details-info-title');
					that.printLink(that.requestData, title);
					
					wrap.append(title);
					
					that.similarityDetails.append(wrap);
					
	
					that.printParamsTable(that.similarityDetails);
					
					that.resizeMid();
					
				},
				hover_in: function(el, item) { 
				
					// Cleanup
					$('.popover').remove();
					var that = this;
				
					// Init
					var RID = item.dataID;
					var similarity = telepath.sessionflow.lookupSimilarity(RID);
					
					// Check
					if(similarity === false) { return; }
	
					function getRow(lbl, data) {
						var row = $('<tr>');
						var td_1 = $('<td>').html(lbl).addClass('tele-alert-info-key');
						var td_2 = $('<td>').html(data).addClass('tele-alert-info-value');
						row.append(td_1).append(td_2);
						return row;
					}
					
					// console.log(similarity);
					
					similarity._source._score = similarity._score;
					similarity = similarity._source;
					
					// console.log(similarity);
					
					// Fill table data
					var table =  $('<table>').addClass('tele-alert-info-table');
					
					table.append(getRow('Time:', date_format('d/m/y | H:i:s', similarity.ts)));
					//table.append(getRow('Severity:', this.getSeverity(alert.numeric_score)));
					table.append(getRow('Applications:', escapeHtml(similarity.host)));
					table.append(getRow('IP:', similarity.ip_orig));
					table.append(getRow('Location:', '<span class="flag flag-' + similarity.country_code + '"></span>' + 
										'<span class="tele-country">' + telepath.countries.a2n(similarity.country_code) + '</span>'));
					if(similarity.username){
						table.append(getRow('User:', escapeHtml(decodeURIComponent(similarity.username))));
					}
					
					table.append(getRow('Similarity:', similarity._score + '%'));
					
					//if(alert.user && alert.user != '') {
					//	table.append(getRow('User:', alert.user));
					//}
									
					telepath.sessionflow.pop = $('<div>').css({ 
						position: 'absolute', 
						top: $(el).offset().top,
						left: $(el).offset().left + 70
					});
				
					$('body').append(telepath.sessionflow.pop);
					$(telepath.sessionflow.pop).popover({ 
					title: similarity.orig_ip, 
					html: true,
					content: '<table cellspacing="10">' + table.html() + '</table>',
					}).popover('show');
									
										
				},
				hover_out: function(el, item) {	
					
					if(telepath.sessionflow.pop) {
						telepath.sessionflow.pop.remove();
					}
					
				}
		}});
		
		this.similaritiesList.css("height", "300px !important"); // Weird :)
		this.similaritiesList.trigger('resize');
                 //Bind resize hooks

                $(window).resize(function () { that._resize() });
	
	},
	_resize: function() {
		if (this.overlay.contentEl.height())
		{
			var height = this.overlay.contentEl.height() - this.requestScoreEl.outerHeight() - $('.tele-alert-info-table', this.overlay.contentEl).outerHeight() - 2*20 - 55;
			$('.tele-similarities-list .tele-block').height(height - 20);
			//$('.tele-block .tele-list').height(offset - 50);
			$('.tele-similarities-list .tele-list').mCustomScrollbar("update");
		}

	},
	init: function(SID, IP, list, is_alert, alerts_names, searchkey) {
	
		this.session = false;	
		this.SID  = SID;
		this.IP  = IP;
		this.list = list;
		this.searchkey = searchkey;
		this.alerts_names = alerts_names;

		if (searchkey)
		{
			this.filter = 'Search';
		}
		else if(is_alert && is_alert == 'alert') {
			this.filter = 'Alerts';
		} else {
			this.filter = 'All';
		}
		
		
		if (telepath.access.admin || telepath.access.perm.Sessionflow_get){
			this.buildUI();
			this.loadSession(this.SID);
		}else{
			telepath.dialog({msg:'Access denied. No permissions to view Session Flow.'});
		};
		

	},
	loadSession: function() {

		this.mode = 'session';
		var that = this;
		this.container.empty().append(telepath.loader);
		
		that.session = {};
		
		// Load session stats, then items
		telepath.ds.get('/sessionflow/get_session_stats', { sid: that.SID, searchkey: that.searchkey }, function (data) {
			
			that.session.stats = data.items;
			
			telepath.ds.get('/sessionflow/get_sessionflow', { sid: that.SID, filter: that.filter, searchkey: that.searchkey, alerts: that.alerts_names, ip: that.IP }, function (data) {
				that.session.items = data.items;
				that.showSession();
			});
			
		});

	},
	buildUI: function() {
		
		var that = this;
		
		// Rebind
		$(document).unbind('keydown', telepath.sessionflow.keyDown);
		$(document).bind('keydown', telepath.sessionflow.keyDown);
		$(document).bind('overlay_destroy', telepath.alert.destroy);
		
		// Show loading
		telepath.overlay.init('alerts', 'Loading Sessionflow', true, 500);
		this.overlay = telepath.overlay;
		this.container = this.overlay.contentEl;
		
		if(this.list) {
			
			var total = 0;
			var index = 0;
			
			$('.tele-listitem', this.list).each(function () {
				var itemRID = $(this).data('tele-listitem').options.itemID;
				if(itemRID == that.RID) {
					index = total;
				}
				that.RIDS.push(itemRID);
				total++;
			});
			
			// 
			
			// Show pagination
			this.pagination = $('<div>').pagination({ 
				current: index, 
				count:   total, 
				name: that.alertsList ? 'Alert' : 'Session', 
				callback: function (itemIndex) {
					// console.log(itemIndex);
					that.SID = that.RIDS[itemIndex];
					that.loadSession();
					
				}
			});
			
			this.overlay.headerEl.append(this.pagination);
		
		}
	
	},
	showSession: function() {
		
		// Cleanup
		var that = this;
		this.container.empty();
		this.selectedIndex = 0;
		
		// Find selected index
		// ( is the RID index that initialized the session flow within the session )
		
		for(x in that.session.requests) {
			var request = that.session.requests[x];
			if(request.RID == that.RID) {
				this.selectedIndex = parseInt(x);
				break;
			}
			// Copy flows into requests
			for(y in that.session.flows) {
				var flow = that.session.flows[y];
				if(flow.RID == request.RID) {
					that.session.requests[x].business_id = flow.business_id;
					that.session.requests[x].business_status = flow.business_status;
				}
			}
		}
		
		// Build Layout
		this.boxLeft  = $('<div>').addClass('tele-box-left');
		this.boxMid   = $('<div>').addClass('tele-box-mid');
		this.boxRight = $('<div>').addClass('tele-box-right');
		this.container.append(this.boxLeft, this.boxMid, this.boxRight);
		
		// Dynamically this.overlay.titleEl.html('Alert #' + this.alertID);
		
		
		var width = this.overlay.contentEl.width();
		var mid_width = width - $(this.boxLeft).outerWidth() - $(this.boxRight).outerWidth() - 50;
		this.boxMid.width(mid_width);
		
		
		// Print Stats
		// ---------------------------------------------------
		var count_all     = this.session.stats.total;
		var count_alerts  = this.session.stats.alerts_count;
		var count_actions = this.session.stats.actions_count;
		var search_count  = this.session.stats.search_count;
		
		var statsEl = $('<div>').addClass('tele-alert-stats');
		$.each({'All': count_all, 'Search': search_count, 'Alerts': count_alerts, 'Actions': count_actions }, function(key, stat) {
			
			// Add filter if we have numeric vaue only, Yuli
			if (stat > 0)
			{
				var stat_filter = $('<div>').addClass('tele-alert-stat-key').html(key).click(function () {
					telepath.sessionflow.filter = key;
					telepath.sessionflow.loadSession();
					$('.tele-alert-stat-key').removeClass('active');
					$(this).addClass('active');
				});
			
				if(telepath.sessionflow.filter == key) {
					stat_filter.addClass('active');
				}
				
				statsEl.append($('<div>').addClass('tele-alert-stat').append(stat_filter)
					.append($('<div>').addClass('tele-alert-stat-value').html(stat)));
			}
			
		});
		this.boxLeft.append(statsEl);
		
		// --------------------------------------------------- 
		
			
		// Print duration
		// ---------------------------------------------------
		var durationEl = $('<div>').addClass('tele-alert-duration')
					.append($('<div>').addClass('tele-alert-duration-start')
					.append($('<div>').addClass('tele-alert-duration-lbl').html('Started:'), 
						$('<div>').addClass('tele-alert-duration-val').html(
							date_format('d/m/y | H:i A', this.session.stats.session_start))
						))
					.append($('<div>').addClass('tele-alert-duration-end')
					.append($('<div>').addClass('tele-alert-duration-lbl').html('Ended:'), 
						$('<div>').addClass('tele-alert-duration-val').html(
							date_format('d/m/y | H:i A', this.session.stats.session_end))));
		this.boxLeft.append(durationEl);
		// ---------------------------------------------------
		
		
		
		// Sort out list
		this.actionsContainer = $('<div>').addClass('tele-requests-list');
		this.boxLeft.append(this.actionsContainer);
		
		// Request Details
		this.requestDetails = $('<div>').addClass('tele-request-details').addClass('tele-popup-2');
		this.boxMid.prepend(this.requestDetails);
		
		// Print Requests
		this.lastAction = -1;
		this.printed = 0;
		
		for(x in that.session.items) {
		
			var req = that.session.items[x];
			that.appendItem(req);
			
		}
		var height = this.container.height() - statsEl.height() - durationEl.height() - 100;
		// Infinitely load additional alerts
		$(this.actionsContainer).css({ 'height': height }); // Stats and times offset
		
		$(this.actionsContainer).mCustomScrollbar({ callbacks:{
				onTotalScroll:function(){
					
					if(that.loading) {
						return;
					}
				
					that.loading = true;
					
					$('.mCSB_container', that.actionsContainer).append($(telepath.loader).css({ float: 'left', clear: 'both', 'bottom': 30, position: 'absolute' }));
					
						var offset = that.session.items.length;
						
						telepath.ds.get('/sessionflow/get_sessionflow', { sid: that.SID, filter: that.filter, offset: offset }, function (data) {
							
							that.loading = false;
							$('.tele-loader', that.actionsContainer).remove();
						
							if(data.items.length > 0) {
								$.each(data.items, function(i, item) {
									that.session.items.push(item);
									that.appendItem(item);
								});
							}
							 
						});
				},
			},
			onTotalScrollOffset:200,
			alwaysTriggerOffsets:false,
			advanced:{ updateOnContentResize:true }
			});
			
		this.updateSelected();
		
		this.overlay.titleEl.html('Session Flow');
		
	},
	appendItem: function (req) {
		
		if(this.lastAction == -1 || (req.business_actions && (req.business_actions[0].name != this.lastAction || req.business_actions[0].status == 2)) || (!req.business_actions && this.lastAction != -1 && this.lastAction != 'Browsing')) {
				
			var action_name = req.business_actions ? req.business_actions[0].name : 'Browsing';
			
			this.actionContainer      = $('<div>').addClass('tele-alert-action');
			this.actionContainerIcon  = $('<div>').addClass('tele-alert-action-icon').addClass('tele-icon').addClass('tele-icon-suspect');
			
			this.actionContainerTitle = $('<div>').addClass('tele-alert-action-title')
												  .text(action_name);

			this.newList  = $('<ul class="tele-list">');
			
			this.actionContainer.append(this.actionContainerIcon)
								.append(this.actionContainerTitle)
								.append(this.newList);
			
			if($('.mCSB_container', this.actionsContainer).size() > 0) {
				$('.mCSB_container', this.actionsContainer).append(this.actionContainer);
			} else {
				this.actionsContainer.append(this.actionContainer);
			}
			
			this.lastAction = action_name;
			
		}
		
		var item = this.formatData(req);
		var newListItem = $('<li>').attr('id', 'alert-item-' + x);
		this.newList.append(newListItem);
		newListItem.listitem(item);
		this.printed++;
	
	},
	rowFormatter: function(item) {
		
		//if(item.score > item.score_avarage) { item.score_average = item.score; }
		return {
			icon: 'alert',
			time: item.ts, 
			progbar: true,
			itemID: item.uid,
			progbarBig: item.progbarBig,
			checkable: item.checkable,
			checked: item.checked,
			progbarValue: item.score_average,
			title: item.uri,
			details: [
				{ key: 'country', value: item.country_code },
				{ key: 'IP', value: item.orig_ip },
				{ key: 'city', value: item.city },
				//{ key: 'user', value: item.user_id }
			]
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
	lookupSimilarity: function(RID) {
		
		for(x in this.similarities.hits.hits) {
			var req = this.similarities.hits.hits[x];
			if(req._id == RID) {
				return req;
			}
		}
		return false;
		
	},
	lookupRequest: function(RID) { // Request for RID or false if not found
		for(x in this.session.items) {
			var req = this.session.items[x];
			if(req.uid == RID) {
				return req;
			}
		}
		
		for(x in this.similarities) {
			var req = this.similarities[x];
			if(req.uid == RID) {
				return req;
			}
		}
		
		return false;
	},
	updateSelected: function() {
	
		$('.selected', this.actionsContainer).removeClass('selected');
		var newSelected = $( "li:nth-child(" + (this.selectedIndex + 1) + ")", this.actionsContainer);
		
		$('.tele-listitem-inner', newSelected).addClass('selected');
		
		if(newSelected.data('tele-listitem') && newSelected.data('tele-listitem').options) {
			var dataID = newSelected.data('tele-listitem').options.dataID;
		} else {
			newSelected = $( "li:nth-child(1)", this.actionsContainer);
			var dataID = newSelected.data('tele-listitem').options.dataID;
		}

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
		if(this.selectedIndex < this.session.items.length) {
			this.selectedIndex++;
		}
		this.updateSelected();
	},
	keyDown: function(e) {
		// Up/Down Navigation
		if(e.keyCode == '38') { e.preventDefault(); telepath.sessionflow.scrollUp(); }
		if(e.keyCode == '40') { e.preventDefault(); telepath.sessionflow.scrollDown(); }
		
	},
	destroy: function() {
		// Unbind
		$(document).unbind('overlay_destroy', telepath.alert.destroy);
		$(document).unbind('keydown', telepath.alert.keyDown);
	},
	initAlertTools: function() {
		
		// Archive Alert
		this.archiveAlert = $('<div>').btn({ icon: 'archive', text: 'Archive Alert', callback: function () { }});
		
		// Move To
		this.moveTo = $('<div>').teleDropdown({ icon: 'moveTo', value: 'Move To', callback: function () {	}});
		
		// Edit Rule
		this.editRule = $('<div>').btn({ icon: 'editRule', text: 'Edit Rule', callback: function () { }});
		
		var wrap = $('<div>').addClass('tele-overlay-tools');

		wrap.append('<div class="tele-navsep"></div>')
			.append(this.archiveAlert)
			.append('<div class="tele-navsep"></div>')
			.append(this.moveTo)
			.append('<div class="tele-navsep"></div>')
			.append(this.editRule);
		
		this.overlay.headerEl.append(wrap);
		
	},
	initRequestTools: function() {
		
		// Harmful
		this.harmfulBtn = $('<div>').btn({ icon: 'harmful', text: 'Mark as Harmful', callback: function () { }});
				
		// Safe
		this.safeBtn = $('<div>').btn({ icon: 'safe', text: 'Not harmful', callback: function () { }});
		
		// Archive 
		this.archiveBtn = $('<div>').btn({ icon: 'archive', text: 'Archive', callback: function () {  }});

		var wrap = $('<div>').addClass('tele-overlay-tools');

		wrap.append('<div class="tele-navsep"></div>')
			.append(this.harmfulBtn)
			.append('<div class="tele-navsep"></div>')
			.append(this.safeBtn)
			.append('<div class="tele-navsep"></div>')
			.append(this.archiveBtn);
		
		this.overlay.headerEl.append(wrap);
		
	},
	formatData: function(item) {
		
		var that = this;
		
		if(item._id) { item._source.uid = item._id }
		if(item._source) { item = item._source }
		
		var result = {};
		
		// Sometimes its malformed
		try {
			result.title = decodeURIComponent(item.title ? item.title : item.uri);
		} catch(e) {
			result.title = '';
		}
		

		result.progbar = true;
		result.time    = item.ts;
		result.timeFormat = 'h:i A';
		result.offset = 20;
		result.icon   = 'suspect';
		result.dataID = item.uid;
		
		result.raw = item;
		
		var alert = item.alerts_count > 0;
		
		//if(item.score > item.score_average) { item.score_average = item.score }
		
		/*if(item.alerts_count > 0) {
			$.each(item.alerts, function (i, x) {
				if(x.score > item.score_average) {
					item.score_average = x.score;
				}
			});
		}*/

		result.progbarValue = item.score_average;

		if(alert !== false) {
			result.icon   = 'alert';
			alert = item.alerts[0];
			result.description  = alert.name;
			result.progbarValue = item.score_average;
		} else {
			if(item.avg_score > 85) {
				result.icon = 'suspect_red';
			}
		}
		
		result.callback = function (widget, el) {
			$('.selected', that.actionsContainer).removeClass('selected');
			$('.tele-listitem-inner', widget.element).addClass('selected');
			that.expandRequest(widget.options.dataID);
		};

		return result;
		
	},
	showAlert: function(data) {
		
	},
	expandRequest: function(uid) {
		
		if(this.currentUID == uid) {
			// check if it is really open, Yuli
			var ch = this.requestDetails.children();
			if (ch.length > 0)
			{
				return;
			}
		} else {
			this.currentUID = uid;
		}
		var that = this;
	
		this.requestDetails.empty();
		// remove id from html
		//this.requestDetails.append(uid);
		this.requestDetails.append(telepath.loader);
		
		var req = this.lookupRequest(uid);
		if(req) {
			this.requestInfo = req;
		} else {
			// console.log('RID ' + uid + ' was not found in requests');
			return;
		}
		
		
		this.boxRight.empty();
		
		// Request scores
		this.printRequestScores(req, this.boxRight);
		
		// Alert information
		var alert = req.alerts_count > 0;
		
		$('.tele-overlay-tools', this.overlay.headerEl).remove();
		
		this.printRequestInfo(this.boxRight);

		// temporary dismiss this features
		//if(alert !== false) {
		//	// TODO: Debug Icons
		//	//this.initAlertTools();
		//	this.initRequestTools();
		//} else {
		//	this.initRequestTools();
		//}
		telepath.sessionflow.reloadFlag = Date.now();
	
		// Load similarities
		telepath.ds.get('/similarities/', { param_type: 'request', uid: uid }, function(data, flag) {
			if (flag && telepath.sessionflow.reloadFlag && flag != telepath.sessionflow.reloadFlag)
			{
				// session flow item was changed !
				return;
			}
			that.showSimilarities(data);
		}, null, telepath.sessionflow.reloadFlag);
		
		// Load param data
		$('.tele-similarity-details').remove();
		telepath.ds.get('/sessionflow/get_sessionflow_params', { uid: uid }, function(data, flag) {
			if (flag && telepath.sessionflow.reloadFlag && flag != telepath.sessionflow.reloadFlag)
			{
				// session flow item was changed !
				return;
			}
			that.expandRequestData(data.items);
		}, null, telepath.sessionflow.reloadFlag);
		
	}, 
	expandRequestData: function(data) {
		$('.loader', this.requestDetails).remove();
		this.requestData = data;
		
		this.updateTitle(this.requestData);
	
		this.printParams();
	},
	updateTitle: function(request) {
		/*
		// Lookup alert
		var alert = this.lookupAlert(request.RID);
		
		// Page name
		var title = request.title != '' ? request.title : request.display_path;
	
		// Alert name
		if(alert !== false) {
			title = alert.name;
		}
		
		// Update title
		this.overlay.titleEl.html(title);
		*/
	},
	printParams: function(container) {
		var container = this.requestDetails;
		container.empty();
		this.printParamsFilters(container);
		this.printAlertDetails(container);
		this.printParamsTable(container);
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
				alertObj: that,
				callback: function (widget) {
					telepath.alert.filters[filter.type] = widget.options.checked;
					this.alertObj.printParams();
			}});
			that.filtersContainer.append(filter_el);
		});
		
	},
	printLink: function(request, container) {
		
		
		
		var path = 'http://' + request.host + request.uri;
		// Add get parameters to URL
		var get_params = {};
		$.each(request.parameters, function (i, param) {
			if (param.type == 'G') {
				get_params[param.name] = param.value;
			}
		});
		if (!$.isEmptyObject(get_params)) {
			path += '?' + $.param(get_params);
		}

		var link = $('<a>').attr('target', '_blank').text(path).attr('href', escapeHtml(path)).attr('title' ,path);
		var action = request.business_id ? this.lookupAction(request.business_id) : 'Browsing';
		container.append(link);
		container.append('&nbsp;(' + action + ')');
	
	},
	printAlertDetails: function(container) {
	
		// Container
		this.alertDetails = $('<div>').addClass('tele-alert-details-info');
		container.append(this.alertDetails);

		// Title
		this.alertDetailsTitle = $('<div>').addClass('tele-alert-details-info-title');
		this.printLink(this.requestInfo, this.alertDetailsTitle);
		
		// Time
		this.alertDetailsTimeWrap   = $('<div>').addClass('tele-alert-details-info-time-wrap');
		this.alertDetailsTimeLabel  = $('<div>').addClass('tele-alert-details-info-time-label').text('Request time:');
		this.alertDetailsTime  		= $('<div>').addClass('tele-alert-details-info-time').text(date_format('d/m/y | H:i:s', this.requestInfo.ts));
		this.alertDetailsTimeWrap.append(this.alertDetailsTimeLabel).append(this.alertDetailsTime);

		// Response status
		this.alertDetailsResponseWrap   = $('<div>').addClass('tele-alert-details-info-response-wrap');
		this.alertDetailsResponseLabel  = $('<div>').addClass('tele-alert-details-info-response-label').text('Response Status:');
		this.alertDetailsResponse  		= $('<div>').addClass('tele-alert-details-info-response').text( this.requestInfo.status_code);
		this.alertDetailsResponseWrap.append(this.alertDetailsResponseLabel).append(this.alertDetailsResponse);
		
		//if(this.requestData.score > this.requestData.score_average) { this.requestData.score_average = this.requestData.score }
		
		// Severity
		//if(this.requestData.score_average > 0 && this.requestData.score_average <= 1) {
		//	this.requestData.score_average = this.requestData.score_average * 100;
		//}
		var severityPercent         = parseInt(this.requestData.score_average * 100) + '%';
		this.alertSeverityWrap      = $('<div>').addClass('tele-alert-severity-wrap');
		this.alertSeverityLabel     = $('<div>').addClass('tele-alert-severity-label').text('Severity');
		this.alertSeverityPercent   = $('<div>').addClass('tele-alert-severity-percent').text(severityPercent);
		this.alertSeverityProgBar   = $('<div>').addClass('tele-alert-severity-progbar');
		this.alertSeverityProgValue = $('<div>').addClass('tele-alert-severity-progbar-value').css({ width: severityPercent });
		
		this.alertSeverityWrap.append(this.alertSeverityLabel).append(this.alertSeverityPercent).append(this.alertSeverityProgBar);
		this.alertSeverityProgBar.append(this.alertSeverityProgValue);
		
		this.alertDetails.append(this.alertSeverityWrap).append(this.alertDetailsTitle).append(this.alertDetailsTimeWrap).append(this.alertDetailsResponseWrap);
		
	},
	printRequestInfo: function(container) {
		
		// Setup containers
		
		this.alertInfo      = $('<div>').addClass('tele-alert-info');
		//this.alertInfoTitle = $('<div>').addClass('tele-alert-info-title').text(alert.name);
		//this.alertInfoLabel = $('<div>').addClass('tele-alert-info-label').text('Description:');
		//this.alertInfoDesc  = $('<div>').addClass('tele-alert-info-description').text(alert.description);
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
		
		table.append(getRow('Time:', date_format('m/d/y | H:i:s', this.requestInfo.ts)));
		//table.append(getRow('Severity:', this.getSeverity(alert.numeric_score)));
		table.append(getRow('Applications:', escapeHtml(this.requestInfo.host)));
		table.append(getRow('IP:', this.requestInfo.ip_orig));
		table.append(getRow('Location:', '<span class="flag flag-' + this.requestInfo.country_code + '"></span>' + 
							'<span class="tele-country">' + telepath.countries.a2n(this.requestInfo.country_code) + '</span>'));
		if(this.requestInfo.username){
			table.append(getRow('User:', escapeHtml(decodeURIComponent(this.requestInfo.username))));
		}


		//if(alert.user && alert.user != '') {
		//	table.append(getRow('User:', alert.user));
		//}
		
				
		// Append All
	/*	this.alertInfo.append(this.alertInfoTitle)
//					  .append(this.alertInfoLabel)
					  .append(this.alertInfoDesc)
					  .append();
		*/			  
		container.append(this.alertInfoTable);
		
	
	},
	printParamsTable: function(container) {
		
		var tableWrap = $('<div>').addClass('tele-alert-params-table-wrap');
		var table 	  = $('<table>').addClass('tele-alert-params-table');
		
		var tableWidth = (parseInt(this.boxMid.css('width')) - 50);
	
		$.each(this.requestData.parameters, function(i, param) {

			var param_display = telepath.alert.filters[param.type];
			if(param_display) {
				
				var row = $('<tr>');
				var col_name  = $('<td>').addClass('tele-param-name').html(escapeHtml(param.name)).attr('title',unescapeHtml(escapeHtml(param.name)));
				var col_data  = $('<td>').addClass('tele-param-data').html(escapeHtml(param.value));
				var col_score = $('<td>').addClass('tele-param-score').html(parseInt(param.score_data) + '%');
				
				/*if(parseInt(param.attribute_score_normal)) {
					col_score.addClass('severe');
					col_data.addClass('severe');
				}*/
				if (parseInt(param.score_data)>30){
					row.css({"color":"red"})
				}
				row.append(col_name).append(col_data).append(col_score);
				table.append(row);
				
			}
			
		});
		
		container.append(tableWrap);
		tableWrap.append(table);
		
		//$(tableWrap).mCustomScrollbar({ advanced:{ updateOnContentResize: true } });
		
	}
	
}
