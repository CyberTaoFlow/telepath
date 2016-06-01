telepath.alerts = {
	changeFilter: function(field, newValue, oldValue, eOpts) {
		
		/*
		console.log(field.id);
		console.log(newValue);
		console.log(oldValue);
		*/
		// FROM
		var from_date = Ext.getCmp('a_startDate').value;
		if(from_date) {
			from_date = new Date(from_date);
			var from_time = Ext.getCmp('a_fromHour').rawValue.split(':');
			from_date.setHours(from_time[0]);
			from_date.setMinutes(from_time[1]);
			from_date = from_date.getTime() / 1000;
		}
		
		// TO
		var to_date = Ext.getCmp('a_endDate').value;
		if(to_date) {
			to_date = new Date(to_date);
			var to_time = Ext.getCmp('a_toHour').rawValue.split(':');
			to_date.setHours(to_time[0]);
			to_date.setMinutes(to_time[1]);
			to_date = to_date.getTime() / 1000;
		}	
		
		switch(field.id) {
			/*	
			case 'a_startDate':
				
				if(from_date > to_date) {
					field.setValue(oldValue);
				}
				
			break;
		
			case 'a_fromHour':
				
				if(from_date > to_date) {
					field.setValue(oldValue);
				}
			
			break;
			
			case 'a_endDate':
				
				if(from_date > to_date) {
					field.setValue(oldValue);
				}
				
			break;
			case 'a_toHour':
				
				if(from_date > to_date) {
					field.setValue(oldValue);
				}
			
			break;
			*/
		}
	
	},
	pagingBeforeChange: function (pagingtoolbar, page, eOpts) {
		
		Ext.getStore('alerts').loadPage( page, { params: {
			sortorder: alerts_sortorder,
			sortfield: alerts_sortfield,
			val : currVal,
			variable : currType,
			mode:alerts_mode,
			page:page,
			filter: telepath.alerts.filter.params
		} });
		
		return false;
	
	},
	filter: {
		
		params: JSON.stringify({ from_date: '', to_date: '', selected_apps: [], ip: [], rule: [] }),
		init: function () {

			if(telepath.access.has_perm('Rules', 'get')) {
				this.initRuleTree();
			} else {
				$("#a_filter_rule").before('<span style="padding: 5px; float: left;">No access to rules list.</span>');
				Ext.getCmp('a_filter_rule').hide();
			}
			
			if(!telepath.access.has_perm('Alerts', 'del')) {
				Ext.getCmp('alerts-remove').hide();
				$("#alerts-remove").next().hide(); // .toolbar-seperator
				Ext.getCmp('a_alertsTable').columns[0].hide();
			}

			Ext.getStore('alerts_ips').load();
			
			$('#alertFilters-innerCt').append($('<div>').teleSelect({ label: 'Application', type: 'subdomain', values: [{ text: 'All', id: '-1', sub_id: '-1' }], click: function () { 
				Ext.getCmp('d_apply').show();
				Ext.getCmp('d_cancel').show();
			} }));
			
		},
		apply: function() {
			
			// Apps
			var selected_apps = $('#alertFilters-innerCt .tele-multi').data('tele-tele-select').result();
			
			// FROM
			var from_date = Ext.getCmp('a_startDate').value;
			if(from_date) {
				from_date = new Date(from_date);
				var from_time = Ext.getCmp('a_fromHour').rawValue.split(':');
				from_date.setHours(from_time[0]);
				from_date.setMinutes(from_time[1]);
				from_date = from_date.getTime() / 1000;
			}
			
			// TO
			var to_date = Ext.getCmp('a_endDate').value;
			if(to_date) {
				to_date = new Date(to_date);
				var to_time = Ext.getCmp('a_toHour').rawValue.split(':');
				to_date.setHours(to_time[0]);
				to_date.setMinutes(to_time[1]);
				to_date = to_date.getTime() / 1000;
			}
			
			// IP
			var selection_ip = Ext.getCmp('a_filter_ip').getSelectionModel().selected.items;
			var ip_array = [];
			$.each(selection_ip, function(i, item) {
				ip_array.push(item.data.IP);
			});
			
			// RULES
			var selection_rule = Ext.getCmp('a_filter_rule').getSelectionModel().selected.items;
			var rule_array = [];
			$.each(selection_rule, function(i, item) {
				var id = item.data.id.split('_');
				rule_array.push({ type: id[0], id: id[1] });
			});

			this.params = JSON.stringify({
				
				from_date: from_date ? from_date : '',
				to_date: to_date ? to_date : '',
				selected_apps: selected_apps,
				ip: ip_array,
				rule: rule_array

			});
			
			// Perhaps these values were previously set by investigate or homepage, clear them when we apply new filters
			currType="None"; // the type for alerts filter
			currVal="None";	 // the value for the alerts filter
			
			get_alerts(currType,currVal);
		
		},
		reset: function () {
			
			Ext.getCmp('a_startDate').reset();
			Ext.getCmp('a_fromHour').reset();
			Ext.getCmp('a_endDate').reset();
			Ext.getCmp('a_toHour').reset();
			Ext.getCmp('a_filter_ip').getSelectionModel().deselectAll();
			Ext.getCmp('a_filter_rule').getSelectionModel().deselectAll();
			
			currType="None"; // the type for alerts filter
			currVal="None";	 // the value for the alerts filter
			
			$('#alertFilters-innerCt .tele-multi').remove();
			$('#alertFilters-innerCt').append($('<div>').teleSelect({ label: 'Application', type: 'subdomain', values: [{ text: 'All', id: '-1', sub_id: '-1' }], click: function () { 
				Ext.getCmp('d_apply').show();
				Ext.getCmp('d_cancel').show();
			} }));
			
			telepath.alerts.filter.apply();
		
		},
		initRuleTree: function () {
		
			var that = this;
			
			this.a_rulesStore = Ext.getStore('a_rulesStore');
			this.loadingNode = { 
				text: 'Loading...', 
				expandable: false, 
				leaf: true, 
				expanded: false, 
				iconCls: 'tree-icon-loading',
			};
			
			this.a_rulesStore.addListener('refresh', function () {
				
				// Show loading
				that.a_rulesStore.setRootNode(that.loadingNode);
				// Show categories
				
				telepath.util.request('/rules', { mode:"get_alert_rule_categories" }, function(data) {
					
					that.a_rulesStore.setRootNode({ text: 'All Rules', expanded: true });
					var node = that.a_rulesStore.getRootNode();
					
					if(data.items) {
						$.each(data.items, function(i, item) {
							
							var newNode = { id: 'category_' + item.id, text: item.category, expandable: item.expandable, expanded: false, type: 'category' };
							node.appendChild(newNode);
							
						});
					}
					
				}, 'Error getting rule categories.');
			
			});
			
			this.a_rulesStore.addListener('expand', function (node, eOpts) {
				
				switch(node.data.type) {
					
					// Expand category
					case 'category':
						
						node.removeAll();
						node.appendChild(that.loadingNode);
					
						telepath.util.request('/rules', { mode:"expand_alert_category", category_id: node.data.id.split('_')[1] }, function(data) {
					
							node.removeAll();
							
							if(data.items) {
								$.each(data.items, function(i, item) {
									
									var newNode = { id: 'group_' + item.id, text: item.text, expandable: false, expanded: false, type: 'group' };
									node.appendChild(newNode);
									
								});
							}
							
						}, 'Error expanding rule category ' + node.data.id.split('_')[1]);
						
					break;
					
					// Expand group
					case 'group':
								
					break;
					
				}
				
			});
			
			this.a_rulesStore.update();
			
		}
	
	},

	debug: false,
	initialized: false,
	init: function() {
		
		// Alerts
		Ext.StoreManager.getByKey('alerts_ips').proxy.url = telepath.controllerPath + '/alerts/get_alerts_ips';
		Ext.StoreManager.getByKey('alerts').proxy.url 	  = telepath.controllerPath + '/alerts/get_alerts';
		
		if(typeof(a_node_tip) == 'undefined') {
			a_node_tip = Ext.create('MyApp.view.a_node_tip');
		}
		this.refresh();
		
		// Bind doubleclick
		Ext.getCmp('a_alertsTable').addListener(
			'itemdblclick',
			function(dv, record, item, index, e) {
				Ext.getCmp('a_sessionDetails').expand();
				select_row(record);
			}
		);
		
		this.filter.init();
		
		this.initialized = true;
	
	},
	
	refresh: function() {
	
		setTimeout(function () {
			
			var current_update = new Date().getTime();
			if(current_update - last_update > 2000 || last_update == 0) {
				
				// Refresh Alerts
				get_alerts(currType,currVal);
			
			}
			
		}, 100);

	},

	deleteSelected: function () {
		
		// Count
		var n = $( ".alerts-checkbox:checked" ).length;
		
		if(n == 0) {
			Ext.Msg.alert('Status', 'You havent selected any alerts to delete.');
			return;
		}
		
		// Confirm
		Ext.Msg.confirm('Delete Alerts', 'Are you sure you want to delete selected ' + n + ' alert' + (n > 1 ? 's' : '') + '?' , function(btn) {
		
			if (btn == 'yes') {
				
				// Collect IDS
				var ids_to_remove = [];
				$('.alerts-checkbox:checked').each(function () {
					ids_to_remove.push($(this).attr('rel'));
				});

				
				telepath.util.request('/alerts/del_alerts', { ids_to_remove: ids_to_remove }, function(data) {
					
					telepath.alerts.refresh();
					Ext.Msg.alert('Status', 'Alert' + (n > 1 ? 's' : '') + ' deleted successfully.');
					
				}, 'Error deleting alerts.');
	
			}
			
		});

	}
	
}

