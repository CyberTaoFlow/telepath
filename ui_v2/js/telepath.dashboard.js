var panelsOrder = new Array(4);
var panelMoveTimer = false;
var points = [];
var clusters = [];
var bad_points = [];
var hiddenClusters =[];
var d_map = Ext.getCmp('d_mapPanel')
var alertsNum;
var d_suspects_selected_cell;
var trendsNum;
var refreshFreq;
var app = 'All';
var suspectsNum;
var start;
var end;

var d_selected_record;
var d_selected_index;

var d_refreshPrevState;
var d_alertsNumPrevState;
var d_suspectsNumPrevState;
var d_startDatePrevState;
var d_endDatePrevState;
var d_appsPrevState = 'All';
var d_endDateFlag = false;
var d_refreshFlag = false;
var d_startDateFlag = false;
var d_appsFlag = false;
var d_prev_height;
var countryHovered;
var rule_hovered_on_pie;
var alertType;
var lastRulesList = [];
var lastAlertsCounts = new Array(5);

var ovi;
var points = [];
var clusters = [];
var bad_points = [];
var hiddenClusters =[];

var countries_store=false;
var alerts_pie;
var lastSessionCounts = new Array(5);

var currentBubble = false;
var ETATimer = false;

telepath.dashboard = {
	
	panelMove: function(id, x, y) {
		if(panelMoveTimer) {
			clearTimeout(panelMoveTimer);
		}
		panelMoveTimer = setTimeout(function () {
			telepath.dashboard.rearrangePanels(id, x, y);
		}, 250);
	},
	updatePanelOrder: function() {
		
		setTimeout(function () {
		
			try {
			
				var height = Ext.getCmp('d_draggableRow').getHeight()/2;
				var width = Ext.getCmp('d_draggableRow').getWidth()/2;
				
				$.each(panelsOrder, function(i, panel) { 
				
					var x; 
					var y; 
					switch(i) { 
						case 0: x=0; y=x; break; 
						case 1: x=width; y=0; break; 
						case 2: x=0; y=height; break; 
						case 3: x=width; y=height; break; 
					} 
					
					Ext.getCmp(panel).setPosition(x,y); 
					
				});
				
			} catch(e) { }
		
		}, 250);

	},
	rearrangePanels: function(id, x, y) {
		
		var height = Ext.getCmp('d_draggableRow').getHeight()/2;
		var width = Ext.getCmp('d_draggableRow').getWidth()/2;
		
		var replaceWith;
		var toReplace;
		var changed = false;
		
		for (var i=0;i<panelsOrder.length;i++) {
			if (panelsOrder[i] == id) toReplace = i;       
		}
		//case 1, replace the moved one with the one in  (1,1)
		if (x<width && y<height && toReplace!=0) {
		
			//take the first one of the array (id)       
			replaceWith = panelsOrder[0];
			panelsOrder[0] = id;
			panelsOrder[toReplace] = replaceWith;
			changed = true;

		}

		//case 2, replace the moved one with the one in  (1,2)

		else if (x>width && y<height && toReplace!=1) {
		
			replaceWith = panelsOrder[1];
			panelsOrder[1] = id;
			panelsOrder[toReplace] = replaceWith;
			changed = true;
			
		}

		//case 3, replace the moved one with the one in  (2,1)

		else if (x<width && y>height && toReplace!=2) {
		
			replaceWith = panelsOrder[2];
			panelsOrder[2] = id;
			panelsOrder[toReplace] = replaceWith;
			changed = true;
			
		}

		else if (x>width && y>height && toReplace!=3) {
		
			replaceWith = panelsOrder[3];
			panelsOrder[3] = id;
			panelsOrder[toReplace] = replaceWith;
			changed = true;

		}
		
		if(changed) {
			telepath.dashboard.updatePanelOrder();
			updatePanelsOrderInServer();
		}

	},
	timer: function() {
		
		//var interval  = Ext.getCmp('d_refresh').getValue() * 60 ;
		//if (interval!=undefined & interval!=null) {	} else { interval = 300000; }
		//setInterval( function(){ RefreshDashboardComponents(); }, interval );
	
	},
	event: function(type, element) {
		
		if(type == 'render' && element.xtype == 'dashboard') {
			
			if(!telepath.access.has_perm('Dashboard', 'set')) {
				Ext.getCmp('d_settings').hide();
			}
			
			$('#d_settings-innerCt').append($('<div>').teleSelect({ label: 'Application', type: 'subdomain', values: [{ text: 'All', id: '-1', sub_id: '-1', root: true }], click: function () { 
				Ext.getCmp('d_apply').show();
				Ext.getCmp('d_cancel').show();
			} }));
			
			//return;
			
			// Attach Investigate Tooltip
			Ext.getCmp('i_suspects').getView().on('render', function(view) {
				view.tip = Ext.create('Ext.tip.ToolTip', {
					// The overall target element.
					target: view.el,
					// Each grid row causes its own seperate show and hide.
					delegate: view.cellSelector,
					// Moving within the row should not hide the tip.
					trackMouse: true,
					// Render immediately so that tip.body can be referenced prior to the first show.
					renderTo: Ext.getBody(),
					listeners: {
						// Change content dynamically depending on which element triggered the show.
						beforeshow: function updateTipBody(tip) {
							
							var gridColums = view.getGridColumns();
							var column = gridColums[tip.triggerElement.cellIndex];
							var val=view.getRecord(tip.triggerElement.parentNode);

							if(column.dataIndex == 'td1') {
								if(val.data.td24 != '' && !isNaN(parseInt(val.data.td24 * 100))) {
									tip.update('User Reputation : ' + parseInt(val.data.td24 * 100) + '%');
								} else {
									return false;
								}
							} else {
								return false;
							}
						}
					}
				});
			});
			
			// Attach Dashboard Tooltip
			Ext.getCmp('d_suspectsPanel').getView().on('render', function(view) {
				view.tip = Ext.create('Ext.tip.ToolTip', {
					// The overall target element.
					target: view.el,
					// Each grid row causes its own seperate show and hide.
					delegate: view.cellSelector,
					// Moving within the row should not hide the tip.
					trackMouse: true,
					// Render immediately so that tip.body can be referenced prior to the first show.
					renderTo: Ext.getBody(),
					listeners: {
						// Change content dynamically depending on which element triggered the show.
						beforeshow: function updateTipBody(tip) {
							
							var gridColums = view.getGridColumns();
							var column = gridColums[tip.triggerElement.cellIndex];
							var val=view.getRecord(tip.triggerElement.parentNode);

							if(column.dataIndex == 'td1') {
								
								if(val.data.td24 && !isNaN(parseInt(val.data.td24 * 100))) {
									tip.update('User Reputation : ' + parseInt(val.data.td24 * 100) + '%');
								} else {
									return false;
								}

							} else {
								return false;
							}
						}
					}
				});
			});
			
			setTimeout(function () {

				telepath.dashboard.initAlertsPie();
				telepath.dashboard.initCountryPie();
				
				createTrendsAreaChart();
				createSessionAreaChart();
				
				RefreshDashboardComponents('settings,dashboard_order');
				
				// Resize Panels
				adjust_panel_sizes();
				
				// yepnope({ load: [ "http://api.maps.ovi.com/jsl.js", "js/map.js" ], complete: function () { console.log('Map Initialized'); }});
				// setTimeout(function () { try { map(); } catch(err) { } }, 3000);
				
				// -----------------------------------------------
				// Timed Events
				// -----------------------------------------------
				
				// var mapHtml = '<iframe src="Htmls/Dashboard.html" width="100%" height="100%"  style="z-index:5" frameBorder="0"></iframe>';
				// Timeout for Map
				// setTimeout(function () { $(Ext.getCmp('d_mapPanel').body.dom).html(mapHtml); }, 500);
				
				var scaleColors = ['#C4E8B2', '#60BB3C'];
				
				$(Ext.getCmp('d_mapPanel').body.dom).empty().vectorMap({
					map: 'world_en',
					backgroundColor: '#FFFFFF',
					color: '#CECECE',
					hoverOpacity: 0.7,
					selectedColor: '#666666',
					enableZoom: true,
					showTooltip: true,
					/*values: this.options.data,*/
					scaleColors: scaleColors,
					normalizeFunction: 'polynomial'
				});
				
				$('#d_mapPanel-body').css({ 'text-align': 'center' });
				
				
				// Timeout for Dashboard Timer
				setTimeout(function(){ telepath.dashboard.timer(); }, 10000);
				
			}, 500);

		}

	},
	initAlertsPie: function () {
	
		// Init Store
		var alerts_pie_store = Ext.define('MyApp.store.d_alertsPieStore', {
	    extend: 'Ext.data.Store',
	    constructor: function(cfg) {
		var me = this;
		cfg = cfg || {};
		me.callParent([Ext.apply({
		    storeId: 'd_alertsPieStore',
		    data:[],
		    proxy: {
				type: 'memory',
		    },
		    fields: [{ name: 'name' },{ name: 'id' },{ name: 'percentage' },{ name: 'data' }]
		}, cfg)]);
	    }
		}).create();

		
		// Init Theme
		Ext.chart.theme.Browser_alerts = Ext.extend(Ext.chart.theme.Base, {
			constructor: function(config) {
			Ext.chart.theme.Base.prototype.constructor.call(this, Ext.apply({
				colors: [ '#ececec' ]
			}, config));
			}
		});
	
    var alerts_chart = new Ext.chart.Chart({
        animate: true,
        store: alerts_pie_store,
        theme: 'Browser_alerts',
        axes: [{
            type: 'Numeric',
            position: 'bottom',
            fields: ['data'],
            label: {
               renderer: Ext.util.Format.numberRenderer('0,0')
            },
			font: '8px Lucida Grande',
            title: 'Count'
        }, {
            type: 'Category',
            position: 'left',
            fields: ['name'],
			hideMode: 'offsets',
            title: 'Rule'
        }],
        series: [{
            type: 'bar',
            axis: 'bottom',
            xField: 'name',
            yField: 'data',
            highlight: true,
			highlightCfg: {
                fill: '#d7d2d2',
				stroke: '#d7d2d2',
				'stroke-width': 1,
				cursor: 'pointer'
            },
            label: {
                display: 'outside',
                field: 'percentage',
                renderer: function(v) { return v; },
                orientation: 'horizontal',
                color: '#333',
            },
			tips: {
				trackMouse: true,
				width: 150,
				height: 18,
				renderer: function(storeItem, item) {

					this.setTitle( 'Number of alerts: ' + storeItem.get('data') );
					rule_hovered_on_pie = storeItem.get();
					
				}
			},
			listeners: {
				itemclick: function (param) {
					
					var rule_id = param.storeItem.get('id');
					
					if (rule_id !="-1") {
						Ext.getCmp("alertsPanel").show();
						get_alerts('Rule', rule_id);
					}
						
				}
			}
        }]
    });
	
	Ext.getCmp('d_LeftPieCont').add(alerts_chart);
	
	},
	initCountryPie: function () {
		
		// Init Store
		countries_store = Ext.define('MyApp.store.d_countriesStore', {
		
			extend: 'Ext.data.Store',
			constructor: function(cfg) {
			
				var me = this;
				cfg = cfg || {};
				me.callParent([Ext.apply({
					storeId: 'd_countriesStore',
					data:[{	name:"", data:1, percentage:0 }],
					proxy: { type: 'memory' },
					fields: [{name: 'name'},{name: 'percentage'},{name: 'data'}]
				}, cfg)]);
				
			}
		}).create();
		
		Ext.chart.theme.Browser_countries = Ext.extend(Ext.chart.theme.Base, {
			constructor: function(config) {
			Ext.chart.theme.Base.prototype.constructor.call(this, Ext.apply({
				colors: [ '#ececec' ]
			}, config));
			}
		});
		
		var countries = new Ext.chart.Chart({
        animate: true,
        store: countries_store,
        theme: 'Browser_countries',
        axes: [{
            type: 'Numeric',
            position: 'bottom',
            fields: ['data'],
            label: {
               renderer: Ext.util.Format.numberRenderer('0,0')
            },
			font: '8px Lucida Grande',
			title: 'Requests'
        }, {
            type: 'Category',
            position: 'left',
            fields: ['name'],
			hideMode: 'offsets',
			label: {
				renderer: function(v) {
					return v == 'No Alerts' ? 'No Alerts' : aliasToName[v];
				}
            },
			title: 'Country'
        }],
        series: [{
            type: 'bar',
            axis: 'bottom',
            xField: 'name',
            yField: 'data',
            highlight: true,
			highlightCfg: {
                fill: '#d7d2d2',
				stroke: '#d7d2d2',
				'stroke-width': 1,
				cursor: 'pointer'
            },
            label: {
                display: 'outside',
                field: 'percentage',
                renderer: function(v) { return v; },
                orientation: 'horizontal',
                color: '#333',
            },
			tips: {
				trackMouse: true,
				width: 150,
				height: 18,
				renderer: function(storeItem, item) {
					
					this.setTitle( 'Number of alerts: ' + storeItem.get('data') );
					rule_hovered_on_pie = storeItem.get();

				}
			},
			listeners: {
				itemclick: function (param) {
					
					var countryCode = param.storeItem.get('name');
					
					Ext.getCmp("alertsPanel").show();
					get_alerts('country', countryCode);
					
				}
			}
        }]
    });

		// Resize
		countries.setHeight(Ext.getCmp('d_RightPieCont').getHeight()-20);
		countries.setWidth(Ext.getCmp('d_RightPieCont').getWidth());
		
		// Render
		Ext.getCmp('d_RightPieCont').add(countries);

	}

}

