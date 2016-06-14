telepath.scheduler = {
	
	displaySchedule: function(store_id) {
		
		if(telepath.scheduler.initialized) {
		
			telepath.scheduler.fetchData(store_id, function (eventsData) {
				telepath.scheduler.showWindow(eventsData);
			});
		
		} else {
			
			telepath.scheduler.loadRes(function () {
			
				telepath.scheduler.initialized = true;
				
				telepath.scheduler.fetchData(store_id, function (eventsData) {
					telepath.scheduler.showWindow(eventsData);
				});
				
			});

		}

	},
	init: function() {
		
		// Load Scheduler 
		telepath.scheduler.loadRes(function () {
			telepath.scheduler.initialized = true;
		});
		
	},
	parseRanges: function(eventsData) {
		
		var events = [];
		
		function GetDaysOfWeek(date) {
			var days = new Array();
			for (var i = 0; i < 7; i++)
			{
				var tmp = new Date(date);
				days[i] = new Date(tmp.setDate(tmp.getDate() - tmp.getDay() + i));
			}
			return days;
		}
		
		var weekday = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
		
		var today = new Date();
		today.setMinutes(0);
		today.setSeconds(0);
		var days = GetDaysOfWeek(today);
		
		var index = 0;
		
		$.each(days, function(i, date) {
			
			var n = weekday[date.getDay()];
		
			var tracking = false;
			
			var current_to = false;
			var current_from = false;
			
			$.each(eventsData[n], function(i, val) {
				
				if(val == '1') {
					
					if(tracking == true) {
						if(i < 23) {
							date.setHours(i + 1);
						} else {
							date.setMinutes(59);
						}
						current_to   = new Date(date);
						
					} else {
					
						date.setHours(i);
						current_from = new Date(date);
						
						if(i < 23) {
							date.setHours(i + 1);
						} else {
							date.setMinutes(59);
						}
						
						current_to   = new Date(date);
					}
					tracking = true;
					
				} else {
				
					if(tracking == true) {
						events.push({ id: index, start: current_from, end: current_to, title: '' });
						index++;
						current_from = false;
						current_to = false;
					}
					
					tracking = false;
					
				}
				
			});
			
			if(current_from && current_to) {
				events.push({ id: index, start: current_from, end: current_to, title: '' });
				index++;
			}
		
		});
		
		return events;
		
	},
	fetchData: function(store_id, callback) {
		
		var mode;
		if (store_id=="c_schedulerStore")
			mode = "get_schedule"
		else if (store_id=="c_reportScheduler")
			mode = "get_report_schedule"
			
		telepath.scheduler.mode = mode;	
		
		telepath.util.request('/config/get_scheduler', { mode: mode }, function(data) {
			callback(data.scheduler);
		}, 'Error while trying to get the scheduler.');
		
	},	
	showWindow: function (eventsData) {
			
			Ext.define('MyApp.view.Scheduler', {
		
				extend: 'Ext.window.Window',
				modal: true,
				height: 445,
				width: 700,
				layout: {
					type: 'fit'
				},
				title: 'Scheduler',
				initComponent: function () {
					var me = this;
					me.callParent(arguments);
				}
			
			});
		
			var scheduler_window = Ext.create('MyApp.view.Scheduler');
			
			// On scheduler window render
			
			scheduler_window.addListener('render', function (component) {
				
				var eventData = {
					events : telepath.scheduler.parseRanges(eventsData)
				};
				
				$(component.body.dom).weekCalendar({
				
					timeslotsPerHour: 1,
					timeslotHeight: 15,
					defaultEventLength: 1,
					data: eventData,
					height: function($calendar) {
						return $(window).height();
					}
					
				});

				function displayMessage(message) {
				//console.log(message);
				}
				
				Ext.create('Ext.Button', {
					text: 'Apply',
					renderTo: component.body.dom,
					margin: '0px 0px 0px 620px',
					iconCls: 'icon-apply',
					handler: function() {
						
						var eventsToSave = [];
						
						$('.wc-cal-event', component.body.dom).each(function () { 
							if(typeof($(this).data('calEvent')) == 'object') {
								eventsToSave.push($(this).data('calEvent'));
							}							
						});
						
						window.eventsToSave = eventsToSave;
						
						var weekday = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
						
						var parsedEvents = {};
						
						for(var i = 0; i < 7; i++) {
						
							parsedEvents[weekday[i]] = [];
							
							for(var x = 0; x < 24; x++) {
								parsedEvents[weekday[i]][x] = 0;
							}
							
						}

						$.each(window.eventsToSave, function(i, event) {
							for(var i = event.start.getHours(); i < (((event.end.getHours() == 23 && event.end.getMinutes() == 59) || event.end.getHours() == 0) ? 24 : event.end.getHours()); i++) {
								parsedEvents[weekday[event.start.getDay()]][i] = 1;
							}
						});
						
						telepath.util.request('/config/set_scheduler', { mode: telepath.scheduler.mode, data: parsedEvents }, function(data) {
							scheduler_window.destroy();
						}, 'Error while trying to save the scheduler.');
						
					}
				});

			});
			
			scheduler_window.show();
		
	},
	show: function() {
		
	},
	loadRes: function (callback) {
	
		yepnope({
		
			load: [
				"lib/css/ui-lightness/jquery-ui-1.10.3.custom.min.css" + get_ts(),
				"lib/weekcalendar/jquery.weekcalendar.css"  + get_ts(),
				"lib/weekcalendar/jquery.weekcalendar.js"  + get_ts()
			],
			
			complete: function () { callback(); }
			
		});
		
	}

}