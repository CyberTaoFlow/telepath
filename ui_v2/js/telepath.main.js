/* BY NDK 2014 */

var SIDS = {};
var mask = {
	show: function () {	Ext.getCmp('main_panel').disable(); },
	hide: function () {	Ext.getCmp('main_panel').enable();	}
}

// --------------------------------- UI USAGE TIMER -----------------------------------

var idleTime = -1; // For the first loop its -1

$(document).ready(function () {

    //Increment the idle time counter every minute.
    var idleInterval = setInterval(timerIncrement, 60000); // 1 minute
	timerIncrement();
    //Zero the idle timer on mouse movement.
    $(this).mousemove(function (e) { idleTime = 0; });
    $(this).keypress(function (e) { idleTime = 0; });
	
});

function timerIncrement() {
	
	if(idleTime < 5) {
		telepath.util.request('/telepath/activity', { flag: 1 }, function (data) { }, function () {});
	} else {
		telepath.util.request('/telepath/activity', { flag: 0 }, function (data) { }, function () {});
	}
	
	idleTime = idleTime + 1;
	
}
// ------------------------------------------------------------------------------------


Ext.onReady(function () {
	
	Ext.Ajax.timeout = 60000;
	
	// STATIC COUNTRY STORE
	var countryArray = [];
	$.each(aliasToName, function(alias, name) {
		countryArray.push({ Alias: alias, Country: name });
	});
	
	var countryStores = ['b_geoInsideAll', 'b_geoOutsideAll', 'rw_geoInsideAll', 'rw_geoOutsideAll', 'Countries', 'b_geoInsideChosen', 'b_geoOutsideChosen'];
	
	window.countryArr = countryArray;
	
	$.each(countryStores, function (i, v) {
	
		var c_store = Ext.StoreManager.getByKey(v);
		c_store.setProxy = { 'type' : 'memory' };
		c_store.loadRawData(countryArray);
		
	});
	
	// INIT APPLICATIONS
	telepath.applications.init();
	
	// INIT STORE PATHS
	telepath.util.setupPaths();
	
	// GET GLOBAL HEADERS
	if(telepath.access.has_perm('Dashboard', 'get') || telepath.access.has_perm('Parameters', 'get')) {
		telepath.util.request('/parameters/get_global_headers', {}, function(data) { window.globalHeaders = data; }, 'Error getting global headers');
	}

	// START!
	telepath.main.init();
	
	// Bind Events
	Ext.getCmp('main_panel').tabBar.hide();
	telepath.main.bindToolBarEvents();
	
});

