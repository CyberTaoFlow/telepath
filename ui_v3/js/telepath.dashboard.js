telepath.dashboard = {
	
	divs:0,
	data: { items: {} },
	sort: 'count',
	dir: false,
	refreshTimer: false,
	refreshInterval: 5,
	reloadFlag: Date.now(),
	loading: false,
	map_mode:false,
	getData: function() {
		
		this.loading = true;
		$('.tele-panel-dashboard-refresh-button').hide();
		$('.tele-panel-dashboard-refresh').css('paddingRight','+=47px');
		setTimeout(function () {
			telepath.dashboard.loading = false;
			$('.tele-panel-dashboard-refresh-button').fadeIn();
			$('.tele-panel-dashboard-refresh').css('paddingRight','-=47px');
		}, 5000);
		
		var that = this;
		
		telepath.ds.get('/dashboard/get_map', { }, function (data, flag) {
			if (flag && telepath.dashboard.reloadFlag && flag != telepath.dashboard.reloadFlag)
			{
				// date filter was changed !
				return;
			}
			telepath.dashboard.map_mode=data.items.map_mode;
			$('.tele-panel-dashboard .tele-panel-subtitle-right .tele-mini-toggle').toggleFlip({flipped:data.items.map_mode=='traffic'}) ;
			telepath.dashboard.data.items.map = data.items.map;
			telepath.dashboard.map.vMap({ data: telepath.dashboard.data.items.map, title: data.items.map_mode=='traffic'?'Traffic over time':'Alerts over time'});
			$(window).trigger('resize');
		}, null, telepath.dashboard.reloadFlag);

		telepath.ds.get('/dashboard/get_chart', { }, function (data, flag) {
			if (flag && telepath.dashboard.reloadFlag && flag != telepath.dashboard.reloadFlag)
			{
				// date filter was changed !
				return;
			}
			telepath.dashboard.data.items.chart = data.items.chart;
			telepath.dashboard.drawGraph();
			$(window).trigger('resize');
		}, null, telepath.dashboard.reloadFlag);

		telepath.ds.get('/dashboard/get_suspects', { sort: this.sort, dir: this.dir }, function (data, flag) {
			if (flag && telepath.dashboard.reloadFlag && flag != telepath.dashboard.reloadFlag)
			{
				// date filter was changed !	
				return;
			}
			telepath.dashboard.data.items.suspects = data.items.suspects;
			telepath.dashboard.initSuspects();
			$(window).trigger('resize');
		}, null, telepath.dashboard.reloadFlag);

		telepath.ds.get('/dashboard/get_alerts', { sort: this.sort, dir: this.dir }, function (data, flag) {
			if (flag && telepath.dashboard.reloadFlag && flag != telepath.dashboard.reloadFlag)
			{
				// date filter was changed !
				return;
			}
			telepath.dashboard.data.items.alerts = data.items.alerts;
			telepath.dashboard.initAlerts();
			$(window).trigger('resize');
		}, null, telepath.dashboard.reloadFlag);

		telepath.ds.get('/dashboard/get_cases', {}, function (data, flag) {
			if (flag && telepath.dashboard.reloadFlag && flag != telepath.dashboard.reloadFlag)
			{
				// date filter was changed !
				return;
			}
			// Hide deleted cases. Yuli
			var cases = [];
                        var index;
						var caseCount=(data.items.cases)?data.items.cases.length:0;
                        // limited the max display to 5, hilik
                      //  var minimum = Math.min(6,caseCount);
                        for (index = 0; index < caseCount; ++index){
				//if (data.items.cases[index].name)
				//{
				cases.push(data.items.cases[index]);
				//}
			}
			//telepath.dashboard.data.items.cases = data.items.cases;
			telepath.dashboard.data.items.cases = cases;
			telepath.dashboard.initCases();
			$(window).trigger('resize');
		}, null, telepath.dashboard.reloadFlag);

	},
	
	// Init CASES
	initCases:function(){
	
		var that = this;		
	
		if(telepath.access.admin || telepath.access.perm.Cases_get) {
			this.divs +=1;
			this.casesList = $('<div>').addClass('tele-dashboard-block');
			$('.tele-panel-dashboard').append(this.casesList);

			this.casesList.teleList({
			title: 'Recent Cases',
			titleCallback: function () {
				$(".tele-nav-cases a").click();
			},
			data: this.data.items.cases,
			formatter: function(item) {
				return telepath['case'].rowFormatter(item, 'dashboard');
			},
			callbacks: telepath.listitem.generic.callbacks_case
			});
		}
	},

	// Init Alerts
	initAlerts: function(){
		
		var that = this;
	
		if(telepath.access.admin || telepath.access.perm.Alerts_get) {
			this.divs +=1;
			this.alertsList = $('<div>').addClass('tele-dashboard-block');
			$('.tele-panel-dashboard').append(this.alertsList);

			this.alertsList.teleList({
			title: 'Recent Alerts',
			titleCallback: function () {
				$(".tele-nav-alerts a").click();
			},
			data: this.data.items.alerts.items,
			formatter: function(item) {
				item.checkable = false;
				return telepath.alert.rowFormatter(item,'dashboard');
			}
			});
		}
	},

	// Init Suspects
	initSuspects: function(){
		
		var that = this;
	
		if(telepath.access.admin || telepath.access.perm.Suspects_get) {
		
			this.divs +=1;
			this.suspectsList = $('<div>').addClass('tele-dashboard-block');
			$('.tele-panel-dashboard').append(this.suspectsList);

			this.suspectsList.teleList({
			title: 'Top Suspects',
			titleCallback: function () {
				$(".tele-nav-suspects a").click();
			},
			data: this.data.items.suspects.items,
			formatter: function(item) {
				item.checkable = false;
				return telepath.suspects.rowFormatter(item,'dashboard');
				}
			});
		}
	},
	refresh: function(callback) {
		this.init();
	},
	init: function () {
		
		//if(this.loading > 0) {
		//	console.log('Still loading, no refresh');
		//	return;
		//}
		
		var that = this;

		this.getData();
		this.resetContainer();
		this.showFilters();
		this.resize();
	
		if($('.tele-panel-dashboard').size() > 0) {
			$(window).resize(function () { that.resize(); });
		}
	},
	drawGraph: function () {
		
		var timeformat = ((telepath.range.end - telepath.range.start) / 3600 > 48) ? "%d/%m/%y" : "%d/%m %h:%M:%S";
	
		var options = {
		
			legend: { show: false },
			series: { lines: { show: true, fill: true }, points: { show: true, fillColor: '#446077' } },
			yaxis: { alignTicksWithAxis: true, labelWidth: 30, ticks: 5, color: '#446077', font: { family: 'Arial', color: '#cccccc', size: 11, weight: "normal" } },
			selection: { mode: "x" },
			xaxis: { alignTicksWithAxis: true, tickColor: '#cccccc', tickLength: 7, font: { family: 'Arial', color: '#cccccc', size: 11, weight: "normal" }, mode: "time", timezone: "browser", timeformat: timeformat },
			grid: { borderColor: '#446077', borderWidth: 1, hoverable:true, clickable:true },
			tooltip:true,
			tooltipOpts:{
				content: "%s | date & time: %x | sessions: %y"
			}
			
		};

		var chartData = [{ label: "Alerts", 		     data: this.data.items.chart.alerts,      color: '#64a5bc' },
						 { label: "Normal",      data: this.data.items.chart.sessions,    color: '#986da0' },
						 { label: "Cases",       data: this.data.items.chart.cases, color: '#ff850b' },
						 { label: "Suspects", data: this.data.items.chart.suspects,    color: '#6ab789' }
						//{label: "Score", data: this.data.items.chart.score}
		];
		
		this.graph.flotGraph({ data: chartData, options: options, dashboard: true, title: 'Overall Transactions' });
	
	},
	resetContainer: function (loading) {
		this.divs = 0;
		var that = this;
		$('.tele-panel-dashboard .tele-panel-subtitle-right .tele-mini-toggle').toggleFlip({flipped:telepath.dashboard.data.items.map =='traffic'}) ;
		var container = $('.tele-panel-dashboard');
		container.empty();
		
		container.append('<div class="tele-panel-topbar"><div class="tele-panel-title">Dashboard</div><div class="tele-panel-topbar-right"></div></div>');
		
		if(!loading) {
		
			container.append('<div class="tele-panel-subtitle"><div class="tele-panel-subtitle-text">Traffic and Alerts Trends</div><div class="tele-panel-subtitle-right"></div></div>');
			
			// Dashboard Toggle
			var trafficToggle = $('<div>').toggleFlip({ left_value: 'Alerts', right_value: 'Traffic',flipped: that.map_mode=='traffic',  flip: function (x,y) {
				that.map.empty();
				that.map.append(telepath.loader);
				if(x) {
					telepath.ds.get('/dashboard', { mode: 'map_traffic' }, function(data) { 
						telepath.dashboard.data.items.map = data.items.traffic;
						that.map.vMap({ data: data.items.traffic, title: 'Traffic over time' });
					});
				} else {
					telepath.ds.get('/dashboard', { mode: 'map_alerts' }, function(data) { 
						telepath.dashboard.data.items.map = data.items.map;
						that.map.vMap({ data: data.items.map, title: 'Alerts over time' });
					});
				}
			}});
			
			
			// Dashboard Toggle
			var realtimeToggle = $('<div>').toggleFlip({ left_value: 'Static', right_value: 'Realtime', flip: function (x,y) {
				if(x) {
					that.graph.hide();
					that.graph_realtime.show();
					telepath.realtime.start(that.graph_realtime);
					$('.tele-panel-subtitle-text').html('Realtime Monitor');
				} else {
					that.graph.show();
					that.graph_realtime.hide();
					telepath.realtime.stop();
					$('.tele-panel-subtitle-text').html('Traffic and Alerts Trends');
				}
			} }).hide();
			
			$('.tele-panel-dashboard .tele-panel-subtitle-right').append(realtimeToggle);
			$('.tele-panel-dashboard .tele-panel-subtitle-right').append(trafficToggle);
			
			// for alerts :: #ff5f5f
			
			// Map
			var graph_div = $('<div>').css("background-color", "#304f68").css("width", "100%");
			var empty_div = $('<div>').css("clear", "both");
			this.map = $('<div>');
			this.graph = $('<div>');
			graph_div.append(this.map);
			graph_div.append(this.graph);
			graph_div.append(empty_div);
			container.append(graph_div);
			
			this.graph_realtime = $('<div>').addClass('tele-graph-realtime');
			container.append(this.graph_realtime);
			
		} else {
		
			container.append(telepath.loader);
			
		}

	},
	resize: function () {

		if($('.tele-panel-dashboard').children().size() == 0) return;
		
		var width = $(window).width();

		var current_div = parseInt(100/this.divs);
		
		if(width < 1025) {
		
			$('.tele-map').width('100%');
			$('.tele-graph').width('100%');
			$('.tele-dashboard-block').width('100%');
		
		} else {
		
			$('.tele-map').width('33%');
			$('.tele-graph').width('67%');
			$('.tele-dashboard-block').width(current_div + '%');
		
		}
	
	},
	updateRefreshInterval: function(interval) {
		
		if(interval) {
			this.refreshInterval = interval;
		}
		
		this.cmdRefreshValue.html(this.refreshInterval + 'm');
		if(this.refreshTimer) {
			clearInterval(this.refreshTimer);
		}
		this.refreshTimer = setInterval(function () {
			telepath.dashboard.refresh();
		}, this.refreshInterval * 60000); // In minutes, need millisecond format
		
	},
	showFilters: function () {
		
		var that = this;
		
		// Container
		var container = $('.tele-panel-dashboard .tele-panel-topbar-right');
		
		container.empty();
		
		// Sort

		// Applications
		
		var filterApps		     = $('<div>').appSelect({ callback: function (app_id) {
			$('.tele-icon-application', filterApps).removeClass('tele-icon-application').addClass('tele-icon-loader');
			telepath.dashboard.refresh(function () {
				$('.tele-icon-loader', filterApps).addClass('tele-icon-application').removeClass('tele-icon-loader');
			});
		}});
		
		
		// Refresh
		var cmdRefresh 			 = $('<div>').addClass('tele-panel-dashboard-refresh');
		var cmdRefreshText       = $('<a>').attr('href', '#').addClass('tele-panel-dashboard-refresh-text').html('Refresh Rate');
		this.cmdRefreshValue      = $('<a>').attr('href', '#').addClass('tele-panel-dashboard-refresh-value').html(this.refreshInterval + 'm');
		var cmdRefreshButton     = $('<a>').attr('href', '#').addClass('tele-panel-dashboard-refresh-button').html('&nbsp;');
		cmdRefresh.append(cmdRefreshText).append(this.cmdRefreshValue).append(cmdRefreshButton);
		
		cmdRefreshButton.click(function () {
		if (!telepath.dashboard.loading){
			$(this).addClass('loader');
			var that = this;
			telepath.dashboard.refresh(function () {
				$(that).removeClass('loader');
			});
		}
		});
		
		// DateRange
		var filterDateRange 	  = $('<div>').daterange({ 
			
			start: telepath.range.start, 
			end: telepath.range.end, 
			change: function(start, end) { 
				telepath.dashboard.reloadFlag = Date.now();
				cmdRefreshButton.addClass('loader');
				telepath.dashboard.refresh(function () {
					cmdRefreshButton.removeClass('loader');
				});

		}});
		
		// Sort filters
		var sortRadios = $('<div>').radios({ 
			title: 'Sort By', 
			items: [
				{id: 'date', icon: 'time', tip: 'Time'},
				{id: 'count', icon: 'bars', tip: 'Count'}
			], 
			selected: this.sort,
			callback: function(e, id) {
				if(that.sort == id) {
					that.dir = !that.dir;
				}			
				that.sort = id;
				that.refresh();
			}
		});
		
		// Append All
		container.append(sortRadios).append('<div class="tele-navsep"></div>').append(filterDateRange).append('<div class="tele-navsep"></div>').append(filterApps).append('<div class="tele-navsep"></div>').append(cmdRefresh);
		
		$('.tele-panel-dashboard-refresh-value').click(function () {
			
			$('body').append('<div class="tele-popup tele-refreshrate-popup"><input class="tele-refreshrate-slider" /></div>');
			
			if($(".tele-refreshrate-popup").css('display') == 'block') {
				$(".tele-refreshrate-popup").fadeOut('fast', function () { $(this).remove(); });
				return;
			}
			
			var top  = $(this).offset().top + $(this).height() + 20;
			var left = ($(this).offset().left + ($(this).width() / 2)) - ($(".tele-daterange-popup").width() / 2) - 150;

			$(".tele-refreshrate-popup").css({ top: top , left: left });
			$(".tele-refreshrate-popup").fadeIn();
			//$(".tele-refreshrate-slider").slider('destroy');
			that.refresh_slider = $(".tele-refreshrate-slider").slider({
				orientation: "horizontal",
				//range: "min",
				max: 60,
				min: 1,
				value: parseInt(telepath.dashboard.refreshInterval),
			}).on('slide', function(ev){
				var value = telepath.dashboard.refresh_slider.val();
				telepath.dashboard.updateRefreshInterval(value);
			});
			
		});
		
	}
}

