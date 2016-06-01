// DASHBOARD COMPONENTS LOADERS

var rearrange = false;
var panelsOrder = new Array(4);


// When we want to refresh all 6 panels on the dashboard
function RefreshDashboardComponents(items) {

	// Refresh these items by default
	if(!items) {
		items = 'eta,countries,map';
		RefreshDashboardComponents('suspects');
		RefreshDashboardComponents('anomalies');
		RefreshDashboardComponents('sessions');
		RefreshDashboardComponents('trends');
	}

	var d_panel = Ext.getCmp('dashboardPanel')
	if (d_panel && d_panel.isVisible()) {
		
		// Migration Debug
		// -----------------------------------
		var selected_apps = JSON.stringify($('#d_settings-body .tele-multi').data('tele-tele-select').result());
		
		var gap = getTimeGap();
		
		telepath.util.request("/dashboard/get_dashboard", { 
		
			selected_apps: selected_apps,
			scale:  Ext.getCmp('d_trendsScale').getValue(),
			items: items,
			gap: gap
			
		}, function(data) {
			
			// Store data in a global namespace
			if(!telepath.dashboard.data) { telepath.dashboard.data = {} };
			$.each(data, function(key, value) {
				telepath.dashboard.data[key] = value;
			});
			
			// Handle Session End (also happens in util.request)
			if(data.logout) {
				location.reload(); // Reload Page
				return; // Stop updating components
			}
			
			// Engine Status
			if(data.status) {
			
				if (data.status.engine == 1){
					$("#telepath-engineToggle .tp-button").removeClass('off');
					$(".telepath-eta").show();
				} else{
					$("#telepath-engineToggle .tp-button").addClass('off');
					$(".telepath-eta").hide();
				}
				$("#telepath-engineToggle .tp-button-inner span").text(data.status.engine == 1 ? 'On' : 'Off');
			
			}
			
			if(data.map) {
				
				var scaleColors = ['#C4E8B2', '#e0e812', '#c8387c', '#ff0078'];
				
				$(Ext.getCmp('d_mapPanel').body.dom).empty().vectorMap({
					map: 'world_en',
					backgroundColor: '#FFFFFF',
					color: '#CECECE',
					hoverOpacity: 0.7,
					selectedColor: '#666666',
					enableZoom: true,
					showTooltip: true,
					values: data.map,
					scaleColors: scaleColors,
					normalizeFunction: 'polynomial',
					onLabelShow: function(event, label, code) {
						
						var country = aliasToName[code.toUpperCase()];
						var count	= telepath.dashboard.data.map[code] ? telepath.dashboard.data.map[code] : 0;
						var text 	= country + '<br>' + count + ' Alert' + (count == 1 ? '' : 's');
						
						$(label).html(text);
						
					},
					onRegionOver: function(event, code, region) {},
					onRegionOut: function(event, code, region) {},
					onRegionClick: function(event, code, region) {
						
						event.preventDefault();
						Ext.getCmp('alertsPanel').show();
						setTimeout(get_alerts("country",code.toUpperCase()),3000);
					
					}
				});
			
			}
			
			// Dashboard Settings
			if(data.settings) {
				
				InitSettings(data.settings);
				//RefreshDashboardComponents();
				try {
					Ext.getCmp('d_apply').hide();
					Ext.getCmp('d_cancel').hide();
					d_appsFlag = true;
					d_updateConfig();	
				} catch(e) {}

			}
			
			// Dashboard Order
			if(data.dashboard_order) {
			
				panelsOrder = new Array(4);
				for (var i=0; i < data.dashboard_order.length && i < 4; i++) {
					panelsOrder[i] = data.dashboard_order[i];
				}
				if (!verifyFields()){
					panelsOrder = [ 'd_trendsPanel','d_anomalyPanel','d_countriesPanel','d_mapPanel' ];
				}
				setTimeout(function () {
					
					var height = Ext.getCmp('d_draggableRow').getHeight()/2;
					var width = Ext.getCmp('d_draggableRow').getWidth()/2;
					
					for(var i = 0 ; i < 4 ; i++) { Ext.getCmp(panelsOrder[i]).suspendEvents(false); }
					
					Ext.getCmp(panelsOrder[0]).setPosition(0,0);
					Ext.getCmp(panelsOrder[1]).setPosition(width,0);
					Ext.getCmp(panelsOrder[2]).setPosition(0,height);
					Ext.getCmp(panelsOrder[3]).setPosition(width,height);
					
					for(var i = 0 ; i < 4 ; i++) { Ext.getCmp(panelsOrder[i]).resumeEvents(); }
					
				}, 50);
			
			}
			
			// Anomalies
			if(data.anomalies) {
				
				for(x in data.anomalies) {
					// Shorten long rule names
					if(data.anomalies[x].name.length > 32) {
						var parts = data.anomalies[x].name.split('/');
						if(parts.length > 2) {
							data.anomalies[x].name = parts[0] + ' ' + parts[parts.length - 1];
						}
						// Another limit just in case page name is too long
						if(data.anomalies[x].name.length > 48) {
							data.anomalies[x].name = data.anomalies[x].name.substr(0, 36) + ' ...';
						}
					}
				}
			
				Ext.getStore("d_alertsPieStore").loadData(data.anomalies);
			}
			
			// ETA
			if(data.eta) {
			
				var timeArr = data.eta.split(" ");
				days = parseInt(timeArr[0].substr(0,timeArr[0].length-1));
				hours = parseInt(timeArr[1].substr(0,timeArr[1].length-1));
				minutes = parseInt(timeArr[2].substr(0,timeArr[2].length-1));
				
				var now  = new Date().getTime();
				var diff = days*24*60*60*1000+hours*60*60*1000+minutes*60*1000;
				var ts   = now + diff;

				if(diff > 0) {
					
					$(".telepath-eta").show();
					$(".tp-eta-timer").empty().countdown({ timestamp : ts });
					
					if(data.status && data.status.engine == 0) {
						$(".telepath-eta").hide();
					}
					
				} else {
					$(".telepath-eta").hide();
				}
				
			}
			
			// Countries
			if(data.countries) {	
							
				if(!countries_store) { return; }
			
				countries_store.loadData(data.countries);
				
			}
			
			// Suspects
			if(data.suspects) {	
				
				// Convert country codes to human readable format
				Ext.getCmp('d_suspectsPanel').getStore().loadData(data.suspects);
			
			}
			
			// Map
			if(data.bad_points || data.clusters) {
				waitForMap(function () {
					setMapData();
				});
			}
			
			if(data.sessions) {
				
				Ext.getCmp('d_sessionsLine').setWidth(Ext.getCmp('d_sessions').getWidth());
				
				var ansArray = data.sessions;
				var ans = [];	
				var fields = [];
				var scaleParam = Ext.getCmp('d_trendsScale').getValue();
				var gap = getTimeGap();
				
				for(var j=ansArray.length-1;j>=0;j--) {
				
					date = new Date(new Date(0).setUTCSeconds(ansArray[j][1]));
					
					if (scaleParam=='Now' | scaleParam=='Hour' | gap == 'hoursToDay' | gap == 'minutesToHour') {
						dateStr = Ext.Date.format(new Date(ansArray[j][1]*1000), 'G:i')
					}
					else {
						dateStr = Ext.Date.format(new Date(ansArray[j][1]*1000), 'd/m/y')
					}
						
					fields.push(dateStr);
					ans.push({	
						name:dateStr,
						Sessions:ansArray[j][0],
						epoch:date.getTime()
					});
					
				}
				
				var store = Ext.getStore('d_sessionsStore');	
				store.removeAll();
				store.loadData(ans);
				
			}
			
			if(data.trends) {
				
				var i = 0;
				var counts = [];
				var newList = [];
				var totalXFields = 0;
				var jsondata = data.trends;
				var gap = getTimeGap();
				
				//push to newList the rules for which we have alerts		
				Ext.Object.each(jsondata, function (key, value){	
						totalXFields = value.length;						
						if (hasAnyAlerts(value)){
							newList.push(key);
						}
				});				
				//check if we need to rebuild the store in case new rules were added from the last minute
				if (checkIfRulesChanged(newList) | newList.length==0)
					createTrendsAreaChart(newList,1);		
				lastRulesList = newList;
				var store = Ext.getCmp('d_trendsLine').getStore();
				store.removeAll();
				var yValues = prepareDatesArray(jsondata,gap);//an array that holds array, each array starts with the json name:date
				for(i=0; i<newList.length; i++){//go on the list of rule names that have at least 1 alert
					Ext.Object.each(jsondata,function(ruleName,count_date_arr){
						if (ruleName==newList[i]){
							for (j=0;j<count_date_arr.length;j++){
								yValues[count_date_arr.length-j-1][ruleName]=count_date_arr[j][0];
							}
						}
					});
				}
				if (newList.length==0){
					for (j=0;j<yValues.length;j++){
						yValues[j]["There Are No Alerts"]=0;
					}					
				}
				store.loadData(yValues);
				setTimeout(function () {
					$('#d_trendsLine rect:last').attr('width', (parseInt($('#d_trendsLine rect:last').attr('width')) + 7) + 'px').attr('ry', 1);
				}, 100);
			
			}

		}, "Error retreiving dashboard data.");
		
	}
}