// Main
telepath.main = {
	
	init: function () {
		
		telepath.main.panels = telepath.access.panels;
		
		$(window).resize(function () {
			adjust_panel_sizes();
		});
		
		var container = $('#telepath-head .telepath-toolbar');

		container.append('<div class="telepath-title"></div>');
		
		$.each(this.panels, function(i, item) {
			
			var el = $('<div>').attr('id', 'btn-' + item.id).addClass('tp-button');
			var el_inner = $('<div>').addClass('tp-button-inner');
			var el_text  = $('<span>').html(item.title);
			
			el_inner.append(el_text);
			el.append(el_inner);
			
			if(item.active) { el.addClass('fcurrent'); }

			if(item.id == 'help') {
				
				var el_help = $('<div>').addClass('telepath-toolbar');
				
				el_help.append('<div id="btn-manual" class="tp-button"><div class="tp-button-inner"><span>User Guide</span></div></div>');
				el_help.append('<div id="btn-hybrid" class="tp-button"><div class="tp-button-inner"><span>HybridSec Website</span></div></div>');
				el_help.append('<div id="btn-about" class="tp-button"><div class="tp-button-inner"><span>About</span></div></div>');
				el_help.append('<div id="btn-user" class="tp-button"><div class="tp-button-inner"><span>User Settings</span></div></div>');
				
				
				el.append(el_help);
			
			}
			
			
			container.append(el);
			container.append('<div class="btnseparator"></div>');

		});
		
		container.append('<div id="telepath-statusBar"></div>');
		container.append('<div class="btnseparator"></div>');
		
		container.append('<div id="telepath-engineToggle"><div id="btn-system" class="tp-button"><div class="tp-button-inner"><span>On</span></div></div></div>');
		container.append('<div class="btnseparator"></div>');
		
		container.append('<div class="telepath-eta"><span class="tp-eta-lbl">Training Time Left:</span><span class="tp-eta-timer"></span></div>');
		container.append('<div class="btnseparator"></div>');
		
		container.append('<div id="telepath-logoutBtn"><div id="btn-logout" class="tp-button"><div class="tp-button-inner"><span>Logout</span></div></div></div>');
		container.append('<div class="clear"></div>');
				
		//telepath.debug.behavior();
		telepath.main.initPanels();
		
		// OPERATION MODE DISPLAY
		telepath.main.statusBar = Ext.create('Ext.form.field.Display', {
		
			xtype: 'displayfield',
			x: 0,
			y: 0,
			height: 30,
			id: 'd_operation',
			width: 120,
			fieldLabel: 'Mode',
			labelPad: 0,
			labelWidth: 45,
			preventMark: true,
			
		}).render("telepath-statusBar");
		
		this.tip = new Ext.ToolTip({
            target: 'd_operation',
            trackMouse: true,
            listeners: {
                beforeshow: function(tip) {
					tip.update('Loading ..');
					telepath.util.request('/config/get_config', { }, function(data) {
						tip.update('Learned ' + data.learning_so_far + ' requests'); 
					}, function () {});
                }
            }
        });
		
		
		
		
		
		// Interval status
		setInterval(function() { telepath.util.statusUpdate(); } , 30 * 1000);
		// Update status now
		telepath.util.statusUpdate();
			
	},
	initPanels: function () {
	
		var requires = [];
		var items    = [];
		
		$.each(telepath.main.panels, function(key, value) {
			if(value.ext) {
				items.push({ xtype: key });
				requires.push(value.ext);
			}
		});
		
		Ext.define('MyApp.view.main_panel', {
			extend: 'Ext.tab.Panel',
			requires: requires,
			border: 0,
			id: 'main_panel',
			width: 1024,
			manageHeight: false,
			activeTab: 0,
			initComponent: function() {
				var me = this;
				Ext.applyIf(me, {
					items: items,
					listeners: {
						tabchange: {
							fn: me.onMain_panelTabChange,
							scope: me
						}
						
					}
				});

				me.callParent(arguments);
			},
			onMain_panelTabChange: function(tabPanel, newCard, oldCard, eOpts) {
				main_panel_tab_change(tabPanel, newCard, oldCard, eOpts);
			}
		});

		$('body').attr('id', 'body').css('overflow', 'hidden');

		if(Ext.getCmp('MyApp.view.main_panel')) {
			telepath.main.panel = Ext.getCmp('MyApp.view.main_panel')
		} else {
			telepath.main.panel = Ext.create('MyApp.view.main_panel')
		}
		telepath.main.panel.render('telepath-main');
		
		setTimeout(function () {
		
			var id = $('#telepath-head .telepath-toolbar .fcurrent').attr('id').split('-')[1];
			if(id !== 'dashboard') {
				main_panel_tab_change(false, { id: id + 'Panel' }, { id: '' }, false);
			}

		}, 100);

	},
	bindToolBarEvents: function () {
		
		$("#telepath-head .telepath-toolbar .tp-button").click(function (e) {
		
		var id = $(this).attr('id').split('-')[1];
		
		switch(id) {
		
			case 'help':
			
			break;
			
			case 'hybrid':
				// HybridSec website
				window.open('http://hybridsec.com', '_blank');
				
			break;
			case 'manual':
				// User manual tab
				window.open(telepath.basePath + '/userguide.pdf', '_blank');
				/*
				new Ext.Window({
					title : "Telepath User Guide",
					width : 980,
					height: 700,
					layout : 'fit',
					items : [{
						xtype : "component",
						autoEl : {
							tag : "iframe",
							src : "userguide.html"
						}
					}]
				}).show();
				*/
				
			break;
			case 'about':
			
				// Version information
				telepath.about_window = Ext.create('MyApp.view.About');
				telepath.about_window.show();
				var about_mask = new Ext.LoadMask(telepath.about_window, {useMsg: false});
				about_mask.show();
				
				telepath.util.request('/telepath/get_about', {}, function(data) { 
					
					Ext.getCmp('c_soft_update_date').setValue(date_format('d/m/y H:i:s', data.last_updated_date));
					Ext.getCmp('c_soft_engine_revision').setValue(data.engine_version);
					
					about_mask.hide();
					
				}, 'Error getting about information');
				
			break;
			
			case 'system':
				
				if(telepath.access.has_perm('Telepath', 'set')) {
				
				var op = 'start';
				
				if(telepath.engine.status) { op = 'stop'; }
				
				telepath.util.request('/telepath/' + op, {}, function(data) { 
					telepath.util.statusUpdate();
				}, 'Error ' + (op == 'start' ? 'starting' : 'stopping') + ' telepath.');
				
				} else {
					
					Ext.MessageBox.alert('Access Denied', 'You are not authorized to start/stop telepath.');
					
				}
				
			break;
			
			case 'logout':
				
				telepath.util.request('/auth/logout', {}, function(data) { 
					location.reload(true);
				}, 'Error logging out.');

			break;
			
			case 'user':
				
				if(!telepath.user) {
					yepnope({
						load: ["js/telepath.user.js"],
						complete: function () { telepath.user.showWindow();	}
					});
				} else {
					telepath.user.showWindow();
				}
			
			break;
			
			default:
				Ext.getCmp(id + 'Panel').show();
			break;
		
		
		}
		
		}).hover(function () {
		
			var id = $(this).attr('id').split('-')[1];
			
			switch(id) {
			
				case 'help':
					$('.telepath-toolbar', this).show();
				break;
			
			}
			
		}, function () {
			
			if($(this).parents('.telepath-toolbar').size() == 1) {
				$('.tp-button .telepath-toolbar').not(this).hide();
			}

		});
	
	}
	
};