/*
 * Function: d_cancel_click_handler
 * 
 * Invoked:
 * 	When the cancel button was clicked. The function will set the fields with their previous values
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
function d_cancel_click_handler() {

	Ext.getCmp('d_trendsScale').setValue('Custom Range');

	Ext.getCmp('d_apply').hide();
	Ext.getCmp('d_cancel').hide();
	Ext.getCmp('d_refresh').setValue(d_refreshPrevState);
	Ext.getCmp('d_startDate').setValue(new Date(d_startDatePrevState));
	Ext.getCmp('d_endDate').setValue(new Date(d_endDatePrevState));
	
	$('#d_settings-body .tele-multi input').val(d_appsPrevState).attr('itemID', '-1');
	
	d_endDateFlag = false;
	d_startDateFlag = false;
	d_suspectsNumFlag = false;
	d_alertsNumFlag = false;
	d_refreshFlag = false;
	d_appsFlag = false;	
	
}

/*
 * Function: d_trendsScale_change
 * 
 * Invoked:
 * 	When a new Display period has been selected. The function will set the dates accordingly
 * 
 * 
 * Parameters:
 * 	newValue - string, the value selected
 * 
 * Returns:
 * 	undefined	
 * 
 */
function d_trendsScale_change(newValue){
	d_startDateFlag = true;
	Ext.getCmp('d_endDate').setValue(new Date());
	if (newValue=='Week'){
		Ext.getCmp('d_startDate').setValue(new Date((new Date().getTime() - (60 * 60 * 24 * 7 * 1000))));
		Ext.getCmp('d_startHour').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_endDate').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_endHour').setValue(new Date(new Date().getTime()));
	}
	else if (newValue=='Month'){
		Ext.getCmp('d_startDate').setValue(new Date((new Date().getTime() - (60 * 60 * 24 * 30 * 1000))));
		Ext.getCmp('d_startHour').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_endDate').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_endHour').setValue(new Date(new Date().getTime()));
	}
	else if (newValue=='Year'){
		Ext.getCmp('d_startDate').setValue(new Date((new Date().getTime() - (60 * 60 * 24 * 365 * 1000))));
		Ext.getCmp('d_startHour').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_endDate').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_endHour').setValue(new Date(new Date().getTime()));
	}
	else if (newValue=='Day'){
		Ext.getCmp('d_startDate').setValue(new Date((new Date().getTime() - (60 * 60 * 24 * 1 * 1000))));
		Ext.getCmp('d_startHour').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_endDate').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_endHour').setValue(new Date(new Date().getTime()));
	}
	else if(newValue == 'Hour' | newValue == 'Now') {
		Ext.getCmp('d_startDate').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_startHour').setValue(new Date((new Date().getTime() - (60 * 60 * 1 * 1 * 1000))));
		Ext.getCmp('d_endDate').setValue(new Date(new Date().getTime()));
		Ext.getCmp('d_endHour').setValue(new Date(new Date().getTime()));
	}
	Ext.getCmp('d_apply').show();
	Ext.getCmp('d_cancel').show();
}


