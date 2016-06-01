telepath.config.actions = {
	
	list: [],
	getList: function() {
		
		var that = this;
		telepath.ds.get('/workflow/groups_get_general_cb', {}, function(data) {
			that.list = data.items;
		});
		
	},
	expand: function(obj, callback) {
		
		var that = this;
		
		var type = obj.data && obj.data.type ? obj.data.type : 'root';
		var url  = '';
		var postData = { };
		
		switch(type) {
			default:
			case 'root':
				url  = '/applications/get_expand';
				postData.type = 'root';
				postData.actions = true;
				if(telepath.config.actions.searchString) { postData.search = telepath.config.actions.searchString; }
			break;
			case 'app':
				url = '/actions/get_app_actions';
				postData.host    = obj.data.id;
				postData.type    = 'application';
				postData.context = 'actions';
				postData.domain = obj.data.domain;
			break;
		}
		
		telepath.ds.get(url, postData, function(data) {

			var treeData = [];
			
			// Validate data, return if null
			if(!data.items || data.items.length == 0) {
				callback.call(that, treeData);
				$(telepath.config.actions.contentLeftWrap).mCustomScrollbar('update');
				return;
			}
			
			data = data.items;
			
			switch(postData.type) {
			
				case 'root':
				
					$.each(data, function(i, row) {
					
						var text = row.host;
						//var children = [{ children: true, text: 'Root Domain', data: {id: row.host, type: 'app', ssl: row.ssl_flag }}];
						//if (typeof row.subdomains != "undefined" && row.subdomains != null && row.subdomains.length > 0){
						//	$.each(row.subdomains, function(i,subdomain){
						//		var sub = { children: true, text: subdomain, data: {id: subdomain, type: 'app' }};
						//		children.push(sub);
						//	})
						//}
						//var obj = { children: children, text: text};
						var childrens = [];
						if (typeof row.actions != "undefined" && row.actions != null && row.actions.length > 0) {

							$.each(row.actions, function (i, action) {
								var flow_obj = {
									children: false,
									text: action.action_name,
									icon: "tele-icon tele-icon-actions",
									data: {id: action.action_name, type: "action", raw: action}
								};
								childrens.push(flow_obj);
							});
						}

						if (typeof row.subdomains != "undefined" && row.subdomains != null && row.subdomains.length > 0 && typeof row.open != "undefined" && row.open) {
							$.each(row.subdomains, function (i, subdomain) {
								var flow_obj = {
									children: true,
									text: subdomain,
									data: {id: subdomain, type: "app", domain: "subdomain"}
								};
								childrens.push(flow_obj);
							});

						}

						var obj = {
							children: (childrens.length > 0) ? childrens : true,
							state: {opened: (childrens.length > 0) ? true : false},
							text: text,
							data: {id: row.host, type: 'app', ssl: row.ssl_flag, domain: "root"}
						}
						treeData.push(obj);
						
					});
					
				break;
				case 'application':
					
					/*if(data.action_categories.length > 0) {
					
						$.each(data.action_categories, function(i, row) {
						
							var obj = { children: false, text: row.name, data: {id: row.name, type: "action_category" }};
							
							if(row.expandable) {
							
								obj.children = [];
								
								$.each(data.actions, function(i, flow) {
									
									if(flow.category == row.name) {
										var flow_obj = { children: false, text: flow.group, data: {id: flow.id, type: "action" }};
										obj.children.push(flow_obj);
									}

								});
								
							}
							
							treeData.push(obj);
							
						});
					
					}*/
					
					if(data.actions.length > 0) {
					
						$.each(data.actions, function(i, action) {
							var flow_obj = { children: false, text: action.action_name, icon:"tele-icon tele-icon-actions",data: {id: action.action_name, type: "action", raw: action }};
							treeData.push(flow_obj);
						});
					
					}
					if(data.subdomains.length > 0) {
						$.each(data.subdomains, function(i, subdomain) {
							var flow_obj = { children: true, text: subdomain, data: {id: subdomain, type: "app", domain: "subdomain" }};
							treeData.push(flow_obj);
						});

					}
				
				break;
				default:
					//
				break;
			}
			

			callback.call(that, treeData);
			$(telepath.config.actions.contentLeftWrap).mCustomScrollbar('update');
			
		});
		
	},
	data: [],
	deleteFlow: function(node) {
		
		var that = this;
		
		telepath.ds.get('/actions/set_delete_action', {
			
			uid: node.uid,
			action: node.action_name,
			application: node.application
			
		}, function(data) {
			
			that.reload();
			
		});
				
	},

	input: function(){
		var that = this;
		var icon= $("#search-button");
		if (that.searchString.length>0)
			icon.addClass('icon-delete-input2').removeClass("tele-search-button");
		else
			icon.removeClass('icon-delete-input2').addClass("tele-search-button");

		that.reload();
		//that.ruleTree.jstree('search', that.searchString);

	},

	reload: function () {
		
		if(telepath.action.recorder.timer) {
			clearTimeout(telepath.action.recorder.timer);
		}

		var that = this;

		/*$("#search-button").on("click", function (event) {
			that.searchString = '';
			$(".tele-config-bar-left .tele-search-input").prop("value", that.searchString);
			that.input();

		});


		if (typeof that.searchString != 'undefined'){
			$(".tele-config-bar-left .tele-search-input").prop("value", that.searchString);
			that.input();
		}


		// add search on client site on key up event
		$(".tele-config-bar-left .tele-search-input").keyup('input', function () {
			that.searchString = $(this).val();
			console.log(that.searchString);
			that.input();
		});*/

		that.ruleTree = $('<div>');
	
		that.ruleTree.jstree({
		core : { data : telepath.config.actions.expand },
		plugins: ["json_data","wholerow", "theme", "grid", "search"],
		grid: {
			columns: [
				{width: 370 },
				{value: function (node) {
					/*
					return $('<div>').btn({ icon: 'edit', callback: function () {
						telepath.config.actions.editCat(node.id);
					}});
					*/
					
				}, width: 40 },
				{value: function (node) {

					if(node.type == 'action') {
						return $('<div>').btn({ icon: 'delete', callback: function () {
							telepath.config.actions.deleteFlow(node.raw);
						}});
					}


				}, width: 40 }
			],
			resizable:true,
		}/*,
			search: {
				"fuzzy":false,
				"case_insensitive": true,
				"show_only_matches" : true,
				search_callback : function (str, node) {
					if(node.text === str) { return true; }
				}
			}*/
		}).on('changed.jstree', function (e, data) {

			data.instance.element.find('.jstree-wholerow').css('background-color', '#FFFFFF');
			data.instance.element.find('.jstree-wholerow-hovered').css("background-color", "rgba(189, 189, 189, 0.85)");

			telepath.config.actions.contentRight.empty();
			telepath.config.actions.barRight.empty();
			telepath.config.actions.createCat.hide();
			
			if(data.node.data.type == 'action') {
				telepath.config.action.editAction(data.node.data.raw);
			}
			if(data.node.data.type == 'app') {
				telepath.action.currentApp = data.node.data.id;
				telepath.action.recorder.init();
				telepath.config.actions.createCat.show();
			}

		})/*.on('ready.jstree', function(e, data) {
			data.instance.search(that.searchString);
		});*/
		
		//that.contentLeftWrap = $('<div>');
		//that.contentLeft.empty().append(that.contentLeftWrap);
		//that.contentLeftWrap
	
		that.contentLeft.empty().append(that.ruleTree);
	
		$(that.contentLeft).mCustomScrollbar({
			scrollButtons:{	enable: false },
			scrollInertia: 150,
			advanced: {
				updateOnContentResize: true
			}
		});
				
	},
	init: function () {
		this.data=null;
		this.initTools();
		this.reload();
	},
	initTools: function() {	
		
		var that = this;
		
		// Search
		this.search = $('<div>').teleSearch({ callback: function (e, txt) {
			that.searchString = txt;
			//that.reload();
		}});
		
		// Create
		that.createCat = $('<div>').btn({ icon: 'plus', text: 'New Category', callback: function () {
			
			telepath.dialog({ msg: 'Enter new name for action category', type: 'prompt', callback: function(name) {
			
				
				
			}});
			
		}}).hide();
				
		this.barLeft.append(this.search).append(this.createCat);

		$("#search-button").on("click", function (event) {
			that.searchString = '';
			$(".tele-config-bar-left .tele-search-input").prop("value", that.searchString);
			that.input();

		});


		if (typeof that.searchString != 'undefined'){
			$(".tele-config-bar-left .tele-search-input").prop("value", that.searchString);
			that.input();
		}

		var typingTimer;                //timer identifier
		var doneTypingInterval = 1000;

		// add search on client site on key up event
		$(".tele-config-bar-left .tele-search-input").keyup('input', function () {
			clearTimeout(typingTimer);
			if ($(".tele-config-bar-left .tele-search-input").val()){
				typingTimer = setTimeout(function(){
					that.searchString= $(".tele-config-bar-left .tele-search-input").val();
					that.input();
				}, doneTypingInterval);
			}
		});
	}
}