function verifyFields(){
	var foundTrends,foundMap,foundCountries,foundAnomaly=false;
	for (var i=0;i<panelsOrder.length;i++){
		if (panelsOrder[i]=='d_trendsPanel') foundTrends=true;
		if (panelsOrder[i]=='d_anomalyPanel') foundAnomaly=true;
		if (panelsOrder[i]=='d_countriesPanel') foundCountries=true;
		if (panelsOrder[i]=='d_mapPanel') foundMap=true;
	}
	if (foundTrends & foundMap & foundCountries & foundAnomaly) return true;
	else return false;
}

function updatePanelsOrderInServer() {

	if (!verifyFields()) { return; }
	if(!telepath.access.has_perm('Dashboard', 'set')) { return; }
	
	telepath.util.request('/dashboard/set_dashboard_order', { order: panelsOrder.toString() }, function(data) {

		select_row(a_selected_row);
		return;
		
	}, 'Failed to set dashboard order.');

}

// HELPER FUNCTIONS

/*
 * Function:  checkIfRulesChanged
 * 
 * Invoked:
 * 	When we want to compare two arrays of rule groups strings, to know whether we need to reload the line graph of trends.
 * 	
 * 
 * 
 * Parameters:
 * 
 * 
 * 
 * Returns:
 * 	bool - 	true if we need to reload, false otherwise.
 * 
 */

