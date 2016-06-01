$.widget( "tele.condition", {
 
    options: {
		type: false,
		data: { value: [], negate: false },
		selected: [],
		negate: false,
    },
    _create: function() {
	
        this.element.addClass( "tele-condition" );
        this._update();

    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
	
	getCondition: function() {
		
		var result = [];
		
		switch(this.options.type) {
			case 'rules':

				result = this.options.data.value.trim();
				// Just in case, get rid of trail or prefix comma
				if(result.length > 1) {
					if(result.substr(0,1) == ',') {
						result = result.substr(1);
					}
					if(result.substr(result.length - 1, 1) == ',') {
						result = result.substr(0, result.length - 1);
					}
				}
			
			break;
			case 'application':
			
				result = '';
				$('input', this.element).each(function () {
					result = result + $(this).val().trim() + ',';
				});
				result = result.substr(0, result.length - 1);
			
			break;
			
			case 'country':
				
				result = $('.tele-country-list', this.element).data('tele-teleCountry').getSelected();
				
			break;
			
			case 'time':
			
				var eventsToSave = [];
				
				// Gather Objects
				$('.wc-cal-event', this.options.element).each(function () { 
					if(typeof($(this).data('calEvent')) == 'object') {
						eventsToSave.push($(this).data('calEvent'));
					}		
				});

				// Setup Result Array
				var weekday = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
				var result = {};
				
				// Prep Result Array
				for(var i = 0; i < 7; i++) {
					result[weekday[i]] = [];
					for(var x = 0; x < 24; x++) { result[weekday[i]][x] = 0; }
				}
				
				// Fill Result Array
				$.each(eventsToSave, function(i, event) {
					for(var i = event.start.getHours(); i < (((event.end.getHours() == 23 && event.end.getMinutes() == 59) || event.end.getHours() == 0) ? 24 : event.end.getHours()); i++) {
						result[weekday[event.start.getDay()]][i] = 1;
					}
				});

			break;
			
			case 'IP':
				result = '';
				$('.tele-ip-wrap', this.options.element).each(function () {
					
					var is_range = $('.tele-mini-toggle', this).data('tele-toggleFlip').options.flipped;
					
					var ip_start = $('.tele-ip:first', this).data('tele-ip').getIP();
					var ip_end   = $('.tele-ip:last', this).data('tele-ip').getIP();
					
					if(is_range) {
						if(ip_start && ip_end && ip2long(ip_start) < ip2long(ip_end)) {
							result = result + ip_start + '-' + ip_end + ',';
						}
					} else {
						if(ip_start) {
							result = result + ip_start + ',';
						}
					}

				});
				result = result.substr(0, result.length - 1);
			
			break;

			case 'parameter':

				result = '';
				$('input', this.element).each(function () {
					result = result + $(this).val().trim() + ',';
				});
				result = result.substr(0, result.length - 1);


				break;
			
		}
		
		return { type: this.options.type, negate: this.options.negate, value: result };
		
	},
    _update: function() {
		
		var that = this;
		this.element.empty();
		
		/*this.removeEl = $('<div>').btn({ icon: 'delete', text: '', callback: function () {
			that.element.remove();
		}}).addClass('tele-condition-remove');*/
		
		this.element.addClass('tele-condition-' + this.options.type);
		
		var negates = true;
		
		switch(this.options.type) {
			
			case 'advanced':
				
				negates = false;
				
				function get_slider(className, title, value, min, max) {
					
					var slider_val = $('<div>').addClass('tele-condition-slider-value');
					var wrap   = $('<div>').addClass('tele-condition-slider').addClass(className);
					var title  = $('<div>').addClass('tele-title-2').html(title);
					
					function refreshValue() {
						var value = slider.slider('value');
						slider_val.html(value);
					}
					
					var slider = $('<div>').slider({
						orientation: "horizontal",
						range: "min",
						max: max,
						value: value,
						min: min,
						slide: refreshValue,
						change: refreshValue
					});
					
					refreshValue();
					
					wrap.append(title).append(slider).append(slider_val);
					return wrap;
					
				}
				
				this.element.append(get_slider('reputation', 'User Reputation', 0, 0, 100));
				this.element.append(get_slider('score', 'Average Score', 0, 0, 100));
				
				var checked = false;
				
				this.cbVelocity = $('<div>').teleCheckbox({
					checked: checked,
					label: 'Geographic change of position velocity',
					callback: function (el, v) {
						var checked = this.checked;
					}
				});
				
				this.element.append(this.cbVelocity);
				
				this.element.append(get_slider('time', 'Time (Seconds)', 0, 0, 3600));
				this.element.append(get_slider('time', 'Distance (Km\'s)', 0, 0, 1000));
				
			
			break;
			
			case 'rules':
			
				this.ruleTree = $('<div>').teleTree({ 
					type: 'rules', 
					grid: {
						columns: [
							{width: 380 },
							{value: function (node) {
								
								if(node.type !== 'group') {
									return;
								}
								
								var checked = false;
								
								var opts = that.options.data.value.split(',');
										
								for(x in opts) {
									
									var opt = opts[x].split('::');
									if(opt.length == 2) {
										if(node.category == opt[0].trim() && node.id == opt[1].trim()) {
											checked = true;
										}
									}
								
								}

								var cb = $('<div>').teleCheckbox({
									checked: checked,
									callback: function (el, v) {
										
										var checked = this.checked;
										var found   = false;
										
										// Cast to array
										if(that.options.data.value == []) {
											that.options.data.value = '';
										}
										
										var opts = that.options.data.value.split(',');
										
										for(x in opts) {
											
											var opt = opts[x].split('::');
											if(opt.length == 2) {
												if(node.category == opt[0].trim() && node.id == opt[1].trim()) {
													found = true;
												}
											}
										
										}
										
										if(found && !checked) {
											that.options.data.value = that.options.data.value.replace(node.category + '::' + node.id, '');
											that.options.data.value = that.options.data.value.replace(',,',',');
										}
										
										if(!found && checked) {
											// make sure we have comma in the string
											if (that.options.data.value.length > 0 && that.options.data.value.substr(that.options.data.value.length-1, 1) !== ',') {
												that.options.data.value = that.options.data.value + ',';
											}
											that.options.data.value = that.options.data.value + node.category + '::' + node.id;
										}
										// remove last comma
										if(that.options.data.value.substr(that.options.data.value.length-1, 1) == ',') {
											that.options.data.value = that.options.data.value.substr(0, that.options.data.value.length - 1);
										}
										
										// console.log(that.options.data.value);
									
									}
								});
								return cb;

								
							}, width: 50 }
						],
						resizable:true
					},
					callback: function(e, data) {
						if(data.node.data.type == 'rule') {
							// console.log(data);
						}
					}
				});
				
				this.element.append(this.ruleTree);
			
			break;
			
			case 'application':

				var values = that.options.data.value.split(',');

				if(values.length){

					for(var x in values ){

						var filterApps = $('<div>').teleSelect({ type: 'subdomain', values: [{ text: values[x], id: '-1', sub_id: '-1', root: true }], click: function () { } });
						this.element.append(filterApps);
					}
				}
				else{

					var filterApps = $('<div>').teleSelect({ type: 'subdomain', values: [{ text: 'All', id: '-1', sub_id: '-1', root: true }], click: function () { } });
					this.element.append(filterApps);
				}


				//var filterApps = $('<div>').teleSelect({ type: 'subdomain', values: this.options.data.value, click: function () { } });

				
			break;
			
			case 'country':

				var values = that.options.data.value.split(',');

				var cList  = $('<ul>').addClass('tele-country-list').teleCountry({values:values});

				this.element.append(cList);
				
			break;
			
			case 'IP':

				$.each(that.options.data.value.split(','), function (i, ip) {
					that.element.append(old_getRangeUI(ip, that.element));
				});
				
				// Another blank
				this.element.append(getRangeUI('', that.element));
				
			break;
			
			case 'velocity':
				this.element.append('Velocity Condition');
			break;
			
			case 'user':
				this.element.append('User Condition');
			break;
			
			case 'time':
			
				var calendar = $('<div>').addClass('tele-condition-calendar');
				this.element.append(calendar);
				var eventData = [];
				
				$(calendar).weekCalendar({
				
					timeslotsPerHour: 1,
					timeslotHeight: 15,
					defaultEventLength: 1,
					data: eventData,
					height: function($calendar) {
						return 365;
					}
					
				});
				
			break;
			
			case 'parameter':
				
				var paramBrowse = $('<div>').teleBrowse({ label: 'Select Parameter', mode: 'param',value: that.options.data.value });
				this.element.append(paramBrowse);
				
			break;
		
		}
		
		// Negative checkbox
		if(negates) {
		
			var notWrap = $('<div>').addClass('tele-condition-not-wrap');
			
			that.options.negate = that.options.data.negate;
			
			var notCb  = $('<span>').teleCheckbox({ inputFirst: true, checked: this.options.data.negate, label: 'Negate (not selected)', callback: function() {
				that.options.negate = this.checked;
			}}).addClass('tele-condition-not');
			notWrap.append(notCb);
			this.element.prepend(notWrap);
			
		}
		// Title + Remove Cmd
		// this.conditionTitle = $('<div>').addClass('tele-condition-title').text(this.options.type + ' Condition');
		// this.element.prepend(this.conditionTitle).prepend(this.removeEl);

    }

});