/*
 * Function:  d_startDate_select
 * 
 * Invoked:
 * 	When start date is chosen. The function will  set the display period to be custom range and verify that the 
 * 	start date was not later than the end date
 * 
 * 
 * Parameters:
 * 	all parameters are as for the select even fired for a date field
 * 
 * Returns:
 * 	undefined	
 * 
 */
function d_startDate_select(that,date,eOpts){
	
	d_startDateFlag = true;
	Ext.getCmp('d_trendsScale').setValue('Custom Range');
	if (date.getTime() > Ext.getCmp('d_endDate').getValue().getTime()){
		Ext.Msg.alert('Error', 'Start Date cannot be later than End Date');
		that.setValue(new Date(start*1000))
		return;
	}
	Ext.getCmp('d_apply').show();
	Ext.getCmp('d_cancel').show();	
	
}


function get_startTimeValue() {
	return (parseInt(Ext.getCmp('d_startHour').value.getHours()) * 3600) + (Ext.getCmp('d_startHour').value.getMinutes() * 60);
}

function get_endTimeValue() {
	return (parseInt(Ext.getCmp('d_endHour').value.getHours()) * 3600) + (Ext.getCmp('d_endHour').value.getMinutes() * 60);
}


function d_startHour_select(combo,records,eOpts) {
	
	d_startDateFlag = true;
	try {
	Ext.getCmp('d_trendsScale').setValue('Custom Range');
	if (Ext.getCmp('d_startDate').value.getTime() + get_startTimeValue() > Ext.getCmp('d_endDate').getValue().getTime() + get_endTimeValue()){
		Ext.Msg.alert('Error', 'Start Date cannot be later than End Date');
		combo.setValue('00:00');
		return;
	}
	} catch(e) {}
	Ext.getCmp('d_apply').show();
	Ext.getCmp('d_cancel').show();	

}
function d_endHour_select(combo,records,eOpts) {

	d_startDateFlag = true;
	try {
	Ext.getCmp('d_trendsScale').setValue('Custom Range');
	if (Ext.getCmp('d_startDate').value.getTime() + get_startTimeValue() > Ext.getCmp('d_endDate').getValue().getTime() + get_endTimeValue()){
		Ext.Msg.alert('Error', 'Start Date cannot be later than End Date');
		combo.setValue('23:59');
		return;
	}
	} catch(e) {}
	Ext.getCmp('d_apply').show();
	Ext.getCmp('d_cancel').show();	

}
/*
 * Function:  d_endDate_select
 * 
 * Invoked:
 * 	When end date is chosen. The function will  set the display period to be custom range and verify that the 
 * 	end date was not earlier than the start date
 * 
 * 
 * Parameters:
 * 	all parameters are as for the select even fired for a date field
 * 
 * Returns:
 * 	undefined	
 * 
 */	