//check if new rules were applied,if so, we need to reconstruct the area chart with new rules
function checkIfRulesChanged(newList){
	var found;
	for(var i=0;i<newList.length;i++){
		found=false;
		for(var j=0;j<newList.length;j++){
			if (newList[i] == lastRulesList[j])
				found = true;
		}
		if (!found)
			return true;//the list was changed
	}
	return false;
}

//checks if each rule has at least one alert, if so, it returns true
function hasAnyAlerts(value){
	var count = 0;
	for (var i=0;i<value.length;i++){
		count+=value[i][0];
	}
	if (count==0)
		return false;
	return true;
}

/*
 * Function:  prepareDatesArray
 * 
 * Invoked:
 * 	Before creating a new graph object for session and alerts, we need to prepare an array of dates for the x axis.	
 * 	
 * 
 * 
 * Parameters:
 * 	jsonObj - object.the response from the server. key = rule group name, value = an array [number_of_alerts,epoch date]
 * 	gap - string, a string indicating the gap between start date and end date. i.e hoursToDay, dayToWeek ...
 * 
 * 
 * Returns:
 * 	array -  [{epoch:12345,name:"1/1/13"},...]
 * 
 * See Also:
 * 	<getTimeGap>
 * 
 */
//gets the json obj and returns an array containing json with name:"relevant date"
function prepareDatesArray(jsonObj,gap){
	var scaleParam = Ext.getCmp('d_trendsScale').getValue();
	var ans = [];
	var date,dateStr;
	var stop=false;
	Ext.Object.each(jsonObj,function(key,value){
		for(var j=value.length-1;j>=0;j--){
			if(stop)
				return;
			date = new Date(new Date(0).setUTCSeconds(parseInt(value[j][1])))
			
			if (scaleParam=='Now' | scaleParam=='Hour' | gap == 'hoursToDay' | gap == 'minutesToHour') {
				dateStr = Ext.Date.format(new Date(value[j][1]*1000), 'G:i');
			}
			else {
				dateStr = Ext.Date.format(new Date(value[j][1]*1000), 'd/m/y');
			}
			ans.push({	
					name:dateStr,
					epoch:date.getTime()
				});
		}
		stop=true;
	});
	return ans;

}
/*
 * Function:  getTimeGap
 * 
 * Invoked:
 * 	Before creating a new graph object for session and alerts, we need to get an estimate of the scale we want to get
 * 	
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	string - "minutesToHour" | "hoursToDay" | "dayToWeek" | "weekToMonth" | "monthTo6months" | "6monthsToYear" | "aYearPlus"
 * 
 * See Also:
 * 	<prepareDatesArray>
 * 
 */
function getTimeGap(){
	var difference	= Ext.getCmp("d_endDate").getValue() - Ext.getCmp("d_startDate").getValue();
	var gap;
	if (difference>=0 & difference<=1000*60*60)
		gap = 'minutesToHour'
	else if (difference>1000*60*60 & difference<=1000*60*60*24)	
		gap = 'hoursToDay'
	else if (difference>1000*60*60*24 & difference<=1000*60*60*24*7)	
		gap = 'dayToWeek';
	else if(difference>1000*60*60*24*7 & difference<=1000*60*60*24*30)
		gap = 'weekToMonth';
	else if(difference>1000*60*60*24*30 & difference<=1000*60*60*24*365/2)
		gap = 'monthTo6months';
	else if(difference>1000*60*60*24*30 & difference<=1000*60*60*24*365)
		gap = '6monthsToYear';
	else if (difference>1000*60*60*24*365)
		gap = 'aYearPlus';	
	return gap;
}


