Expandhook = false;

function AppTreePanel(context, panel, app_limit, expand_data){
	
	// SETUP
	var that     = this;
	this.context = context;
	this.panel   = panel;
	this.app_limit = app_limit;
	this.controller   = '/apptree/';
	this.lastExpanded = false;	
	this.rootID = 0;
	this.filter = 'ShowAll';
	this.searchOptions = [];

	// SETUP STORE
	switch(context) {
		case 'Application':
			if(!Ext.StoreManager.lookup('AppTreeApps')) {
				Ext.StoreManager.add(Ext.create('MyApp.store.AppTreeApps'));
			}
			panel.bindStore(Ext.StoreManager.lookup('AppTreeApps'));
		break;
		case 'Workflow':
			if(!Ext.StoreManager.lookup('AppTreeFlows')) {
				Ext.StoreManager.add(Ext.create('MyApp.store.AppTreeFlows'));
			}
			panel.bindStore(Ext.StoreManager.lookup('AppTreeFlows'));
			
			// Workflow Multiselect Model
			panel.getSelectionModel().addListener('selectionchange', function(e, o) { 
				
				// Deselect on selection of flow / cat / app
				if(e.lastSelected && e.lastSelected.data) {
					if(e.lastSelected.data.type != 'flow_page' && e.lastSelected.data.type != 'attribute') {
						e.select(e.lastSelected);
					}
				}
				
			});
			
		break;
		case 'Parameters':
			if(!Ext.StoreManager.lookup('AppTreeParams')) {
				Ext.StoreManager.add(Ext.create('MyApp.store.AppTreeParams'));
			}
			panel.bindStore(Ext.StoreManager.lookup('AppTreeParams'));
			panel.store.clearData();
		break;
	}
	
	this.store = panel.store;
	
	// STORE EXPAND
	if(Expandhook) {
		this.store.removeListener('expand', Expandhook);
	}

	Expandhook = function (nodeinterface, eOpts) { 
		that.expand(nodeinterface, eOpts);
	};

	//if(!this.store.hasListener('expand')) {
		
	//}

	// LEFT CLICK HANDLER
	panel.addListener('itemclick', function (view, record, obj) {
		that.leftClickHandler(panel, record, that.context);
	});
	
	panel.addListener('itemdblclick', function (view, record, obj) {
		that.dblClickHandler(panel, record, that.context);
	});
	
	// BIND RIGHT click handler
	panel.addListener('itemcontextmenu', function(view, record, item, index, event) {
		that.rightClickHandler(record, event);
	});
	
	// RENDER EVENT	
	panel.addListener('render', function () { 
		
		var searchbar = panel.getComponent('AppTreeSearch');
		var toolbar   = panel.getComponent('AppTreeToolbar');
		var topbar    = panel.getComponent('AppTreeTopBar');
		
		
		if(context != 'Parameters') {
			
		} else {
			var select = toolbar.add({ xtype: 'button', iconCls: 'context-menu-select', text: 'Select' });
			select.disable();
		}
		
		// Refresh
		toolbar.add({ xtype: 'button', iconCls: 'icon-refresh', text: 'Refresh' })
			   .addListener('click', function () { that.refresh(true); });

		switch(context) {
			
			case 'Application':
			
				topbar.show();
				
				if(telepath.access.has_perm('Applications', 'add')) {
					topbar.add({ xtype: 'button', iconCls: 'context-menu-applicationAdd', text: 'Create Application' })
						  .addListener('click', function () { telepath.config.appCreateNew(); });
				}
				
				// Control for autolearn new apps
				topbar.add(Ext.getCmp('c_RAP11'));
				Ext.getCmp('c_RAP11').show();
			
			case 'Workflow':	
			
				$('#w_config-body .app-tree-search-type').hide();
			
			case 'Parameters':
			
			
				// TODO:: Applications (includes subdomains)
				function doSearch() {
					
					var searchOptions = {
						'text'		   : searchbar.getComponent('AppTreeSearchInput').value,
						'pages'		   : searchbar.getComponent('AppTreeSearchPages').value,
						'parameters'   : searchbar.getComponent('AppTreeSearchParams').value,
						'apps'  	   : searchbar.getComponent('AppTreeSearchApps').value
					}
					
					that.searchOptions = searchOptions;
					
					if(that.app_limit) {
						// Refresh just selected app
						that.store.getRootNode().collapse();
						that.store.getRootNode().expand();
						
					} else {
						// Refresh all apps list
						that.store.setRootNode(this.loadingNode);
						that.refresh();
					}
					
				}
				
				searchbar.getComponent('AppTreeSearchButton').addListener('click', function () {
					doSearch();
				});
				
				setTimeout(function () {
					
					searchbar.getComponent('AppTreeSearchInput').inputEl.addListener('keyup', function (e) {
						setTimeout(function () {
							doSearch();
						}, 500);
					});
					
				}, 100);

			break;

		}
		
		// HIDE VALUE COLUMN
		setTimeout(function () {
			that.panel.columns[1].hide();
		}, 50);
		
	});
	
	// STATIC NODES
	this.loadingNode = { 
		text: 'Loading...', 
		expandable: false, 
		leaf: true, 
		expanded: false, 
		iconCls: 'tree-icon-loading',
	};
	
	this.recordNode = { 
		text: 'Record new Action', 
		expandable: false, 
		leaf: true, 
		expanded: false, 
		iconCls: 'tree-icon-record',
		type: 'record',
	};
	
	this.noResultsNode = {
		text: 'No Results',
		leaf: true,
		expandable: false,
		expanded: false
	}
	
	// START
	this.panel.setRootNode(this.loadingNode);
	
	var expand_hooked = false;
	
	if(expand_data) {
		
		var appNode = { 
			id: 'application_' + expand_data.app_id, 
			expandable: true, 
			expanded: true,
			type: 'application',
			text: expand_data.hostname,
			children: []
		};

		var rootNode = that.panel.setRootNode(appNode);
		
		var subdomainNode = rootNode.appendChild(
			{ 
				id: 'subdomain_' + expand_data.subdomain_id, 
				expandable: true, 
				expanded: true,
				type: 'subdomain',
				text: expand_data.subdomain_name,
				children: []
			}
		);
		
		var dir_path  = expand_data.display_path.split('/');
		var last_node = subdomainNode;
		var page_name = dir_path.pop();
		
		$.each(dir_path, function (i, val) {
		
			if(val != '') {
			
				childNode = {};
				childNode.expanded   = true;
				childNode.expandable = true;
				childNode.iconCls = 'tree-icon-directory';
				childNode.type = 'directory';
				childNode.text = val;
				childNode.id = val + '_' + Math.floor((Math.random()*1000)+1);
				
				last_node = last_node.appendChild(childNode);
				
			}
			
		});
		
		expand_hooked = true;
		this.store.addListener('expand', Expandhook);
		
		childNode = {};
		childNode.expanded   = true;
		childNode.expandable = true;
		childNode.iconCls 	 = 'tree-icon-page';
		childNode.id 		 = 'page_' + expand_data.page_id;
		childNode.type 		 = 'page';
		childNode.text 		 = page_name;
		
		page_node = last_node.appendChild(childNode);
	
		// Traverse path while creating directory nodes 
		// Then create page node
		// Call expand on page node to get all params
		// Perhaps even mark current
		
		return;
	}
	
	if(!expand_hooked) {
		this.store.addListener('expand', Expandhook);
	}
	
	// Limit to single APP
	if(this.app_limit) {
		
		var newNode = { 
			id: this.app_limit, 
			expandable: true, 
			expanded: false, 
			type: 'application', 
		};
		
		setTimeout(function () {
			that.panel.setRootNode(newNode).expand();
		}, 100);

	} else {
		
		setTimeout(function () {
			// Load all APPS
			that.refresh();
		}, 100);
		
		
	}

}