var last_update = 0;

var a_selected_row;
var alerts_selected_index;
//var rule;
//var ip;
//var sid;
var alerts_sortorder="DESC";	//sort order for the table
var alerts_sortfield="date";	//sort field for the table
var alerts_mode = 'get_alerts'; //the mode for getting alerts from the server
var currType="None";		//the type for alerts filter
var currVal="None";		    //the value for the alerts filter
var alerts_cell_num;		//the cell index clicked on the table
var alerts_row_num;		//the row index clicked on the table
var search_window ;		//the filter window
var showFlow = 0;
var alerts_refresh_interval;	//the interval javascript object for the alerts table refresh
var a_prevField;		//sort field for the table
var a_prevOrder;		//sort order for the table
var a_alert_params;		//the Ext.gridpanel object for the params when hovering the param value column
//the right click menu for the table
var alerts_menu = Ext.create('Ext.menu.Menu', { 
	floating: true,
	hidden: true,
	hideMode:'display',//crucial to fix black squares bug in chrome
	id: 'a_menu',
	itemId: 'a_menu',
	items: [
		{
			xtype: 'menuitem',
			id: 'a_MenuSid',
			text: 'Investigate this session'
		},
		{
			xtype: 'menuitem',
			id: 'a_MenuIp',
			text: 'Investigate by IP'
		},
		{
			xtype: 'menuitem',
			id: 'a_MenuUser',
			text: 'Investigate by user',
			hidden:true
		},
		{
			xtype: 'menuitem',
			id: 'a_MenuAdd',
			text: 'Add to Investigate Filter'
		},
		{
			xtype: 'menuitem',
			id: 'a_showFlow',
			text: "Show session flow"
		},
		{
			xtype: 'menuitem',
			id: 'a_markNormal',
			text: "Add IP to whitelist"
		},
		{
		    xtype: 'menuitem',
		    id: 'a_copy_menu',
		    text: 'Copy to clipboard'
		}

	]
});
	

