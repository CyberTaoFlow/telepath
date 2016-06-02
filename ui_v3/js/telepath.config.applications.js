telepath.config.applications = {

	sort: 'name',
	dir: true,

	formatSearchData: function(data) {

		var treeData = [];
		$.each(data, function(i, row) {

			var children = false;

			/*if(row.type == 'app' || row.type == 'page') {
				children = true;
			}*/
			if(row.type == 'app') {
				children = true;
			}

			var obj  = { children: children, name: row.text, text: row.text, data: row, 'icon': 'tele-icon-' + row.type};

			if(row.type == 'page') {
				obj.text = row.host + obj.text;
			}
			if(row.type == 'param') {
				obj.data.host = row.host;
				obj.data.uri = row.uri;
				obj.text = row.host + row.uri + " - " + row.text + "&nbsp;(" + row.param_type + ")";
			}

			treeData.push(obj);

		});
		return treeData;

	},
	formatData: function(data,count, root) {
		if(!root) { root = '' }
		// Recurse for children, use json key as text to display
		if(!data) return;
		count = count !== false;
		var that = this;
		var treeData = [];
		$.each(data, function(i, row) {
			var text = row.host;
			var children = false;
			if (count) {
			//	text += '&nbsp;(' + row.learning_so_far + ')';
				children = (typeof row.subdomains != "undefined" && row.subdomains != null && row.subdomains.length > 0) ? row.subdomains : false;
			}

			var obj = {
				children: children,
				state: {opened: (typeof row.open != "undefined" && row.open) ? true : false},
				text: text,
				data: {type: 'app', host: row.host, count: row.learning_so_far },
				icon: 'tele-icon-app'
			};
			treeData.push(obj);
		});
		return treeData;

	},
	formatDataPages: function(data, i, host) {

		if(!data) { return; }

		var that = this;
		var treeData = [];
		$.each(data, function(i, row) {

			if(typeof(row) == 'object') {

				// EXPAND = LEVEL
				var obj  = { children: that.formatDataPages(row, i, host), text: i, data: { type: 'dir', text: i }, 'icon': 'tele-icon-dir'};
				treeData.push(obj);

			} else {

				var obj  = { children: true, text: i, data: { type: 'page', path: row, host: host }, 'icon': 'tele-icon-page'};
				treeData.push(obj);
				// EXPAND = PARAM
			}

		});
		return treeData;

	},
	expand: function(obj, callback) {

		var that = this;

		telepath.ds.get('/applications/get_expand', { search: telepath.config.applications.searchString, context: 'applications', learning_so_far: true, sort: telepath.config.applications.sort, dir: telepath.config.applications.dir }, function(data) {

			var treeData = telepath.config.applications.formatData(data.items);

			callback.call(that, treeData);
			$(".tele-search-input").attr("disabled", false);
		});
	
	},

	//change search icon and get result
	input: function(){
		var that = this;
		var icon= $("#search-button");
		if (that.searchString.length>0)
			icon.addClass('icon-delete-input2').removeClass("tele-search-button");
		else
			icon.removeClass('icon-delete-input2').addClass("tele-search-button");

		that.reload();

		//that.appTree.jstree('search', that.searchString);

	},
	data:[],
	reload: function () {

		var that = this;


		that.appTree = $('<div>');

		that.contentLeftWrap = $('<div>').css({ padding: 0, height: $(that.contentLeft).parent().height() - 20 });
		that.contentLeft.empty().append(that.contentLeftWrap);
		that.contentLeftWrap.append(that.appTree);

		$(that.contentLeftWrap).mCustomScrollbar({
			scrollButtons:{	enable: false },
			scrollInertia: 150,
			advanced: {
				updateOnContentResize: true
			}
		});

		that.data=telepath.config.applications.expand;

		that.appTree.jstree({
		core : { data : that.data, progressive_render: true },
		plugins: ["json_data","wholerow", "theme", "grid", "contextmenu", "search"],
		contextmenu: { items: telepath.contextMenu },
		grid: {
			columns: [
				{ width: 320 },
				{ width: 50, value: "count", cellClass: "learning-so-far" },
				{ value: function (node) {
					return $('<div>').btn({ icon: 'edit', callback: function (tree) {
						$nodeParent = tree.element.parents('[role="treeitem"]');

						telepath.config.application.editApp(node.host, $nodeParent);
					}});

				}, width: 40 },
				{value: function (node) {
					return $('<div>').btn({ icon: 'delete', callback: function (tree) {

						$nodeParent = tree.element.parents('[role="treeitem"]');

						telepath.config.application.deleteApp(node.host, $nodeParent)
					}});

				}, width: 40 }
			],
			resizable:true
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
			// console.log('App Changed');
			// console.log(data);
			if(data && data.node) {
				data.instance.element.find('.jstree-wholerow').css('background-color', '#FFFFFF');
				data.instance.element.find('.jstree-wholerow-hovered').css("background-color", "rgba(189, 189, 189, 0.85)");
				telepath.config.application.editApp(data.node.data.host);
			}
		}).on('hover_node.jstree',function(e,data){
			$("#"+data.node.id +' a').prop('title', data.node.text);
			$("#"+data.node.id+' .learning-so-far' ).prop('title', 'Overall Transactions');
		})
		/*.on('ready.jstree', function(e, data) {
			data.instance.search(that.searchString);
		});*/

	},

	init: function () {
		this.data=null;
		this.initTools();
		$(".tele-config-bar-left .tele-search-input").attr("disabled", true);
		this.reload();
	},
	initTools: function() {

		var that = this;

		// Sort filters
		var sortRadios = $('<div>').radios({
			title: 'Sort By',
			items: [
				{id: 'name', icon: 'arrow', tip: 'ABC'},
				{id: 'count', icon: 'bars', tip: 'Count'}
			],
			selected: this.sort,
			callback: function(e, id) {
				if(that.sort == id) {
					that.dir = !that.dir;
				}
				that.sort = id;
				that.reload();
			}
		});

		var rightPanel=$('<div>').attr('id', 'sort-radio').css('float','right').append(sortRadios);
		$('.tele-panel-topbar').append(rightPanel);

		// Search
		this.search = $('<div>').teleSearch({ callback: function (e, txt) {
			that.searchString = txt;

		}});



		// Create
		this.create     = $('<div>').btn({ icon: 'plus', text: 'New', callback: function () {
			telepath.config.application.createApp();
		} });

		this.barLeft.append(this.search).append(this.create);

		var typingTimer;                //timer identifier
		var doneTypingInterval = 1000;

		$(".tele-config-bar-left .tele-search-input").keyup('input', function () {
			clearTimeout(typingTimer);
			if ($(".tele-config-bar-left .tele-search-input").val()){
				typingTimer = setTimeout(function(){
					that.searchString  = $(".tele-config-bar-left .tele-search-input").val();
					that.input();
				}, doneTypingInterval);
			}
		});

		$("#search-button").on("click", function (event) {
			that.searchString = '';
			$(".tele-config-bar-left .tele-search-input").prop("value", that.searchString);
			that.input();

		});

		if (typeof that.searchString != 'undefined'){
			$(".tele-config-bar-left .tele-search-input").prop("value", that.searchString);
			that.input();
		}
	}
};

