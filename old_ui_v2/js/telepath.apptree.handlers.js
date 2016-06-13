function get_parent_app_id(record) {
	if(record.data.type == 'application') {
		return record.data.id;
	} else {
		return get_parent_app_id(record.parentNode);
	}
}
function get_parent_app_name(record) {
	if(record.data.type == 'application') {
		return record.raw.app_domain;
	} else {
		return get_parent_app_name(record.parentNode);
	}
}

// Flow Category -- Rename
AppTreePanel.prototype.renameFlowCategory = function (record) {
	
	var that = this;
	
	context_prompt('Business action name', 'Rename business action "' + record.raw.name + '"', function (text) {
		
		var category_id = record.raw.id;
		
		$.post(telepath.controllerPath + '/workflow/set_category_name', { category_id: category_id, category_name: text }, function (data) {
		
			if(data.success) {
				record.raw.name = data.items.category_name;
				record.set('text', data.items.category_name);
			} else {
				Ext.Msg.alert('Error', data.error);
			}
			
		} , 'json');
	
	}, record.raw.name);
	
}
// Flow Category -- Remove
AppTreePanel.prototype.removeFlowCategory = function(record) {

	context_confirm('Business action remove', 'Remove business action category "' + record.raw.name + '" ?', function () {
		
		var category_id = record.raw.id;
		
		$.post(telepath.controllerPath + '/workflow/del_category', { category_id: category_id }, function (data) {
		
			if(data.success) {
				record.remove();
			} else {
				Ext.Msg.alert('Error', data.error);
			}
			
		} , 'json');
								
	});

}


// Flow -- Cut
AppTreePanel.prototype.cutFlow = function (record) {
	
	var that = this;
	
	if(that.cutData) {
		that.cutData.set('text', that.cutData.raw.group);
		that.cutData = false;
	}
	
	var text = record.raw.group;
	text = '<span style="opacity: 0.6">' + text + '</span>';
	record.set('text', text);

	that.cutData = record;
	
}

AppTreePanel.prototype.pasteFlow = function (record) {
	
	var that = this;
	
	// Sanity check here as well
	if(!that.cutData) return;
	
	var loading      = record.appendChild(that.loadingNode);
	
	record.expand();
	
	var category_id  = record.raw.id;
	var flow_id      = that.cutData.raw.id;
	
	$.post(telepath.controllerPath + '/workflow/set_category_flow', { category_id: category_id, flow_id: flow_id }, function (data) {
		
		loading.remove();
		if(data.success) {
			
			// Append to category
			record.appendChild(that.cutData);
			
		} else {
			Ext.Msg.alert('Error', data.error);
		}
		
		// Revert title to normal
		var text = that.cutData.raw.group;
		that.cutData.set('text', text);
		
		// Release cut data
		that.cutData = false;
		
		// Refresh tools
		that.leftClickHandler(that.panel, record, 'Workflow');
		
		
	} , 'json');

}

AppTreePanel.prototype.removeFlowItems = function(items) {
	
	var that = this;
	
	var recordNode = items[0].parentNode.parentNode;
	var parentNode = items[0].parentNode;
	
	Ext.suspendLayouts();
	
	for(index in items) {
		
		this.removeAtt(parentNode, items[index].raw.att_name);
		items[index].remove();
				
	}

	Ext.resumeLayouts(true);

	window.recordflow.refresh(recordNode);

}

// Flow -- Rename
AppTreePanel.prototype.renameFlow = function (record) {
	
	var that = this;
	
	context_prompt('Business action name', 'Rename business action "' + record.data.text + '"', function (text) {
					
		record.set('text', text);
		record.set('value', '<span style="color: red">Modified</span>');
		
		that.saveFlow(record);
					
	}, record.data.text);
	
}