function d_endDate_select(that,date,eOpts){
	
	d_endDateFlag = true;
	Ext.getCmp('d_trendsScale').setValue('Custom Range');
	if (date.getTime()<=Ext.getCmp('d_startDate').getValue().getTime()){
		Ext.Msg.alert('Error', 'End Date cannot be eariler than Start Date');
		that.setValue(new Date(end*1000))
		return;
	}
	Ext.getCmp('d_apply').show();
	Ext.getCmp('d_cancel').show();		
}



/*
 * Function:  badPointsHandler
 * 
 * Invoked:
 * 	When the user clicks a point on the map. The function switches to the alerts tab and shows the relevant alert
 * 
 * 
 * Parameters:
 * 	evt - event object of ovi maps
 * 	msg_id - int, the msg_id of the alert clicked
 * 
 * Returns:
 * 	undefined	
 * 
 */	
function badPointsHandler(evt,msg_id,mode){	
	
	if(mode == 'mouseover') {
		
		$.get(telepath.controllerPath + '/alerts/get_alerts', {
				start: 0,
				limit: 30,
				sortorder: alerts_sortorder,
				sortfield: alerts_sortfield,
				val : msg_id,
				variable : "id"
		}, function (data) {
			
			try {

				var item     = data.items[0];
				
				var bubbleTemp = $('<div>');
				var bubbleLink = $('<a>');
				
				var bubbleText = (item.td10 != 'Unknown' ? item.td10 + ' , ' : '') + aliasToName[item.td9] + '<br>' + item.td5;

				bubbleLink.html(bubbleText);
				bubbleTemp.append(bubbleLink);
				currentBubble = infobubbles.addBubble(bubbleTemp.html(), evt.target.coordinate);
				
				$("iframe").each(function () {
					
					if($(this)[0].contentWindow && $(this)[0].contentWindow.$) {
					
						$(this)[0].contentWindow.$('.ovi_mp_bubble_content a')
						.unbind('click')
						.bind('click', function (e) {
							e.preventDefault();
							Ext.getCmp('alertsPanel').show();
							setTimeout(get_alerts("country",item.td9),3000);
							Ext.getCmp('main_panel').enable();
						}).hover(function () {
							$(this).css('text-decoration', 'underline');
						}, function () {
							$(this).css('text-decoration', 'none');
						});
					
					}
					
				});
				
			}
			catch(err) {
				Ext.Msg.alert('Error #210', 'Error parsing json data for map infobubble :: msg_id ' + msg_id);
				return;
			}	
			
		}, 'json');
	
	} 

	if(mode == 'click') {
	
		Ext.getCmp('alertsPanel').show();
		setTimeout(function () {
			
			$.get(telepath.controllerPath + '/alerts/get_alerts', {
				start: 0,
				limit: 30,
				sortorder: alerts_sortorder,
				sortfield: alerts_sortfield,
				val : msg_id,
				variable : "id"
			}, function (data) {
				var item     = data.items[0];
				get_alerts("country",item.td9);
			});
		
		}, 500);

		Ext.getCmp('main_panel').enable();
		
	}
	
}

