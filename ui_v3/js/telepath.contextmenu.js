telepath.contextMenu = function(obj) {
	
	if(
	   obj.data.type == 'application' ||
	   obj.data.type == 'page' ||
	   obj.data.type == 'param'
	) {
	
	var items = {};

	if(obj.data.raw.alias == '') {
	
		items.createAlias = {
			label: "Create Alias",
			icon: 'tele-icon tele-icon-alias_edit',
			action: function () {
				
				switch(obj.data.type) {
				
					case 'application':	
						telepath.handlers.app.aliasAdd(obj.data.id, function (data) {
							$('.jstree').jstree("rename_node", obj, data.app.display_name);
							obj.data.raw.alias = data.app.display_name;
						});
					break;
					case 'page':
						telepath.handlers.page.aliasAdd(obj.data.id, function (data) {
							$('.jstree').jstree("rename_node", obj, data.page.title);
							obj.data.raw.alias = data.page.title;
						});
					break;
					case 'param':
						telepath.handlers.param.aliasAdd(obj.data.id, function (data) {
							$('.jstree').jstree("rename_node", obj, data.param.att_alias);
							obj.data.raw.alias = data.param.att_alias;
						});
					break;
					
				}

			}
		}
		
	} else {
		
		items.editAlias = {
			label: "Edit Alias",
			icon: 'tele-icon tele-icon-alias_edit',
			action: function () {
				
				switch(obj.data.type) {
				
					case 'application':	
						telepath.handlers.app.aliasEdit(obj.data.id, obj.data.raw.alias, function (data) {
							$('.jstree').jstree("rename_node", obj, data.app.display_name);
							obj.data.raw.alias = data.app.display_name;
						});
					break;
					case 'page':
						telepath.handlers.page.aliasEdit(obj.data.id, obj.data.raw.alias, function (data) {
							$('.jstree').jstree("rename_node", obj, data.page.title);
							obj.data.raw.alias = data.page.title;
						});
					break;
					case 'param':
						telepath.handlers.param.aliasEdit(obj.data.id, obj.data.raw.alias, function (data) {
							$('.jstree').jstree("rename_node", obj, data.param.att_alias);
							obj.data.raw.alias = data.param.att_alias;
						});
					break;
					
				}
				
			}
		}
	
		items.deleteAlias = { 
			label: "Remove Alias",
			icon: 'tele-icon tele-icon-delete',
			action: function () {
				
				switch(obj.data.type) {
				
					case 'application':	
						telepath.handlers.app.aliasRemove(obj.data.id, function (data) {
							$('.jstree').jstree("rename_node", obj, data.app.app_domain);
							obj.data.raw.alias = '';
						});
					break;
					case 'page':
						telepath.handlers.page.aliasRemove(obj.data.id, function (data) {
							$('.jstree').jstree("rename_node", obj, data.page.text);
							obj.data.raw.alias = '';
						});
					break;
					case 'param':
						telepath.handlers.param.aliasRemove(obj.data.id, function (data) {
							$('.jstree').jstree("rename_node", obj, data.param.att_name);
							obj.data.raw.alias = '';
						});
					break;
					
				}
				
			}
		}
		
	}
	
	if(obj.data.type == 'param') {
		
		if(obj.data.raw.mask == '0') {
			
			items.maskAdd = { 
				label: "Mask Parameter",
				icon: 'tele-icon tele-icon-alias_edit',
				action: function () {
					telepath.handlers.param.maskAdd(obj.data.id, obj.data.text, function (data) {});
				}
			}
			
		} else {
			
			items.maskAdd = { 
				label: "Unmask Parameter",
				icon: 'tele-icon tele-icon-alias_edit',
				action: function () {
					telepath.handlers.param.maskRemove(obj.data.id, obj.data.text, function (data) {});
				}
			}
			
		}
		
	}
	
	if(obj.data.type == 'application' && obj.data.context == 'applications') {
	
		items.uploadLogs = {
			label: "Upload Logs",
			icon: 'tele-icon tele-icon-uploadLogs',
			action: function () {
				logmode_init(obj.data.id);
			}
		}
	
	}
	
	return items;
	
	} else {
		return false;
	}
}