// Flow -- Save
AppTreePanel.prototype.saveFlow = function(record, skipConfirm, newFlow) {

	// NEW FORMAT
	var app_id = record.parentNode.data.id;
	var json   = [];
	
	// Iterage Pages
	$.each(record.childNodes, function (i, childNode) {
	
		var rawData = childNode.raw.data;
		var jsonNode = { page_id : parseInt(rawData.page_id) };
		
		// Iterage Params
		if(rawData.params && rawData.params.length > 0) {
			jsonNode.params = [];
			$.each(rawData.params, function (i, param) {
				jsonNode.params.push( { att_id: parseInt(param.att_id), data: param.data } );
			});
		}
		
		json.push(jsonNode);
		
	});
	
	// OLD FORMAT
	context_confirm('Business action save', 'Save business action "' + record.data.text + '" ?', function () {
		
		var app_id      = record.parentNode.data.id;
		var flow_name   = record.data.text;
		var flow_pages  = [];
		var flow_id		= record.data.id.split('_');
		
		$.each(record.childNodes, function (i, childNode) {
			var page_id = childNode.data.id.split('_');
			flow_pages.push(page_id[2]); // FLOW_PAGE_<PAGEID>_RND
		});
		
		flow_pages = flow_pages.join(',');
		
		if(flow_id[0] == 'flow') {
			save_edited_flow(flow_id[1], flow_name, flow_pages, record, json);
		} else {
			save_new_flow(app_id, flow_name, flow_pages, record, json);
		}
						
	}, skipConfirm, function () {
		
		if(newFlow) {
			record.remove();
			window.recordflow.clear();
		}
	
	});

}

// Flow -- Remove
AppTreePanel.prototype.removeFlow = function(record) {

	context_confirm('Business action remove', 'Remove business action "' + record.data.text + '" ?', function () {
				
		var flow_id	  = record.data.id.split('_');
		var flow_name = record.data.text;
		
		if(flow_id[0] == 'flow') {
			delete_flow(flow_id[1], flow_name, record);
		} else {
			record.remove();
		}
		
		window.recordflow.clear();
								
	});

}

// Flow -- Remove Page from Flow
AppTreePanel.prototype.removePageFromFlow = function(record) {
	
	var record_id    = record.data.id;
	var record_index = record.data.index;

	// 1 Level up to record node
	var recordNode = record.parentNode;

	record.parentNode.set('value', '<span style="color: red">Modified</span>');
	record.remove();

	window.recordflow.refresh(recordNode);

}

AppTreePanel.prototype.removeAtt = function(node, removeName) {
	
	if(!node.raw.data || !node.raw.data.params) return;
	
	var params       = node.raw.data.params;
	var newParams    = [];
	
	// Rebuild param
	$.each(params, function (i, param) {
		if(param.att_name != removeName) {
			newParams.push(param);
		}
	});
	
	node.raw.data.params = newParams;
	
}

// Flow -- Remove Attribute from current page
AppTreePanel.prototype.removeAttributeFromPage = function(record) {
	
	// 2 Levels up to record node
	var recordNode   = record.parentNode.parentNode;
	var removeName   = record.data.text;
	var parentNode   = record.parentNode;
	
	// Remove from raw
	this.removeAtt(parentNode, removeName);
	
	// Remove from tree
	record.remove();

	// Refresh
	window.recordflow.refresh(recordNode);
	recordNode.set('value', '<span style="color: red">Modified</span>');
	
}

// Flow -- Remove all occurances of attribute from flow
AppTreePanel.prototype.removeAttributeFromFlow = function(record) {

	// 2 Levels up to record node
	var recordNode   = record.parentNode.parentNode;
	var removeName   = record.data.text;
	
	// Traverse all pages finding that attribute ID
	for(i = 0; i < recordNode.childNodes.length; i++) {
	
		var node = recordNode.childNodes[i];

		// Try remove from RAW
		this.removeAtt(node, removeName);
		
		// Try remove from TREE
		if(node.childNodes && node.childNodes.length > 0) {
			
			for(x = 0; x < node.childNodes.length; x++) {
				
				if(node.childNodes[x]) {
					var child = node.childNodes[x];
					var childName = child.data.text;
					if(childName == removeName) {
						child.remove();
					}
				}
			
			}

		}

	}

	// Refresh
	window.recordflow.refresh(recordNode);
	recordNode.set('value', '<span style="color: red">Modified</span>');	
}

// Page -- Alias Add
AppTreePanel.prototype.pageAliasAdd = function(record) {
	
	var that = this;
	
	if(record.data.type == 'flow_page') {
		
		var page_id    = record.raw.data.page_id;
		telepath.handlers.page.aliasAdd(page_id, function (data) {
			that.updateRow(record, data.page);
		});
		
	}
	
	if(record.data.type == 'page') {
		
		var page_id    = record.raw.id;
		telepath.handlers.page.aliasAdd(page_id, function (data) {
			that.updateRow(record, data.page);
		});
		
	}

}