/*
 * Function: show_alerts_menu
 * 
 * Invoked:
 * 	When right clicking the alerts table.
 * 	The function opens the menu
 * 	binded to the itemcontextmenu event of alerts table
 * 
 * Parameters:
 *	tablepanel - the alert table gridpanel object of extjs
 *	record - the record object of the row clicked
 * 	item - 
 * 	index - the row number clicked
 * 	e - the event object
 * 	options - an empty object
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */

function show_alerts_menu(tablepanel,record,item,index,e,options) {

	e.preventDefault();
	a_selected_row = record;
	alerts_menu.showAt(e.xy[0],e.xy[1]-20);

	var dataIndex = alerts_selected_index;
	//cellNum = index;
	if (dataIndex=="td1" |
	 	dataIndex=="td2" | 
		dataIndex=="td3" | 
		dataIndex=="td4" | 
		dataIndex=="td9" | 
		dataIndex=="td10"| 
		dataIndex=="td11" 	)
		Ext.getCmp("a_MenuAdd").enable();
	else	
		Ext.getCmp("a_MenuAdd").disable();


}


//////// alerts right click menu buttons binding //////////////////
Ext.getCmp('a_MenuSid').addListener(
	'click',
	function(){
		
		Ext.getCmp('i_filterSession').setValue(a_selected_row.get('td2'));
		Ext.getCmp("investigatePanel").show();
		
			var params = {};
			
			// FROM
			var date_from = new Date((a_selected_row.data.td1 * 1000) - (7 * 24 * 3600 * 1000));
			Ext.getCmp('i_fromHour').setValue(date_from.getHours() + ":" + date_from.getMinutes());
			Ext.getCmp('i_startDate').setValue(date_from);
			
			params['fromdate'] = date_from.getTime() / 1000;

			// TO
			var date_to   = new Date((a_selected_row.data.td1 * 1000) + (7 * 24 * 3600 * 1000));
			Ext.getCmp('i_toHour').setValue(date_to.getHours() + ":" + date_to.getMinutes());
			Ext.getCmp('i_endDate').setValue(date_to);
			params['todate']= date_to.getTime() / 1000;
			
			params['SID']=a_selected_row.get('td2');		
			
			investigate_main(0, params);
		
	}
);

