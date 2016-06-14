var c_addOrEditAgent;
var selectedScheduleCell;
var app_id_selected;
var edit_mode_flag = false;
var c_updateInterval;
var c_ctl2Interval;
var interfaceSelect;

function updateRelevantFields() {
}

telepath.config = {

	testmail: function() {
		
		if(Ext.getCmp('c_reportTest').isValid()) {
			
			if(Ext.getCmp('c_reportSMTP').value == '') {
				Ext.Msg.alert('Error', 'SMTP Server cant be blank.');
				return;
			}
			
			telepath.util.request('/cron/testmail', { 
				'smtp':		 Ext.getCmp('c_reportSMTP').value,
				'smtp_port': Ext.getCmp('c_reportSMTP_port').value,
				'smtp_user': Ext.getCmp('c_reportUser').value,
				'smtp_pass': Ext.getCmp('c_reportPassword').value,
				'test_mail': Ext.getCmp('c_reportTest').value
			}, function(data) {
			
				Ext.Msg.alert('Success', 'Test mail was sent.');
			
		}, 'Error while trying to send test mail');
			
		} else {
			Ext.Msg.alert('Error', 'Invalid mail supplied.');
		}
	
	},	
	subdomainSelect: function(id) {
		
		// Get subdomain IP's and populate store
		// Ext.getCmp('c_app_general_button').disable();
		// set_readonly_app_general_info(true);
		
		Ext.getCmp('c_app_authentication_button').disable();
		Ext.getCmp('c_app_ssl_button').disable();
		Ext.getCmp('c_app_general_button').btnEl.dom.click();
				
		reset_app_general_info();
		reset_app_authentication_info();
		reset_app_ssl_info();
		reset_app_advanced_info();
		
		telepath.util.request('/applications/get_subdomain', { id: id }, function(data) {
	
			store = Ext.getCmp('c_app_general_add_ip_list').getStore();
			var ips_array = data.items.ips;
			for(var i=0;i<ips_array.length;i++){
				if (ips_array[i]!="")
					store.add({
						ip:ips_array[i]
					});
			};
			set_readonly_app_general_info(false);
		
		}, 'Error while trying to get subdomain details');

	},
	getInterfaceSelect: function (value) {
		
		var html = '<select class="interfaceSelect">';
		
		$.each(telepath.interfaces.data, function (i, interfaceName) {
			var selected = interfaceName == value ? 'selected="selected"' : '';
			html += '<option ' + selected + ' value=' + interfaceName + '>' + interfaceName + '</option>';
		});
		
		html += '</select>';
		return html;
		
	},
	debug: true,
	reload: function () {
		
		telepath.config.apptreeCTL.refresh();
		
		if(telepath.access.has_perm('Config', 'get')) {
		
			// Load Stores
			$.each(['c_ip_whitelist_store', 'c_ip_ignorelist_store'], function (i, v) {
				Ext.StoreManager.getByKey(v).load();
			});
			
			initConfigFields();
			
		}
		
		
		telepath.config.appCancelEdit();
		
	},
	init: function () {
		
		// Disable controls
		
		set_readonly_app_general_info(true);
		set_readonly_app_authentication_info(true);
		set_readonly_app_ssl_info(true);
		set_readonly_app_advanced_info(true);
		
		// Do the tabs
		
		var container = Ext.getCmp('configTabs').el.dom;
		var tabs_container = $('<div>').addClass('telepath-toolbar');
		
		var tabs = [];
		
		if(telepath.access.has_perm('Config', 'set')) {
			tabs.push({ id: 'save', text: 'Save' });
		}
		
		tabs.push({ id: 'cancel', text: 'Cancel' });
		tabs.push({ id: 'network', text: 'Network' });
		tabs.push({ id: 'operation', text: 'Operation Mode' });
		tabs.push({ id: 'reports', text: 'Reports' });
		tabs.push({ id: 'applications', text: 'Web Applications', className: ' fcurrent' });
		
		
		Ext.getCmp('configTabs').tabBar.hide();
		
		$(container).prepend(tabs_container);

		// Tab Buttons
		$.each(tabs, function(i, tab) {
			var className = '';
			if(tab.className) {
				className = tab.className;
			}
			$(tabs_container).append('<div id="btn-' + tab.id + '" class="tp-button' + className + '"><div class="tp-button-inner"><span>' + tab.text + '</span></div></div><div class="btnseparator"></div>');
		});
		
		// Tab Buttons Handler
		
		$(".telepath-toolbar .tp-button", container).click(function () {
			
			var id = $(this).attr('id').split('-')[1];
			
			switch(id) {
				
				case 'save':
				
					updateConfig();
				
				break;
				case 'cancel':
				
					telepath.config.reload();
				
				break;
				default:
				
					$('.fcurrent', container).removeClass('fcurrent');
					$(this).addClass('fcurrent');
					Ext.getCmp(id + 'Panel').show();
				
				break;
			
			}
			
		});
		
		Ext.getCmp('reportsPanel').addListener('show', function(e) { 
			/*
			if($('#ckeditor').size() == 0) {
				
				var editorContainer = $('<div>').attr('id', 'ckeditor');
				editorContainer.css({ 'height': 400, 'width': 900, 'margin': '18px 0px 0px 400px', 'float': 'left' });
				editorContainer.append('<img src="images/loader.gif">');
				
				yepnope({
				load: [	"lib/ckeditor/ckeditor.js" ],
					complete: function () { 
						
						var editor;
						var html = '';
						var config = {};
						
						$('img', editorContainer).remove();
						editor = CKEDITOR.appendTo( 'ckeditor', config, html );
						
					}
				});
				
				$('#reportsPanel-innerCt').append(editorContainer);
			
			}
			*/
		});
		
		// Load Stores
		$.each(['c_ip_whitelist_store', 'c_ip_ignorelist_store'], function (i, v) {
			Ext.StoreManager.getByKey(v).load();
		});
		
		// Display fields
		setTimeout(function () { initConfigFields(); }, 2000);
		
		if(telepath.access.has_perm('Applications', 'get')) {
			
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
		
			telepath.config.apptree    = Ext.create('MyApp.view.MultiSelectAppTreePanel');
			telepath.config.apptreeCTL = new AppTreePanel('Application', telepath.config.apptree);
			Ext.getCmp('c_apptree_container').add(telepath.config.apptree);
			
			if(!telepath.access.has_perm('Applications', 'set')) {
				Ext.getCmp('c_app_save_button').hide();
			}
			
		} else {
			
			telepath.config.apptree = {};
			telepath.config.apptreeCTL = { reload: function () { /* Nothing here */ } };
			
			$('#c_apptree_container').append('<span style="float: left; padding: 10px;">No access to application list.</span>');
			Ext.getCmp('c_app_config').hide();
			
		}
		
		
	
		// Load Scheduler
		if(!telepath.scheduler) {
			yepnope({
				load: [	"js/telepath.scheduler.js" + get_ts() ],
				complete: function () { telepath.scheduler.init(); }
			});
		}
		
		// Bind Scheduler Buttons
		var btn_operation_schedule = Ext.getCmp('btn_operation_schedule');
		var btn_reports_schedule   = Ext.getCmp('btn_reports_schedule');
		
		btn_operation_schedule.addListener('click', function () {
			telepath.scheduler.displaySchedule("c_schedulerStore");
		});
		
		btn_reports_schedule.addListener('click', function () {
			telepath.scheduler.displaySchedule("c_reportScheduler");
		});
		
		telepath.config.container = container;
		$(window).trigger('telepath_config_loaded');
		
	},
	appsReload: function() {
		
		reset_app_general_info();
		reset_app_authentication_info();
		reset_app_ssl_info();
		reset_app_advanced_info();
		set_readonly_app_general_info(true);
		set_readonly_app_authentication_info(true);
		set_readonly_app_ssl_info(true);
		set_readonly_app_advanced_info(true);
		
		if(telepath.config && telepath.config.apptreeCTL) {
			telepath.config.apptreeCTL.refresh();
		}
		if(telepath.workflow && telepath.workflow.apptreeCTL) {
			telepath.workflow.apptreeCTL.refresh();
		}
		
		initConfigFields();
		
	},
	appCreateNew: function () {
		
		var win = Ext.WindowManager.get('application_wizard')
		if (win==undefined)
		win = Ext.create('MyApp.view.application_wizard');
		win.show();
	
	},	
	appStartEdit: function () {
		
		edit_mode_flag = true;
		set_readonly_app_general_info(false);
		set_readonly_app_authentication_info(false);
		set_readonly_app_ssl_info(false);
		set_readonly_app_advanced_info(false);
		Ext.getCmp('c_app_save_button').enable();
		Ext.getCmp('c_app_cancel_button').enable();	
		Ext.getCmp('c_app_config').expand();
	
	},
	appCancelEdit: function () {
		
		edit_mode_flag = false;
		set_readonly_app_general_info(true);
		set_readonly_app_authentication_info(true);
		set_readonly_app_ssl_info(true);
		set_readonly_app_advanced_info(true);
		Ext.getCmp('c_app_save_button').disable();
		Ext.getCmp('c_app_cancel_button').disable();
		
		if(telepath.config.apptree.selModel.selected.items.length > 0) {
			var app_id = telepath.config.apptree.selModel.selected.items[0].data.id;
			telepath.config.appBeforeEdit(app_id);
		}
		
	},
	appBeforeEdit: function (app_id) {
		
		var that = this;
		
		set_readonly_app_general_info(true);
		set_readonly_app_authentication_info(true);
		set_readonly_app_ssl_info(true);
		set_readonly_app_advanced_info(true);
	
		telepath.util.request('/config/get_app', { id: app_id }, function(data) {
			load_data_per_app(data.items[0]);
			Ext.getCmp('c_app_authentication_button').enable();
			Ext.getCmp('c_app_ssl_button').enable();
			that.appStartEdit();
		}, 'Failed retreiving application settings.');
	
	},
	openParameters: function (sendTarget) {
		
		sendTo = sendTarget;
		
		if(sendTo == "c_app_redirect_page") {
			selection_state = "page";
		}
		
		if(sendTo == "Application Authentication Form") {
			selection_state = "parameter";
		}
		
		if(sendTo == "app_wizard_redirect_page") {
			Ext.Msg.alert('Error', 'You cant configure redirect page from within application wizard.');
			return;
		}
		
		if(sendTo == "c_app_logout_page") {
			selection_state = "page";
		}
		
		if(sendTo == "c_app_logout_param") {
			selection_state = "parameter";
		}
		
		telepath.parameters.show(false);
		
	},
	addToWhitelistClick: function () {
		
		telepath.ipconfig.show(false, 'Add IP to Whitelist', function (ip) {
		
			Ext.getCmp('c_net_ip_whitelist').getStore().add({ td0:ip });
			
		});
	
	},
	addToIgnorelistClick: function () {
		
		telepath.ipconfig.show(false, 'Add IP to Ignorelist', function (ip) {
		
			Ext.getCmp('c_net_ip_ignorelist').getStore().add({ td0:ip });
			
		});
	
	}

}