// Page -- Alias Remove
AppTreePanel.prototype.pageAliasRemove = function(record) {
	
	var that = this;
	
	if(record.data.type == 'flow_page') {
		
		var page_id    = record.raw.data.page_id;
		telepath.handlers.page.aliasRemove(page_id, function (data) {
			that.updateRow(record, data.page);
		});
		
	}
	if(record.data.type == 'page') {
		
		var page_id    = record.raw.id;
		telepath.handlers.page.aliasRemove(page_id, function (data) {
			that.updateRow(record, data.page);
		});
		
	}
	
}

// Page Alias Edit
AppTreePanel.prototype.pageAliasEdit = function (record) {
	
	var that = this;
	
	if(record.data.type == 'flow_page') {
		
		var page_id    = record.raw.data.page_id;
		var page_alias = record.raw.data.alias;
		
		telepath.handlers.page.aliasEdit(page_id, page_alias, function (data) {
			that.updateRow(record, data.page);
		});
		
	}
	if(record.data.type == 'page') {
		
		var page_id    = record.raw.id;
		var page_alias = record.raw.alias;
		
		telepath.handlers.page.aliasEdit(page_id, page_alias, function (data) {
			that.updateRow(record, data.page);
		});
		
	}
	
}

// Application -- Workflow Category Add
AppTreePanel.prototype.appWorkflowCategoryAdd = function(record) {
	
	var that = this;
	
	record.expand();
	
	context_prompt('Business action category name', 'Enter new business action category name', function (text) {
		
		var loading = record.appendChild(that.loadingNode);
		var app_id  = record.raw.app_id;
			
		$.post(telepath.controllerPath + '/workflow/add_category', { app_id: app_id, category_name: text }, function (data) {
			
			loading.remove();
			if(data.success) {
				
				var newNode = {
					text: text,
					type: 'flow_category',
					id: 'flow_category_' + data.items.category_id,
					expandable: true,
					expanded: false
				}
				
				var newRecord = record.appendChild(newNode);
				newRecord.raw = data;
				
			} else {
				Ext.Msg.alert('Error', data.error);
			}
			
		} , 'json');

	});
	
}

// Application -- Alias Add
AppTreePanel.prototype.appAliasAdd = function(record) {
	
	var that = this;
	
	telepath.handlers.app.aliasAdd(record.data.id, function (data) {
		that.updateRow(record, data.app);
	});

}

// Application -- Alias Remove
AppTreePanel.prototype.appAliasRemove = function(record) {
	
	var that = this;
	
	telepath.handlers.app.aliasRemove(record.data.id, function (data) {
		that.updateRow(record, data.app);
	});
	
}

// Application -- Alias Edit
AppTreePanel.prototype.appAliasEdit = function (record) {

	var that = this;
	
	telepath.handlers.app.aliasEdit(record.data.id, record.raw.display_name, function (data) {
		that.updateRow(record, data.app);
	});

}

// Application -- Remove Application
AppTreePanel.prototype.appDelete = function (records) {

	if(records.length == 1) {
		
		var record = records[0];
		context_confirm('Application', 'Delete "' + record.data.text + '" application?', function () {
			telepath.applications.del_application(record.data.id);
		});
	
	} else {
		
		var ids = [];
		
		$.each(records, function(i, record) { ids.push(record.data.id); });
				
		context_confirm('Application', 'Delete ' + ids.length + ' selected applications?', function () {
			telepath.applications.del_application(ids.join(','));
		});
		
	}

}

// Application -- Upload Logs
AppTreePanel.prototype.appUploadLogs = function (record) {
	
	Ext.create('MyApp.view.c_logmode_dialog').show();
	yepnope({
		load: [
			"js/telepath.logmode.js"
		],
		complete: function () {
			logmode_init(record.data.id);
		}
	});
	
}

// Param -- Alias Add
AppTreePanel.prototype.paramAliasAdd = function(record) {
	
	var that = this;
	telepath.handlers.param.aliasAdd(record.raw.att_id, function (data) {
		that.updateRow(record, data.param);
	});

}

// Param -- Alias Remove
AppTreePanel.prototype.paramAliasRemove = function(record) {
	
	var that = this;
	telepath.handlers.param.aliasRemove(record.raw.att_id, function (data) {
		that.updateRow(record, data.param);
	});
		
}

