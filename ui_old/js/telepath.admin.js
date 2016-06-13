telepath.system = {
	
	init: function() {
		
		$(window).bind( 'telepath_config_loaded', function () {
			
			yepnope({
				load: [	
					"js/telepath.users.js" + get_ts(), 
					"js/telepath.groups.js" + get_ts(), 
					"js/telepath.permissions.js" + get_ts(), 
					"js/telepath.audit.js" + get_ts()
				],
				complete: function () {  
				
				var stores = ['users', 'groups', 'permissions', 'applications'];
				$.each(stores, function(i, store) {
					if(!Ext.StoreManager.lookup(store + 'Store')) {
						Ext.StoreManager.add(Ext.create('MyApp.store.' + store + 'Store'));
					}
				});

				var admin_tab = $('<div>Administration</div>').attr('id', 'telepath-start');
			
				$(".telepath-toolbar", telepath.config.container).prepend('<div class="btnseparator"></div>');
				$(".telepath-toolbar", telepath.config.container).prepend(admin_tab);

				telepath.system.render();
				
				/*setTimeout(function () {
					telepath.groups.showWindow();
					telepath.groups.window.hide();
					telepath.groups.window.show();
				}, 1000);*/
				
				/*setTimeout(function () {
					telepath.users.showWindow();
					telepath.users.window.hide();
					telepath.users.window.show();
				}, 1000);*/
				
				}
			});

		});
	
	},
	render: function() {
			
			var sysStartButton = $("#telepath-start");
			
			var tabs_container = $('<div>').addClass('telepath-toolbar'); //$(".telepath-toolbar #telepath-start", telepath.config.container);

			sysStartButton.append(tabs_container);

			sysStartButton.click(function () {
				$(this).addClass('expanded');
				tabs_container.toggle();
			}).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover'); });
			
			var menuItems = [
				{ id: 'users', text: 'Users' },
				{ id: 'groups', text: 'Groups' },
//				{ id: 'permissions', text: 'Permissions' },
				{ id: 'audit', text: 'Activity Log' },
			];
				
			// Tab Buttons
			$.each(menuItems, function(i, tab) {
				var className = '';
				if(tab.className) {
					className = tab.className;
				}
				$(tabs_container).append('<div id="btn-' + tab.id + '" class="tp-button' + className + '"><div class="tp-button-inner"><span>' + tab.text + '</span></div></div>');
			});
			
			tabs_container.hide();
			
			$(".telepath-toolbar .tp-button").click(function () {
			
				var id = $(this).attr('id').split('-')[1];
				
				switch(id) {
				
					case 'audit':
						
						yepnope({ load: [ telepath.basePath + '/js/telepath.audit.js?' + new Date().getTime() ], 
							complete: function () {
								telepath.audit.show(); 
							}
						});
						
					break;
					
					case 'users':
					
						telepath.users.showWindow();
						
					break;
					case 'groups':
						
						telepath.groups.showWindow();
						
					break;
					case 'permissions':
						
						telepath.permissions.showWindow();
						
					break;
					
				}
				
				tabs_container.hide();

			});
	
	}

}
telepath.system.init();