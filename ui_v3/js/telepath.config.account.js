telepath.config.account = {

	data: [],
	init: function (id, container) {
		
		this.container = container;
		if(id != 'new') {
			this.loadData(id);
		}
		
	},
	loadData: function(id) {
		
		var that = this;
		telepath.ds.get('/users/get_user', { id: id }, function (data) {
			that.showData(data.items);
		});
	
	},
	showData: function(data) {
		
		var that = this;
		
		this.data = data;
		// Create List
		this.form = $('<div>');
		
		this.form.teleForm({
			title: 'User Editor',
			data: data.user,
			items: [
				{ type: 'checkbox', label: 'Enabled', dataIndex: 'active' },
				{ type: 'text', label: 'Login', dataIndex: 'login' },
				{ type: 'text', label: 'Email', dataIndex: 'email' },
				{ type: 'text', label: 'First Name', dataIndex: 'first_name' },
				{ type: 'text', label: 'Last Name', dataIndex: 'last_name' },
				{ type: 'text', label: 'Company', dataIndex: 'company' },
				{ type: 'text', label: 'Phone', dataIndex: 'phone' },
				{ type: 'password', label: 'Password', dataIndex: 'password' }
			],
			callback: function () {
				
				
			}
		});
		
		this.container.empty().append(this.form);


		$(".tele-button-cancel", this.container).click(function () {
			telepath.config.accounts.getUsers();
		});

		$(".tele-button-apply", this.container).click(function () {
		
			// console.log('Saving user');
				
			var userData = { user: {}, perm: [], groups: [], apps: [], ranges: [] };
			
			// Reset errors
			$("input", that.container).css({ borderColor: '#999' });
			
			// Is active
			userData.user.active = $("[rel='active'] .tele-checkbox-checkbox", that.container).hasClass('checked');
			
			userData.user['id'] = that.data.user.id;
			
			// Copy field data
			var fields = [ 'login', 'email', 'first_name', 'last_name', 'company', 'phone' ];
			$.each(fields, function(i, field) {
				userData.user[field] = $("[rel='" + field + "'] input", that.container).val();
			});
			
			// Validate password change
			var pass_1 = $(".tele-password input:first", that.container).val();
			var pass_2 = $(".tele-password input:last" , that.container).val();
			
			
			if(pass_1 != '' && pass_2 != '') {
				if(pass_1 != pass_2) {
					$(".tele-password input", that.container).css({ borderColor: 'red' });
					telepath.dialog({ msg: 'Password change mismatch.' });
					return;
				}
				
				userData.user.password = pass_1;
			}
			
			if(userData.user.id == 'new' && pass_1 == '') {
				
				telepath.dialog({ msg: 'Must provide password for new user.' });
				return;
				
			}
			
			// Validate email
			
			if(userData.user.login == '') {
				
				telepath.dialog({ msg: 'User login cant be blank.' });
				return;
				
			}
			
			if(userData.user.email != '') {
				
				function validateEmail(email){
					return /^([^\x00-\x20\x22\x28\x29\x2c\x2e\x3a-\x3c\x3e\x40\x5b-\x5d\x7f-\xff]+|\x22([^\x0d\x22\x5c\x80-\xff]|\x5c[\x00-\x7f])*\x22)(\x2e([^\x00-\x20\x22\x28\x29\x2c\x2e\x3a-\x3c\x3e\x40\x5b-\x5d\x7f-\xff]+|\x22([^\x0d\x22\x5c\x80-\xff]|\x5c[\x00-\x7f])*\x22))*\x40([^\x00-\x20\x22\x28\x29\x2c\x2e\x3a-\x3c\x3e\x40\x5b-\x5d\x7f-\xff]+|\x5b([^\x0d\x5b-\x5d\x80-\xff]|\x5c[\x00-\x7f])*\x5d)(\x2e([^\x00-\x20\x22\x28\x29\x2c\x2e\x3a-\x3c\x3e\x40\x5b-\x5d\x7f-\xff]+|\x5b([^\x0d\x5b-\x5d\x80-\xff]|\x5c[\x00-\x7f])*\x5d))*$/.test( email );
				}
				
				if(!validateEmail(userData.user.email)) {
					$("[rel='email'] input", that.container).css({ borderColor: 'red' });
					telepath.dialog({ msg: 'Bad email address.' });
					return;
				}
				
			}
			
			userData.user.mode = $('.tele-radio-knob', that.toggleType).parent().attr('rel');
			
			$('.tele-user-permissions .tele-checkbox-checkbox.checked').each(function () { 
				userData.perm.push($(this).attr('dataid')); 
			});
			$('.tele-user-groups .tele-checkbox-checkbox.checked').each(function () { 
				userData.groups.push($(this).attr('dataid')); 
			});
			
			switch(userData.user.mode) {
			
				case 'hosts':
					
					result = '';
					$('input', that.limitApps).each(function () {
						userData.apps.push($(this).val().trim());
					});
				
				break;
				case 'ranges':
				
					$('.tele-ip-wrap', that.limitRanges).each(function () {
				
						var is_range = $('.tele-mini-toggle', this).data('tele-toggleFlip').options.flipped;
						var ip_start = $('.tele-ip:first', this).data('tele-ip').getIP();
						var ip_end   = $('.tele-ip:last', this).data('tele-ip').getIP();
						
						if(is_range) {
							if(ip_start && ip_end && ip2long(ip_start) < ip2long(ip_end)) {
								userData.ranges.push(ip_start + '-' + ip_end);
							}
						} else {
							if(ip_start) {
								userData.ranges.push(ip_start);
							}
						}

					});
				
				break;
				
			}
			
			if(userData.user.id == 'new') {
				
				telepath.ds.get('/users/add_user', { items: userData }, function (data) {
					// console.log(data);
					telepath.config.accounts.loadData();
				});
			
			} else {
						
				telepath.ds.get('/users/set_user', { items: userData }, function (data) {
					// console.log(data);
					telepath.config.accounts.loadData();
				});
			
			}
			
		});
		
		// Apps limit
		var apps_data = [];
		$.each(that.data.apps, function(i, app) {
			apps_data.push({ text: app });
		});
		
		var userGroupsContainer = $('<div>').addClass('tele-user-groups-container');
		that.container.append(userGroupsContainer);
		that.groupsContainer = $('<div>').addClass('tele-user-groups');
		userGroupsContainer.append('<div class="tele-title-1">User Groups</div>').append(that.groupsContainer);
		
		$.each(telepath.config.accounts.groups, function (i, group) {
			
			var checked = false;
			$.each(that.data.groups, function (i, user_group) {
				if(user_group.id == group.id) {
					checked = true;
				}
			});
				
			var cb = $('<div>').teleCheckbox({
				checked: checked,
				label: group.name,
				dataID: group.id,
				callback: function (el, v) {}
				
			});
				
			that.groupsContainer.append(cb);
			
		});
		
		userGroupsContainer.append(that.groupsContainer);

		that.permissions = $('<div>').addClass('tele-user-permissions').append(telepath.loader).append('<div class="tele-title-1">Permissions</div>');


		// Create tabs UI
		that.perm_tabs   = $('<div>').addClass('tele-perm-tabs');
		that.tabsEl = $('<div>').addClass('tabs');
		that.tabsUl = $('<ul>');
		that.perm_tabs.append(that.tabsUl).append(that.tabsEl);
		
		// Define tab headers
		var tabs = [
			{ id: 'get', text: 'Retrieve' },
			{ id: 'set', text: 'Modify' }
			// { id: 'add', text: 'Create' },
			// { id: 'del', text: 'Delete' }
		];
		
		// Populate headers and create content divs
		for(x in tabs) {
			var tab = tabs[x];
			var tabEl = $('<div>').attr('id', 'tele-perm-' + tab.id);
			var tabLi = $('<li>');
			var tabA  = $('<a>').attr('href', '#tele-perm-' + tab.id).text(tab.text);
			tabLi.append(tabA);
			that.tabsUl.append(tabLi);
			that.tabsEl.append(tabEl);
			
		}
		
		telepath.ds.get('/permissions/get_list', { }, function (data) {
			
			$('.tele-loader', that.permissions).remove();
			
			$.each(data.items, function (i, row) {
				
				var checked = false;
				$.each(that.data.perm, function (i, perm) {
					if(perm.id == row.id) {
						checked = true;
						return false;
					}
				});
								
				var cb = $('<div>').teleCheckbox({
				checked: checked,
				label: row.description,
				dataID: row.id,
				callback: function (el, v) {}
				});
				
				$('#tele-perm-' + row['function']).append(cb);
				
				//that.permissions.append(cb);
			
			});
			
			that.perm_tabs.tabs({ 
		
				// heightStyle: 'fill',
				// autoHeight: false,
				// animate: false,
				activate: function( event, ui ) {	
				}
				
			});
			
		});
		
		that.permissions.append(that.perm_tabs);
		that.container.append(that.permissions);
		
		that.limitApps = $('<div>').teleSelect({ type: 'subdomain', values: apps_data, click: function () { } }).hide();
		that.limitRanges = $('<div>').addClass('tele-limit-ranges').hide();
		
		that.toggleType = $('<div>').teleRadios({
			title: 'Configure applications access limits',
			radios: [ 
				{ key: 'none',   label: 'None', 			  checked: that.data.apps.length == 0 && that.data.ranges.length == 0 }, 
				{ key: 'ranges', label: 'IP Ranges', 		  checked: that.data.apps.length == 0 && that.data.ranges.length > 0 },
				{ key: 'hosts',  label: 'Specific hostnames', checked: that.data.apps.length > 0 && that.data.ranges.length == 0 },
			], callback: function(radio) {
			
			that.limitApps.hide();
			that.limitRanges.hide();
			
			switch(radio.key) {
				case 'none':
				break;
				case 'ranges':
					that.limitRanges.show();
				break;
				case 'hosts':
					that.limitApps.show();
				break;
			}
							
		}}).addClass('tele-limit-type');
		
		that.container.append(that.toggleType);
		that.container.append(that.limitApps);
			
		function Old_getRangeUI(data) {

			if(!data) { data = ''; } else { data = data.trim() }
			
			var is_range = data.split('-').length > 1;
								
			var ipWrap   = $('<div>').addClass('tele-ip-wrap');
			var ipStart  = $('<div>').addClass('tele-ip').ip({ data: data.split('-')[0] });
			var ipDash   = $('<div>').addClass('tele-ip-dash').html('_');
			
			var ipEnd    = $('<div>').addClass('tele-ip').ip({ data: is_range ? data.split('-')[1] : '' });
			
			if(!is_range) {
				ipDash.hide();
				ipEnd.hide();
			}
			
			var ipAdd = $('<div>').addClass('tele-ip-add')
									 .addClass('tele-icon')
									 .addClass('tele-icon-plus')
									 .hover(function () { $(this).addClass('hover'); }, 
											function () { $(this).removeClass('hover'); })
									 .click(function () { that.limitRanges.append(getRangeUI()); });
			
			var ipRemove = $('<div>').addClass('tele-ip-remove')
									 .addClass('tele-icon')
									 .addClass('tele-icon-minus')
									 .hover(function () { $(this).addClass('hover'); }, 
											function () { $(this).removeClass('hover'); })
									 .click(function () { $(this).parent().remove(); });
									 
			var ipToggle = $('<div>').toggleFlip({ 
			
				left_value: 'Single', 
				right_value: 'Range',
				flip: function () {
					ipEnd.toggle();
					ipDash.toggle();
				},
				flipped: is_range,
				
			});
			
			ipWrap.append(ipAdd).append(ipRemove).append(ipToggle).append(ipStart).append(ipDash).append(ipEnd);
			return ipWrap;
			
		}

		function getRangeUI(data, container) {

			//if(!data) { data = ''; } else { data = data.trim() }

			var is_range= data.from!=data.to;

			//var to = data.to?data.to:data.from;

			var ipWrap   = $('<div>').addClass('tele-ip-wrap');
			var ipStart  = $('<div>').addClass('tele-ip').ip({ data: data.from });
			var ipDash   = $('<div>').addClass('tele-ip-dash').html('_');

			var ipEnd    = $('<div>').addClass('tele-ip').ip({ data: data.to});

			if(!is_range) {
				ipDash.hide();
				ipEnd.hide();
			}

			var ipAdd = $('<div>').addClass('tele-ip-add')
				.addClass('tele-icon')
				.addClass('tele-icon-plus')
				.hover(function () { $(this).addClass('hover'); },
				function () { $(this).removeClass('hover'); })
				.click(function () { container.append(getRangeUI()); });

			var ipRemove = $('<div>').addClass('tele-ip-remove')
				.addClass('tele-icon')
				.addClass('tele-icon-minus')
				.hover(function () { $(this).addClass('hover'); },
				function () { $(this).removeClass('hover'); })
				.click(function () { RangeRemove(this); /* $(this).parent().remove(); */ });

			var ipToggle = $('<div>').toggleFlip({

				left_value: 'Single',
				right_value: 'Range',
				flip: function () {
					ipEnd.toggle();
					ipDash.toggle();
				},
				flipped: is_range
			});

			ipWrap.append(ipAdd).append(ipRemove).append(ipToggle).append(ipStart).append(ipDash).append(ipEnd);
			return ipWrap;

		}
					
					
		// Load provided by settings
		$.each(that.data.ranges, function (i, ip) {
			that.limitRanges.append(getRangeUI(ip));
		});
					
		// Another blank
		
		that.limitRanges.append(getRangeUI(''));
			
		that.container.append(that.limitRanges);
		
	}
	
}