telepath.realtime = {
	container: false,
	timer: false,
	count: 0,
	options: {
		legend: {
			show: true,
			color: 'yellow',
			//labelFormatter: null or (fn: string, series object -> string)
			labelBoxBorderColor: "white",
			//noColumns: number
			position: "sw", // or "nw" or "se" or "sw"
			//margin: number of pixels or [x margin, y margin]
			backgroundColor: 'white',
			backgroundOpacity: 1,
			margin: 10
			//container: null or jQuery object/DOM element/jQuery expression
	    },
		series: { lines: { show: true, fill: true }, points: { show: true, fillColor: '#446077' }, color: 'yellow' },
		xaxis: {
			mode: "time",
			tickFormatter: function (v, axis) {
				var date = new Date(v);
				var hours = date.getHours() < 10 ? "0" + date.getHours() : date.getHours();
				var minutes = date.getMinutes() < 10 ? "0" + date.getMinutes() : date.getMinutes();
				var seconds = date.getSeconds() < 10 ? "0" + date.getSeconds() : date.getSeconds();
				return hours + ":" + minutes + ":" + seconds;
			},
			axisLabel: "Time",
			axisLabelUseCanvas: true,
			axisLabelFontSizePixels: 12,
			axisLabelFontFamily: 'Verdana, Arial',
			axisLabelPadding: 10
		},
		yaxes: [
			{
				axisLabel: "Dropped",
				axisLabelUseCanvas: true,
				axisLabelFontColor: '#cecece',
				font: { family: 'Arial', color: 'white', size: 11, weight: "normal" },
				axisLabelPadding: 6,
				min: 0
			}, {
				position: "right",
				axisLabel: "Requests",
				axisLabelUseCanvas: true,
				axisLabelFontColor: '#cecece',
				font: { family: 'Arial', color: 'white', size: 11, weight: "normal" },
				axisLabelPadding: 6
			}, {
				position: "right",
				axisLabel: "Packets",
				axisLabelUseCanvas: true,
				axisLabelFontColor: '#cecece',
				font: { family: 'Arial', color: 'white', size: 11, weight: "normal" },
				axisLabelPadding: 14
			}
		],
		grid: { borderColor: '#446077', borderWidth: 1 },
	},
	offsets: {
		init: false,
		packets: 0,
		loss: 0,
		requests: 0
	},
	start: function (container) {
	
		// console.log('starting realtime');
		this.container = container;
		
		var totalPoints = 60;
		var updateInterval = 1000;
		var now = new Date().getTime() - (totalPoints * updateInterval);
		
		this.data = { packets: [], loss: [], requests: [] };

		for (var i = 0; i < totalPoints; i++) {
			var temp = [now += updateInterval, 0];
			this.data.packets.push(temp);
			this.data.loss.push(temp);
			this.data.requests.push(temp);
		}
		
		/*var options = {
			yaxis: { alignTicksWithAxis: true, labelWidth: 30, ticks: 5, color: '#446077', font: { family: 'Arial', color: '#cccccc', size: 11, weight: "normal" } },
			xaxis: { alignTicksWithAxis: true, tickColor: '#cccccc', tickLength: 7, mode: "time", timezone: "browser", timeformat: timeformat },
		};*/

		
		var dataset = [
			{ label: "Packets:", data: this.data.packets, lines: { fill: true, lineWidth: 1.2 }, color: "#6ab789" },
			{ label: "Dropped:", data: this.data.loss, lines: { lineWidth: 1.2}, color: "#64a5bc", yaxis: 3 },
			{ label: "Requests:", data: this.data.requests, color: "#986da0", bars: { show: true }, yaxis: 2 }
		];
		
		$.plot($(this.container), dataset, this.options);
		
		this.timer = setInterval(this.getData, updateInterval);
		
	},
	stop: function () {
		clearInterval(this.timer);
		this.container.empty();
		// console.log('stopping realtime');
	},
	tick: function() {
		// console.log(telepath.realtime.count);
		telepath.realtime.count = telepath.realtime.count + 1;
	},
	getData: function() {
		
		$.ajaxSetup({ cache: false });

		$.ajax({
			url: telepath.controllerPath + "/realtime/index",
			dataType: 'json',
			success: function (_data) {	
				
				_data['capture.kernel_packets'] = parseInt(_data['capture.kernel_packets']);
				_data['capture.kernel_drops'] = parseInt(_data['capture.kernel_drops']);
				
				if(!telepath.realtime.offsets.init) {
					telepath.realtime.offsets.packets  = _data['capture.kernel_packets'];
					telepath.realtime.offsets.loss	   = _data['capture.kernel_drops'];
					telepath.realtime.offsets.requests = _data['cmd_set'];
					telepath.realtime.offsets.init = true;
					return;
				} 
				
				telepath.realtime.data.packets.shift();
				telepath.realtime.data.loss.shift();
				telepath.realtime.data.requests.shift();
				
				var now = new Date().getTime();
				
				telepath.realtime.data.packets.push([ now, _data['capture.kernel_packets'] - telepath.realtime.offsets.packets ]);
				telepath.realtime.data.loss.push([ now, _data['capture.kernel_drops'] - telepath.realtime.offsets.loss ]);
				telepath.realtime.data.requests.push([ now, _data.cmd_set - telepath.realtime.offsets.requests ]);
				
				var dataset = [
					{ label: "Dropped", data: telepath.realtime.data.loss, lines: { lineWidth: 1.2}, color: "#640000" },
					{ label: "Packets", data: telepath.realtime.data.packets, lines: { fill: true, lineWidth: 1.2 }, color: "#986da0", yaxis: 3 },
					{ label: "Requests", data: telepath.realtime.data.requests, color: "#6ab789", yaxis: 2 }
				];
							
				$.plot($(telepath.realtime.container), dataset, telepath.realtime.options);
				
				telepath.realtime.offsets.packets  = _data['capture.kernel_packets'];
				telepath.realtime.offsets.loss	   = _data['capture.kernel_drops'];
				telepath.realtime.offsets.requests = _data['cmd_set'];
			
							
			},
			error: function () {
				
			}
		});
		
		$.ajaxSetup({ cache: true });
		
	}
}