/*
 * Function:  InitSettings
 * 
 * Invoked:
 * 	After we get the dashborad settings from the server, this function will assign all values to their corresponding fields
 * 	
 * 
 * 
 * Parameters:
 * 	jsondata - object. holds the json object we got from the server
 * 
 * Returns:
 * 	undefined	
 * 
 */
function InitSettings(jsondata) {

	Ext.getCmp('d_trendsScale').setValue('Custom Range');
	
	start = jsondata.time_start;
	Ext.getCmp('d_startDate').setValue(new Date(parseInt(jsondata.time_start)*1000));
	Ext.getCmp('d_startHour').setValue(new Date(parseInt(jsondata.time_start)*1000));
	end = jsondata.time_end;
	Ext.getCmp('d_endDate').setValue(new Date(parseInt(jsondata.time_end)*1000));
	Ext.getCmp('d_endHour').setValue(new Date(parseInt(jsondata.time_end)*1000));
	
	/*
	if(jsondata.date_min) {
		Ext.getCmp('d_startDate').setMinValue(new Date(jsondata.date_min * 1000 - 24*36000));
		Ext.getCmp('d_endDate').setMinValue(new Date(jsondata.date_min * 1000 - 24*36000));
	}
	if(jsondata.date_max) {
		Ext.getCmp('d_startDate').setMaxValue(new Date(jsondata.date_max * 1000 + 24*36000));
		Ext.getCmp('d_endDate').setMaxValue(new Date(jsondata.date_max * 1000  + 24*36000));
	}
	*/
				
	refreshFreq = parseInt(jsondata.timerInterval);
	Ext.getCmp('d_refresh').setValue(parseInt(jsondata.timerInterval));
			
	ps_app = "All";
	d_refreshPrevState = parseInt(jsondata.timerInterval);
	ps_refreshNum = parseInt(jsondata.timerInterval);
	d_startDatePrevState = parseInt(jsondata.time_start)*1000;
	ps_startDate = parseInt(jsondata.time_start)*1000;
	d_endDatePrevState = (new Date()).getTime();
	ps_endDate = (new Date()).getTime();
	
	// Operation Mode
	operation_mode = parseInt(jsondata.operation_mode);
	Ext.getCmp('d_operation').setValue((operation_mode==1?"Training":(operation_mode==2?"Production":"Hybrid")));
	
}

/*
 * Function:  d_updateConfig
 * 
 * Invoked:
 * 	The function checks which value was changed on the dashboard settings and refreshes the relevant panels to that change.
 * 	
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined
 * 
 * 
 */

function d_updateConfig(){
		
		var startHour = Ext.getCmp('d_startHour').getValue();
		var endHour   = Ext.getCmp('d_endHour').getValue();
		
		var date1 = (Ext.getCmp("d_startDate").getValue().getTime()/1000) + (startHour.getHours() * 3600) + (startHour.getMinutes() * 60);
		var date2 = (Ext.getCmp("d_endDate").getValue().getTime()/1000) + (endHour.getHours() * 3600) + (endHour.getMinutes() * 60);

		Ext.Ajax.request({
		url: telepath.controllerPath + '/dashboard/set_dashboard',
		params: {
			date1: date1,
			date2: date2,
			refresh:Ext.getCmp("d_refresh").getValue(),
		},
		success:function(response){
			RefreshDashboardComponents(); // Refresh all dashboard components
		},
		failure: function(response, opts) {
			console.log('server-side failure with status code ' + response.status);
		}	
	});

}

/*
 * Function:  createSessionAreaChart
 * 
 * Invoked:
 * 	The function renders the the session graph. is called everytime we want to load the graph with different values on the x axis
 * 	
 * 
 * 
 * Parameters:
 * 	fields - optional,string array, [x_name1,x_name2....]
 * 	
 * 
 * Returns:
 * 	undefined
 * 
 * See Also:
 *
 * 	<init_dashboard>
 */