AppTreePanel.prototype.prependGlobalHeaders = function(node) {
	
	if(window.globalHeaders && window.globalHeaders.items) {
		
		var newNode = {
			type: 'global-headers',
			expandable: true,
			expanded: false,
			children: [],
			text: '<span style="color: red;">Global Headers</span>',
			iconCls: 'tree-icon-global-headers',
		};
		
		$.each(window.globalHeaders.items, function(i, header) {
			
			var headerNode = {
				expanded: false,
				type: 'global-param',
				leaf: true,
				expandable: false,
				iconCls: 'tree-icon-attributeH',
				text: header.header,
				id: 'global_param_' + header.value
			};

			newNode.children.push(headerNode);
		
		});
		
		node.insertBefore(newNode);
		
	}

}

// Row formatter
AppTreePanel.prototype.updateRow = function(record, row) {

	var type = record.data.type;
	var update = {};
	var updated = false;
	
	switch(type) {
		
		case 'subdomain':
			
			update.text = row.subdomain_name;
			
		break;
		
		case 'directory':

			update.text = row.text;
		
		break;
		
		case 'application':
			
			if(row.display_name != '') {
				update.text = '<span style="color:green;">' + row.display_name + '</span>&nbsp(' + row.app_domain + ')';
			} else {
				update.text = row.app_domain;
			}
			
			if(row.results) {
				update.text += ' (' + row.results + ' results)';
			}
			
		break;
		
		case 'page':
			
			if(row.text == '') { row.text = '/'; }
			
			if(row.title != '') {
				update.text = '<span style="color:green;">' + row.title + '</span>&nbsp(' + row.text + ')';
			} else {
				update.text = row.text;
			}
		
		break;
		
		case 'flow_page': 
			
			if(row.title != '') {
				update.text = '<span style="color:green;">' + row.title + '</span>&nbsp(' + row.text + ')';
			} else {
				update.text = row.text;
			}
			
			record.set(update);
			
			return;
		
		break;
		
		case 'attribute':
			
			row.att_name = decodeURIComponent(row.att_name);
			
			if(row.att_alias != '') {
				update.text = '<span style="color:green;">' + row.att_alias + '</span>&nbsp(' + row.att_name + ')';
			} else {
				update.text = row.att_name;
			}
			if(row.mask == '1') {
				update.text = update.text + '&nbsp;<span style="color:red;">*</span>';
			}
		
		break;
		
	}
	
	// Update record
	record.set(update);

	// Set Raw Values
	record.raw = row;
	

}