// Linking to new model
function cancel_app_edition() {
	telepath.config.appCancelEdit();
}


function switch_to_training() {

	var backup_or_delete;
	if (Ext.getCmp('c_training_backup_backup').getValue()) {
   		mode = 'set_to_training_backup';
	} else {
		mode = 'set_to_training_delete';
	}
	
	telepath.util.request('/config/' + mode, { }, function(data) {

		Ext.getCmp("c_Production").setValue(false);
		Ext.getCmp("c_Hybrid").setValue(false); 
		Ext.getCmp("d_operation").setValue("Training"); 
		Ext.getCmp('c_training_backup').destroy();
		updateConfig();		
		
	}, 'Error while trying to ' + (mode=='switch_to_training_delete' ? '' : 'backup and') + ' delete all data.');

}


/*
 * Function: stopUpdatingRelevantFields
 * 
 * Invoked:
 * 	When the user checks the 'Production radio button on the Configuration-->Operation Mode.
 * 	The function stops refreshing the number of requests learend so far field on Configuration-->Training Level
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
function stopUpdatingRelevantFields(){
	clearInterval(c_updateInterval);
}


/*
 * Function: initConfigFields
 * 
 * Invoked:
 * 	Everytime we want to take all the configuration saved from the server
 * 	The function gets the information and loads everything to the relevant fields
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function initConfigFields(){
	
	if(!telepath.access.has_perm('Config', 'get')) { return; }
	
	var config_mask = new Ext.LoadMask(Ext.getCmp('configPanel').body, {useMsg: false});
	config_mask.show();
	
	telepath.util.request('/config/get_config', { }, function(jsondata) {

		telepath.config.data = jsondata;
		
		// Set Interface Selection Screen
		telepath.interfaces.init(jsondata.interfaces, jsondata.agents);
		
		// Set Operation Mode Screen
		var operation_mode = jsondata.operation_mode;
		switch(operation_mode) {
			case "1":
				Ext.getCmp('c_Training').suspendEvents();
				Ext.getCmp('c_Training').setValue(true);
				Ext.getCmp('c_Training').resumeEvents();
				break;
			case "2":
				Ext.getCmp('c_Production').setValue(true);
				break;
			case "3":
				Ext.getCmp('c_Hybrid').setValue(true);
				break;
		}

		Ext.getCmp('d_operation').setValue((operation_mode==1?"Training":(operation_mode==2?"Production":"Hybrid")));
		Ext.getCmp('c_tl1').setValue(jsondata.moveToProductionAfter); // Move to production setting
		
		Ext.getCmp('c_RAP11').setValue(jsondata.addUnknownApp==1 ? true:false);
		
		Ext.getCmp('update_titles').setValue(jsondata.update_titles==1 ? true:false);
		
		var exclParameters = jsondata.exclParameters;
		try {
			elements=exclParameters.split(",");
			var store=Ext.getStore('c_exParamsStore');
			store.removeAll();
			for (var i=0;i<elements.length-1;i++){
				if (elements[i])
				    store.add({'param':elements[i]});
			}
		}
		catch(err){
		}
		var sqlblacklist = jsondata.sqlblacklist;
		try {
			elements=sqlblacklist.split(",");
			store=Ext.getStore('c_SanitizedStore');
			store.removeAll();
			for (i=0;i<elements.length;i++){
				if (elements[i])
				    store.add({'exp':elements[i]});
			}
		}
		catch(err){
		}
		
		var pagesRange = jsondata.dontMapPage;
		try {
			var temp = pagesRange.split('-');
			Ext.getCmp('c_pagesRange1').setValue(temp[0]);
			Ext.getCmp('c_pagesRange2').setValue(temp[1]);
			Ext.getCmp('c_ips').setValue(jsondata.excludedIPs);
			Ext.getCmp('c_urlRegex').setValue(jsondata.urlDetectRegex);
		}
		catch(err){
		
		}
		////NETWORK///////////////////
		var load_balancer_on = jsondata.load_balancer_on;
		Ext.getCmp('c_networkCheckBox').setValue(load_balancer_on);
		a = (Ext.getCmp('c_networkCheckBox').getValue() ? Ext.getCmp('c_LbCont').enable() : Ext.getCmp('c_LbCont').disable() );
		var load_balancer_ip = jsondata.load_balancer_ip;
		try{
			elements=load_balancer_ip.split(",");
			store=Ext.getStore('c_NetworkIpsStore');
			store.removeAll();
			for (i=0;i<elements.length;i++){
    				if (elements[i])
				    store.add({'IP':elements[i]});
			}
		}
		catch(err){
		}
		var load_balancer_header = jsondata.load_balancer_header;
		store = Ext.getCmp('c_lb_headers').getStore()
		store.removeAll()
		elements = load_balancer_header.split(',')
		for (i=0;i<elements.length;i++){
			if (elements[i])
			    store.add({'header':elements[i]});
		}
		Ext.getCmp('c_NetworkProxy').setValue(jsondata.proxy_ip);
		Ext.getCmp('c_NetworkPort').setValue(jsondata.proxy_port);
		Ext.getCmp('c_net_proxy_on').setValue(jsondata.proxy_flag==1?true:false);
		Ext.getCmp('c_net_proxy_off').setValue(jsondata.proxy_flag==1?false:true);
			
		////REPORT///////////////////
		Ext.getCmp('c_reportOn').setValue((jsondata.enable_report==1?true:false));    
		Ext.getCmp('c_reportOff').setValue((jsondata.enable_report==1?false:true));

		Ext.getCmp('c_reportSMTP').setValue(jsondata.smtp);
		Ext.getCmp('c_reportSMTP_port').setValue(jsondata.smtp_port);
		Ext.getCmp('c_reportUser').setValue(jsondata.rep_user); 
		Ext.getCmp('c_reportPassword').setValue(jsondata.rep_pass); 
		Ext.getCmp('c_reportME').setValue(jsondata.maxEvents);
		Ext.getCmp('c_reportAll').setValue((jsondata.repAllEvents==1?true:false));    
		
		// Input mode
		Ext.getCmp('operation_input_mode').setValue(jsondata.input_mode ? jsondata.input_mode : 'sniffer');
		
		var report_freq = parseInt(jsondata.reportFreq)
		if (report_freq<60){
			Ext.getCmp('c_reportFreqField2').setValue("Minutes");
			Ext.getCmp('c_reportFreqField1').setValue(jsondata.reportFreq);
		}
		else if (report_freq<(60*24)){
			Ext.getCmp('c_reportFreqField1').setValue(jsondata.reportFreq/60);
			Ext.getCmp('c_reportFreqField2').setValue("Hours");
		}
		else{
			Ext.getCmp('c_reportFreqField1').setValue(jsondata.reportFreq/(24*60));
			Ext.getCmp('c_reportFreqField2').setValue("Days");			
		}

		var report_window = parseInt(jsondata.reportWindow)
		if (report_window<60){
			Ext.getCmp('c_reportTBField2').setValue("Minutes");
			Ext.getCmp('c_reportTBField1').setValue(jsondata.reportWindow);
		}
		else if (report_window<(60*24)){
			Ext.getCmp('c_reportTBField1').setValue(jsondata.reportWindow/60);
			Ext.getCmp('c_reportTBField2').setValue("Hours");
		}
		else{
			Ext.getCmp('c_reportTBField1').setValue(jsondata.reportWindow/(24*60));
			Ext.getCmp('c_reportTBField2').setValue("Days");			
		}
		var groups =[];	
		if (jsondata.rule_groups_report!="")
			Ext.getCmp('c_reportGroupCB').setValue(true)				
		else 
			Ext.getCmp('c_reportGroupCB').setValue(false)
		try {
			var idArray = jsondata.rule_groups_report.split(",");

			Ext.Object.each(idArray,function(key,value){
				Ext.Object.each(jsondata.rule_groups_names,function(key1,value1){
					if (parseInt(value)==value1[1]){
						groups.push(value1[0]);
						return;
					}
				});
			});
			Ext.getCmp('c_reportGroupField').setValue(groups);
		}
		catch(err){
		}
		//alerts and logs
		(jsondata.write_to_syslog==1 ? Ext.getCmp("c_syslogOn").setValue(true):Ext.getCmp("c_syslogOff").setValue(true))
 		Ext.getCmp("c_syslogAddress").setValue(jsondata.remote_syslog_ip);

		// INIT USER AGENTS
		try {
		
			var headers = jsondata.regex.headers;
			var URL 	= jsondata.regex.URL;
			
			i=0;

			while( i<headers.length & !(headers.charAt(i)=='(' & headers.charAt(i+1)=='(') ){
				i++;
			}	
			headers=headers.substring(i+2,headers.length-2);
			var elements=headers.split(")|(");
			var store=Ext.getStore("c_NetworkUAsStore");
			store.removeAll();
			for(i = 0;i<elements.length;i++){
				store.add({UA:elements[i]});
			}
			
			URL=URL.substring(3,URL.length-1);
			elements=URL.split(")|(\\.");
			store = Ext.getStore("c_NetworkExtsStore");
			store.removeAll();
			for(i = 0;i<elements.length;i++){
				store.add({ext:elements[i]});
			}
			
		}
		catch(err){
			console.log('Failed parsing agents regex response');
		}
		
		config_mask.destroy();
		
		
		
	}, 'Error while trying to initialize configuration fields.');

	Ext.getCmp('c_net_ip_whitelist').getStore().load();

}

function toggleReports(component) {
	
	if(Ext.getCmp('c_reportOn').el) {
	
		if(Ext.getCmp('c_reportOn').value) {
			Ext.getCmp('reports_config').enable();
		} else {
			Ext.getCmp('reports_config').disable();
		}
		
	}
		
}

///////////////////////////////////////////////////////////Operation Mode///////////////////////////////////////////////
/*
 * Function: updateConfig
 * 
 * Invoked:
 * 	When the user clicks the 'Apply' button on the configuration panel.
 * 	The function takes all the information from the fields and saves it on the database
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
 
function updateConfig(){


	// operation mode
	var operation  = (Ext.getCmp("c_Training").getValue()? "1" : (Ext.getCmp("c_Production").getValue()? "2" : (Ext.getCmp("c_Hybrid").getValue()? "3":console.log("error while getting operation mode"))));

	// Advanced Settings
	var addUnknownApp = (Ext.getCmp("c_RAP11").getValue() ? 1 :0);
	
	var excludeGlobal ="";
	Ext.getStore('c_exHeadersStore').each(function(record){
	    excludeGlobal+=record.get("header")+",";
	},this);
	var exclParameters ="";
	Ext.getStore('c_exParamsStore').each(function(record){
	    exclParameters+=record.get("param")+",";
	},this);
	
	var sqlblacklist="";
	Ext.getStore('c_SanitizedStore').each(function(record){
	    sqlblacklist+=record.get('exp')+",";
	},this);
	
	//var dontMapPage = Ext.getCmp("c_pagesRange1").getValue()+"-"+Ext.getCmp("c_pagesRange2").getValue();
	//var excludedIPs = Ext.getCmp("c_ips").getValue();
	//var urlDetectRegex = Ext.getCmp("c_urlRegex").getValue();

	var lb_ip="";
	
	var moveToProductionAfter = Ext.getCmp('c_tl1').getValue();
	
	//Network
	Ext.getCmp('c_NetworkIps').getStore().each(function(record){
	    lb_ip+=record.get("IP")+",";
	},this);
	var lb_header="";
	Ext.getCmp('c_lb_headers').getStore().each(function(record){
	    lb_header+=record.get("header")+",";
	},this);
	var useBalancer = (Ext.getCmp("c_networkCheckBox").getValue() ? 1 :0);

	var UserAgentarr=[];
	var UserAgentliststring="";
	Ext.getCmp('c_NetworkUA').getStore().each(function(record){
		if(record.get('UA') != '') {
			UserAgentarr.push(record.get('UA'));
		}
	},this);
	
	if(UserAgentarr.length > 0) {
		UserAgentliststring=UserAgentarr.join(")|(");
		UserAgentliststring="(User-Agent:)(.*)(("+UserAgentliststring+"))";
	}

	var lb_UserAgent = UserAgentliststring;

	var Extensionarr=[];
	var Extensionliststring="";
	Ext.getCmp('c_NetworkExts').getStore().each(function(record){
		if(record.get("ext") != '') {
			Extensionarr.push(record.get("ext"));
		}
	},this);
	
	if(Extensionarr.length > 0) {
		Extensionliststring=Extensionarr.join(")|(\\.");
		Extensionliststring="(\\."+Extensionliststring+")";
	}

	var lb_Extension = Extensionliststring;
	var proxy_flag = (Ext.getCmp('c_net_proxy_on').getValue()?1:0);
	if (Ext.getCmp('c_NetworkProxy').isValid()){
		var proxy_ip = Ext.getCmp('c_NetworkProxy').getValue();
	}
	else{
		proxy_ip = "";
		Ext.getCmp('c_NetworkProxy').clearInvalid();
	}
	if (Ext.getCmp('c_NetworkPort').isValid()){
		var proxy_port = Ext.getCmp('c_NetworkPort').getValue();
	}
	else{
		proxy_port = "";
		Ext.getCmp('c_NetworkPort').clearInvalid();
	}
	
	//whitelist ip
	var ip_wls=""
	var ip_wl;
	Ext.getCmp('c_net_ip_whitelist').getStore().each(function(record){
		ip_wl = record.get('td0')
		if (ip_wl){
			ip_wls+=ip_wl+","
		}
	})
	if (ip_wls)
		ip_wls = ip_wls.substring(0,ip_wls.length-1)
		
	//whitelist ip
	var ip_ils=""
	var ip_il;
	Ext.getCmp('c_net_ip_ignorelist').getStore().each(function(record){
		ip_il = record.get('td0')
		if (ip_il){
			ip_ils+=ip_il+","
		}
	})
	if (ip_ils)
		ip_ils = ip_ils.substring(0,ip_ils.length-1)	
		
	//REPORTS

	var smtp =   Ext.getCmp("c_reportSMTP").getValue();
	var smtp_port =   Ext.getCmp("c_reportSMTP_port").getValue();
	var rep_user =  Ext.getCmp("c_reportUser").getValue();
	var rep_pass =  Ext.getCmp("c_reportPassword").getValue();
	var reportFreq = Ext.getCmp("c_reportFreqField1").getValue();

	var freq = Ext.getCmp("c_reportFreqField2").getValue();
	if (freq=="Hours")
		reportFreq = reportFreq*60;
	if (freq=="Days")
		reportFreq = reportFreq*60*24;
	var maxEvents =  Ext.getCmp("c_reportME").getValue();      
	var reportWindow = Ext.getCmp("c_reportTBField1").getValue();
	freq = Ext.getCmp("c_reportTBField2").getValue();
	if (freq=="Hours")
		reportWindow = reportWindow*60;
	if (freq=="Days")
		reportWindow = reportWindow*60*24;
	var repAllEvents = (Ext.getCmp("c_reportAll").getValue()?1:0);

	var enableReport = (Ext.getCmp("c_reportOn").getValue()?1:0);

	var groupsArray =[];
	var tmp = "";
	var reportGroup = (Ext.getCmp("c_reportGroupCB").getValue()?1:0);
	if(reportGroup){
	    groupsArray = Ext.getCmp('c_reportGroupField').getValue();
	    Ext.Object.each(groupsArray,function(key,value){
		Ext.getCmp('c_reportGroupField').getStore().each(function(record){
		    if (value==record.get('group')){
		        tmp += record.get('id')+",";
		    }
		});
	    });
	}
	var reportGroups = tmp;
	
	//SYSLOG AND ALERTS
	
	if(!telepath.interfaces.validate()) {
		return;
	}
	
	var write_to_syslog = (Ext.getCmp("c_syslogOn").getValue()?1:0);
	var syslog_server_address = Ext.getCmp("c_syslogAddress").getValue();
	
	var update_titles = (Ext.getCmp("update_titles").getValue()?1:0);
	
	var operation_input_mode = Ext.getCmp('operation_input_mode').getValue();
	
	var params = {
	
		//========================= Operation Mode   =======
		op_mode: operation,
		input_mode: operation_input_mode,
		//========================= Training Level   =======  
		moveToProductionAfter:moveToProductionAfter,
		//========================= Traffic Analysis   =======
		excludeGlobal:excludeGlobal,
		addUnknownApp:addUnknownApp,
		exclParameters:exclParameters,
		sqlblacklist:sqlblacklist,
		//========================= Network   =======
		useLoadBalancer: useBalancer,
		loadBalancerIP: lb_ip,
		loadBalanceHead: lb_header,
		UserAgent: lb_UserAgent,
		Extension: lb_Extension,
		proxy_ip: proxy_ip,
		proxy_port: proxy_port,
		proxy_flag: proxy_flag,
		ip_whitelist: ip_wls,
		ip_ignorelist: ip_ils,
		//========================= Reports =====
		rep_user:rep_user,
		rep_pass:rep_pass,
		maxEvents:maxEvents,
		smtp:smtp,
		smtp_port:smtp_port,
		repAllEvents:repAllEvents,
		reportWindow:reportWindow,
		reportFreq:reportFreq,
		enableReport:enableReport,
		reportGroups:reportGroups,
		reportGroup:reportGroup,
		//========================= Logs & Alerts   =======
		syslog_server_address:syslog_server_address,
		write_to_syslog:write_to_syslog,
		//========================= Network Interfaces =======
		agents: telepath.interfaces.get(),
		update_titles: update_titles
		
	};
	
	telepath.util.request('/config/set_config', params, function(data) {

		telepath.util.blinkSaved();
		initConfigFields();		
		
	}, 'Error while trying to save the new configuration.');

}