function createSessionAreaChart(fields,destroyOrNot){
	var storeToCreate;
	fields = ['Sessions'] 
	storeFields = ['name','Sessions','epoch']

	if (fields!=undefined){
		for(var i=0;i<fields.length;i++)
			storeFields.push(fields[i])
	}
	storeToCreate = {
	    fields: storeFields,
	    id: 'd_sessionsStore',
	    data: [

	    ]
	}
	if (Ext.getStore('d_sessionsStore')!=undefined){
		try{
			Ext.getStore('d_sessionsStore').destroy();  	
		}
		catch(err){

		}
	}
	var storeCreated = Ext.create('Ext.data.JsonStore', storeToCreate);
	if (Ext.getCmp('d_sessionsLine')!=undefined){
		try{
			Ext.getCmp('d_sessionsLine').destroy();
		}
		catch(err){

		}
	}
	var chartToCreate  = createLineChart(fields,storeCreated,'Sessions');
	var trendsAreaChart;
	sessionAreaChart = Ext.create('Ext.chart.Chart',chartToCreate );
	Ext.getCmp('d_sessions').add(sessionAreaChart);
	//sessionAreaChart.setHeight(Ext.getCmp('d_sessions').getHeight()-50);
	//sessionAreaChart.setWidth(Ext.getCmp('d_sessions').getWidth()-5);

}


function handle_trends_graph_click(a,b,c,d){
	//alert('clicked trends');
} 

function handle_sessions_graph_click(a,b,c,d){
	//alert('clicked sessions');
} 
/*
 * Function:  createTrendsAreaChart
 * 
 * Invoked:
 * 	The function renders the the alert trends graph. is called everytime we want to load the graph with different values on the x axis
 * 	
 * 
 * 
 * Parameters:
 * 	fields - optional,string array, [x_name1,x_name2....]
 * 	
 * 
 * Returns:
 * 	undefined
 * 
 * See Also:
 *
 * 	<init_dashboard>
 */
//this function gets a list of fields and creates a line chart 
function createTrendsAreaChart(fields,destroyOrNot){
	var storeToCreate;
	storeFields = ['name','epoch']
	if (fields==undefined){
		fields = ['No Alerts'];
 		for(var i=0;i<fields.length;i++)
			storeFields.push(fields[i])
		storeToCreate = {
		    fields: storeFields,
		    data: [
			{ 'name': 'Now',   'No Alerts':0}
		    ]
		}

	}
	else if (fields.length==0){
		fields = ['No Alerts'];
 		for(var i=0;i<fields.length;i++)
			storeFields.push(fields[i])
		storeToCreate = {
		    fields: storeFields,
		    id: 'd_trendsLineStore'
		}
	}
	else{
 		for(var i=0;i<fields.length;i++)
			storeFields.push(fields[i])
		storeToCreate = {
		    fields: storeFields,
		    id: 'd_trendsLineStore',
		    data: [

		    ]
		}
	}

	if (Ext.getCmp('d_trendsLine')!=undefined){
		Ext.getCmp('d_trendsLine').destroy();
	}
	//destroy befroe recreating to avoid memory leak
	if (Ext.getStore('d_trendsLineStore')!=undefined){
		Ext.StoreManager.unregister(Ext.getStore('d_trendsLineStore'))
	}
	var d_barStore = Ext.create('Ext.data.JsonStore', storeToCreate);
	//destroy befroe recreating to avoid memory leak

	var chartToCreate  = createLineChart(fields,d_barStore,'Trends');
	var trendsAreaChart;
	if (Ext.getCmp('d_trendsArea')!=undefined){
		Ext.getCmp('d_trendsArea').destroy();
	}
	trendsAreaChart = Ext.create('Ext.chart.Chart',chartToCreate );
	Ext.getCmp('d_trendsPanel').add(trendsAreaChart);

}


/*
 * Function:  createTrendsAreaChart
 * 
 * Invoked:
 * 	Only from the see also functions. Helps create the graph extjs object
 * 	
 * 
 * 
 * Parameters:
 * 	fields - optional,string array, [x_name1,x_name2....]
 * 	store - an extjs store object of the store to add to the graph object
 * 	type - string, 'Trends' | 'Sessions'
 * 	
 * 
 * Returns:
 * 	undefined
 * 
 * See Also:
 *
 * 	<createTrendsAreaChart>
 * 	<createSessionAreaChart>
 */
function createLineChart(fields,store,type){
	var series = []//new Array(fields.length);
	var series_json;
	for (var i=0;i<fields.length;i++){
		series_json = {
				    type: 'line',
				    highlight: {
					size: 7,
					radius: 7
				    },
				    axis: 'left',
				    xField: 'name',
				    yField: fields[i],
				    markerConfig: {
					type: 'circle',
					size: 2,
					radius: 2,
					'stroke-width': 0
				    }

			}
		if (type=='Trends'){
			series_json.listeners = {}
			series_json.listeners.itemclick = function(a,b,c,d){
						handle_trends_graph_click(a,b,c,d);
				    	}
		}
		else{
			series_json.listeners = {}
			series_json.listeners.itemclick = function(a,b,c,d){
						handle_sessions_graph_click(a,b,c,d);
				    	}
		}
		series.push(series_json);
	}
	ans =  {
		    renderTo: Ext.getBody(),
		    width: Ext.getCmp((type=='Trends'?'d_trendsPanel':'d_sessions')).getWidth(),//500,
		    height: 300,
		    animate: true,
		    id : (type=='Trends'?'d_trendsLine':'d_sessionsLine'),
		    store: store,
		    axes: [
			{
			    type: 'Numeric',
			    position: 'left',
			    fields: fields,
			    label: {
				renderer: Ext.util.Format.numberRenderer('0,0')
			    },
			    title: (type=='Trends'?'Intensity':'Sessions'),
			    grid: true,
			    minimum: 0
			},
			{
			    type: 'Category',
			    position: 'bottom',
			    fields: ['name'],
			    title: 'Time',
			    grid: true,
			    label: {
				rotate: {
				    degrees: 45
				}
			    }
			}
		    ],
		    series: series
		};

	if (type =='Trends')
		ans['legend'] = {
			position: 'left'
		    }
			
	return ans;


}

