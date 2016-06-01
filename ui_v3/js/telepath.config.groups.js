telepath.groups = {
	
	showWindow: function() {
		
		if(!telepath.groups.window) {
			telepath.groups.window = Ext.create('MyApp.view.groupsWindow');
		}
		telepath.groups.window.show();
		telepath.groups.loadStore();

	},
	loadStore: function (callback) {
		
		telepath.util.request('/groups/get_list', { }, function (data) {

			Ext.getStore('groupsStore').loadData(data.items);
			if(typeof callback == 'function') {
				callback();
			}
			
		}, 'Error loading group list.');
		
	},
	edit: function(record) {
		
		telepath.groups.editWindow = Ext.create('MyApp.view.groupWindow');
		telepath.groups.editWindow.show();
		
		telepath.groups.group_mask = new Ext.LoadMask(telepath.groups.editWindow.body.el, {msg:"Please wait..."});
		telepath.groups.group_mask.show();
		
		telepath.util.request('/groups/get_group', { id: record.data.id }, function (data) {

			telepath.groups.editData = data;
			
			var fields = ['name', 'description'];
			$.each(fields, function(i, field) {
				Ext.getCmp('group_' + field).setValue(data['items']['group'][field]);
			});

			telepath.groups.group_mask.destroy();
			
			telepath.users.loadStore(function () {
				
				telepath.groups.group_sel = Ext.getCmp('group_users').getSelectionModel();
				
				var selection = [];
				
				$.each(Ext.getStore('usersStore').data.items, function(i, users_user) { 
					$.each(telepath.groups.editData['items']['users'], function(x, group_user) {
						if(users_user.data.id == group_user.id) {
							selection.push(users_user);
						}
					});
				});
				
				telepath.groups.group_sel.select(selection);
				
			});
			
			telepath.permissions.loadStore(function () {
				
				telepath.groups.group_sel = Ext.getCmp('group_permissions').getSelectionModel();
				
				var selection = [];
				
				$.each(Ext.getStore('permissionsStore').data.items, function(i, perm_perm) { 
					$.each(telepath.groups.editData['items']['perm'], function(x, group_perm) {
						if(perm_perm.data.id == group_perm.id) {
							selection.push(perm_perm);
						}
					});
				});
				
				telepath.groups.group_sel.select(selection);
				
			});
			
		}, 'Error loading group information.');
		
	},
	save: function() {
		
		telepath.groups.group_mask = new Ext.LoadMask(telepath.groups.editWindow.body.el, {msg:"Please wait..."});
		telepath.groups.group_mask.show();
		
		var params = telepath.groups.editData;
		
		var group_users = [];
		
		$.each(Ext.getCmp('group_users').getSelectionModel().selected.items, function (i, group_user) {
			group_users.push(group_user.data.id);
		});
		
		var group_permissions = [];
		
		$.each(Ext.getCmp('group_permissions').getSelectionModel().selected.items, function (i, group_perm) {
			group_permissions.push(group_perm.data.id);
		});

		params['items']['users'] = group_users;
		params['items']['perm']  = group_permissions;
		
		var fields = ['name', 'description'];
		$.each(fields, function(i, field) {
			params['items']['group'][field] = Ext.getCmp('group_' + field).value;
		});
		
		var path = '/groups/' + (params['items']['group']['id'] == 'new' ? 'add' : 'set') + '_group';
		
		telepath.util.request(path, params, function (data) {
		
			telepath.groups.editWindow.destroy();
			telepath.groups.loadStore();
			telepath.groups.edit({ data: { id: data.items.group_id } });

		}, function () {
			
			telepath.groups.group_mask.destroy();
			Ext.Msg.alert('Error', data.error);
		
		});

	},
	create: function () {
		
		telepath.groups.editWindow = Ext.create('MyApp.view.groupWindow');
		telepath.groups.editWindow.show();
		Ext.getCmp('group_delete').disable();
		Ext.getCmp('group_activity').disable();
		telepath.groups.editData = { items: { group: { id: 'new' } , perm: [], users: [] } };
		telepath.users.loadStore();
		
	},
	deleteClick: function () {
	
		var id = telepath.groups.editData.items.group.id;
		
		// console.log(id);
		
		telepath.groups.group_mask = new Ext.LoadMask(telepath.groups.editWindow.body.el, {msg:"Please wait..."});
		telepath.groups.group_mask.show();
		
		telepath.util.request('/groups/del_group', { id: id } , function (data) {
			
			telepath.groups.editWindow.destroy();
			telepath.groups.loadStore();

		}, function () {
		
			telepath.groups.group_mask.destroy();
			Ext.Msg.alert('Error', data.error);
			
		});
	}
	
}