Ext.getCmp('a_copy_menu').addListener(
	'click',
	function(){
		var column = alerts_cell_num;
		var record = a_selected_row;
		var dataIndex = alerts_selected_index;
		var text = a_selected_row.get(dataIndex);
		if(dataIndex == 'td1') {
			text = date_format('d/m/y H:i:s', text);
		}
		copy_to_clipboard(text);
	}
);

Ext.getCmp('a_MenuAdd').addListener(
	'click',
	function(that,e,eOPts){
		
		// If we dont have this element initialized yet, quickly reset update timer, load/hide the panel
		if(!Ext.getCmp("i_filterCountry")) {
			last_update = new Date().getTime();
			Ext.getCmp('investigatePanel').show();
			Ext.getCmp('alertsPanel').show();
		}

		var column = alerts_cell_num - 1;
		var dataIndex = alerts_selected_index;
		if (dataIndex=="td2"){//SID		
			Ext.getCmp("i_filterSession").setValue(a_selected_row.get("td2"));
		}
		else if(dataIndex=="td3"){//User		
			Ext.getCmp("i_filterUser").setValue(a_selected_row.get("td3"));
		}
		else if(dataIndex=="td4"){//IP		
			Ext.getCmp("i_filterIp").setValue(a_selected_row.get("td4"));
		}
		else if(dataIndex=="td9"){//Country		

			Ext.getCmp("i_filterCountry").setValue(aliasToName[a_selected_row.get('td9')]);
			Ext.getCmp("i_filterCity").setValue(a_selected_row.get('td10')); // UNDEF
			
		}
		else if(dataIndex=="td10"){//City
			Ext.getCmp("i_filterCity").setValue(a_selected_row.get("td10"));
		}
		else if(dataIndex=="td1"){//Date		
		
			var date =  new Date(a_selected_row.get(dataIndex) * 1000);

			date.setHours(date.getHours() - 6);
			
			Ext.getCmp('i_startDate').setValue(date);
			Ext.getCmp('i_fromHour').setValue(date.getHours() + ":" + date.getMinutes());
			
			date.setHours(date.getHours() + 12);
			
			Ext.getCmp('i_toHour').setValue(date.getHours() + ":" + date.getMinutes());
			Ext.getCmp('i_endDate').setValue(date);
			
		}
		else if(dataIndex=="td11"){//Application
			var app_name = a_selected_row.get("td11");
			if (!app_name)
				return;
			var app_record = Ext.getCmp('i_filterApps').getStore().findRecord('app',a_selected_row.get("td11"));
			if (!app_record){
				Ext.Msg.alert('Info', 'Could not add application to investigation filter.');
				return;
			}
			Ext.getCmp('i_filterApps').setValue(app_record);
		}
	}
);