/* Dashboard Suspects */


/////////////////////////////start of suspects menu and binding/////////////////////////////
/*
 * Function: suspectsMenu(var)
 * 
 * Invoked:
 * 	The right click menu for the suspects panel
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
var suspectsMenu =  Ext.create('Ext.menu.Menu',{
                    xtype: 'menu',
                    floating: true,
                    hidden: true,
					hideMode:'display',//crucial to fix black squares bug in chrome
                    id: 'd_suspects_menu',
                    width: 160,
                    items: [
                        {
                            xtype: 'menuitem',
                            id: 'd_suspects_menu1',
                            text: 'Investigate this value'
                        },
                        {
                            xtype: 'menuitem',
                            id: 'd_suspects_menu2',
                            text: 'Investigate session'
                        },
                        {
                            xtype: 'menuitem',
                            id: 'd_suspects_menu3',
                            text: 'Investigate by IP'
                        },
                        {
                            xtype: 'menuitem',
                            id: 'd_suspects_menu5',
                            text: 'Add to Investigate Filter'
                        },
                        {
                            xtype: 'menuitem',
                            id: 'd_suspects_menu6',
                            text: 'Add IP to whitelist'
                        },
                        {
                            xtype: 'menuitem',
                            id: 'd_suspects_menu7',
                            text: 'Copy to clipboard'
                        }			
                    ]
                });

Ext.getCmp('d_suspects_menu1').addListener(
		'click',
		function(that,  record,  index,  eOpts){
			if(d_selected_index!=undefined){
				Investigate(d_selected_index);
			}
		}
);

Ext.getCmp('d_suspects_menu2').addListener(
		'click',
		function(that,  record,  index,  eOpts) {
			
			Ext.getCmp('investigatePanel').show();
			
			var params = {};
			var date   = new Date(d_selected_record.data.td0 * 1000);
			
			// IP
			params['SID']      = d_selected_record.data.td13

			// FROM
			date.setHours(date.getHours() - 1);
			Ext.getCmp('i_fromHour').setValue(date.getHours() + ":" + date.getMinutes());
			Ext.getCmp('i_startDate').setValue(date);
			params['fromdate'] = date.getTime() / 1000;

			// TO
			date.setHours(date.getHours() + 2);
			Ext.getCmp('i_toHour').setValue(date.getHours() + ":" + date.getMinutes());
			Ext.getCmp('i_endDate').setValue(date);
			params['todate'] = date.getTime() / 1000;
			
			// START
			investigate_main(0, params);
		
		}
);

Ext.getCmp('d_suspects_menu3').addListener(
		'click',
		function() {
			
			Ext.getCmp('investigatePanel').show();
			
			var params = {};
			var date   = new Date(d_selected_record.data.td0 * 1000);
			
			// IP
			params['user_ip'] = d_selected_record.data.td2;

			// FROM
			date.setHours(date.getHours() - 1);
			Ext.getCmp('i_fromHour').setValue(date.getHours() + ":" + date.getMinutes());
			Ext.getCmp('i_startDate').setValue(date);
			params['fromdate'] = date.getTime() / 1000;

			// TO
			date.setHours(date.getHours() + 2);
			Ext.getCmp('i_toHour').setValue(date.getHours() + ":" + date.getMinutes());
			Ext.getCmp('i_endDate').setValue(date);
			params['todate'] = date.getTime() / 1000;
			
			// START
			investigate_main(0, params);
			
		}
);

Ext.getCmp('d_suspects_menu5').addListener(
		'click',
		function(that,  record,  index,  eOpts){
			
			Ext.getCmp('investigatePanel').show();
			
			var dataIndex = d_selected_index;//num of cell
			
			switch(dataIndex) {
				
				case 'td0': // Date
					
					var date   = new Date(d_selected_record.data.td0 * 1000);
	
					// FROM
					date.setHours(date.getHours() - 1);
					Ext.getCmp('i_startDate').setValue(date);
					Ext.getCmp('i_fromHour').setValue(date.getHours() + ":" + date.getMinutes());

					// TO
					date.setHours(date.getHours() + 2);
					Ext.getCmp('i_endDate').setValue(date);
					Ext.getCmp('i_toHour').setValue(date.getHours() + ":" + date.getMinutes());
				
				break;
				
				case 'td2': // IP
					
					Ext.getCmp('i_filterIp').setValue(d_selected_record.data.td2);	
					
				break;
				case 'td4': // Country + City
					
					Ext.getCmp('i_filterCity').setValue(d_selected_record.data.td3 != 'Unknown' ? d_selected_record.data.td3 : '');
					Ext.getCmp('i_filterCountry').setValue(aliasToName[d_selected_record.data.td4]);
				
				break;
				case 'td12': // SCORE
				
					Ext.getCmp('i_filterAvgTot0').setValue("Average");
					Ext.getCmp('i_filterComparator0').setValue("At Least");
					Ext.getCmp('i_filterScore0').setValue(d_selected_record.data.td12);				
				
				break;
			}
			
			Ext.getCmp('dashboardPanel').show();
			
		}	
);

Ext.getCmp('d_suspects_menu6').addListener(
		'click',
		function(that,  record,  index,  eOpts) {
		
			var ip = d_selected_record.data.td2;
			
			telepath.ipconfig.show(ip, 'Add IP to Whitelist', function (ip) {
				telepath.ipconfig.add_ip_to_whitelist(ip);
			});
			
		}
);

Ext.getCmp('d_suspects_menu7').addListener(
		'click',
		function(that,  record,  index,  eOpts){
			if(d_selected_index != undefined){
				var text = d_selected_record.data[d_selected_index];
				if(d_selected_index == 'td0') {
					text = date_format('d/m/y H:i:s', text);
				}
				if(d_selected_index == 'td4') {
					text = aliasToName[d_selected_record.data['td4']] + (d_selected_record.data['td3'] != 'Unknown' ? ', ' + d_selected_record.data['td3'] : '');
				}
				copy_to_clipboard(text);
			}
		}
);

/*
 * Function:  Investigate
 * 
 * Invoked:
 * 	when we want to investigate a session or ip, we send the record of the suspect table here and it will add the value to the right filter on the investigate tab
 * 	and investigate that value
 * 	
 * 
 * 
 * Parameters:
 * 	record - model object of sencha. the record clicked
 * 
 * 
 * Returns:
 * 	undefined
 * 
 * 
 */