// Param -- Alias Edit
AppTreePanel.prototype.paramAliasEdit = function (record) {
	
	var that = this;
	telepath.handlers.param.aliasEdit(record.raw.att_id, record.raw.att_alias != '' ? record.raw.att_alias : record.raw.att_name, function (data) {
		that.updateRow(record, data.param);
	});

}

// Param -- Mask
AppTreePanel.prototype.paramMask = function(record) {
	
	var that = this;
	telepath.handlers.param.maskAdd(record.raw.att_id, record.raw.att_alias != '' ? record.raw.att_alias : record.raw.att_name, function (data) {
		that.updateRow(record, data.param);
	});
	
}

// Param -- UnMask
AppTreePanel.prototype.paramUnMask = function(record) {
	
	var that = this;
	telepath.handlers.param.maskRemove(record.raw.att_id, record.raw.att_alias != '' ? record.raw.att_alias : record.raw.att_name, function (data) {
		that.updateRow(record, data.param);
	});
	
}

// MAIN LEFT CLICK HANDLER
AppTreePanel.prototype.leftClickHandler = function (panel, record, context) {

	var that = this;
	
	var searchbar = panel.getComponent('AppTreeSearch');
	var toolbar   = panel.getComponent('AppTreeToolbar');
	var select    = false;
	
	toolbar.removeAll();
	
	if(that.context == 'Parameters') {
	
		select = toolbar.add({ xtype: 'button', iconCls: 'context-menu-select', text: 'Select' });
		select.disable();
		
	}
			
	// TODO:: Rewrite this bit 
	
	switch(record.data.type) {
		
		case 'show_more_dir':
		case 'show_more_file':
		case 'show_more_apps':
		case 'show_more_subdomains':
		
			var dir_limit = 100;
			var file_limit = 100;
			var subdomains_limit = 100;
			var app_limit  = 100;
			var dir_printed = 0;
			var file_printed = 0;
			var app_printed  = 0;
			var subdomains_printed  = 0;
			
			var more_data = record.raw.more;
			
			window.more_data = record;
			
			Ext.suspendLayouts();
			
			$.each(more_data, function(i, row) {
				
				var childNode = { };
				
				childNode.expanded   = false;
				childNode.expandable = true;
				childNode.iconCls = 'tree-icon-' + row.type;
				childNode.type = row.type;
				
				switch(row.type) {
					case 'page':
					
						childNode.id = 'page_' + row.id;
						childNode.expandable = row.expandable;
						
						if(file_printed < file_limit) {
							var newRecord = record.parentNode.insertChild(record.data.index, childNode);
							that.updateRow(newRecord, row);
						} else {
							return false;
						}
						
						file_printed++;
					
					break;
					case 'directory':
					
						childNode.id = row.text + '_' + Math.floor((Math.random()*1000)+1);
						
						if(dir_printed < dir_limit) {
							var newRecord = record.parentNode.insertChild(record.data.index, childNode);
							that.updateRow(newRecord, row);
						} else {
							return false;
						}
						
						dir_printed++;
					
					break;
					case 'application':
						
						childNode.id  = row.app_id;
						childNode.ssl = row.ssl_flag;
						
						if(app_printed < app_limit) {
							var newRecord = record.parentNode.insertChild(record.data.index, childNode);
							that.updateRow(newRecord, row);
						} else {
							return false;
						}
						
						app_printed++;						
					
					break;
					
					case 'subdomain':
						
						childNode.expanded   = false;
						childNode.expandable = true;
						childNode.type = row.type;
						childNode.id = 'subdomain_' + row.subdomain_id;
						
						if(subdomains_printed < subdomains_limit) {
							var newRecord = record.parentNode.insertChild(record.data.index, childNode);
							that.updateRow(newRecord, row);
						} else {
							return false;
						}
						
						subdomains_printed++;						
					
					break;
				
				}

			});
			
			for(var i = 0; i < file_printed + dir_printed + app_printed + subdomains_printed; i++) {
				record.raw.more.shift();
			}
			
			if(record.raw.more.length == 0) {
				record.remove();
			} else {
				if(record.data.type == 'show_more_dir') {
					record.set('text', 'Show more directories (' + record.raw.more.length + ' not shown)');
				}
				if(record.data.type == 'show_more_file') {
					record.set('text', 'Show more files (' + record.raw.more.length + ' not shown)');
				}
				if(record.data.type == 'show_more_apps') {
					record.set('text', 'Show more applications (' + record.raw.more.length + ' not shown)');
				}
				if(record.data.type == 'show_more_subdomains') {
					record.set('text', 'Show more subdomains (' + record.raw.more.length + ' not shown)');
				}
			}
			
			Ext.resumeLayouts(true);
			
		
		break;
		
		case 'application':
			
			if(that.context == 'Parameters') {
				return;
			}
			
			// Local ID
			var id 	  = record.data.id;
			
			// Context Specific
			switch(context) {
				
				case 'Application':
					
					telepath.config.appBeforeEdit(id);
					
					
					// Edit Application
					/*toolbar.add({ xtype: 'button', iconCls: 'context-menu-applicationEdit', text: 'Edit' })
						   .addListener('click', function () {	telepath.config.appStartEdit();	});*/
					
					
					// Seperator
					toolbar.add({ xtype: 'tbseparator' });
				
					// Upload Logs
					toolbar.add({ xtype: 'button', iconCls: 'context-menu-applicationLogs', text: 'Upload Logs' })
						   .addListener('click', function () { that.appUploadLogs(record); });
					
				break;
				
				case 'Workflow':
					
					selected_app = record;
					update_graph_by_app();
					
					toolbar.add({ xtype: 'button', iconCls: 'icon-add', text: 'Add Category' })
								.addListener('click', function () { that.appWorkflowCategoryAdd(record); });
					
					toolbar.add({ xtype: 'tbseparator' });

				break;
				
			}
			
			if(telepath.access.has_perm('Applications', 'del')) {
			
				// Delete Application
				toolbar.add({ xtype: 'button', iconCls: 'context-menu-applicationRemove', text: 'Delete' })
					   .addListener('click', function () { 
					   // Pass on all selected records
					   that.appDelete(that.panel.getSelectionModel().selected.items); 
				});
			
			}
		
		break;
		
		case 'subdomain':
			
			// Local ID
			var id 	  = record.data.id;
			
			// Context Specific
			switch(context) {
				
				case 'Application':
					
					telepath.config.subdomainSelect(id);
					
				break;
				
			}
			
		
		break;
		
		case 'flow':
			
			if(!record.data.expanded && record.data.expandable) {
				if(window.recordflow) {
					window.recordflow.clear();
				}
				record.parentNode.collapseChildren();
				record.expand();
			}
			
			if(telepath.access.has_perm('Workflow', 'set')) {
			
				toolbar.add({ xtype: 'button', iconCls: 'icon-cut', text: 'Cut' })
					   .addListener('click', function () { that.cutFlow(record); });

				toolbar.add({ xtype: 'button', iconCls: 'context-menu-edit', text: 'Edit' })
					   .addListener('click', function () { that.renameFlow(record); });
				
				// Seperator
				toolbar.add({ xtype: 'tbseparator' });
				
				toolbar.add({ xtype: 'button', iconCls: 'context-menu-save', text: 'Save' })
					   .addListener('click', function () { that.saveFlow(record); });
				   
			}
			
			
			if(telepath.access.has_perm('Workflow', 'del')) {
			
				toolbar.add({ xtype: 'button', iconCls: 'context-menu-remove', text: 'Remove' })
					   .addListener('click', function () { that.removeFlow(record); });
				   
			}
			
		
		break;
		
		case 'flow_page': 
			
			toolbar.add({ xtype: 'button', iconCls: 'context-menu-save', text: 'Save' })
				   .addListener('click', function () { that.saveFlow(record.parentNode); });
			
		break;
		
		case 'flow_category':
			
			if(telepath.access.has_perm('Workflow', 'set')) {
			
				if(that.cutData) {
					toolbar.add({ xtype: 'button', iconCls: 'icon-cut', text: 'Paste' })
					   .addListener('click', function () { that.pasteFlow(record); });
				}

				toolbar.add({ xtype: 'button', iconCls: 'icon-edit', text: 'Rename' })
					   .addListener('click', function () { that.renameFlowCategory(record); });
				   
			}
			
			if(telepath.access.has_perm('Workflow', 'del')) {
			
				toolbar.add({ xtype: 'button', iconCls: 'context-menu-remove', text: 'Remove' })
					   .addListener('click', function () { that.removeFlowCategory(record); });
				   
			}
			
			if(window.recordflow) {
				window.recordflow.clear();
			}
			
		break;
		
		case 'record':
			
			/*var ans = {"success":"true","SID":"fb3f279596ce3ee5daec7bad9f2ad03b5b6bf026","time":"1384873735"};
		
			w_sid_to_record = ans['SID'];
			w_app_to_record = selected_app.id;
			w_recording_start_time = ans['time'];*/

			selected_app = record.parentNode;
			recordNode = record;

			/*start_recording();
			return;*/
			
			if (Ext.WindowManager.get("w_workflow_record")==undefined) {
				Ext.create('MyApp.view.w_workflow_record').show();
			}
			else {
				Ext.WindowManager.get("w_workflow_record").show();
			}
			
			
		break;
		
		case 'record_stop':
			
			clearInterval(w_recording_interval);
			stop_blinking_record_sign();
			
			// Convert Record to Flow
			recordNode.childNodes[0].remove();
			recordNode.set('iconCls', 'tree-icon-flow');
			recordNode.set('type', 'flow');
			recordNode.set('value', '<span style="color: red">Not saved yet</span>');
			recordNode.set('id', 'recording_' + w_sid_to_record + '_' + Math.floor((Math.random()*1000)+1));
			
			// Add a New Record Button
			if(telepath.access.has_perm('Workflow', 'add')) {
			
				var newRecordNode = that.recordNode;
				newRecordNode.id = 'record_' + recordNode.parentNode.data.id;
				recordNode.parentNode.appendChild(newRecordNode);
				
			}
			
			// Ask to save the flow
			that.saveFlow(recordNode, true, true);
		
		break;
		
		case 'page':
		
			if(that.context == 'Parameters' && selection_state == 'page') {
				
				select.enable();
				
				select.addListener('click', function () { 
					telepath.parameters.handlerSelectPage(record);
				});
				
			}
			
		break;
		
		case 'attribute':
			
			// Allow select parameter
			if(that.context == 'Parameters' && selection_state == 'parameter') {
			
				select.enable();
				
				select.addListener('click', function () { 
					telepath.parameters.handlerSelectParam(record);
				});
				
			}
			if(that.context == 'Workflow') {
				toolbar.add({ xtype: 'button', iconCls: 'context-menu-save', text: 'Save' })
				   .addListener('click', function () { that.saveFlow(record.parentNode.parentNode); });
			}
			
			
		break;
		
		case 'global-param':
			
			if(that.context == 'Parameters' && selection_state == 'parameter') {
			
				select.enable();
				
				select.addListener('click', function () { 
					telepath.parameters.handlerSelectParam(record);
				});
				
			}
			
		break;
		
	}
	
	// Refresh
	toolbar.add({ xtype: 'button', iconCls: 'icon-refresh', text: 'Refresh' })
		   .addListener('click', function () { that.refresh(true); });

}

