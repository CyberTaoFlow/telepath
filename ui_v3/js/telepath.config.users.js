telepath.users = {
	
	toggle: function(what) {
		
		telepath.users.perm_sel = Ext.getCmp('user_permissions').getSelectionModel();

		var selection = [];
		
		$.each(Ext.getStore('permissionsStore').data.items, function(i, perm_perm) { 
			var bool = Ext.getCmp('user_toggle_' + perm_perm.data['function']).pressed;
			if(bool) {
				selection.push(perm_perm);
			}
		});
		
		telepath.users.perm_sel.select(selection);
	
	},
	showWindow: function() {
		
		if(!telepath.users.window) {
			telepath.users.window = Ext.create('MyApp.view.usersWindow');
		}
		telepath.users.window.show();
		telepath.users.loadStore();

	},
	loadStore: function (callback) {
		
		$.post(telepath.controllerPath + '/users/get_list', {}, function (data) {
			Ext.getStore('usersStore').loadData(data.items);
			if(typeof callback == 'function') {
				callback();
			}
		}, 'json');
		
	},
	edit: function(record) {
		
		telepath.users.editWindow = Ext.create('MyApp.view.userWindow');
		telepath.users.editWindow.show();
		
		telepath.users.user_mask = new Ext.LoadMask(telepath.users.editWindow.body.el, {msg:"Please wait..."});
		telepath.users.user_mask.show();
		
		$.post(telepath.controllerPath + '/users/get_user', { id: record.data.id }, function (data) {
			
			telepath.users.editData = data;
			
			var fields = ['login', 'active', 'email', 'first_name', 'last_name', 'company', 'phone'];
			$.each(fields, function(i, field) {
				Ext.getCmp('user_' + field).setValue(data['items']['user'][field]);
			});

			telepath.users.user_mask.destroy();
			
			telepath.groups.loadStore(function () {
				
				telepath.users.group_sel = Ext.getCmp('user_groups').getSelectionModel();
				
				var selection = [];
				
				$.each(Ext.getStore('groupsStore').data.items, function(i, groups_group) { 
					$.each(telepath.users.editData['items']['groups'], function(x, user_group) {
						if(groups_group.data.id == user_group.id) {
							selection.push(groups_group);
						}
					});
				});
				
				telepath.users.group_sel.select(selection);

			});
			
			telepath.permissions.loadStore(function () {
				
				telepath.users.perm_sel = Ext.getCmp('user_permissions').getSelectionModel();
				
				var selection = [];
				
				$.each(Ext.getStore('permissionsStore').data.items, function(i, perm_perm) { 
					$.each(telepath.users.editData['items']['perm'], function(x, user_perm) {
						if(perm_perm.data.id == user_perm.id) {
							selection.push(perm_perm);
						}
					});
				});
				
				telepath.users.perm_sel.select(selection);
				
			});
			
			telepath.applications.get_list(function (data) {
			
				Ext.getStore('applicationsStore').loadData(data.items);
				
				telepath.users.apps_sel = Ext.getCmp('user_applications').getSelectionModel();
				
				if(!telepath.users.editData['items']['apps'] || telepath.users.editData['items']['apps'].length == 0) {
					telepath.users.editData['items']['apps'] = [ -1 ];
				}
				
				var selection = [];
				
				$.each(Ext.getStore('applicationsStore').data.items, function(i, apps_app) { 
					$.each(telepath.users.editData['items']['apps'], function(r_id, app_id) {
						if(apps_app.data.id == parseInt(app_id)) {
							selection.push(apps_app);
						}
					});
				});
				
				telepath.users.apps_sel.select(selection);
				
				telepath.users.apps_sel.addListener('selectionchange', function(e, o) { 
					
					if(e.lastSelected && e.lastSelected.data) {
						if(e.lastSelected.data.id == -1) {
							e.select(e.lastSelected);
						}
					}
				
				});
				
			});
						
		}, 'json');
		
	},
	save: function() {
		
		var params = telepath.users.editData;
		
		var user_groups = [];
		
		$.each(Ext.getCmp('user_groups').getSelectionModel().selected.items, function (i, user_group) {
			user_groups.push(user_group.data.id);
		});
		
		var user_permissions = [];
		
		$.each(Ext.getCmp('user_permissions').getSelectionModel().selected.items, function (i, user_perm) {
			user_permissions.push(user_perm.data.id);
		});
		
		var user_applications = [];
		
		$.each(Ext.getCmp('user_applications').getSelectionModel().selected.items, function (i, user_app) {
			user_applications.push(user_app.data.id);
		});
		
		params['items']['groups'] = user_groups;
		params['items']['perm']   = user_permissions;
		params['items']['apps']   = user_applications;
		
		var fields = ['login', 'active', 'email', 'first_name', 'last_name', 'company', 'phone'];
		$.each(fields, function(i, field) {
			params['items']['user'][field] = Ext.getCmp('user_' + field).value;
		});
		
		// Validate login
		var login = params['items']['user']['login'];
		if(login == '' || typeof login == 'undefined') {
			Ext.Msg.alert('Error', 'Login can\'t be blank');
			return;
		}
		
		// Validate email
		var email = params['items']['user']['email'];
		var email_regex = /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
		if(email != '' && !email_regex.test(email)) {
			Ext.Msg.alert('Error', 'Incorrect email');
			return;
		}
		
		// Password change validate
		var pass_1 = Ext.getCmp('user_password_1').value;
		var pass_2 = Ext.getCmp('user_password_2').value;
		
		if(pass_1 != '' && pass_2 != '' && pass_1 !== pass_2) {
			Ext.Msg.alert('Error', 'Password Mismatch');
			return;
		}
		
		// Password change
		if(pass_1 != '' && pass_2 != '' && pass_1 === pass_2) {
			params['items']['user']['password'] = pass_1;
		}
		
		// New user password
		if(params['items']['user']['id'] == 'new' && (pass_1 == '' || typeof pass_1 == 'undefined')) {
			Ext.Msg.alert('Error', 'New user must have password');
			return;
		}

		telepath.users.user_mask = new Ext.LoadMask(telepath.users.editWindow.body.el, {msg:"Please wait..."});
		telepath.users.user_mask.show();
		
		var path = params['items']['user']['id'] == 'new' ? 'add_user' : 'set_user';
		
		$.post(telepath.controllerPath + '/users/' + path, params, function (data) {
			
			if(data.success) {
			
				telepath.users.editWindow.destroy();
				telepath.users.loadStore();
				telepath.users.edit({ data: { id: data.items.user_id } });
				
			} else {
				
				telepath.users.user_mask.destroy();
				Ext.Msg.alert('Error', data.error);
				
			}
			
		}, 'json');

	},
	create: function () {
		
		telepath.users.editWindow = Ext.create('MyApp.view.userWindow');
		telepath.users.editWindow.show();
		Ext.getCmp('user_delete').disable();
		Ext.getCmp('user_activity').disable();
		telepath.users.editData = { items: { user: { id: 'new', login: '', email: '' } , perm: [], groups: [], apps: [] } };
		telepath.groups.loadStore();
		
	},
	deleteClick: function () {
	
		var id = telepath.users.editData.items.user.id;
		
		// console.log(id);
		
		telepath.users.user_mask = new Ext.LoadMask(telepath.users.editWindow.body.el, {msg:"Please wait..."});
		telepath.users.user_mask.show();
		
		$.post(telepath.controllerPath + '/users/del_user', { id: id } , function (data) {
			
			if(data.success) {
			
				telepath.users.editWindow.destroy();
				telepath.users.loadStore();
				
			} else {
				
				telepath.users.user_mask.destroy();
				Ext.Msg.alert('Error', data.error);
				
			}

		}, 'json');
	}
	
}