Ext.getCmp('a_MenuIp').addListener(
	'click',
	function(){
	
		var params = {};
		
		params['user_ip']=a_selected_row.get('td4');
		
		Ext.getCmp("investigatePanel").show();
		
		// FROM
		var date_from = new Date((a_selected_row.data.td1 * 1000) - (7 * 24 * 3600 * 1000));
		Ext.getCmp('i_fromHour').setValue(date_from.getHours() + ":" + date_from.getMinutes());
		Ext.getCmp('i_startDate').setValue(date_from);
		
		params['fromdate'] = date_from.getTime() / 1000;

		// TO
		var date_to   = new Date((a_selected_row.data.td1 * 1000) + (7 * 24 * 3600 * 1000));
		Ext.getCmp('i_toHour').setValue(date_to.getHours() + ":" + date_to.getMinutes());
		Ext.getCmp('i_endDate').setValue(date_to);
		params['todate']= date_to.getTime() / 1000;
		
		Ext.getCmp('i_filterIp').setValue(a_selected_row.get('td4'));
		
		investigate_main(0, params);

	}
);

Ext.getCmp('a_MenuUser').addListener(
	'click',
	function(){
		var params={};
		if (a_selected_row.get('td3')=="" || a_selected_row.get('td3').length==1){
			Ext.Msg.alert('Info', 'Cannot investigate an empty value.');
			return;
		}
		
		Ext.getCmp("investigatePanel").show();
		
		params["user"]=a_selected_row.get('td3');
		
		// FROM
		var date_from = new Date((a_selected_row.data.td1 * 1000) - (7 * 24 * 3600 * 1000));
		Ext.getCmp('i_fromHour').setValue(date_from.getHours() + ":" + date_from.getMinutes());
		Ext.getCmp('i_startDate').setValue(date_from);
		params['fromdate'] = date_from.getTime() / 1000;

		// TO
		var date_to   = new Date((a_selected_row.data.td1 * 1000) + (7 * 24 * 3600 * 1000));
		Ext.getCmp('i_toHour').setValue(date_to.getHours() + ":" + date_to.getMinutes());
		Ext.getCmp('i_endDate').setValue(date_to);
		params['todate']= date_to.getTime() / 1000;
		
		Ext.getCmp('i_filterUser').setValue(a_selected_row.get('td3'));
		investigate_main(0, params);		
		
	}
);


Ext.getCmp('a_showFlow').addListener(
	'click',
	function() {
		Ext.getCmp('a_sessionDetails').expand();
		select_row(a_selected_row); //old graph
	}
);

Ext.getCmp('a_markNormal').addListener(
	'click',
	function(){
		var ip = Ext.getCmp('a_alertsTable').getSelectionModel().getSelection()[0].data.td4
		
		telepath.ipconfig.show(ip, 'Add IP to Whitelist', function (ip) {
				telepath.ipconfig.add_ip_to_whitelist(ip);
		});
		
	}
);

