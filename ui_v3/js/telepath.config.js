telepath.config = {
	
	loaded: false,
	init: function () {
		
		// console.log('Configuration online');
		this.outerContainer = $('.tele-panel-config');
		this.initTabs();

	},
	initTabs: function() {
	
		var that = this;
		// remove canvas for loading map of countries (Yuli)
		$('canvas').remove();	

		this.panelTopBar = $('<div>').addClass('tele-panel-topbar');
		this.outerContainer.append(this.panelTopBar);
		
		this.tabs = [];

		if(telepath.access.admin) {
			this.tabs.push({ icon: 'accounts', text: 'Telepath Users' });
		}
		
		//this.tabs.push({ icon: 'notifications', text: 'Notifications' });
		
		if(telepath.access.admin || telepath.access.perm.Applications_get) {
			this.tabs.push({ icon: 'applications', text: 'Applications' });
		}
		
		if(telepath.access.admin || telepath.access.perm.Rules_get) {
			this.tabs.push({ icon: 'rules', text: 'Rules' });
		}
		
		if(telepath.access.admin || telepath.access.perm.Workflow_get) {
			this.tabs.push({ icon: 'actions', text: 'Business Actions' });
		}

		/*if(telepath.access.admin || telepath.access.perm.WebUsers_get) {
			this.tabs.push({ icon: 'users', text: 'Web Users' });
		}
*/
		if(telepath.access.admin || telepath.access.perm.Config_get) {
			this.tabs.push({ icon: 'system', text: 'Advanced' });
		}
		
		for(x in this.tabs) {
			var tab   = this.tabs[x];
			tab.callback = function(widget) {
			
				$('.tele-config-tab .active').removeClass('active');
				$('.tele-icon', widget.element).addClass('active');
				$('.tele-button-text', widget.element).addClass('active');
				
				var id = widget.options.icon;
				
				that.currentPanel = id;
				
				// Cleanup container
				that.container.empty();
				$("#file-upload").hide();
				
				// Pass container and shared layout function
				telepath.config[id].container  = that.container;
				telepath.config[id].initLayout = that.initLayout;
				telepath.config[id].resizeLayout = that.resizeLayout;
				
				// Init Panel
				telepath.config[id].initLayout();
				telepath.config[id].resizeLayout();
				telepath.config[id].init();
				
				$(window).resize(function () {
					telepath.config[id].resizeLayout();
				});
				
			}
			var tabBtn = $('<div>').btn(tab).addClass('tele-config-tab');
			this.panelTopBar.append(tabBtn);
			this.panelTopBar.append('<div class="tele-navsep"></div>'); // Sep
		}
		
		$('.tele-navsep:last', this.panelTopBar).remove();
		
		this.container = $('<div>').addClass('tele-panel-config-inner');
		this.outerContainer.append(this.container);
		this.container.append(telepath.loader);
		telepath.config.load();
		
	},
	load: function (dont_start) {
		
		/*
		if(this.loaded) {
			if(dont_start) { return; }
			this.start();
			return;
		}
		
		yepnope({ 
			load: [
				// Config Scripts
				"js/telepath.config.account.js",
				"js/telepath.config.accounts.js",
				// Action
				"js/telepath.config.action.js",
				"js/telepath.config.actions.js",
				// Application
				"js/telepath.config.application.js?",
				"js/telepath.config.applications.js",
				"js/telepath.config.notifications.js",
				// Rule
				"js/telepath.config.rule.js",
				"js/telepath.config.rules.js",
				// Users
				"js/telepath.config.user.js",
				"js/telepath.config.users.js",
				"js/telepath.config.groups.js",
				// System
				"js/telepath.config.system.js",

			], 
			complete: function() {
				telepath.config.loaded = true;
				
				if(dont_start) { return; }
				
				telepath.config.start();
		}});
		*/
		
		this.start();
		
	},
	start: function() {
		var that = this;
		setTimeout(function () {
			that.container.empty();
			$('.tele-button.tele-config-tab:nth-child(1) .tele-button-text').trigger('click');
		}, 500);
		
	},
	initLayout: function () {
		
		this.barEl		  = $('<div>').addClass('tele-config-bar').addClass('tele-panel-subtitle');
		this.barLeft 	  = $('<div>').addClass('tele-config-bar-left');
		this.barRight 	  = $('<div>').addClass('tele-config-bar-right');
		this.contentEl    = $('<div>').addClass('tele-config-content');
		this.contentLeft  = $('<div>').addClass('tele-config-content-left');
		this.contentRight = $('<div>').addClass('tele-config-content-right');
		
		this.barEl.append(this.barLeft).append(this.barRight);
		this.contentEl.append(this.contentLeft).append(this.contentRight);
		
		this.container.append(this.barEl).append(this.contentEl);
		
	},
	resizeLayout: function() {
		
		// console.log('Settings Resize');
		//console.log(this);
		
		this.barEl.css({ height: 40, width: '100%', background: '#333333' });
		
		var height = $(window).height();
		var width  = $(window).width();
		
		if(width < 1200) {
			
			if(width < 1024) {
				$('.tele-config-tab').css({ margin: '5px 10px 0px 10px' });
			} else {
				$('.tele-config-tab').css({ margin: '5px 20px 0px 20px' });
			}
			
		} else {
			$('.tele-config-tab').css({ margin: '5px 30px 0px 30px' });
		}
		
		$('.tele-body').css({ height: height });
		
		var offset = height - 
					 $('.tele-header').outerHeight() - 
					 $('.tele-panel-topbar').outerHeight() - 
					 $('.tele-panel-subtitle').outerHeight();
		
		$('.tele-block').height(offset - 20);
		$('.tele-block .tele-list').height(offset - 50);
		$('.tele-list').mCustomScrollbar("update");
		
		this.contentEl.css({ height: offset });
		
		if(this.contentRight.hasClass('tele-stacked')) {
			
			this.contentLeft.width(width);
			this.contentRight.width(width);
			this.contentRight.css({ height: offset - this.contentLeft.outerHeight() - 20 });
		
		} else {
		
			var magic = 520;
			
			this.contentLeft.width(magic);
			this.contentRight.width(width - magic - 20);
			
			this.contentLeft.css({ height: offset });
			$('.tele-tree', this.contentLeft).css({ height: offset }).mCustomScrollbar("update");
			
			this.contentRight.css({ height: offset });
			
		}
		
		
		this.barLeft.width(magic);
		this.barRight.width(width - magic - 20);
		
	},
	// return if the search value start with the value
	startsWith2: function(str, prefix) {
		if (str.length < prefix.length)
			return false;
		for (var i = prefix.length - 1; (i >= 0) && (str[i] === prefix[i]); --i) {
			continue;
		}
		return i < 0;
	}
}