AppTreePanel.prototype.updParam = function (record, row) {
	
	var update = {};
	
	if(row.att_alias !== '') {
		update.text = '<span style="color:green;">' + row.att_alias + '</span>&nbsp(' + decodeURIComponent(row.att_name) + ')';
	} else {
		update.text = row.att_name;
	}
	if(row.mask == '1') {
		update.text = update.text + '&nbsp;<span style="color:red;">*</span>';
	}
	
	record.set(update);
	record.raw = row;
	
}

AppTreePanel.prototype.tplApp = function(row) {

	return newNode;
	
}

AppTreePanel.prototype.loadMore = function(more_node) {
	
}

// TREE -- BASE
AppTreePanel.prototype.refresh = function (force) {
	
	var that = this;
	
	if(force) {
		that.panel.setRootNode(that.loadingNode);
	}
	
	Ext.suspendLayouts();
	
	telepath.util.request('/applications/get_apptree_index', { search:  this.searchOptions }, function(data) {
		
		//var el = that.panel.getComponent('AppTreeTopBar').el.dom;
		//$('.x-box-inner',el).append('Total: ' + data.total);
		
		data = data.items;
		
		var new_root = { text: 'Telepath DB', expandable: true, expanded: true, children: [] };
		that.store.setRootNode(new_root);
		var node = that.store.getRootNode();
		
		// Global Headers
		if(that.context == 'Parameters') {
			that.prependGlobalHeaders(node);
		}
		
		var app_printed = 0;
		var app_limit   = 100;

		var app_more_node = false;
		
		// Load applications
		$.each(data, function (i, row) {
			
			row.type = 'application';
			
			// Allways allow expand workflow app
			if(that.context == 'Workflow') {
				row.expandable = true;
			}
			
			var newNode = { 
				id: row.app_id, 
				expandable: row.expandable, 
				expanded: false, 
				type: row.type, 
				ssl: row.ssl_flag,
				iconCls: 'tree-icon-' + row.type + (row.app_type != '' ? '-' + row.app_type : '')
			};
		
			if(app_printed < app_limit) {
				var record = node.appendChild(newNode);
				that.updateRow(record, row);
			} else {
				if(!app_more_node) {
					app_more_node = { leaf: true, text: 'Show more files...', type: 'show_more_apps' };
					app_more_node = node.appendChild(app_more_node);
					app_more_node.raw.more = [];
					app_more_node.raw.more.push(row);
				} else {
					app_more_node.raw.more.push(row);					
				}
			}
			
			app_printed++;
	
		});
		
		if(app_more_node) {
			app_more_node.set('text', 'Show more applications (' + app_more_node.raw.more.length + ' not shown)');
		}
		
		if(node.childNodes.length == 0) {
			node.insertChild(0 , that.noResultsNode);
		}
		
		Ext.resumeLayouts(true);
		
	}, 'Error retreiving apptree index.');
	
}

