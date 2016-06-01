telepath.config.system = {

	init: function () {
		
		// Get full width
		this.contentRight.addClass('tele-stacked');
		this.contentLeft.addClass('tele-stacked');
		this.contentLeft.css({ width: '100%', minWidth: '100%', height: 100, maxHeight: 100 });
		this.contentRight.css({ width: '100%', minWidth: '100%' });
		this.contentRight.append(telepath.loader);
		this.loadConfig();
		
		this.showConfigSteps();
		this.contentRight.mCustomScrollbar({ advanced:{ updateOnContentResize: true },
			scrollButtons:{ enable: false } });
		
	},
	loadConfig: function() {
		
		var that = this;
		
		telepath.ds.get('/config/get_config', { }, function (data) {
			
			that.data = data;
			that.showConfig();
			
		}, 'Error loading configuration.');
	
	},
	showConfigSteps: function () {
		
		var that = this;
		
		this.steps = [
			{ id: 'mode', label: 'Operation Mode' },
			{ id: 'reports', label: 'Reports' },
		//	{ id: 'reports', label: 'Permissions' },
		//	{ id: 'audit', label: 'Audit Logs' },
			{ id: 'network', label: 'Network' },
			//{ id: 'lb', label: 'Load Balancers' },
			{ id: 'whitelist', label: 'Whitelists' },
		//	{ id: 'request', label: 'Request Analysis' },
			// { id: 'ua_ignore', label: 'UserAgent Ignore List' },
			{ id: 'ext_ignore', label: 'Extension Ignore List' },
		];
		
		this.stepContainer = $('<div>').addClass('tele-config-system-steps');
		this.contentLeft.append(this.stepContainer);

		$.each(this.steps, function(i, stepData) {
			
			var step  = $('<div>').addClass('tele-config-system-step').attr('id', 'tele-config-step-' + stepData.id);
			var index = $('<div>').addClass('tele-config-system-step-index').html(i + 1);
			var lbl   = $('<div>').addClass('tele-config-system-step-label').html(stepData.label);
			
			step.append(index).append(lbl);
			
			that.stepContainer.append(step);
			
			var arrow = $('<div>').addClass('tele-config-system-step-arrow');
			if(i == 0) {
				step.addClass('active');
			}
			if(i !== that.steps.length - 1) {
				that.stepContainer.append(arrow);
			}
			
			step.click(function () {
				
				var stepId = $(this).attr('id').split('-')[3];
				$('.tele-config-system-tab').hide();
				$("#file-upload").hide();
				$('.tele-config-system-' + stepId).show();
				if(stepId=="mode"){
					$("#file-upload").show();
				}
				
				// Paint it orange..
				$('.active', that.stepContainer).removeClass('active'); // Cleanup
				$(this).addClass('active'); // Self
				$(this).prevAll().addClass('active'); // Previous
				$('.tele-config-content-right').mCustomScrollbar('update');
				
			});

		});
		
		$(window).resize(function () {
			
			if($('.tele-config-system-steps').size() > 0) {
				setTimeout(function() {
					telepath.config.system.updWidth();
				}, 100);
			}
			
		});
		
		this.updWidth();
	
	},
	updWidth: function() {
		
		var mid_width  = ($('.tele-config-system-step').outerWidth() * (this.steps.length)) / 2;
		var mid_screen = this.stepContainer.parent().width() / 2;
	
		this.stepContainer.css({ marginLeft: mid_screen - mid_width - 40 });
	
	},
	saveConfig: function() {
		
		var that = this;

		//calEvents = that.cal.weekCalendar("getEvents");
		//console.log($(that.scheduler).data);
	
		// Build output data object
		var data = {};
		
		// Input Mode
	
		var t_engine  = this.telepathEngineToggle.data('tele-toggleFlip').options.flipped;	
		var t_inline  = this.reverseProxyToggle.data('tele-toggleFlip').options.flipped;
		var t_sniffer = this.snifferToggle.data('tele-toggleFlip').options.flipped;
		data.engine_mode_id = (t_engine) ? 1: 0;
		data.sniffer_mode_id = (t_sniffer) ? 1: 0;
		data.reverse_proxy_mode_id = (t_inline) ? 1: 0;
		
		data.input_mode = 'off';
		
		if(t_sniffer && t_inline) {
			data.input_mode = 'both';
		} else {
			if(t_inline)  { data.input_mode = 'inline'; }
			if(t_sniffer) { data.input_mode = 'sniffer';  }
		}
		
		// Operation Mode
		//var selected_opmod = this.opmod.data('tele-teleRadios').options.checked;
		//switch(selected_opmod) {
		//	case 'training':   data.operation_mode_id = 1; break;
		//	case 'production':     data.operation_mode_id = 2; break;
		//	case 'hybrid':     data.operation_mode_id = 3; break;
		//}


		data.move_to_production_id = $('input', this.move_to_production_id).val();
		
		// Reports
	
		data.add_unknown_applications_id = this.addUnknownAppToggle.data('tele-toggleFlip').options.flipped ? 1 : 0;

		// Syslog
		data.write_to_syslog_id  = this.syslogToggle.data('tele-toggleFlip').options.flipped ? 1 : 0;
		data.syslog_ip_id = $('input', this.syslogIP).val();
				
		// Proxy
		data.proxy_mode_id = this.proxyToggle.data('tele-toggleFlip').options.flipped ? 1 : 0;
		data.proxy_ip_id   = $('input', this.proxyIP).val();
		data.proxy_port_id = $('input', this.proxyPort).val();
		
		// SMTP
		data.smtp      = $('input', this.smtpServer).val();
		data.smtp_port_id = $('input', this.smtp_port_id).val();
		data.rep_user  = $('input', this.smtpUser).val();
		data.rep_pass_id  = $('input', this.smtpPass).val();
		
		
		
		// Load Balancer
		// IPS
		/*data.loadbalancer_mode_id = this.lbToggle.data('tele-toggleFlip').options.flipped ? 1 : 0;
		data.loadbalancerips_id = [];
		$('.tele-ip', this.lbIPs).each(function () {
			var ip = $(this).data('tele-ip').getIP();
			if(ip) {
				data.loadbalancerips_id.push(ip);
			}
		});
		
		// De-Dupe
		data.loadbalancerips_id = data.loadbalancerips_id.filter(function(elem, pos) {
			return data.loadbalancerips_id.indexOf(elem) == pos;
		});
		data.loadbalancerips_id = data.loadbalancerips_id.join(',');
		
		// Headers
		data.loadbalancerheaders_id = [];
		$('input', this.lbHeaders).each(function () {
			var value = $(this).val();
			if(value != '') {
				data.loadbalancerheaders_id.push(value);
			}
		});
		data.loadbalancerheaders_id = data.loadbalancerheaders_id.join(',');*/


		/*var loadbalancer  = this.loadbalancer_mode_id.data('tele-toggleFlip').options.flipped;

		data.loadbalancer_mode_id = (loadbalancer) ? 1: 0;*/

		data.loadbalancer_mode_id = this.ipToggle.data('tele-toggleFlip').options.flipped ? 1 : 0;
		//data.loadbalancer_mode_id = this.ipToggle.data('toggleFlip').options.flipped ? 1 : 0;

		data.header_balances=[];

		//headerBalances

		$('.tele-input input', this.headerBalances).each(function () {
			if($(this).val() != '') {
				data.header_balances.push($(this).val());
			}
		});

		data.ip_balances=[];

		$('.tele-ip-wrap-lb', this.ips).each(function () {

			var is_range = $('.tele-mini-toggle', this).data('tele-toggleFlip').options.flipped;

			var ip_start = $('.tele-ip:first', this).data('tele-ip').getIP();
			var ip_end   = $('.tele-ip:last', this).data('tele-ip').getIP();

			if(is_range) {
				if(ip_start && ip_end && ip2long(ip_start) < ip2long(ip_end)) {
					data.ip_balances.push({from: ip_start ,to: ip_end});
				}
			} else {
				if(ip_start) {
					data.ip_balances.push({from:ip_start, to: ip_start});
				}
			}

		});

		// De-Dupe
		data.ip_balances = data.ip_balances.filter(function(elem, pos) {
			return data.ip_balances.indexOf(elem) == pos;
		});




		// IP Whitelist
		data.whitelist = [];

		$('.tele-ip-wrap', this.whitelist).each(function () {
					
			var is_range = $('.tele-mini-toggle', this).data('tele-toggleFlip').options.flipped;
			
			var ip_start = $('.tele-ip:first', this).data('tele-ip').getIP();
			var ip_end   = $('.tele-ip:last', this).data('tele-ip').getIP();
			
			if(is_range) {
				if(ip_start && ip_end && ip2long(ip_start) < ip2long(ip_end)) {
					data.whitelist.push({from: ip_start ,to: ip_end});
				}
			} else {
				if(ip_start) {
					data.whitelist.push({from:ip_start,to: ip_start});
				}
			}

		});
		
		// De-Dupe
		data.whitelist = data.whitelist.filter(function(elem, pos) {
			return data.whitelist.indexOf(elem) == pos;
		});
		
		data.agents = [];
		// Network Interfaces
		$('.tele-network-wrap', this.interfaces).each(function () {
		
			//var IDX  = $('.tele-network-idx', this).html();
			var NAME = $('.tele-network-name input', this).val();
			var ETH  = $('.tele-network-select', this).val();
			var MASK = $('.tele-network-filter input', this).val();
			
			data.agents.push({ /*idx: IDX,*/ agent_name: NAME, interface_name: ETH, pcap_filter: MASK });

		});

		data.regex = { headers: '', URL: '' };
				
		var useragents = [];
		// User Agent Ignore List
		$('.tele-input input', this.useragents).each(function () {
			if($(this).val() != '') {
				useragents.push($(this).val());		
			}
		});
		
		var URL = [];
		// Extension Ignore List
		$('.tele-input input', this.extentsions).each(function () {
			if($(this).val() != '') {
				URL.push($(this).val());		
			}
		});
		
		if(useragents.length > 0) {
			useragents = useragents.join(")|(");
			useragents = "(User-Agent:)(.*)(("+ useragents +"))";
			data.regex.headers = useragents;
		}

		if(URL.length > 0) {
			//URL = URL.join(")|(\\.");
			//URL = "(\\." + URL + ")";
			data.regex.URL = URL;
		}



		data.scheduler={};

		var Events = $(that.cal).weekCalendar("serializeEvents");

		var weekday = new Array(7);
		weekday[0]=  "Sunday";
		weekday[1] = "Monday";
		weekday[2] = "Tuesday";
		weekday[3] = "Wednesday";
		weekday[4] = "Thursday";
		weekday[5] = "Friday";
		weekday[6] = "Saturday";



		$.each(weekday, function(i,val){

			data.scheduler[val]=[];

		});

		$.each(Events,function(i, val){

			var from = new Date(val.start);

			var to = new Date(val.end);

			var dey = weekday[from.getDay()];

			data.scheduler[dey].push({from:parseInt(from.getHours()),to:parseInt(to.getHours())});


		});

		data.app_list_was_changed_id= '1';

		telepath.ds.get('/config/set_config', data, function (data) {
			
			//that.data = data;
			//that.showConfig();
			
			telepath.dialog({ msg: 'Configuration Saved' });
			
		}, 'Error saving configuration.');
		
	},
	updateEngineStatus: function () {
		if($(telepath.config.system.telepathEngineToggle).size() > 0) {
			telepath.ds.get('/telepath/get_engine_status', { }, function (data) {
				if ($(telepath.config.system.telepathEngineToggle).data('teleToggleFlip').options)
				{
					$(telepath.config.system.telepathEngineToggle).data('teleToggleFlip').options.flipped = data.status;
				}
				$(telepath.config.system.telepathEngineToggle).data('teleToggleFlip')._update();
			});
		}
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
			var current_to = '';
			var current_from = '';
			$.each(eventsData, function (num, value) {

				if (n==value._id) {


					$.each(value._source.times, function (j, val) {

						date.setHours(val.from);
						date.setMinutes(0);
						date.setSeconds(0);
						current_from = new Date(date);
						//date.setHours(i + 1);
						date.setHours(val.to);
						
						current_to = new Date(date);
						events.push({id: index, start: current_from, end: current_to, title: ''});
						index++;
					});
				}
			});

		});

