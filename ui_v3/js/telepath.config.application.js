telepath.config.application = {
	validate: function() {
		
		var that = this;
		
		// Start collecting information for data save
		// Validate user input, if errors - alerts + prevent form submission
		
		var app_data = {};
		
		/* APPLICATION DETAILS */
		/* *************************************************************** */
		$('#tele-app-details').click();
		
		// APP DOMAIN
		app_data.host = $('input', this.AD_app_domain).val();
		$('input', this.AD_app_domain).css({ borderColor: '#555' });
		if(app_data.host.length > 256) {
			telepath.dialog({ type: 'alert', title: 'Application Settings', msg: 'Application host too long' });
			$('input', this.AD_app_domain).css({ borderColor: 'red' });
			return false;
		}
		if(app_data.host == '') {
			telepath.dialog({ type: 'alert', title: 'Application Settings', msg: 'Application host cant be empty' });
			$('input', this.AD_app_domain).css({ borderColor: 'red' });
			return false;
		}


		// Added automatically toggle
		app_data.top_level_domain = this.TopLevelDomainAppToggle.data('tele-toggleFlip').options.flipped ? 1 : 0;



		//Operation Mode ID

		var selected_opmod = this.opmod.data('tele-teleRadios').options.checked;
		switch(selected_opmod) {
			case 'training':   app_data.operation_mode = 1; break;
			case 'production':     app_data.operation_mode = 2; break;
			case 'hybrid':     app_data.operation_mode = 3; break;

		}

		app_data.move_to_production = $('input', this.move_to_production).val();
		
		// DISPLAY NAME
		app_data.display_name = $('input', this.AD_display_name).val();
		$('input', this.AD_display_name).css({ borderColor: '#555' });
		if(app_data.display_name.length > 64) {
			$('input', this.AD_display_name).css({ borderColor: 'red' });
			telepath.dialog({ type: 'alert', title: 'Application Settings', msg: 'Application display name too long' });
			return false;
		}
		
		// COOKIES
		app_data.AppCookieName = [];
		// Collect
		$('input', this.app_cookies).each(function () {
			if($(this).val() != '') {
				app_data.AppCookieName.push($(this).val());
			}
		});
		// De-Dupe
		app_data.AppCookieName = app_data.AppCookieName.filter(function(elem, pos) {
			return app_data.AppCookieName.indexOf(elem) == pos;
		});
		// Join
		app_data.AppCookieName = app_data.AppCookieName.join(',');
		
		// IPS
		app_data.app_ips = [];
		$('input', this.app_ips).each(function () {
			//var ip = $(this).data('tele-ip').getIP();
			if($(this).val() != '') {
				app_data.app_ips.push($(this).val());
			}
		});
		// De-Dupe
		app_data.app_ips = app_data.app_ips.filter(function(elem, pos) {
			return app_data.app_ips.indexOf(elem) == pos;
		});
		// Join
		app_data.app_ips = app_data.app_ips.join(',');
		
		/* AUTHENTICATION */
		/* *************************************************************** */
		$('#tele-app-auth').click();
		
		app_data.ntlm        = 0;
		app_data.basic_flag  = 0;
		app_data.form_flag   = 0;
		app_data.digest_flag = 0;
		
		var mode = this.userIdentification.data('teleTeleRadios').options.checked;
		
		switch(mode) {
			case 'NTLM':
				app_data.ntlm = 1;
			break;
			case 'Basic':
				app_data.basic_flag = 1;
			break;
			case 'Form':
				app_data.form_flag = 1;
				
				// Username parameter is required here
				app_data.form_param_name = this.usernameParameter.teleBrowse('option', 'text');

				// unknown
				//app_data.form_param_id   = this.usernameParameter.teleBrowse('option', 'id');

				$('input', this.usernameParameter).css({ borderColor: '#555' });
				if(!app_data.form_param_name || app_data.form_param_name == '' || app_data.form_param_name.length > 64 || parseInt(app_data.form_param_id) == 0) {
					$('input', this.usernameParameter).css({ borderColor: 'red' });
					telepath.dialog({ type: 'alert', title: 'Application Settings', msg: 'Application authentication username parameter is missing.' });
					return false;
				}
				
				//console.log(app_data);
				
			break;
			case 'Digest':
				app_data.digest_flag = 1;
			break;
			//default:
			//	// By default automatic mode is selected
			//break;
		}

		app_data.ssl_flag = this.app_ssl_toggle.data('teleTeleRadios').options.checked == "On" ? 1 : 0;
		app_data.app_ssl_certificate = $('input', this.app_ssl_certificate).data('file');
		app_data.app_ssl_private = $('input', this.app_ssl_private).data('file');

		// Success Criteria -- Toggle
		//app_data.form_authentication_flag = this.SC_toggle.teleRadios('option', 'checked') == 'On' ? 1 : 0;

		// Success Criteria -- Cookie
		app_data.cookie_mode = this.SC_cookie_toggle.data('teleTeleRadios').options.checked == "On" ? 1 : 0;
		app_data.cookie_name = $('input', this.SC_cookie_name).val();
		app_data.cookie_value = $('input', this.SC_cookie_value).val();
		app_data.cookie_value_appearance = this.SC_cookie_flag.data('teleTeleRadios').options.checked == 'appears' ? 1 : 0;

		// Success Criteria -- Redirect
		//app_data.redirect_mode = this.SC_redirect_toggle.data('teleTeleRadios').options.checked == "On" ? 1 : 0;
		//app_data.redirect_page = this.SC_redirect_browse.teleBrowse('option', 'filename');
		////app_data.form_authentication_redirect_page_id   	 = this.SC_redirect_browse.teleBrowse('option', 'id');
		//app_data.redirect_status_code = this.SC_redirect_range.data('tele-teleInput').input.val();

		// Success Criteria -- Body Value
		app_data.body_value_mode = this.SC_body_toggle.data('teleTeleRadios').options.checked == "On" ? 1 : 0;
		app_data.body_value_html = this.SC_body_input.data('tele-teleInput').input.val();

		if(that.app_id=='new'){
			app_data.learning_so_far='0';
			app_data.subdomains='';
			app_data.eta='1d 0h 0m';
		}


		telepath.ds.get('/applications/set_app', app_data, function (data) {
			telepath.config.applications.reload();
			//that.editApp(app_data.host);
		});
		
	},
	createApp: function() {

		this.editApp('new');
	},


	editApp: function(app_id, $nodeParent) {
		
		var that = this;

		this.nodeParent=$nodeParent;
		// Containers
		this.container = telepath.config.applications.contentRight;
		this.toolbar   = telepath.config.applications.barRight;
		
		// Cleanup
		this.container.empty();
		this.toolbar.empty();
		
		// Tab Containers
		this.tabsEl   = $('<div>').addClass('tabs');
		this.tabsUl   = $('<ul>');
		this.tabsEl.append(this.tabsUl);
		
		// Tab Declaration
		var tabs = [
			{ id: 'details', text: 'Application Details' },
			// remove Pages and Parameters for now (Yuli)
			//{ id: 'params', text: 'Pages and Parameters' },
			{ id: 'auth', text: 'Authentication' },
			{ id: 'ssl', text: 'SSL' },
			// remove advanced field for now, Yuli
			//{ id: 'advanced', text: 'Advanced' }
		];
		
		// Tab Print
		for(x in tabs) {
			var tab = tabs[x];
			var tabEl = $('<div>').attr('id', 'tele-app-' + tab.id);
			var tabLi = $('<li>');
			var tabA  = $('<a>').attr('href', '#tele-app-' + tab.id).text(tab.text);
			tabLi.append(tabA);
			this.tabsUl.append(tabLi);
			this.tabsEl.append(tabEl);
			
			if(app_id == 'new' && tab.id != 'details')  {
				//tabLi.hide();
			}
			
		}
		
		// Buttons
		
		this.buttonsEl = $('<div>').addClass('tele-form-buttons');
		this.applyBtn  = $('<a href="#" class="tele-button tele-button-apply">Apply</a>');
		this.cancelBtn = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');
		
		this.buttonsEl.append(this.applyBtn).append(this.cancelBtn);
		this.container.append(this.buttonsEl);
		
		// BIND Validate
		this.applyBtn.click(function () { 
			that.validate(that);
		});
		
		// BIND Cancel -- Simply reload
		this.cancelBtn.click(function () {
			that.editApp(that.app_id);
		});
		
		// Append and init Tabs
		this.container.append(this.tabsEl);
		this.container.append(this.buttonsEl);
		this.tabsEl.tabs({ 
		
			heightStyle: 'fill',
			autoHeight: false,
			animate: false,
			activate: function( event, ui ) {
				if(ui.newPanel.selector == "#tele-app-params" /* && ui.newPanel.text() == '' */) {
					that.showPagesParams();
				}
			}
			
		});
		
		this.toolbar.append(this.tabsUl);

                this.app_id = app_id;
		
                // New Application Template
                this.app_data = {
                                app_domain: '',
                                display_name: '',
                                // Add default cookies for new app
								// Not needed. We get it from server side (Moshe)
                                //cookie_suggestion: 'PHPSESSID,PHPSESSIONID,JSESSIONID,ASPSESSIONID,ASP.NET_SessionId,VisitorID,SESS',
								//ip_suggestion: '',
                                app_ips: '',
                                form_param_name: '',
                                form_param_id: '',
                                ntlm: 0,
                                basic_flag: 0,
                                form_flag: 0,
                                digest_flag: 0,
                                cookie_mode: 0,
                                cookie_name: '',
                                cookie_value: '',
                                ssl_flag: 0,
                                AppCookieName: '',
                                //redirect_status_code: '',
								operation_mode: '1',
								eta: '1d 0h 0m',
								top_level_domain: '1'
		};
	
		if(app_id == 'new') {
			this.showApp();
		} else {
			// Load Application
			this.loadApp(app_id);

		}
		
	},
	loadApp: function (app_host) {
		
		var that = this;
		this.app_data.app_domain = app_host;
		this.app_data.host = app_host;
		//var ip_suggestions_str = '';


		// will go inside if this host is really an application	
		telepath.dsync.get('/applications/get_app', { host: app_host }, function(data) {

			if (data.items && data.items[0]) {
				that.app_data = data.items[0];

				//if (!data.app_data.cookie_suggestions)

				// Load cookie suggestions from the backend (Yuli)
				/*	telepath.dsync.get('/applications/get_cookie_suggestion', {app_id: app_host}, function (data) {
				 cookie_suggestions_str = '';
				 if (data.items && data.items[0]) {
				 for (c in data.items) {
				 if (cookie_suggestions_str != '')
				 cookie_suggestions_str = cookie_suggestions_str + ',';
				 cookie_suggestions_str = cookie_suggestions_str + data.items[c].cookie;
				 }
				 }
				 console.log(cookie_suggestions_str);
				 that.app_data.cookie_suggestion = cookie_suggestions_str;
				 });
				 }*/
				//if (ip_suggestions_str) {
				//	that.app_data.ip_suggestion = ip_suggestions_str;
				//}
			}
		});

		//telepath.dsync.get('/applications/get_ip_suggestion', { app_id: app_host }, function(data) {
		//	//ip_suggestions_str = '';
		//	//if (data.items && data.items[0]) {
		//	//        for (c in data.items)
		//	//        {
		//	//                if (ip_suggestions_str != '')
		//	//                        ip_suggestions_str = ip_suggestions_str + ',';
		//	//                ip_suggestions_str = ip_suggestions_str + data.items[c].ip;
		//	//        }
		//	//}
		//	console.log(data);
		//	if (data.items)
		//		that.app_data.ip_suggestion = data.items;
		//});
		// show app edit dialog in any case (i.e. app does not exists) (Yuli)
		that.showApp();
		
	},
	showPagesParams: function() {
		$("#tele-app-params").html('');
		this.teleBrowser = $('<div>').teleBrowser({ root: { type: 'application', id: this.app_id } });
		this.teleBrowserTitle = $('<div>').addClass('tele-title-1').html('Pages and Parameters List');		
		$("#tele-app-params").append(this.teleBrowserTitle).append(this.teleBrowser);
		
	},
	showApp: function() {
		
		var that = this;
		
		// Application Details
		
		var title = $('<div>').addClass('tele-title-1').html('Application Details').appendTo('#tele-app-details');
		
		this.AD_app_domain   = $('<div>').teleInput({ label: 'Application Host', value: that.app_data.host });
		this.AD_display_name = $('<div>').teleInput({ label: 'Display Name', value: that.app_data.display_name });
		
		$('#tele-app-details').append(this.AD_app_domain).append(this.AD_display_name);


		this.c_mode = $('<div>').addClass('tele-config-system-tab tele-config-system-mode');
		this.container.append(this.c_mode);

		$('<div>').addClass('tele-title-1 ').html('Top Level Domain').appendTo('#tele-app-details');
		this.TopLevelDomainAppToggle = $('<div>').toggleFlip({ left_value: 'Off', right_value: 'On', flipped: that.app_data.top_level_domain=='1' }).addClass('tele-TopLevelDomainApp-toggle').appendTo('#tele-app-details');

		$('<div>').addClass('tele-title-1').html('Operation Mode').appendTo('#tele-app-details');

		var selected_opmod = '';
		switch(that.app_data.operation_mode) {
			case '1':	selected_opmod = 'training';   break;
			case '2':	selected_opmod = 'production';	   break;
			case '3':	selected_opmod = 'hybrid';	   break;
		}

		this.app_operation=$('<div>').appendTo('#tele-app-details').css({'width': '300px'});

		this.eta = $('<div>').html('ETA: ' + that.app_data.eta);

		this.opmod = $('<div>').teleRadios({
			checked: selected_opmod,
			radios: [
				{ key: 'training',   label: 'Training' },
				{ key: 'hybrid',   label: 'Hybrid' },
				{ key: 'production', label: 'Production'}

			],callback: function(radio){
				if(radio.key == 'training') {
					that.eta.show();
				} else {
					that.eta.hide();
				}
			}}).addClass('tele-config-opmod').appendTo(this.app_operation);

		this.eta.appendTo(this.app_operation).css({float: 'left',clear: 'both','margin-top': '5px'});



		// MV2Prod after
		this.move_to_production = $('<div>').teleInput({
			label: 'Move to production per application after',
			suffix: 'requests',
			width: 70,
			value: that.app_data.move_to_production
		}).addClass('tele-config-mv2prod').appendTo('#tele-app-details');

		/*
		this.app_details = $('<div>').teleForm({
			title: 'Application Details',
			data:  this.app_data,
			items: [
				{ type: 'text', label: 'Application Domain', dataIndex: 'app_domain' },
				{ type: 'text', label: 'Display Name'	   , dataIndex: 'display_name' },
			]
		}).appendTo('#tele-app-details');
		*/
		
		// APP DETAILS -- Cookies
		
		var cookie_val = that.app_data.AppCookieName.split(',');
		
		this.app_cookies = $('<div>').teleMulti({ values: cookie_val, title: 'Application Cookies', template: function(element, value) {
			
			element.teleInput({ value: value });
			
			$('.tele-input-input', element).autocomplete({ 
				autoFill: true,
				source: that.app_data.cookie_suggestion,
				minLength: 0
			}).focus(function () {
				$(this).autocomplete('search', $(this).val());
			});
			
		} }).appendTo('#tele-app-details');

		// APP DETAILS -- IPS
		
		var ips_val = that.app_data.app_ips.split(',');
		/*
		this.app_ips     = $('<div>').teleMulti({ values: ips_val, title: 'Application IP Addresses', template: function(element, value) {
			element.ip({ data: value });
		} }).appendTo('#tele-app-details');
		*/
		this.app_ips     = $('<div>').teleMulti({ values: ips_val, title: 'Application IP Addresses', template: function(element, value) {
			element.teleInput({ value: value });
			$('.tele-input-input', element).autocomplete({
				autoFill: true,
				source: that.app_data.ip_suggestion,
				minLength: 0
			}).focus(function () {
				$(this).autocomplete('search', $(this).val());
			}).keydown(function(e) {

				var key = e.charCode || e.keyCode || 0;
				if (key >= 96 && key <= 105)
				{
					key = key - 48;
				}
				if(e.ctrlKey && key == 86)
					return;
				// Arrows, HOME, END, Numbers, Keypad Numbers
                                if(key == 8 || key == 9 || key == 46 || key == 190 ||
                                   (key >= 35 && key <= 40) ||
                                   (key >= 48 && key <= 57) ||
                                   (key >= 96 && key <= 105)) {

                                } else {
                                    // Stop Event
                                    return false;
                                }

			});
		} }).appendTo('#tele-app-details');

		// AUTHENTICATION
		// AUTHENTICATION -- User Identification
		
		this.usernameParameter = $('<div>')
			.teleBrowse({ 
				label: 'Username Parameter', 
				value: that.app_data.form_param_name, 
				id: that.app_data.host,
				mode: 'param'
			})
			.appendTo('#tele-app-auth').hide()
			.css({ position: 'absolute', top: 100, left: 300 });


		var userID_val='';
		if(telepath.config.application.app_data.form_flag == '1') {
			userID_val = 'Form';
			this.usernameParameter.show();
		}
		if(telepath.config.application.app_data.ntlm == '1') {
			userID_val = 'NTLM';
		}
		if(telepath.config.application.app_data.basic_flag == '1') {
			userID_val = 'Basic';
		}
		if(telepath.config.application.app_data.digest_flag == '1') {
			userID_val = 'Digest';
		}



		this.userIdentification = $('<div>').teleRadios({
			title: 'Authentication',
			checked: userID_val,
			radios: [ 
				//{ key: 'Automatic', label: 'Automatic' },
				{ key: 'Form', label: 'Form' },
				{ key: 'Basic', label: 'Basic' },
				{ key: 'Digest', label: 'Digest' },
				{ key: 'NTLM', label: 'NTLM' },
				{ key: '', label: 'None' }
			], callback: function(radio) {
				
				that.usernameParameter.hide();
				
				switch(radio.key) {
					case 'Form':
						that.usernameParameter.show();
					break;
				}
				
		}}).css({ 'float': 'left', clear: 'both', width: 300, inputFirst: true }).appendTo('#tele-app-auth');
		
		// AUTHENTICATION -- Success Criteria
		
		var SC_wrap = $('<div>');		
		
		this.SC_cookie_name   = $('<div>').teleInput({ label: 'Name', value: that.app_data.cookie_name });
		this.SC_cookie_value  = $('<div>').teleInput({ label: 'Value', value: that.app_data.cookie_value });
		this.SC_cookie_flag_val = that.app_data.cookie_value_appearance == '1' ? 'appears' : 'missing';
		this.SC_cookie_flag   = $('<div>').teleRadios({
			checked: this.SC_cookie_flag_val,
			radios: [ 
				{ key: 'missing', label: 'Value is missing' }, 
				{ key: 'appears', label: 'Value appears' }
			], callback: function(radio) {
			
				switch(radio.key) {

				}
				
		}});

		// SC_cookie
		// this.SC_cookie_toggle = $('<div>').teleCheckbox({ inputFirst: true, label: 'Cookie', checked: that.app_data.cookie_mode == '1' });
		this.SC_cookie_toggle = $('<div>').teleRadios({
			title: 'Cookie',
			checked: that.app_data.cookie_mode == '1' ? 'On' : 'Off',
			radios: [
				{ key: 'On', label: 'On' },
				{ key: 'Off', label: 'Off' }
			], callback: function(radio) {
				that.SC_cookie_name.hide();
				that.SC_cookie_value.hide();
				that.SC_cookie_flag.hide();
				switch(radio.key) {
					case 'On':
						that.SC_cookie_name.show();
						that.SC_cookie_value.show();
						that.SC_cookie_flag.show();
						break;


				}

			}}).css({ clear: 'both', 'float': 'left' }).addClass('tele-radio-on-off').appendTo('#tele-app-auth');


		$('.tele-input-input', this.SC_cookie_name).autocomplete({ 
			autoFill: true,
			source: that.app_data.cookie_suggestion,
			minLength: 0
		}).focus(function () {
			$(this).autocomplete('search', $(this).val());
		});
		
		SC_wrap.append('<div class="tele-form-hr">');
		
		SC_wrap.append(this.SC_cookie_toggle).append(this.SC_cookie_name).append(this.SC_cookie_value).append(this.SC_cookie_flag);

		SC_wrap.append('<div class="tele-form-hr">');
		
		// SC_redirect
				
		//this.SC_redirect_browse = $('<div>').teleBrowse({
		//	label: 'Page', value: that.app_data.redirect_page, id: that.app_data.host, type: 'page' });
		//this.SC_redirect_range  = $('<div>').teleInput({ label: 'Response status',
		//	value: that.app_data.redirect_status_code , type:'number' ,range:{min:200, max:600} });
        //
		////this.SC_redirect_toggle = $('<div>').teleCheckbox({ inputFirst: true, label: 'Redirect', checked: that.app_data.redirect_mode == '1' });
		//this.SC_redirect_toggle = $('<div>').teleRadios({
		//	title: 'Redirect',
		//	checked: that.app_data.redirect_mode == '1' ? 'On' : 'Off',
		//	radios: [
		//		{ key: 'On', label: 'On' },
		//		{ key: 'Off', label: 'Off' },
		//	], callback: function(radio) {
        //
		//		that.SC_redirect_browse.hide();
		//		that.SC_redirect_range.hide();
		//		 switch(radio.key) {
		//		 case 'On':
		//			 that.SC_redirect_browse.show();
		//			 that.SC_redirect_range.show();
		//		 break;
		//		 }
        //
		//	}}).css({ clear: 'both', 'float': 'left' }).addClass('tele-radio-on-off').appendTo('#tele-app-auth');
        //
        //
		//SC_wrap.append(this.SC_redirect_toggle).append(this.SC_redirect_browse).append(this.SC_redirect_range);
		//
		//SC_wrap.append('<div class="tele-form-hr">');
		
		// SC_body
		

		
		this.SC_body_input  = $('<div>').teleInput({ label: 'Value to search in HTML body', value: that.app_data.body_value_html }).addClass('tele-app-auth-body');

		//SC_body_toggle = $('<div>').teleCheckbox({ inputFirst: true, label: 'Body Value', checked: that.app_data.body_value_mode == '1' });
		this.SC_body_toggle = $('<div>').teleRadios({
			title: 'Body Value',
			checked: that.app_data.body_value_mode == '1'? 'On' : 'Off',

			radios: [
				{ key: 'On', label: 'On' },
				{ key: 'Off', label: 'Off' },
			], callback: function(radio) {

				that.SC_body_input.hide();

				 switch(radio.key) {
				 case 'On':
					 that.SC_body_input.show();
				 break;
				 }

			}}).css({ clear: 'both', 'float': 'left' }).addClass('tele-radio-on-off').appendTo('#tele-app-auth');

		SC_wrap.append(this.SC_body_toggle).append(this.SC_body_input);
		
	/*	var SC_toggle_val = that.app_data.form_authentication_flag == '1' ? 'On' : 'Off';
		
		//var SC_title = $('<div>').css({ marginTop: 55, marginBottom: 0 }).addClass('tele-title-1').html('Success Criteria').appendTo('#tele-app-auth');
		
		this.SC_toggle = $('<div>').teleRadios({
			title: 'Success Criteria',
			checked: SC_toggle_val,
			radios: [ 
				{ key: 'On', label: 'On' }, 
				{ key: 'Off', label: 'Off' },
			], callback: function(radio) {
				
				SC_wrap.hide();
				
				switch(radio.key) {
					case 'On':
						SC_wrap.show();
					break;
				}
				
		}}).css({ clear: 'both', 'float': 'left' }).addClass('tele-radio-on-off').appendTo('#tele-app-auth');*/
		SC_wrap.appendTo('#tele-app-auth');
		
		// SSL
		var app_ssl_wrap   = $('<div>').addClass('tele-app-ssl-wrap');
		var app_ssl_toggle_val = that.app_data.ssl_flag == '1' ? 'On' : 'Off';
		this.app_ssl_toggle = $('<div>').teleRadios({
			title: 'SSL',
			checked: app_ssl_toggle_val,
			radios: [ 
				{ key: 'On', label: 'On' }, 
				{ key: 'Off', label: 'Off' },
			], callback: function(radio) {
				
				app_ssl_wrap.hide();
				
				switch(radio.key) {
					case 'On':
						app_ssl_wrap.show();
					break;
				}
				
		}}).appendTo('#tele-app-ssl');
		
		var got_cert = that.app_data.ssl_data && that.app_data.ssl_data.subject;
		
		this.app_ssl_certificate = $('<div>').teleFile({ label: 'Certificate', value: got_cert ? 'Uploaded' : '' , type: 'certificate' }).appendTo(app_ssl_wrap);
		this.app_ssl_private     = $('<div>').teleFile({ label: 'Private Key', value: got_cert ? 'Uploaded' : '' , type: 'privatekey'  }).appendTo(app_ssl_wrap);
		this.app_ssl_port        = $('<div>').teleInput({ label: 'Server Port', value: that.app_data.ssl_server_port }).appendTo(app_ssl_wrap);
		// Make sure we don't have the password here from PHP

		this.app_ssl_password    = $('<div>').teleInput({ label: 'Certificate Password', value: '', pass: true }).appendTo(app_ssl_wrap);
		
		$('#tele-app-ssl').append('<div class="tele-form-hr">');
		app_ssl_wrap.appendTo('#tele-app-ssl');
		$(app_ssl_wrap).append('<div class="tele-form-hr">');

		if(that.app_data.ssl_data && that.app_data.ssl_data.subject) {
			$.each(that.app_data.ssl_data.subject, function(key,val) {
				$('<div style="clear:both;">').html(key + ' = ' + val).appendTo(app_ssl_wrap);
			});
		}
		if (that.nodeParent) {
			that.nodeParent.parent().find(".jstree-wholerow").css('background-color', '#FFFFFF');
			that.nodeParent.find('.jstree-wholerow').css("background-color", "rgba(189, 189, 189, 0.85)");

		}

	},
	deleteApp: function(app_id, $nodeParent) {
		
		context_confirm('Delete Application', 'Are you sure you want to delete this application?', function () {

			telepath.ds.get('/applications/del_app', { app_id: app_id }, function(data) {
				if(data.success){
					$nodeParent.remove();
				}
				//telepath.config.applications.reload();
			}, 'Error deleting application');
			
		});
		
	},
	initToolBar: function() {
				
	}
}