function Investigate(dataIndex) {
	
	var params = {};
	
	Ext.getCmp('investigatePanel').show();
	
	switch(dataIndex) {
		
		case 'td0':
		
			var date = new Date(d_selected_record.data.td0 * 1000);
	
			// FROM
			date.setHours(date.getHours() - 1);
			Ext.getCmp('i_fromHour').setValue(date.getHours() + ":" + date.getMinutes());
			Ext.getCmp('i_startDate').setValue(date);
			params['fromdate']= date.getTime() / 1000;

			// TO
			date.setHours(date.getHours() + 2);
			Ext.getCmp('i_toHour').setValue(date.getHours() + ":" + date.getMinutes());
			Ext.getCmp('i_endDate').setValue(date);
			params['todate']= date.getTime() / 1000;
			
		break;
		
		case 'td2':
			
			// IP
			params['user_ip'] = d_selected_record.data.td2;
			Ext.getCmp('i_filterIp').setValue(d_selected_record.data.td2);
			
		break;
		
		case 'td4':
		
			// CITY 
			Ext.getCmp('i_filterCity').setValue(d_selected_record.data.td3 != 'Unknown' ? d_selected_record.data.td3 : '');
			params['city'] = d_selected_record.data.td3;
			
			// COUNTRY
			Ext.getCmp('i_filterCountry').setValue(aliasToName[d_selected_record.data.td4]);
			params['country'] = d_selected_record.data.td4;
			
		break;
	
	}
	
	investigate_main(0, params);

}

/////////////////////////////suspects panel events/////////////////////////////
/*
 * Function:  d_suspectsPanel_rightclick
 * 
 * Invoked:
 * 	when the user right clicks the suspects panel
 * 
 * 
 * Parameters:
 * 	all parameters are as for the select even fired for a date field
 * 
 * Returns:
 * 	undefined	
 * 
 */	
function d_suspectsPanel_rightclick(that,record,item,index,event,eOPts){

	/*if(d_suspects_selected_cell!=undefined) {		
	
		if (d_suspects_selected_cell[1]!=2 & d_suspects_selected_cell[1]!=2 ){
			Ext.getCmp('d_suspects_menu1').disable();
		}
		else{
			Ext.getCmp('d_suspects_menu1').enable();	
		}
		
	}*/
	
	event.preventDefault();
	suspectsMenu.showAt(event.xy[0],event.xy[1]-20);
	
}
/*
 * Function:  d_suspectsPanel_celldblclick
 * 
 * Invoked:
 * 	when the user double clicks a cell in the suspects panel. Function switches to investigate tab and shows requests from that session.
 * 
 * 
 * Parameters:
 * 	all parameters are as for the select even fired for a date field
 * 
 * Returns:
 * 	undefined	
 * 
 */	
function d_suspectsPanel_celldblclick(that,  td,  cellIndex, record,  tr,  rowIndex,  e,  eOpts ) {

	if (cellIndex != 0) return;
		
	var params = {};	
	var date   = new Date(record.data.td0 * 1000);
	
	// FROM
	date.setHours(date.getHours() - 1);
	Ext.getCmp('i_fromHour').setValue(date.getHours() + ":" + date.getMinutes());
	Ext.getCmp('i_startDate').setValue(date);
	params['fromdate']= date.getTime() / 1000;

	// TO
	date.setHours(date.getHours() + 2);
	Ext.getCmp('i_toHour').setValue(date.getHours() + ":" + date.getMinutes());
	Ext.getCmp('i_endDate').setValue(date);
	params['todate']= date.getTime() / 1000;
	
	// Session
	params['SID'] = record.data.td13;
	Ext.getCmp('i_filterSession').setValue(record.data.td13);
	
	// START
	Ext.getCmp('investigatePanel').show();
	investigate_main(0, params);
	
}

