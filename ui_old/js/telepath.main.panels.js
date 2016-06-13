var adjustClock = false;
var oldWorkflow = true;

function main_panel_tab_change(tabPanel, newCard, oldCard, eOpts) {

	adjust_panel_sizes();
	
	$(".telepath-toolbar .tp-button.fcurrent").removeClass('fcurrent');
	
	var buttonId = '#btn-' + newCard.id.substr(0, newCard.id.length - 5);
	$(buttonId).addClass('fcurrent');
	
	switch(newCard.id) {
		
		case 'workflowsPanel':
			
			if(!telepath.workflow) {
			
				wf_resources = [
					"js/telepath.workflow.js",
					"js/telepath.workflow.record.js",
				];

				yepnope({
					load: wf_resources,
					complete: function () {
						telepath.workflow.init();
					}
				});
				
			}
		
		break;
		
		case 'alertsPanel':
			
			Ext.getStore('general_flow_group_store').load();
			
			if(!telepath.alerts.initialized) {
				telepath.alerts.init();
			}
		
		break;
		
		case 'dashboardPanel':
		
			telepath.dashboard.updatePanelOrder();
			RefreshDashboardComponents();
			
		break;
		
		case 'investigatePanel':
			
			Ext.getStore('general_flow_group_store').load();
			
			if(!telepath.investigate.initialized) {
				telepath.investigate.init();
			}
		
		break;
		
		case 'behavioralPanel':
			
			Ext.getStore('general_flow_group_store').load();
			
			if(!telepath.rules) {
			
				Ext.getCmp('main_panel').disable();
				
				yepnope({
					load: [
						"js/telepath.rules.js",
					],
					complete: function () {
						telepath.rules.init();
						Ext.getCmp('main_panel').enable();
					}
				});
				
			}
			
		break;
		
		case 'configPanel':
			
			if(!telepath.config) {
			
				Ext.getCmp('main_panel').disable();
								
				yepnope({
					load: [
						"js/telepath.interfaces.js",
						"js/telepath.config.js",
					],
					complete: function () {
						telepath.config.init();
						Ext.getCmp('main_panel').enable();
					}
				});
							
			}
			
		break;
		
	}
	
	switch(oldCard.id) {
		
		case 'dashboardPanel':
			
			var suspects_large = Ext.getCmp('d_suspectsLarge');
			if(suspects_large){
				suspects_large.hide();
			}
			var map_large = Ext.getCmp('d_mapLarge');
			if(map_large){
				map_large.hide();
			}
			
		break;
		
		case 'alertsPanel':
			
			hide_alert_params_window();
			
		break;
		
		case 'workflowsPanel':
			// Hide the children!
			Ext.getCmp('w_node_menu_horizontal').hide();
		break;
		
		case 'investigatePanel':
			Ext.getCmp('RequestAttributes') ? Ext.getCmp('RequestAttributes').destroy() : false;
		break;
		
	}
	
	//adjust_panel_sizes();
	
}

function adjust_panel_sizes() {
	
	if(adjustClock) {
		clearTimeout(adjustClock);
	}
	
	adjustClock = setTimeout(adjust_panel_sizes_tick, 100);

}

// Have a clock to not resize on every pixel moved
function adjust_panel_sizes_tick() {
	
	var currentPanel = Ext.getCmp('main_panel').tabBar.activeTab ? Ext.getCmp('main_panel').tabBar.activeTab.card.id : 'none';
	var main_container = $('#telepath-main');
	
	main_container.css({ height: $(window).height(), width: $(window).width() });
	
	// debug modes for different resolutions
	/*if($('#telepath-resolutions').size() == 0) {
		
		var telepath_resolution = $('<div>').css({ position: 'absolute', top: 10, left: 10 }).attr('id', 'telepath-resolutions');
		var resolution_options = ['800x600','1024x768','1280x800', '1400x1050', '1600x1200'];
		var resolution_select  = $('<select>');
		
		$.each(resolution_options, function(i, res) {
			var resolution_option = $('<option>').val(res).text(res);
			resolution_select.append(resolution_option);
		});
		
		resolution_select.change(function () {
			
			var new_res = $(this).val();
			new_res = new_res.split('x');
			$(main_container).css({ width: new_res[0], height: new_res[1] });
			adjust_panel_sizes_tick();
			
			$(main_container).css('marginTop', ( $(window).height() - new_res[1] ) / 2);
			
		});
		
		telepath_resolution.append(resolution_select);
		$(body).append(telepath_resolution);
		$(main_container).css('border', '2px solid black');
		
	}*/
	
	var width 		 = $(main_container).width();
	var offset       = $("#telepath-head").height();
	var height		 = $(main_container).height() - offset;
	var scrollTop 	 = $(main_container).scrollTop();
	
	// Set main_panel (outer) first
	
	Ext.getCmp('main_panel').setHeight(height);
	Ext.getCmp('main_panel').setWidth(width);
	
	// Tweak out per resolution settings
	
	if(width < 1200) {
		$('#telepath-head').css('fontWeight', 'normal');
		$('.tp-button span').css({ paddingLeft: '25px' });
	} else {
		$('#telepath-head').css('fontWeight', 'bold');
		$('.tp-button span').css({ paddingLeft: '30px' });
	}
	
	// End Tweak

	switch(currentPanel) {
		
		// Dashboard
		case 'dashboardPanel':
			
			Ext.getCmp('dashboardPanel').setHeight(height);
			Ext.getCmp('dashboardPanel').setWidth(width);
			
			width = width - 18;
			
			Ext.getCmp('d_faccade').setWidth(width);
			Ext.getCmp('d_trendsPanel').setWidth(width/2);
			Ext.getCmp('d_anomalyPanel').setWidth(width/2);
			Ext.getCmp('d_mapPanel').setWidth(width/2);
			Ext.getCmp('d_countriesPanel').setWidth(width/2);
			Ext.getCmp('d_thirdRow').setWidth(width);
			Ext.getCmp('d_suspectsPanel').setWidth(width/2);
			Ext.getCmp('d_sessions').setWidth(width/2);
			
			setTimeout(function () {
				if($('#d_trendsLine rect:last').attr('ry') == '0') {
					$('#d_trendsLine rect:last').attr('width', (parseInt($('#d_trendsLine rect:last').attr('width')) + 7) + 'px');
				}
			}, 100);
			
			telepath.dashboard.updatePanelOrder();
		
		break;
		
		// Alerts
		case 'alertsPanel':
		
			Ext.getCmp('alertsPanel').setHeight(height);
		
		break;
		
		// Investigate
		case 'investigatePanel':
		
			Ext.getCmp('investigatePanel').setHeight(height);
			Ext.getCmp('investigatePanel').setWidth(width);
			Ext.getCmp('i_filterPanel').setHeight(400);
		
		break;
		
		// Workflow
		case 'workflowsPanel':
			
			Ext.getCmp('workflowsPanel').setHeight(height);
			
			Ext.getCmp('w_config').setWidth(300);
			Ext.getCmp('w_diagram').setWidth(width-300);
			
		break;
		
		// Behavior Rules
		case 'behavioralPanel':
			Ext.getCmp('behavioralPanel').setHeight(height);
		break;
		
		// Configuration
		case 'configPanel':
			
		break;
		
	}
	
}