telepath.debug = {

	alerts: function () {
		telepath.main.panels = { alerts: 'MyApp.view.Alerts' };
		telepath.main.initPanels();
		
		setTimeout(function () {
			main_panel_tab_change(false, { id: 'alertsPanel' }, { id: '' }, false);		
		}, 500);
		
	},
	investigate: function () {
		telepath.main.panels = { investigate: 'MyApp.view.Investigate' };
		telepath.main.initPanels();
		main_panel_tab_change(false, { id: 'investigatePanel' }, { id: '' }, false);		
	},
	workflow: function () {
		telepath.main.panels = { workflow: 'MyApp.view.Workflow' };
		telepath.main.initPanels();
		main_panel_tab_change(false, { id: 'workflowsPanel' }, { id: '' }, false);
	},
	behavior: function () {
		telepath.main.panels = { behavior: 'MyApp.view.Behavior' };
		telepath.main.initPanels();
		main_panel_tab_change(false, { id: 'behavioralPanel' }, { id: '' }, false);
		
		/*
		yepnope({
			load: [
				"lib/jsondiff/jsondiffpatch.html.css",
				"lib/jsondiff/jsondiffpatch.js",
				"lib/jsondiff/jsondiffpatch.html.js",
				"lib/jsondiff/diff_match_patch_uncompressed.js",
			],
			complete: function () {
				var diffEl = $('<div>');
				diffEl.css({ position: 'absolute', height: 300, width: 300, top: 0, right: 0, padding: 10, border: '1px solid green', background: 'white', zIndex: 1000 }).attr('id', 'jsonDiff'); 
				$('body').append(diffEl);
			}
		});
		*/
		
	},
	config: function () {
		telepath.main.panels = { configuration: 'MyApp.view.Configuration' };
		telepath.main.initPanels();
		main_panel_tab_change(false, { id: 'configPanel' }, { id: '' }, false);
		
		setTimeout(function () {
			telepath.scheduler.displaySchedule("c_schedulerStore");
		}, 2000);
		
	}
	
}
