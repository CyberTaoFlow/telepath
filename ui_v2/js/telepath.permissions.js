telepath.permissions = {
	
	showWindow: function() {
		
		if(!telepath.permissions.window) {
			telepath.permissions.window = Ext.create('MyApp.view.permissionsWindow');
		}
		telepath.permissions.window.show();
		telepath.permissions.loadStore();

	},
	loadStore: function (callback) {
		
		$.post(telepath.controllerPath + '/permissions/get_list', {}, function (data) {
			Ext.getStore('permissionsStore').loadData(data.items);
			if(typeof callback == 'function') {
				callback();
			}
		}, 'json');
		
	},
	edit: function(record) {
		
		telepath.permissions.editWindow = Ext.create('MyApp.view.permissionWindow');
		telepath.permissions.editWindow.show();
		
		telepath.permissions.perm_mask = new Ext.LoadMask(telepath.permissions.editWindow.body.el, {msg:"Please wait..."});
		telepath.permissions.perm_mask.show();
		
		$.post(telepath.controllerPath + '/permissions/get_permission', { id: record.data.id }, function (data) {
			
			telepath.permissions.editData = data;

			var fields = ['class', 'function', 'alias', 'description', 'params'];
			$.each(fields, function(i, field) {
				Ext.getCmp('perm_' + field).setValue(data['items']['permission'][field]);
			});

			telepath.permissions.perm_mask.destroy();
			
			telepath.applications.get_list(function () {
			
				Ext.getStore('applicationsStore').loadData(data.items);
				
				telepath.permissions.group_sel = Ext.getCmp('perm_applications').getSelectionModel();
				
				if(!telepath.permissions.editData['items']['applications'] || telepath.permissions.editData['items']['applications'].length == 0) {
					telepath.permissions.editData['items']['applications'] = [ { id: -1 } ];
				}
				
				var selection = [];
				
				$.each(Ext.getStore('applicationsStore').data.items, function(i, apps_app) { 
					$.each(telepath.permissions.editData['items']['applications'], function(x, perm_app) {
						if(apps_app.data.id == perm_app.id) {
							selection.push(apps_app);
						}
					});
				});
				
				telepath.permissions.group_sel.select(selection);
				
				telepath.permissions.group_sel.addListener('selectionchange', function(e, o) { 
					
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
		
		telepath.permissions.perm_mask = new Ext.LoadMask(telepath.permissions.editWindow.body.el, {msg:"Please wait..."});
		telepath.permissions.perm_mask.show();
		
		var params = telepath.permissions.editData;
		
		var perm_applications = [];
		
		$.each(Ext.getCmp('perm_applications').getSelectionModel().selected.items, function (i, app) {
			perm_applications.push(app.data.id);
		});

		params['items']['applications'] = perm_applications;
		
		var fields = ['class', 'function', 'alias', 'description', 'params'];
		
		$.each(fields, function(i, field) {
			params['items']['permission'][field] = Ext.getCmp('perm_' + field).value;
		});
		
		$.post(telepath.controllerPath + '/permissions/set_permission', params, function (data) {
			
			if(data.success) {
			
				telepath.permissions.editWindow.destroy();
				telepath.permissions.loadStore();
				telepath.permissions.edit({ data: { id: data.items.permission_id } });
				
			} else {
				
				telepath.permissions.perm_mask.destroy();
				Ext.Msg.alert('Error', data.error);
				
			}

		}, 'json');

	},
	create: function () {
		
		telepath.permissions.editWindow = Ext.create('MyApp.view.permissionWindow');
		telepath.permissions.editWindow.show();
		Ext.getCmp('perm_delete').disable();
		Ext.getCmp('perm_activity').disable();
		telepath.permissions.editData = { items: { permission: { id: 'new' } , applications: [] } };
		telepath.permissions.loadStore();
		
	},
	deleteClick: function () {
	
		var id = telepath.permissions.editData.items.permission.id;
		
		telepath.permissions.perm_mask = new Ext.LoadMask(telepath.permissions.editWindow.body.el, {msg:"Please wait..."});
		telepath.permissions.perm_mask.show();
		
		$.post(telepath.controllerPath + '/permissions/del_permission', { id: id } , function (data) {
			
			if(data.success) {
			
				telepath.permissions.editWindow.destroy();
				telepath.permissions.loadStore();
				
			} else {
				
				telepath.permissions.perm_mask.destroy();
				Ext.Msg.alert('Error', data.error);
				
			}

		}, 'json');
		
	}
	
}