/*	
			$.each(eventsData[n], function(num, val) {
				i = parseInt(num.substring(1));
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
*/

		var that = this;

		that.index=index;

		return events;
		
	},
	showConfig: function () {
		
		var that = this;
	
		// Container		
		this.contentRight.empty();
		this.container = $('<div>').addClass('tele-config-container').appendTo(this.contentRight);
		
		// -----------------------------------------------------------
		// Operation Mode
		// -----------------------------------------------------------
		this.c_mode = $('<div>').addClass('tele-config-system-tab tele-config-system-mode');
		this.container.append(this.c_mode);
		
		$('<div>').addClass('tele-title-1').html('Hybrid Mode Schedule').appendTo(this.c_mode);
        //
		//var selected_opmod = '';
		//switch(this.data.operation_mode_id) {
		//	case '1':	selected_opmod = 'training';   break;
		//	case '2':	selected_opmod = 'production';	   break;
		//	case '3':	selected_opmod = 'hybrid';	   break;
		//}
		
		this.scheduler = $('<div>').addClass('tele-scheduler').appendTo(this.c_mode);
		
		/*this.opmod = $('<div>').teleRadios({
		checked: selected_opmod,
		radios: [ 
			{ key: 'training',   label: 'Training' },
			{ key: 'hybrid',     label: 'Hybrid' },
			{ key: 'production', label: 'Production' },
		], callback: function(radio) {
			
			if(radio.key == 'hybrid') {
				that.scheduler.show();
			} else {
				that.scheduler.hide();
			}
			
		}}).addClass('tele-config-opmod').appendTo(this.c_mode);*/
		
		telepath.ds.get('/config/get_scheduler', { mode: "get_schedule" }, function(data) {
			
			var eventData = { events : that.parseRanges(data.scheduler)	};
			that.cal = $(that.scheduler).weekCalendar({
		
				timeslotsPerHour: 1,
				timeslotHeight: 15,
				defaultEventLength: 1,
				data: eventData,
				height: function($calendar) {
					return 400;
				},
				resizable : function(calEvent, element) { return false;},
				// event drag is disabled. the user had to remobe old and create new event
				draggable : function(calEvent, element) { return false;},
				eventNew: function(calEvent, element, dayFreeBusyManager,
								   calendar, mouseupEvent) {
					calEvent.id=that.index;
					that.index++;
				},
				eventDelete: function(calEvent, element, dayFreeBusyManager,
									  calendar, clickEvent) {
						calendar.weekCalendar('removeEvent', calEvent.id);

				},
				eventDrop: function(newCalEvent, oldCalEvent, element){
				}
			});


		}, 'Error while trying to get the scheduler.');

		// MV2Prod after
		this.move_to_production_id = $('<div>').teleInput({
			label: 'Default learning threshold per application',
			suffix: 'Requests', 
			width: 70, 
			value: this.data.move_to_production_id
		}).addClass('tele-config-mv2prod').appendTo(this.c_mode);

		$('<div>').addClass('tele-title-1 ').html('Learn New Applications').appendTo(this.c_mode);
                this.addUnknownAppToggle = $('<div>').toggleFlip({ left_value: 'Off', right_value: 'On', flipped: this.data.add_unknown_applications_id
				== '1' }).addClass('tele-addUnknownApp-toggle').appendTo(this.c_mode);

                //$('<p>').html('ETA: ' + this.data.eta_id ).appendTo(this.c_mode);

		// Wrapper
		this.engineControls = $('<div>').addClass('tele-engine-controls').appendTo(this.c_mode);
		
		// Engine
		$('<div>').addClass('tele-title-2').html('Telepath Engine').appendTo(this.engineControls);
		
		this.telepathEngineToggle = $('<div>').toggleFlip({ left_value: 'Off', right_value: 'On', flipped: this.data.engine_mode_id == '1', flip: function (value) {
			
			telepath.ds.get('/telepath/set_engine_' + (value ? 'start' : 'restart'), { }, function (data) {
			
			});
		
		} }).appendTo(this.engineControls);
		
		// Rev-Proxy
		$('<div>').addClass('tele-title-2').html('Reverse Proxy').appendTo(this.engineControls);
		this.reverseProxyToggle = $('<div>').toggleFlip({ left_value: 'Off', right_value: 'On', flipped: this.data.reverse_proxy_mode_id == "1" }).appendTo(this.engineControls);
		
		// Sniffer
		$('<div>').addClass('tele-title-2').html('Sniffer').appendTo(this.engineControls);
		this.snifferToggle 		= $('<div>').toggleFlip({ left_value: 'Off', right_value: 'On', flipped: this.data.sniffer_mode_id == "1" }).appendTo(this.engineControls);
		
		// Sniffer
		$('<div>').addClass('tele-title-2').html('Webservice').appendTo(this.engineControls).css({ opacity: 0.3 });
		this.webserviceToggle = $('<div>').toggleFlip({ left_value: 'Off', right_value: 'On', disabled: true }).appendTo(this.engineControls);

		// File upload
		//this.fileUpload=$('<div>').addClass('file-upload').appendTo(this.c_mode);

		if (!$("#file-upload").length) {
			this.fileUpload = $('<div>').attr('id', 'file-upload').appendTo($('.tele-content'));

			this.dragandrophandler=$('<div>').attr('id', 'dragandrophandler').appendTo(this.fileUpload);

			$('<div>').addClass('dragandroptext').html('Drag & drop files you want to upload here').prependTo(this.dragandrophandler);

			$('<div>').addClass('statusbar-container').appendTo(this.dragandrophandler);

			$('<input>').attr('id', 'input').attr('type', 'file').attr('multiple', 'true').css({
				width: '0px',
				height: '0px',
				overflow: 'hidden'
			}).appendTo(this.fileUpload);

			this.buttonContainer=$('<div>').addClass('tele-button-container').appendTo(this.fileUpload);

			$('<a href="#" class="tele-button tele-button-apply disabled">Process</a>').click(function (e) {
				$(this).addClass('disabled');

				telepath.ds.get('/config/upload_to_db', {}, function (data) {
					$('#file-upload .statusbar').remove();
				}, function () {
					uploadError.show();
					var interval = setInterval(function () {
						telepath.ds.get('/config/upload_to_db', {}, function (data) {
							$('#file-upload .statusbar').remove();
							uploadError.hide();
							clearInterval(interval);
						}, 'Load to database error.');
					}, 60000);
				});
			}).appendTo(this.buttonContainer);

			var uploadError = $('<div>').addClass('upload-error').html('An error occurred during the loading. An automatic process will be triggered in a minute.').appendTo(this.fileUpload).hide();



			$(document).ready(function()
		{
			var handler = $("#dragandrophandler");
			var text = $(".dragandroptext");
			var container = $(".statusbar-container");

			$('#input').change(function(e){
				var files = e.currentTarget.files;

				//We need to send dropped files to Server
				handleFileUpload(files,container);
			})

			text.on('click', function(e){
			$('#input').click();

			});
			handler.on('dragenter', function (e)
			{
				e.stopPropagation();
				e.preventDefault();
				$(this).css('border', '5px solid rgb(193, 193, 193)');
			});
			handler.on('dragover', function (e)
			{
				e.stopPropagation();
				e.preventDefault();
			});
			handler.on('drop', function (e)
			{

				$(this).css('border', '5px dotted rgb(193, 193, 193)');
				e.preventDefault();
				var files = e.originalEvent.dataTransfer.files;

				//We need to send dropped files to Server
				handleFileUpload(files,container);
			});
			$(document).on('dragenter', function (e)
			{
				e.stopPropagation();
				e.preventDefault();
			});
			$(document).on('dragover', function (e)
			{
				e.stopPropagation();
				e.preventDefault();
				handler.css('border', '5px dotted rgb(193, 193, 193)');
			});
			$(document).on('drop', function (e)
			{
				e.stopPropagation();
				e.preventDefault();
			});

			window.onbeforeunload = function () {
			if ($('#file-upload .statusbar').length) {
				return 'The upload process is not finished yet. You will lost the data.';
			}
		};

			window.onunload = function () {
				if ($('#file-upload .statusbar').length) {
					telepath.ds.get('/config/empty_folder', {}, function (data) {
					});
				}
			};

		});
		}
		else{
			$("#file-upload").show();
		}

		//Read the file contents using HTML5 FormData() when the files are dropped.
		function handleFileUpload(files,obj)
		{
			for (var i = 0; i < files.length; i++)
			{
				var fd = new FormData();
				fd.append('file', files[i]);

				var status = new createStatusbar(obj); //Using this we can set progress.
				status.setFileNameSize(files[i].name,files[i].size);
				sendFileToServer(fd,status);

			}
		}

		//Send FormData() to Server using jQuery AJAX API
		function sendFileToServer(formData,status)
		{
			var uploadURL = telepath.controllerPath+ "/config/do_upload"; //Upload URL
			var extraData ={}; //Extra Data.
			var jqXHR=$.ajax({
				xhr: function() {
					var xhrobj = $.ajaxSettings.xhr();
					if (xhrobj.upload) {
						xhrobj.upload.addEventListener('progress', function(event) {
							var percent = 0;
							var position = event.loaded || event.position;
							var total = event.total;
							if (event.lengthComputable) {
								percent = Math.ceil(position / total * 100);
							}
							//Set progress
							status.setProgress(percent);
						}, false);
					}
					return xhrobj;
				},
				url: uploadURL,
				type: "POST",
				contentType:false,
				processData: false,
				cache: false,
				data: formData,
				success: function(data){
					if(data.success){
						status.setProgress(100,data.loader_mode);
						//$("#status1").append("File uploaded<br>");
					}
					else{
						status.displayError(data.error);
					}
				}
			});

			status.setAbort(jqXHR);
			status.setDelete();
		}


		that.rowCount=0;

		function createStatusbar(obj)
		{
			that.rowCount++;
			var row="odd";
			if(that.rowCount %2 ==0) row ="even";
			this.statusbar = $("<div class='statusbar "+row+"'></div>");
			this.filename = $("<div class='filename'></div>").appendTo(this.statusbar);
			this.progressBar = $("<div class='progressBar'><div></div></div>").appendTo(this.statusbar);
			this.size = $("<div class='filesize'></div>").appendTo(this.statusbar);
			this.abort = $("<div class='abort'>Abort</div>").appendTo(this.statusbar);
			this.delete = $("<div class='abort'>Delete</div>").appendTo(this.statusbar).hide();
			//this.closeEl     = $('<a>').attr('href', '#').addClass('tele-overlay-close').addClass('tele-icon').addClass('tele-icon-close').appendTo(this.statusbar);



			obj.prepend(this.statusbar);
			//obj.mCustomScrollbar("update");

			this.setFileNameSize = function(name,size)
			{
				var sizeStr="";
				var sizeKB = size/1024;
				if(parseInt(sizeKB) > 1024)
				{
					var sizeMB = sizeKB/1024;
					sizeStr = sizeMB.toFixed(2)+" MB";
				}
				else
				{
					sizeStr = sizeKB.toFixed(2)+" KB";
				}

				this.filename.html(name);
				this.size.html(sizeStr);
			}
			this.setProgress = function(progress,loader_mode)
			{
				var progressBarWidth =progress*this.progressBar.width()/ 100;
				this.progressBar.find('div').animate({ width: progressBarWidth }, 10).html(progress + "% ");
				if(parseInt(progress) >= 100)
				{
					this.abort.hide();
					this.delete.show();
					if(!loader_mode){
						$("#file-upload .tele-button").removeClass('disabled');
					}


				}
			}
			this.setAbort = function(jqxhr)
			{
				var sb = this.statusbar;
				this.abort.click(function()
				{
					jqxhr.abort();
					sb.hide();
				});
			}
			this.setDelete = function()
			{
				var sb = this.statusbar;
				var fn = this.filename.html();
				this.delete.click(function()
				{
					telepath.ds.get('/config/delete_file', {file_name:fn}, function (data) {
						sb.hide();
					}, 'The file was not deleted');

				});
			}
			this.displayError = function(message)
			{
				this.progressBar.hide().after("<div class='error'>"+message+"</div>");
			}
		}


                // TODO:: Scheduler



		// -----------------------------------------------------------
		// Reports
		// -----------------------------------------------------------
		this.c_reports = $('<div>').addClass('tele-config-system-tab tele-config-system-reports');
		this.container.append(this.c_reports);
		
		// $('<div>').addClass('tele-title-1').html('Reports').appendTo(this.c_reports);
		
		// -----------------------------------------------------------
		// SYSLOG
		// -----------------------------------------------------------
		
		$('<div>').addClass('tele-title-1').html('Syslog').appendTo(this.c_reports).addClass('tele-title-syslog');
		this.syslogToggle = $('<div>').toggleFlip({ left_value: 'Off', right_value: 'On', flipped: this.data.write_to_syslog_id == '1' }).addClass('tele-syslog-toggle').appendTo(this.c_reports);
		this.syslogIP     = $('<div>').teleInput({ label: 'Server', width: 120, value: this.data.syslog_ip_id }).addClass('tele-config-syslog-host').appendTo(this.c_reports);
		

		// -----------------------------------------------------------
		// SMTP
		// -----------------------------------------------------------
		$('<div>').addClass('tele-title-1').html('SMTP').appendTo(this.c_reports);
		
		this.smtpServer = $('<div>').teleInput({ label: 'Server'  , width: 200, value: this.data.smtp_ip_id      }).addClass('tele-config-smtp-host').appendTo(this.c_reports);
		this.smtp_port_id   = $('<div>').teleInput({ label: 'Port'    , width: 70,  value: this.data.smtp_port_id }).addClass('tele-config-smtp-port').appendTo(this.c_reports);
		this.smtpUser   = $('<div>').teleInput({ label: 'Username', width: 200, value: this.data.rep_user  }).addClass('tele-config-smtp-username').appendTo(this.c_reports); 
		this.smtpPass   = $('<div>').teleInput({ label: 'Password', width: 200, value: this.data.rep_pass_id, pass: true  }).addClass('tele-config-smtp-password').appendTo(this.c_reports);
		
		
		// SMTP Test button
		this.smtpTestEmail = $('<div>').teleInput({ label: 'Test Email', width: 200 }).css({ 'marginTop': 20 }).addClass('tele-config-smtp-test').appendTo(this.c_reports); 
		var testSMTP   = $('<a href="#" class="tele-button tele-button-apply">Test</a>').click(function () {
			
			var email = $('input', that.smtpTestEmail).val();

			function validateEmail(email) {
				var re = /^([\w-]+(?:\.[\w-]+)*)@((?:[\w-]+\.)*\w[\w-]{0,66})\.([a-z]{2,6}(?:\.[a-z]{2})?)$/i;
				return re.test(email);
			}
			
			if(validateEmail(email)) {
											
				if($('input', that.smtpServer).val() == '') {
					telepath.dialog({ msg: 'SMTP Server cant be blank.' });
					return;
				}
				
				telepath.ds.get('/config/testmail', { 
					'smtp_ip_id':		 $('input', that.smtpServer).val(),
					'smtp_port_id': $('input', that.smtp_port_id).val(),
					'smtp_user': $('input', that.smtpUser).val(),
					'smtp_pass': $('input', that.smtpPass).val(),
					'test_mail': email
				}, function(data) {
					
					telepath.dialog({ msg: 'Test mail was sent.' });
				
				}, 'Error while trying to send test mail');
				
				$('input', that.smtpTestEmail).val('');
				
			} else {
				
				telepath.dialog({ msg: 'Invalid mail supplied.' });
			
			}
			
		}).css({ 'float': 'left','padding': '4px 10px', 'margin-top': '26px' }).h().appendTo(this.c_reports);
		
		
		// -----------------------------------------------------------
		// Load Balancer
		// -----------------------------------------------------------
		
	/*	this.c_lb = $('<div>').addClass('tele-config-system-tab tele-config-system-lb');
		this.container.append(this.c_lb);
		
		$('<div>').addClass('tele-title-1').html('Load Balancer').appendTo(this.c_lb);
		
		this.lbToggle = $('<div>').toggleFlip({ left_value: 'Off', right_value: 'On', flipped: this.data.loadbalancer_mode_id == '1' }).appendTo(this.c_lb);
		this.lbIPs     = $('<div>').teleMulti({ values: this.data.balances.headers, title: 'Load Balancer Headers IPs', template: function(element, value) {
			element.ip({ data: value });
		} }).appendTo(this.c_lb).addClass('tele-config-balancer-headers');
		this.lbHeaders     = $('<div>').teleMulti({ values: this.data.balances.ips, title: 'Load Balancer IPs', template: function(element, value) {
			element.teleInput({ value: value });
		} }).appendTo(this.c_lb).addClass('tele-config-balancer-ips');*/
		
		// -----------------------------------------------------------
		// IP Whitelist
		// -----------------------------------------------------------
		
		this.c_whitelist = $('<div>').addClass('tele-config-system-tab tele-config-system-whitelist');
		this.container.append(this.c_whitelist);
		
		
		
		$.each(this.data.whitelist, function (i, ip) {
			that.c_whitelist.append(getRangeUI(ip, that.c_whitelist));
		});
		
		// Another blank
		this.c_whitelist.append(getRangeUI('', that.c_whitelist));
		
		
		//this.whitelist = $('<div>').teleMulti({ values: .length > 0 ? this.data.whitelist : [ '' ], title: 'IP Whitelist', template: function(element, value) {
		//	element.ip({ data: value });
		//} }).appendTo();
		
		
		// -----------------------------------------------------------
		// Network Interfaces
		// -----------------------------------------------------------

		this.c_network = $('<div>').addClass('tele-config-system-tab tele-config-system-network');
		this.container.append(this.c_network);

		this.row = $('<div class="wrap-network-interfaces">').appendTo(this.c_network);

		this.col1 = $('<div class="tele-network-interfaces-wrap">').appendTo(this.row);

		this.interfaces = $('<div class="interfaces">').teleMulti({ values: this.data.agents, title: 'Network Interfaces', template: function(element, value) {
			
			element.addClass('tele-network-wrap');

			var Wrap      = element;			
			//var IDX       = $('<div>').html(value.idx).addClass('tele-network-idx');
			var Name      = $('<div>').teleInput({ label: 'Name', width: 120, value: value.agent_name }).addClass('tele-network-name');
			var Filter    = $('<div>').teleInput({ label: 'Filter Expression', width: 120, value: value.pcap_filter }).addClass('tele-network-filter');
			var Interface = $('<select>').addClass('tele-network-select');


			Wrap.append(Name).append(Filter).append(Interface);
			$.each(telepath.config.system.data.interfaces, function (i, interfaceName) {
				var selected = interfaceName == value.interface_name ? 'selected="selected"' : '';
				Interface.append('<option ' + selected + ' value=' + interfaceName + '>' + interfaceName + '</option>');
			});
			
			element.append(Wrap);
			
		}}).appendTo(this.col1);

		this.col2 = $('<div class="tele-balancer-wrap">').appendTo(this.row);

		$('<div>').addClass('tele-title-1').html('Load Balancer').appendTo(this.col2);

		var header_balances =this.data.header_balances ? this.data.header_balances:'';

		var headerbalances= this.headerBalances = $('<div>').teleMulti({ values: header_balances, template: function(element, value) {
			element.teleInput({ value: value });
		} });

		//headerbalances.appendTo(this.c_network);

		this.c_lb = $('<div>').addClass('tele-config-system-lb');
		//this.c_network.append(this.c_lb);

		$.each(this.data.ip_balances, function (i, ip) {
			that.c_lb.append(getRangeLB(ip, that.c_lb));
		});

		// Another blank
		var another_ip_balancesthis= this.c_lb.append(getRangeLB('', that.c_lb));

		var state;

		this.data.loadbalancer_mode_id=='0'?state=false:state=true;

		if (!state){
			another_ip_balancesthis.hide();
			headerbalances.hide();
		}

		this.ipToggle = $('<div>').toggleFlip({
			left_value: 'Off', right_value: 'On',

			flip: function () {
				another_ip_balancesthis.toggle();
				headerbalances.toggle();
			},

			flipped: state
		});

		this.col2.append(this.ipToggle).append(headerbalances).append(another_ip_balancesthis).append(this.c_lb);


		// -----------------------------------------------------------
		// Proxy
		// -----------------------------------------------------------


		$('<div>').addClass('tele-title-1').html('Proxy').appendTo(this.row).addClass('tele-title-proxy');

		this.proxyToggle = $('<div>').toggleFlip({ left_value: 'Off', right_value: 'On', flipped: this.data.proxy_mode_id == '1' }).appendTo(this.row);
		this.proxyIP     = $('<div>').teleInput({ label: 'Server', width: 120, value: this.data.proxy_ip_id }).addClass('tele-config-proxy-host').appendTo(this.row);
		this.proxyPort   = $('<div>').teleInput({ label: 'Port', width: 70, value: this.data.proxy_port_id }).addClass('tele-config-proxy-port').appendTo(this.row);


	
		// -----------------------------------------------------------
		// User Agent Ignore List
		// -----------------------------------------------------------
		/* Parse out user agents, little tricky.. */
		/*this.c_ua = $('<div>').addClass('tele-config-system-tab tele-config-system-ua_ignore');
		this.container.append(this.c_ua);
		
		
		var headers = this.data.regex.headers ? this.data.regex.headers : '';
		var pos     = headers.indexOf('((');
		var len     = headers.length;
		var parsed  = headers.substr(pos + 2, len - pos - 4).split(')|(');
		this.useragents = $('<div>').teleMulti({ values: parsed, title: 'User Agent Ignore List', template: function(element, value) {
			element.teleInput({ value: value });
		} }).appendTo(this.c_ua);*/
	
		// -----------------------------------------------------------
		// Extension Ignore List
		// -----------------------------------------------------------
		this.c_ext = $('<div>').addClass('tele-config-system-tab tele-config-system-ext_ignore');
		this.container.append(this.c_ext);
		
		/* Parse out ext ignore list */
		//var URL    = this.data.regex.URL ? this.data.regex.URL : '';
		//var parsed = URL.substring(3,URL.length-1).split(")|(\\.");
		var regex = this.data.regex ? this.data.regex : '';

		this.extentsions = $('<div>').teleMulti({ values: regex, title: 'Extension Ignore List', template: function(element, value) {
			element.teleInput({ value: value });
		} }).appendTo(this.c_ext);
		
		// -----------------------------------------------------------
		// Save / Cancel Buttons
		// -----------------------------------------------------------
		
		var btnContain = $('<div>').addClass('tele-button-container').appendTo(this.contentLeft);
		var saveBtn   = $('<a href="#" class="tele-button tele-button-apply">Save</a>');
		var cancelBtn  = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');
		
		btnContain.append(saveBtn).append(cancelBtn);

		// Callbacks
		saveBtn.click(function (e) {
			e.preventDefault();
			that.saveConfig();
		});
		
		cancelBtn.click(function (e) {
			e.preventDefault();
			that.loadConfig();
		});
		
		$('.tele-config-system-tab').hide();
		$('.tele-config-system-mode').show();
			
		// Updates engine status every 5 seconds
		//if(telepath.config.system.engineTimer) { clearInterval(telepath.config.system.engineTimer);	}
		//telepath.config.system.engineTimer = setInterval(function () { that.updateEngineStatus(); }, 5000);
		//this.container.mCustomScrollbar({ advanced:{ updateOnContentResize: true } });	
	}

}