// TREE -- EXPAND 
AppTreePanel.prototype.expand = function (node, eOpts) {
	// Manually expand tree data
	
	var that = this;
	
	if(node.data.type == 'global-headers') {
		return;
	}
	
	if(
	   (this.context == 'Workflow' && node.data.type == 'flow_page') || 
	   (this.context == 'Workflow' && node.data.type == 'record') ||
	   (this.context == 'Workflow' && node.data.type == 'flow_category')
	  ) {
		return;
	}
		
	node.insertChild(0 , this.loadingNode);
	
	switch(node.data.type) {
		
		case 'application':
		case 'subdomain':
		case 'directory':
		case 'page':
		case 'flow':

			var id   = node.data.id;
			var text = node.data.text;
			
			function findParentApp(node) {
				if(node.data.type == 'application' || node.data.type == 'subdomain') {
					return node.data.id.replace('subdomain_', '');
				} else {
					return findParentApp(node.parentNode);
				}
			}
						
			function getFullRoute(node) {
				text = node.data.text + '/' + text;
				if(node.parentNode.data.type != 'application' && node.parentNode.data.type != 'subdomain') {
					return getFullRoute(node.parentNode);
				}
				return text;
			}
			
			if(node.data.type == 'directory') {
				id = findParentApp(node);
				text = '';
				getFullRoute(node);
			}
			
			if(node.data.type == 'flow' || node.data.type == 'page' || node.data.type == 'subdomain') {
				id = node.data.id.split('_');
				id = id[1];
			}
			
			if(node.data.type == 'application' && this.context == 'Workflow') {
				selected_app = node;
			}
			
			Ext.suspendLayouts();
			
			var controller = this.context == 'Workflow' ? '/workflow/get_expand' : '/applications/get_expand';
			
			telepath.util.request(controller, {
	
				type: node.data.type,
				id:   id,
				text: text,
				context: this.context,
				search: this.searchOptions
				
			}, function (data) {
				
				node.removeAll();
				
				if(node.data.type == 'application' && that.context == 'Parameters') {
					that.prependGlobalHeaders(node);
				}
				
				if(node.data.type == 'application' && data.context && data.context == 'workflow') {
					
					function parseFlowNode(row) {
						
						var childNode = { };
						
						childNode.expanded   = false;
						childNode.text  = row.group;
						childNode.type  = row.type;
						childNode.id    = 'flow_' + row.id + '_' + Math.random(9999);
						childNode.value = row.value;
						childNode.expandable = row.expandable;
						childNode.iconCls = 'tree-icon-' + row.type;
						
						return childNode;
						
					}
					
					// Expanding APPLICATION -> FLOWS
					
					$.each(data.flow_categories, function(i, category_row) {
						
						var categoryNode = { };
						
						categoryNode.expanded   = false;
						categoryNode.text  = category_row.name;
						categoryNode.type  = category_row.type;
						categoryNode.id    = 'flow_category_' + category_row.id;
						
						var categoryNode = node.appendChild(categoryNode);
						
						$.each(data.flows, function(i, flow) {
							if(flow.category_id == category_row.id) {
								
								if(!categoryNode.expanded) { categoryNode.expand(); }
								
								var childNode = parseFlowNode(flow);
								var record = categoryNode.appendChild(childNode);
								record.raw = flow;
								
							}
						});
						
						categoryNode.raw = category_row;
						
					});
					
					$.each(data.flows, function(i, row) {
						
						if(!row.category_id || row.category_id === "0") {
							var childNode = parseFlowNode(row);
							var record = node.appendChild(childNode);
							record.raw = row;
						}

					});
			
					if(telepath.access.has_perm('Workflow', 'add')) {
			
						var recordNode = that.recordNode;
						recordNode.id = 'record_' + id;
						node.appendChild(recordNode);
					
					}
					
					Ext.resumeLayouts(true);
					
					return;
					
				}
				
				if(node.data.type == 'flow' && data.context && data.context == 'workflow') {
				
					// Expanding FLOW -> PAGES
					// TREE
					var page_ids = [];
					
					$.each(data.items, function(i, row) {
					
						var childNode = { };
						
						childNode.expanded   = false;
						childNode.text  = row.alias == false ? row.page_name : '<span class="alias" style="color: green">' + row.alias + '</span>&nbsp;(' + row.page_name + ')';
						childNode.type  = 'flow_page';
						childNode.id    = 'flow_page_' + row.page_id + '_' + Math.random(9999);
						childNode.value = row.page_id;
						childNode.iconCls = 'tree-icon-page';
						
						if(row.params) {
						
							childNode.expandable = true;
							childNode.expanded   = false;
							childNode.children = [];
							
							var newNode = node.appendChild(childNode);
							newNode.raw.data = row;
							
							$.each(row.params, function (i, param) {
								
								var childNode = { };
						
								childNode.expanded   = false;
								childNode.type = 'attribute';
								childNode.id   = row.page_id + '_att_' + param.att_id + '_' + Math.random(9999);
								childNode.expandable = false;
								childNode.leaf = true;
								childNode.iconCls = 'tree-icon-attribute' + param.att_source;
								
								var record = newNode.appendChild(childNode);

								that.updateRow(record, param);
							
							});
							
						} else {
						
							childNode.expandable = false;
							childNode.leaf		 = true;
						
							var newNode = node.appendChild(childNode);
							newNode.raw.data = row;
							
						}
						
						
					});
					
					// Init RF 
					if(!window.recordflow) {
						window.recordflow = new RecordFlow();
					}
					
					// Refresh RF
					window.recordflow.refresh(node);
					
				
				}
				
				// Application expands to subdomains
				if(node.data.type == 'application') {
				
					if(!data.subdomains[0]) {
						data.subdomains = {};
					}
					
					var subdomains_limit = 100;
					var subdomains_printed = 0;
					
					var subdomains_more_node = false;
					
					$.each(data.subdomains, function(i, row) {
					
						var childNode = { };
						
						subdomains_printed++;
						
						row.type = 'subdomain';
						
						childNode.expanded   = false;
						childNode.expandable = true;
						childNode.iconCls = 'tree-icon-' + row.type;
						childNode.type = row.type;
						childNode.id = 'subdomain_' + row.subdomain_id;
						
						if(subdomains_printed < subdomains_limit) {
							var record = node.appendChild(childNode);
							that.updateRow(record, row);
						} else {
							if(!subdomains_more_node) {
								subdomains_more_node = { leaf: true, text: 'Show more subdomains...', type: 'show_more_subdomains' };
								subdomains_more_node = node.appendChild(subdomains_more_node);
								subdomains_more_node.raw.more = [];
								subdomains_more_node.raw.more.push(row);
							} else {
								subdomains_more_node.raw.more.push(row);					
							}
						}
						
					});
					
					if(subdomains_more_node) {
						subdomains_more_node.set('text', 'Show more subdomains (' + subdomains_more_node.raw.more.length + ' not shown)');
					}
										
				}
				
				// If its Subdomain or Directory -- Show Files
				if(node.data.type == 'subdomain' || node.data.type == 'directory') {
				
					if(!data.files[0]) {
						data.files = {};
					}
					
					var dir_limit = 100;
					var file_limit = 100;
					var dir_printed = 0;
					var file_printed = 0;
					
					var dir_more_node = false;
					var file_more_node = false;
					
					$.each(data.files, function(i, row) {
					
						var childNode = { };
						
						childNode.expanded   = false;
						childNode.expandable = row.expandable;
						childNode.iconCls = 'tree-icon-' + row.type;
						childNode.type = row.type;
						
						if(row.type == 'page' || row.type == 'application') {
						
							childNode.id = 'page_' + row.id;
							file_printed++;
							
							if(file_printed < file_limit) {
								var record = node.appendChild(childNode);
								that.updateRow(record, row);
							} else {
								if(!file_more_node) {
									file_more_node = { leaf: true, text: 'Show more files...', type: 'show_more_file' };
									file_more_node = node.appendChild(file_more_node);
									file_more_node.raw.more = [];
									file_more_node.raw.more.push(row);
								} else {
									file_more_node.raw.more.push(row);					
								}
							}
							
							
							
						} else { // Is a DIR
						
							childNode.id = row.text + '_' + Math.floor((Math.random()*1000)+1);
							dir_printed++;
							
							if(dir_printed < dir_limit) {
								var record = node.appendChild(childNode);
								that.updateRow(record, row);
							} else {
								if(!dir_more_node) {
									dir_more_node = { leaf: true, text: 'Show more directories...', type: 'show_more_dir' };
									dir_more_node = node.appendChild(dir_more_node);
									dir_more_node.raw.more = [];
									dir_more_node.raw.more.push(row);
								} else {
									dir_more_node.raw.more.push(row);							
								}
							}
							
						}

					});
					
					if(file_more_node) {
						file_more_node.set('text', 'Show more files (' + file_more_node.raw.more.length + ' not shown)');
					}
					if(dir_more_node) {
						dir_more_node.set('text', 'Show more directories (' + dir_more_node.raw.more.length + ' not shown)');
					}
					
				}

				// If its a Page -- Show Params
				if(node.data.type == 'page') {
				
					$.each(data.params, function(i, row) {
					
						var childNode = { };
						
						childNode.expanded   = false;
						childNode.type = 'attribute';
						childNode.id   = node.data.id + '_att_' + row.att_id;
						childNode.expandable = false;
						childNode.leaf = true;
						childNode.iconCls = 'tree-icon-attribute' + row.att_source;
						
						var record = node.appendChild(childNode);
						that.updateRow(record, row);
						
					});
				
				}

				// END DATA EXPAND
				
				if(node.childNodes.length == 0) {
					node.insertChild(0 , that.noResultsNode);
				}
				
				that.lastExpanded = node;
				
				Ext.resumeLayouts(true);
				
			}, 'Error Expanding Tree');
			
		break;
	}

}
