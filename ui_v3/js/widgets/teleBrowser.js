$.widget( "tele.teleBrowser", {
	
	searchText: '',
    options: {
		mode: 'page',
		root: { type: 'root', id: -1 },
		callback: function(e, data) {},
		grid: {
			columns: [
				{ width: 380 }
			],
			resizable:true
		}
    },
    _create: function() {
        this.element.addClass( "tele-browser" );
        this._update();
	},
     _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
	expand: function(obj, callback, widget) {
		
		var that = this;
		
		// Predefined root
		if(widget && widget.options && widget.options.root && widget.options.root.type == 'application' && obj.id == '#') {
			
			// EXPANDING APP , SHOW PAGES
			
			telepath.ds.get('/applications/get_page', { host: widget.options.root.id, path: "", mode: that.options.mode }, function(data) {
				var host = '';
				var treeData = [];
				if (obj && obj.data && obj.data.host)
				{
					host = obj.data.host;
				} else {
					host = that.options.root.id;
				}
				if(data.items) {
					$.each(data.items, function(i, item) {
					if (item.type != 'page' && item.type != 'dir')
					{
						item.type = 'param';
					}
					treeData.push({ children: false, text: escapeHtml(item.name), icon: 'tele-icon-' + item.type, data: item });
					});
				}

				callback.call(that, treeData);

			});
			return;

		}

		var searchTerm = $('input', window.teleBrowseSearch).val();

		// Perform search
		if(searchTerm != '' && $('input', window.teleBrowseSearch).attr('complete') != 'true') {

			telepath.ds.get('/applications/get_search', { search: searchTerm, /*context: 'applications',*/ mode: that.options.mode  }, function(data) {

					// EXPANDING SEARCH , SHOW MIXED

					var treeData = telepath.config.applications.formatSearchData(data.items);
					callback.call(that, treeData);
					$('input', window.teleBrowseSearch).attr('complete', 'true');


			});

			return;

		}

		if(obj.id == '#') {

			telepath.ds.get('/applications/get_expand', { context: 'applications' }, function(data) {

				// EXPANDING ROOT , SHOW APPS

				var treeData = telepath.config.applications.formatData(data.items,false);

				function enable_expand(treeData) {
					$.each(treeData, function(i, row) {
						if(row.children && row.children.length > 0) {
							enable_expand(row.children);
						} else {
							row.children = true;
						}
					});
				}
				enable_expand(treeData);

				if(that.options.mode == 'param') {
					treeData.unshift({ icon: 'tele-icon-global', text: 'Global Headers', children: true, data: { 'type': 'global' } });
				}

				callback.call(that, treeData);

			});

		} else { // Expanding paths

			if(obj.data.type == 'global') {

				var treeData = [];

				$.each(telepath.global_headers, function (i, x) {
					treeData.push({ children: false, text: x , icon: 'tele-icon-param', data: { type: 'param', global: true, name: x } });
				});

				callback.call(that, treeData);

				return;

			}
			if(obj.data.type == 'app') {

				if (!obj.data.host) {
					obj.data.host = obj.data.text;
				}
				telepath.ds.get('/applications/get_app_pages', { host: obj.data.host }, function(data) {


					var treeData = telepath.config.applications.formatDataPages(data.items, '/', obj.data.host);

					function enable_expand(treeData) {
						$.each(treeData, function(i, row) {
							if(row.children && row.children.length > 0) {
								enable_expand(row.children);
							} else {
								if (that.options.mode == 'page')
									row.children = false;
								else
									row.children = true;
							}
						});
					}
					enable_expand(treeData);

					callback.call(that, treeData);

				});
			}

			if(obj.data.type == 'page' && that.options.mode != 'page') {



				// EXPANDING PAGE , SHOW PARAMS

				// console.log('NEED TO EXPAND A PAGE');

				telepath.ds.get('/applications/get_page', { host: obj.data.host, path: obj.data.path, mode: that.options.mode }, function(data) {

					var treeData = [];

					if(data.items.length == 0) {
						//callback.call(that, []);
						//return;
						treeData.push({ children: false, text: "No parameters" , icon: 'tele-icon-param' });

					}
					else if(data.items) {
						$.each(data.items, function(i, item) {
							item.type = 'param';
							treeData.push({ children: false, text: escapeHtml(item.name) , icon: 'tele-icon-param', data: item });
						});
					}

					callback.call(that, treeData);

				});


			}

			//else {
			//
			//	// EXPANDING APP , SHOW PAGES
			//	// fixing bug Yuli
			//	if (!obj.data.host) {
			//		obj.data.host = obj.data.text;
			//	}
            //
			//	// Trying to fix bug. get_pages instead of get_app. Yuli
			//	telepath.ds.get('/applications/get_page', { host: obj.data.host, path: "", mode: that.options.mode }, function(data) {
			//
			//		//var treeData = telepath.config.applications.formatDataPages(data.items, '/', obj.data.host);
			//		//callback.call(that, treeData);
			//		var treeData = [];
            //
			//		if(data.items.length == 0) {
			//			callback.call(that, []);
			//			return;
			//		}
            //
			//		if(data.items) {
			//			$.each(data.items, function(i, item) {
			//				if (item.type != 'page' && item.type != 'dir')
			//				{
			//					item.type = 'param';
			//				}
			//				treeData.push({ children: false, text: item.name, icon: 'tele-icon-' + item.type, data: item });
			//			});
			//		}
            //
             //                           callback.call(that, treeData);
			//
			//	});
			//
			//}
			
			
		
		}
		
		/*
		// Setup
		this.searchText = $('.tele-search-input', this.search).val();
		
		var that = this;
		var treeType = this.options.type;
		var treeObj  = $.jstree.reference(obj.parent);
		var nodeType = obj.data && obj.data.type ? obj.data.type : this.options.root.type;
		var postData = { id: obj.data ? obj.data.id : this.options.root.id, type: nodeType, text: obj.text, search: this.searchText };
		var postUrl  = '/applications/get_expand';
				
		if(nodeType == 'directory') {
			for(x in obj.parents) {
				var p_id  = obj.parents[x];
				var p_obj = treeObj.get_node(p_id);
				if(p_obj.data.type == 'application' || p_obj.data.type == 'subdomain') {
					postData.id = p_obj.data.id;
					break;
				} else {
					postData.text = p_obj.text + '/' + postData.text;
				}
			}
		}
		
		
		// Get
		
		telepath.ds.get(postUrl, postData, function(data) {
			var treeData = [];
			$.each(data.items, function(i, row) {
				
				if(postData.search == '') {
				
					var obj = { children: true, text: row.name, data: { id: row.id , raw: row }};
					switch(postData.type) {
						case 'root':
							obj.data.type = 'application';
							obj.text = row.alias != '' ? row.alias : row.name;
						break;
						case 'application':
							obj.data.type = 'subdomain';
						break;
						case 'subdomain':
						case 'directory':
						
							obj.data.type = row.type;
							obj.data.raw.alias = row.title;
							obj.text = row.title && row.title != '' ? row.title : row.text;
							
						break;
						case 'page':
							obj.data.id = row.att_id;
							obj.text = row.att_alias != '' ? row.att_alias : row.att_name;
							obj.data.raw.alias = row.att_alias;
							obj.children = false;
							obj.data.type = 'param';
						break;
					}
					
					obj.icon = 'tele-icon-' + obj.data.type;
					treeData.push(obj);
					
				} else {
				
					treeData.push({ 
						children: false, 
						icon: 'tele-icon-' + row.type,
						text: row.name, 
						data: { id: row.id, type: row.type }
					});
				
				}
				
			});
			
			callback.call(that, treeData);
			
		});
		*/
		
	},	
    _update: function() {

		// Init
		var that = this;
		
		this.search = $('<div>').teleSearch({ callback: function(val) {
			$('input', window.teleBrowseSearch).attr('complete', 'false');
			that.teleTree.data('jstree').close_all();
			that.teleTree.data('jstree').refresh();
			that.searchText = val;
		}});
		
		window.teleBrowseSearch = this.search;
		
		this.element.parent().parent().find('.tele-overlay-header').append(this.search);
		
		this.teleTree = $('<div>');
		this.treeCont = $('<div>').css({ clear: 'both', height: 400, overflow: 'auto' });
		
		this.treeCont.append(this.teleTree);
		this.element.append(this.treeCont);
		
		// Start Tree
		this.teleTree.jstree({
			core: { 
				check_callback: true,
				data: function(obj, callback) {
					that.expand(obj, callback, that);
				},
				reopen: true,
				progressive_render: true
			},
			plugins: ["json_data","wholerow", "theme", "contextmenu"],
			//grid: that.options.grid,
			contextmenu: { items: telepath.contextMenu }
		}).on('changed.jstree', function (e, data) {
			that.options.callback(e, data);
		}).on('hover_node.jstree', function (e, data) {
			$('#' + data.node.id).addClass('hover');
		}).on('dehover_node.jstree', function (e, data) {
			$('#' + data.node.id).removeClass('hover');
		}).on('open_node.jstree close_node.jstree', function (e,data) {
			if(e.type === "open_node") {
				//var node = data.node;
				//var tree = that.teleTree.data('jstree').refresh(node);
			}
		});
		
		$(this.treeCont).mCustomScrollbar({
			scrollButtons:{	enable: false },
			scrollInertia: 150,
			advanced: {
				updateOnContentResize: true
			}   
		});
		
    }

});