/*
 * Function: update_investigate_date_filter
 * 
 * Invoked:
 * 	When clicking 'Investigate this session' on the right click menu
 * 	The function sets the date on the investigate filter to a day before and the same day
 * 
 * Parameters:
 * 	epoch_start - an epoch string for the start date to set
 * 	epoch end - an epoch string for the end date to set
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function update_investigate_date_filter(epoch_start,epoch_end){		
		var startDate = new Date(epoch_start);
		startDate.setDate(startDate.getDate()-1);
		Ext.getCmp('i_startDate').setValue(startDate);
		var endDate = new Date(epoch_end);
		endDate.setDate(endDate.getDate()+1);
		Ext.getCmp('i_startDate').setValue(startDate);
		Ext.getCmp('i_endDate').setValue(endDate);
}


/*
 * Function: select_row
 * 
 * Invoked:
 * 	When cliking 'Show Flow' for a row from the right click menu or the toolbar panel
 * 	The function shows the session flow for the record
 * 
 * 
 * Parameters:
 * 	row - a record object of extjs of the row selected
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
var current_alert_group_rule_id = 0;
function select_row(row) 
{
	if (row==undefined)
		return;

	a_selected_row=row;
	var sid     = row.get("td2");
	var rule_id = row.get("td12");
	var epoch   = parseInt(row.get("td1"));
	var ip      = row.get("td4")
	
	var sessionflow = new SessionFlow(sid, epoch, rule_id, ip);
	window.sessionflow = sessionflow;

}

/*
 * Function: start_alerts_refresh
 * 
 * Invoked:
 * 	When the user checks the 'Auto-refresh(Mins)' checkbox on the table toolbar panel
 * 	The function refreshes the alerts table every given interval
 * 
 * Parameters:
 * 	
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function start_alerts_refresh(refresh_rate){
	alerts_refresh_interval = setInterval(
		function(){
			if (Ext.getCmp('alertsPanel').isVisible())
				get_alerts(currType,currVal);				
		},
		1000*60*refresh_rate
	);
}
/*
 * Function: stop_alerts_refresh
 * 
 * Invoked:
 * 	When the user unchecks the 'Auto-refresh(Mins)' checkbox on the table toolbar panel
 * 	The function stops refreshing the alerts table every given interval
 * 
 * Parameters:
 * 	
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function stop_alerts_refresh(){
	clearInterval(alerts_refresh_interval);
}



/*
 * Function: get_alerts
 * 
 * Invoked:
 * 	The MAIN function. Called whenever we want to load alerts to the table
 * 
 * Parameters:
 * 	type - the type of filter we want. "SID", "IP", "Country" or "Rule". Put "None" on type and value to disable filter
 * 	value - the value we want to get for the given type. if type is "None", put "None" for value too.
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function get_alerts(type,value)
{	
	
	$('.x-panel-body', Ext.getCmp('a_sessionDetails').getEl().dom).empty();
	Ext.getCmp('a_sessionDetails').collapse();
	
	if(telepath.alerts.debug) {
		console.log('Refresh alerts, type:: ' + type + ' value:: ' + value);
	}
	
	SIDS = {};
	
	last_update = new Date().getTime();
	
	var params = {
					start: 0,
					limit: 30,
					// other params
					sortorder: alerts_sortorder,
					sortfield: alerts_sortfield,
					val : value,
					variable : type,
					mode:alerts_mode,
					filter: telepath.alerts.filter.params
	    		};
	
	//if its a new value or a new category, start from page 1
	if (currType!=type || currVal!=value) {
		Ext.getStore('alerts').loadPage(1, { params: params });
	}
	//else , continue from the same page
	else {
		Ext.getStore('alerts').load({ params: params });
	}
	
	currType=type;
	currVal=value;

}

/*
 * Function: get_alert_logs
 * 
 * Invoked:
 *      The MAIN function. Called whenever we want to load alerts to the table
 * 
 * Parameters:
 *      type - the type of filter we want. "SID", "IP", "Country" or "Rule". Put "None" on type and value to disable filter
 *      value - the value we want to get for the given type. if type is "None", put "None" for value too.
 * 
 * 
 * Returns:
 *      undefined       
 * 
 * 
 */
function get_alert_logs(sid)
{
	if (!sid)
		return;
	Ext.getStore('alerts').loadPage(1,{
		params: {
			start: 0,
			limit: 30,
			sortorder: alerts_sortorder,
			sortfield: alerts_sortfield,
			'sid' : sid,
			variable : type,
			mode:alert_log_mode
		}
	});
}