// MAIN RIGHT CLICK HANDLER
AppTreePanel.prototype.rightClickHandler = function (record, event) {
	
	var that = this;

	// ---------------------------------------------------
	// GLOBAL
	// ---------------------------------------------------

	var context_select  = {
		text: 'Select',
		iconCls: 'context-menu-select',
		handler: function () {

		}
	}
	
	// ---------------------------------------------------
	// WORKFLOW
	// ---------------------------------------------------
	
	// Flow -- Save
	var menu_flow_save = {
		text: 'Save Flow',
		iconCls: 'context-menu-save',
		handler: function () {
			that.saveFlow(record);
		}
	}
	
	// Flow -- Edit
	var menu_flow_edit  = {
		text: 'Rename Flow',
		iconCls: 'context-menu-aliasEdit',
		handler: function () {
			that.renameFlow(record);
		}
	}
	
	// Flow -- Remove
	var menu_flow_remove  = {
		text: 'Remove Flow',
		iconCls: 'context-menu-remove',
		handler: function () {
			that.removeFlow(record);
		}
	}
	
	var menu_flow_multi_remove = {
		text: 'Remove Selected Items',
		iconCls: 'context-menu-remove',
		handler: function () {
			that.removeFlowItems(that.panel.getSelectionModel().getSelection());
		}
	}
	
	// Flow -- Remove Page 
	var menu_flow_page_remove = {
		text: 'Remove from Flow',
		iconCls: 'context-menu-applicationRemove',
		handler: function () {
			that.removePageFromFlow(record);
		}
	}
	
	var menu_flow_attribute_remove_page = {
		text: 'Remove from Page',
		iconCls: 'context-menu-applicationRemove',
		handler: function () {
			that.removeAttributeFromPage(record);
		}
	}
	
	var menu_flow_attribute_remove_flow = {
		text: 'Remove from Flow',
		iconCls: 'context-menu-applicationRemove',
		handler: function () {
			that.removeAttributeFromFlow(record);
		}
	}
	
	// ---------------------------------------------------
	// PAGE
	// ---------------------------------------------------
	
	// Page -- Alias Add
	var menu_page_alias_add  = {
		text: 'Add Page Alias',
		iconCls: 'context-menu-aliasAdd',
		handler: function () {
			that.pageAliasAdd(record);
		}
	}
	// Page -- Alias Remove
	var menu_page_alias_remove  = {
		text: 'Remove Page Alias',
		iconCls: 'context-menu-aliasRemove',
		handler: function () {
			that.pageAliasRemove(record);
		}
	}
	// Page -- Alias Edit
	var menu_page_alias_edit  = {
		text: 'Edit Page Alias',
		iconCls: 'context-menu-aliasEdit',
		handler: function () {
			that.pageAliasEdit(record);
		}
	}
	
	// ---------------------------------------------------
	// APPLICATION
	// ---------------------------------------------------
	var menu_app_add_workflow_category = {
		text: 'Add workflow category',
		iconCls: 'icon-add',
		handler: function () {
			that.appWorkflowCategoryAdd(record);
		}
	}

	// Application -- Alias Add
	var menu_app_alias_add  = {
		text: 'Set Application Name',
		iconCls: 'context-menu-aliasAdd',
		handler: function () {
			that.appAliasAdd(record);
		}
	}
	// Application -- Alias Remove
	var menu_app_alias_remove  = {
		text: 'Remove Application Name',
		iconCls: 'context-menu-aliasRemove',
		handler: function () {
			that.appAliasRemove(record);
		}
	}
	// Application -- Alias Edit
	var menu_app_alias_edit  = {
		text: 'Rename Application',
		iconCls: 'context-menu-aliasEdit',
		handler: function () {
			that.appAliasEdit(record);
		}
	}
	
	// Application -- Edit
	var menu_app_edit  = {
		text: 'Edit Application',
		iconCls: 'context-menu-aliasEdit',
		handler: function () {
			telepath.config.appStartEdit();
		}
	}
	
	// Application -- Remove
	var menu_app_remove  = {
		text: 'Delete Application',
		iconCls: 'context-menu-applicationRemove',
		handler: function () {
			that.appDelete([ record ]);
		}
	}

	// Application -- Upload Logs
	var menu_app_logmode  = {
		text: 'Upload Logs',
		iconCls: 'context-menu-applicationLogs',
		handler: function () {
			that.appUploadLogs(record);
		}
	}
	
	// ---------------------------------------------------
	// PARAMETERS
	// ---------------------------------------------------
	
	// Param -- Alias Add
	var menu_param_alias_add  = {
		text: 'Add Parameter Alias',
		iconCls: 'context-menu-aliasAdd',
		handler: function () {
			that.paramAliasAdd(record);
		}
	}
	// Param -- Alias Remove
	var menu_param_alias_remove  = {
		text: 'Remove Parameter Alias',
		iconCls: 'context-menu-aliasRemove',
		handler: function () {
			that.paramAliasRemove(record);
		}
	}
	// Param -- Alias Edit
	var menu_param_alias_edit  = {
		text: 'Edit Parameter Alias',
		iconCls: 'context-menu-aliasEdit',
		handler: function () {
			that.paramAliasEdit(record);
		}
	}
	
	// Context Param Mask
	var menu_param_mask  = {
		text: 'Mask Parameter',
		iconCls: 'context-menu-paramMask',
		handler: function () {
			that.paramMask(record);
		}
	}
	
	// Context Param Un-Mask
	var menu_param_unmask  = {
		text: 'Un-Mask Parameter',
		iconCls: 'context-menu-paramUnMask',
		handler: function () {
			that.paramUnMask(record);
		}
	}

	var menuItems = [];
	
	switch(record.data.type) {
	
		case 'application':
			
			if(that.context == 'Workflow' && telepath.access.has_perm('Workflow', 'add')) {
				menuItems.push(menu_app_add_workflow_category);
				menuItems.push({ xtype: 'menuseparator' });
			}
			
			if(telepath.access.has_perm('Applications', 'set')) {
			
			if(record.raw.display_name == '') {
				menuItems.push(menu_app_alias_add);
			} else {
				menuItems.push(menu_app_alias_edit);
				menuItems.push(menu_app_alias_remove);
			}
			
			}
		
			menuItems.push({ xtype: 'menuseparator' });
			menuItems.push(menu_app_logmode);
			
			if(telepath.access.has_perm('Applications', 'del')) {
			
			menuItems.push({ xtype: 'menuseparator' });
			menuItems.push(menu_app_remove);
			
			}
		
		break;
		
		case 'flow':
			
			if(telepath.access.has_perm('Workflow', 'set')) {
				menuItems.push(menu_flow_save);
				menuItems.push(menu_flow_edit);
			}
			
			if(telepath.access.has_perm('Workflow', 'del')) {
				menuItems.push(menu_flow_remove);
			}
		
		break;
		
		case 'flow_page':
			
			if(that.panel.getSelectionModel().selected.items.length > 1) {
				menuItems.push(menu_flow_multi_remove);
				var contextMenu = new Ext.menu.Menu({ items : menuItems });
				contextMenu.showAt(event.getXY());
				event.stopEvent();
				return;
			}
			
			menuItems.push(menu_flow_page_remove);

			if(that.context == 'Parameters' && selection_state == 'page') {
				menuItems.push(context_select);
				menuItems.push({ xtype: 'menuseparator' });
			}

			if(record.raw.data.alias == '') {
				menuItems.push(menu_page_alias_add);
			} else {
				menuItems.push(menu_page_alias_edit);
				menuItems.push(menu_page_alias_remove);
			}
			
			menuItems.push({ text: 'Save Flow', iconCls: 'context-menu-save', handler: function () { 
				that.saveFlow(record.parentNode); 
			}});
			
		break;

		case 'page':
			
			if(that.context == 'Parameters' && selection_state == 'page') {
				menuItems.push(context_select);
				menuItems.push({ xtype: 'menuseparator' });
			}

			if(record.raw.title == '') {
				menuItems.push(menu_page_alias_add);
			} else {
				menuItems.push(menu_page_alias_edit);
				menuItems.push(menu_page_alias_remove);
			}

		break;
		case 'attribute':
			
			if(that.context == 'Workflow') {

				if(that.panel.getSelectionModel().selected.items.length > 1) {
					menuItems.push(menu_flow_multi_remove);
					var contextMenu = new Ext.menu.Menu({ items : menuItems });
					contextMenu.showAt(event.getXY());
					event.stopEvent();
					return;
				}
				
				menuItems.push(menu_flow_attribute_remove_page);
				menuItems.push(menu_flow_attribute_remove_flow);
				menuItems.push({ text: 'Save Flow', iconCls: 'context-menu-save', handler: function () { that.saveFlow(record.parentNode.parentNode); }});
			}
			
			if(that.context == 'Parameters' && selection_state == 'parameter') {
				menuItems.push(context_select);
				menuItems.push({ xtype: 'menuseparator' });
			}

			if(record.raw.mask == '0') {
				menuItems.push(menu_param_mask);
			} else {
				menuItems.push(menu_param_unmask);
			}

			menuItems.push({ xtype: 'menuseparator' });
			
			if(record.raw.att_alias == '') {
				menuItems.push(menu_param_alias_add);
			} else {
				menuItems.push(menu_param_alias_edit);
				menuItems.push(menu_param_alias_remove);
			}

		break;
		
		default:
		case 'directory': 
			
			event.stopEvent();
			return;
		
		break;
		
		case 'global-param':
			
			menuItems.push(context_select);
		
		break;
		
	}
	
	var contextMenu = new Ext.menu.Menu({ items : menuItems });
	contextMenu.showAt(event.getXY());
	event.stopEvent();
	
}

AppTreePanel.prototype.dblClickHandler = function (panel, record, context) {
    
	switch(record.data.type) {
					
		case 'page':
		
			if(context == 'Parameters' && selection_state == 'page') {
				telepath.parameters.handlerSelectPage(record);
			}
			
		break;
		
		case 'attribute':
			
			if(context == 'Parameters' && selection_state == 'parameter') {
				telepath.parameters.handlerSelectParam(record);
			}
			
		break;
		
		case 'global-param':
			
			if(context == 'Parameters' && selection_state == 'parameter') {
				telepath.parameters.handlerSelectParam(record);
			}
			
		break;
	
	}
	
}