function getDateForSessionsCount(string){
	var timeNow = new Date(string);
	var month = timeNow.getMonth()+1;
	if (month.toString().length==1)
		month = "0"+month;
	var day = timeNow.getDate();
	if (day.toString().length==1)
		day = "0"+day;
	var year = timeNow.getFullYear().toString().substring(2);
	var mins = timeNow.getMinutes()-1;//in case the server's clock is one minute late...take a minute before..
	if (mins.toString().length==1)
		mins = "0"+mins;
	var hours = timeNow.getHours();
	if (hours.toString().length==1)
		hours = "0"+hours;	
	
	return day+"-"+month+"-"+year+" "+hours+":"+mins;

}


// MAP RELATED

var mapCallback = false;
var mapInterval = false;

// Loop till there's d_map
function waitForMap(callback) {
	if (d_map==undefined) {
		mapCallback = callback;
		mapInterval = setInterval(function () {	if (d_map==undefined) {	} else { clearInterval(mapInterval); callback(); } }, 500);
	} else {
		callback();
	}
}

function setMapData() {
	
	//add listener for zoom event (hide clusters on bigger zoom etc)
	d_map.addListener("mapviewchangeend", function(evt) {
		if (evt.data & evt.MAPVIEWCHANGE_ZOOM){
			Ext.Object.each(clusters,function(key,value){
				if (value.radius < zoomToRadius(d_map.zoomLevel-1) ){//if the radius is 
					value.set('visibility',true);//make visible
				}
				else		
					value.set('visibility',false);
			});

		}
	});

	if(telepath.dashboard.data.bad_points) {
	
		//remove previous points
		if (bad_points.length!=0){
			Ext.Object.each(bad_points,function(key,value){	d_map.objects.remove(value); });
			bad_points = [];
		}
		
		if(currentBubble !== false && infobubbles) {
			infobubbles.removeBubble(currentBubble);
		}
		
		//create new points
		$.each(telepath.dashboard.data.bad_points, function(key, value) {
			
			if (!( parseFloat(value.latitude) + parseFloat(value.longitude) == 0 )) {
			
				bad_points[key] = new ovi.mapsapi.map.Marker([parseFloat(value.latitude),parseFloat(value.longitude)],{visibility:true,icon:"images/red_point_map.png", data: value.id });
				d_map.objects.add(bad_points[key]);

				//add binding to hover event
				bad_points[key].addListener("mouseover", function(evt) { badPointsHandler(evt,value.id, 'mouseover'); });
				bad_points[key].addListener("mouseout", function(evt)  { badPointsHandler(evt,value.id, 'mouseout');  });
				bad_points[key].addListener("click", function(evt)     { badPointsHandler(evt,value.id, 'click');     });
				
			}
			
		});
		
	}
	
	if(telepath.dashboard.data.clusters) {
		
		//remove old clusters
		if (clusters.length!=0){
			$.each(clusters,function(key,value){ d_map.objects.remove(value); });
			clusters = [];
		}
		
		//add new clusters
		$.each(telepath.dashboard.data.clusters, function (key, value) {
			
			clusters[key] = new ovi.mapsapi.map.Circle (
								//place the circle center here
								[parseFloat(value.x_centoid),parseFloat(value.y_centoid)],
								//radius of 8000 meters
								parseInt(parseFloat(value.radius)*90000.0),
								{
									visibility :true,
									color: "#81a000",
									fillColor: "#9967",
									width: 1,
									cursor: "pointer"
									//eventListener : ('click',function(event){},
								});
								
			d_map.objects.add(clusters[key]);
		
		});
	
	}
	
	// Click on map to remove bubble
	$('iframe').each(function () {
		if($(this)[0].contentWindow.$) {
			$(this)[0].contentWindow.$('#map').click(function () {
				if(currentBubble !== false && infobubbles) {
					infobubbles.removeBubble(currentBubble);
				}
			});
		}
	});
	

}

/*
 * Function:  zoomToRadius
 * 
 * Invoked:
 * 	Aid function for zoom event on map. returns a radius, for which every cluster on the map with bigger radius needs to be hidden, and smaller should be shown
 * 	
 * 
 * 
 * Parameters:
 * 	zoom- int, the zoom level of the event
 * 
 * Returns:
 * 	a - int, the radius to hide from
 * 
 * See Also:
 *
 * 	<map>
 * 
 */
function zoomToRadius(zoom){
	var a = -110572*(zoom-7)+233124;
	return a;
}