/*
 * Function: reset_alerts_table
 * 
 * Invoked:
 * 	When we want to get the first page of the alerts table, without any filtering.
 * 
 * Parameters:
 * 	
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function reset_alerts_table() {

	currType="None";
	currVal="None";
	
	Ext.getStore('alerts').loadPage(1, {
		params: {
			start: 0,
			limit: 30,
			sortorder: alerts_sortorder,
			sortfield: alerts_sortfield,
			val : currType,
			variable : currVal,
			mode:alerts_mode
		}
	});	
}


/*
 * Function: open_alerts_filter
 * 
 * Invoked:
 * 	When the user clicked "Filter" on the table toolbar
 * 	The function opens the alerts filter
 * 
 * Parameters:
 * 	
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function open_alerts_filter() {
	search_window = Ext.create('MyApp.view.a_SearchAlert');
	search_window.show();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Function: create_params_table
 * 
 * Invoked:
 * 	On startup. Creates the table and the store that show up when we hover the Parameter and Value columns
 * 
 * Parameters:
 * 	
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function create_params_table(){
	
	$(".alerts-checkbox-column").click(function () {
		var is_checked = $(".alerts-checkbox-column").is(':checked');
		$(".alerts-checkbox").prop('checked', is_checked);
	});
	
	Ext.define('alert_param', {
		extend: 'Ext.data.Model',
		fields: [
		 	{name: 'par', type: 'string'},
		 	{name: 'val', type: 'string'},
		 	{name: 'par_id', type: 'string'}
		]
	});	
	Ext.create('Ext.data.Store', {
		model: 'alert_param',
		storeId: 'alert_param_store',
		data : [
			// {par: 'Ed',    val: 'Spencer',}
		]
	});
			
	a_alert_params = new Ext.container.Container({
		extend: 'Ext.container.Container',
		//autoRender: true,
		floating: true,
		//height: 250,
		id: 'a_alert_params',
		style: 'background-color:white;',
		width: 50,//
    		listeners: {
			show: {
		    		fn: show_a_alert_params_cont,
			    	scope: this
			}
    		},
		items:[
		        {
				id:'alert_param_table',
			    xtype: 'gridpanel',
				store: Ext.data.StoreManager.lookup('alert_param_store'),
				//hideHeaders:true,
				//height:250,
				forceFit:true,
				resizable:true,
				//hideHeaders:true,
				rowLines:true,
		    		listeners: {
						itemcontextmenu: {
								fn: show_a_alert_params_menu,
								scope: this
						}
					//render: {
				    	//	fn: bind_resize_handler,
					 //   	scope: this
					//}
		    		},
		            	columns: [
		            		{	
						xtype: 'rownumberer',
						maxWidth:30,
						minWidth:30
					},
		        		{       
						id:'alert_param_table_td0',
		                    		xtype: 'gridcolumn',
		                    		dataIndex: 'par',
		                    		text: 'Parameter',
									width: 100
		                	},
		                	{
						id:'alert_param_table_td1',		                		
		                    		xtype: 'gridcolumn',
		                    		dataIndex: 'val',
		                    		text: 'Value',
						editor: {
							xtype: 'textfield',
							allowBlank: false
						}
		                	}
		            	],
		            	viewConfig: {

		            	},
				plugins: [
					Ext.create('Ext.grid.plugin.CellEditing', {
				    		ptype: 'cellediting',
				    		listeners: {
							edit: {
						    		fn: param_tables_edit_handler,
							    	scope: this
							},
							beforeedit:{
						    		fn: param_tables_beforeedit_handler,
							    	scope: this
							},
							canceledit:{
						    		fn: param_tables_canceledit_handler,
							    	scope: this
							}
				    		}
					})
				]
		        }
		]

	});


}

function show_a_alert_params_cont(){
	var width_name = Ext.getCmp('a_par_name_col').getWidth()
	var width_val = Ext.getCmp('a_par_val_col').getWidth()
	Ext.getCmp('alert_param_table_td0').setWidth(width_name-25)
	Ext.getCmp('alert_param_table_td1').setWidth(width_val)
}

var a_alert_params_menu = Ext.create('Ext.menu.Menu', {   
	floating: true,
	hidden: true,
	id: 'a_alert_params_menu',
	items: [
		{
			xtype: 'menuitem',
			id: 'a_alert_params_menu_investigate',
			text: 'Investigate value'
		}
	]
});
Ext.getCmp('a_alert_params_menu_investigate').addListener('click',
	function(){
		var record = Ext.getCmp('alert_param_table').getSelectionModel().getSelection()
		if (!record.length){
			Ext.Msg.alert('Info', 'Please select a row from the table first');
			return
		}
		record = record[0]
		var par_id = record.get('par_id')
		var val = record.get('val')
		var par = record.get('par')
		var params = {}
		params['attribute_filter'] = par_id
		params['attribute_filter_val'] = val
		params['attribute_filter_name'] = par
		Ext.getCmp("investigatePanel").show();		
		investigate_main(0, params);
		
	}	
)

function show_a_alert_params_menu(dataview,record,item,index,e,options){
	//check if the node clicked is a rule or a group
	e.preventDefault();
	a_alert_params_menu.setPosition(e.xy[0],e.xy[1]-20,false);
	a_alert_params_menu.show()
	
}
a_alert_params_menu.addListener(
	'show',
	function(){
		on_edit_mode = true	
	}
)

a_alert_params_menu.addListener(
	'hide',
	function(){
		on_edit_mode = false
	}
)


var on_edit_mode = false
function param_tables_beforeedit_handler(){
	on_edit_mode=true
}
function param_tables_edit_handler(){
	on_edit_mode=false
}
function param_tables_canceledit_handler(){
	on_edit_mode=false
}
var a_show_window = true



/*
 * Function: hovered_alert_params
 * 
 * Invoked:
 * 	Any time we hover the parameter or value column on the table.
 * 	The function makes the panel show and loads the values to it
 * 
 * Parameters:
 * 	
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */


function hovered_alert_params(rowIndex,colIndex) {

	var row_height = $(Ext.getCmp('a_alertsTable').getView().getNode(0)).height();
	var col_pos = Ext.getCmp('a_par_val_col').getPosition();
	var x_offset = col_pos[0];
	var y_offset = row_height*(rowIndex+1)+col_pos[1];
	var scroll_offset = $('#a_alertsTable-body .x-grid-view').scrollTop();
	a_alert_params.setPosition(x_offset,y_offset - scroll_offset);
	var record = Ext.getCmp('a_alertsTable').getStore().getAt(rowIndex);
	Ext.getCmp('alert_param_table').getStore().loadData(record.get('td13'));
	a_show_window = true;
	setTimeout("handle_mouse_on_event()",1000);
	
}

function handle_mouse_on_event() {
	if (a_show_window){
		a_alert_params.setWidth(Ext.getCmp('a_par_name_col').getWidth()+Ext.getCmp('a_par_val_col').getWidth());
		a_alert_params.show();
	};
	a_alert_params.getEl().addListener('mouseleave',
		function(){
			hide_alert_params_window();
		}
	);
}

function hide_alert_params_window() {
	if (on_edit_mode)
		return;
	a_show_window=false;	
	a_alert_params.hide();
	a_alert_params_menu.hide();
}

function resize_alert_params_window() {
	var width_name = Ext.getCmp('a_par_name_col').getWidth();
	var width_val = Ext.getCmp('a_par_val_col').getWidth();
	a_alert_params.setWidth(width_name+width_val);
	Ext.getCmp('a_alertsTable').getView().refresh();
	hide_alert_params_window();
}

function add_parameter_to_current_rule_group(parameter_id) {
	
	telepath.util.request('/rules', { mode:"add_parameter_to_rule_group", rule_group_id:current_alert_group_rule_id, 'parameter_id': parameter_id }, function(data) {

		select_row(a_selected_row);
		return;
		
	}, 'Error adding parameter to rule group.');

}