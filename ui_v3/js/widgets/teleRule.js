$.widget( "tele.teleRule", {
	
    options: {
		data: [],
    },
    _create: function() {
	
        this.element.addClass( "tele-ruletype-select" );
	//var teleBrowser = $('<div>').teleBrowser();
	//this.element.append(teleBrowser);
	this.criterias = 0;
	this._update();
	return this;
		
    },
     _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
	getValues: function () {
		
		// Collapse all
		$('.tele-ruletype-accordion', this.element).data('uiAccordion').active.trigger('click');
		
		var ruleData = [];
		
		$('.tele-ruletype-accordion').hide();
		
		// Quickly expand and collapse all tabs to propogate data and trigger errors if any
		$('.tele-ruletype-accordion').data('uiAccordion').headers.each(function () {
			///open
			$(this).triggerHandler('click');

			//close
			$(this).triggerHandler('click');

		});
		
		// Collect data from all
		$('.tele-ruletype-accordion').data('uiAccordion').headers.each(function () { 
			var data = false;
			data = $(this).data('ruleData');
			if(data) {
				var json = JSON.parse(data);
				if (!$.isEmptyObject(json)) {
					ruleData.push(data);
				}
			}
		});
		
		$('.tele-ruletype-accordion').show();
		
		return ruleData;
		
	},
	addCriteria: function(type, data) {
		
		var that = this;
		
		var headerEl    = $('<h3>').text(that.ruleTypes[type].title).css('text-transform', 'capitalize').addClass('tele-ruletype-' + type).data('ruleData', JSON.stringify(data));
		var iconEl      = $('<div>').addClass('tele-icon').addClass('tele-icon-' + type + '-rule');
		var containerEl = $('<div>').attr('rel', type);
		var briefEl     = $('<span>').addClass('brief').html(that.ruleTypes[type].desc);
		that.accordion.append(headerEl).append(containerEl);

		if (!this.options.data["builtin_rule"])
		{
			var delCriteria = $('<div>').addClass('tele-icon tele-icon-delete').click(function() {
				headerEl.remove();
				containerEl.remove();
			}).h();
			headerEl.prepend(iconEl).append(briefEl).append(delCriteria);
		} else {
			headerEl.prepend(iconEl).append(briefEl);
		}
		
		
		that.accordion.accordion('destroy').accordion({ 

			heightStyle: 'fill',
			collapsible: true,
			active: false,
			autoHeight: false,
			animate: false,
			select: function( event, ui) {
		
			},
			beforeActivate: function( event, ui ) {
				
				if(ui.oldHeader && ui.oldPanel) {
					
					var json = that.buildJSON(ui.oldPanel);
					
					if(!json) {
						return false;
					} else {
						
					}
					
					$(ui.oldHeader).data('ruleData', JSON.stringify(json));
				
				}
				
				var container = $(ui.newPanel);
				
				if(container.length > 0 && !container.hasClass('loaded') && ui.newPanel) {
				
					var type = ui.newPanel.attr('rel');
					var json_string = $(ui.newHeader).data('ruleData');
					var data = [];
					if(json_string && json_string != '') {
						data = JSON.parse(json_string);
					}
					that.editor(type, container, data);
					container.addClass('loaded');
					
				}

			}
		});
		if (this.options.data["builtin_rule"])
		{
			$(iconEl).click();
		}
	
	},
    _update: function() {

		var that = this;
		this.element.empty();
		
		this.ruleTypes = {
			'parameter': {
				title: 'Parameter', 
				desc: 'Inspect specific parameter.'
			},
			'pattern': {
				title: 'Pattern',
				desc: 'Detect repetitive patterns over a predefined period of time.',
			},
			'behaviour': {
				title: 'Behaviour',
				desc: 'Inspect one aspect of the web user\'s behavior such as: landing speed, path, geographic location and so on.',
			},
			'geographic': {
				title: 'Geographic',
				desc: 'Geographic aspect.',
			},
			'aspect': {
				title: 'Bot Intelligence',
				desc: 'Bot Intelligence.',
			}
		};
	
		if (!this.options.data["builtin_rule"])
		{	
			this.buttonsWrap = $('<div>').addClass('tele-ruletype-buttons');
			this.element.append(this.buttonsWrap);
		}
				
		this.accordion = $('<div>').addClass('tele-ruletype-accordion');
		this.element.append(this.accordion);
				
		var mapping = {
			'P': 'parameter',
			'p': 'pattern',
			'b': 'behaviour',
			'g': 'geographic',
			'B': 'aspect'
		};
		if (!this.options.data["builtin_rule"])
		{
			$.each(this.ruleTypes, function(type, data) {
			
				var extra  = $('<div>').addClass('tele-ruletype-button').hover(function () {
					$('*', this).addClass('hover');
				}, function () {
					$('*', this).removeClass('hover');
				});
				var button = $('<a>').html(data.title).click(function () {
					that.addCriteria(type, []);
				}).append($('<div>').addClass('tele-icon tele-icon-plus').h());
				that.buttonsWrap.append(extra.append(button));
			});
		}
		
		this.accordion.accordion();

		$.each(this.options.data.criteria, function (i, val) {
			
			if(mapping[val.kind]) {
				that.addCriteria(mapping[val.kind], val);
			}
		
		});
		
					
		/*this.element.mCustomScrollbar({
			scrollButtons:{	enable: false },
			advanced:{ updateOnContentResize: true }
		});*/
		
	
    },
	buildJSON: function(c) {
		
		json = {};
		
		$('*', c).removeClass('error');
		
		var type = $(c).attr('rel');
		
		switch(type) {
		
			case 'parameter': 
				
				json.kind = 'P';
				var method = $('.tele-parameter-type .tele-radio-knob',c).parent().attr('rel');
				// console.log(method);
				
				switch(method) {

					case 'Parameter':

						// 1. Collect param
						$('.tele-browse input', c).removeClass('error');
						var param = $('.tele-browse input', c).data('selected');

						if(typeof(param) == 'string') {
							param = JSON.parse(param);
						}

						//if the `getValues` above opened a dialog, stop now
						if($('.tele-overlay-dialog').is(':visible')){
							return;
						}


						if(!param) {
							telepath.dialog({ title: 'Rule Editor', msg: 'You must browse for a parameter.' });
							$('.tele-browse input', c).addClass('error');

							return false;
						}


						// 2. Collect app + uri

						if(param.type == 'page') {

							// INVALID IN THIS CONTEXT
							json.type = 'specific';

						} else {
							if(param.type == 'param') {
								if(param.global) {
									json.type = 'inclusive';
									json.inclusive = {
										"paramname": param.paramname
									}
								} else {
									json.type = 'specific';
									json.specific = {
										"domain":    param.domain,
										"pagename":  param.pagename,
										"paramname": param.paramname
									}
								}
							}
						}

						// 3. Match type

						json.subtype = $('.tele-string-inspection .tele-radio-knob',c).parent().attr('rel');
						switch(json.subtype) {

							case 'heuristic':

								break;
							case 'regex':
							case 'stringmatch':
								json.negate    = $('.tele-rule-string-inspection .tele-checkbox .checked',c).size() > 0;
								json.str_match = $('.tele-rule-string-inspection .tele-input-str-'+ json.subtype +' input',c).val();

								if(json.str_match == '') {
									telepath.dialog({ title: 'Rule Editor', msg: 'You must specify match pattern / regex' });
									$('.tele-rule-string-inspection input').addClass('error');
									return false;
								}

								break;

							case 'fuzzylength':
								json.length    = $('.tele-rule-string-inspection .tele-rule-dropdown').val();
								break;

							case 'exactlength':

								json.length = $('#rule-slider-length').data('value');

								break;


							case 'rangelength':

								json.length = $('#rule-slider-between', c).data('sliderValue').join('-');

								break;

							case 'distance':

								json.distance = $('#rule-slider-similarity').data('value');

								break;

						}

						break;
					
					case 'Request':
						
						json.method = '';
						
						$('.tele-rule-request-wrap .checked',c).each(function() {
							json.method = json.method + $(this).parent().text().substr(0,1) + ','; 
						});
						
						json.method = json.method.substr(0, json.method.length - 1);

						// Nothing checked
						if(json.method.length<1){
							json.method='Request';
						}
					

						json.subtype = $('.tele-string-inspection .tele-radio-knob',c).parent().attr('rel');
						json.negate    = $('.tele-rule-string-inspection .tele-checkbox-str-'+ json.subtype +' .checked',c).size() > 0;
						json.str_match = $('.tele-rule-string-inspection .tele-input-str-'+ json.subtype +' input',c).val();

						//if the `getValues` above opened a dialog, stop now
						if($('.tele-overlay-dialog').is(':visible')){
							return;
						}

						if(json.str_match == '') {
									telepath.dialog({ title: 'Rule Editor', msg: 'You must specify match pattern / regex' });
									$('.tele-rule-string-inspection input').addClass('error');
									return false;
								}
						
						json.type = 'global';
					
					break;

					case 'Response':


						json.method = $('.tele-response-settings .tele-radio-knob', c).parent().attr('rel');
						var message;

						if (json.method == 'StatusCode') {
							json.subtype = 'code';
							json.negate = false;
							var is_range = $('.tele-mini-toggle', c).data('tele-toggleFlip').options.flipped;
							var sc_start = $('.tele-sc:first input', c).val();
							var sc_end = $('.tele-sc:last input', c).val();
							json.str_match='';
							if (is_range) {
								if (sc_start && sc_end && sc_start < sc_end) {
									json.str_match = sc_start + '-' + sc_end;
								}
							} else {
								if (sc_start) {
									json.str_match = sc_start;
								}
							}
							message = 'You must specify a status code';
						}
						else {
							json.subtype = $('.tele-string-inspection .tele-radio-knob', c).parent().attr('rel');
							json.negate = $('.tele-rule-string-inspection .tele-checkbox-str-' + json.subtype + ' .checked', c).size() > 0;
							json.str_match = $('.tele-rule-string-inspection .tele-input-str-' + json.subtype + ' input', c).val();
							message = 'You must specify match pattern / regex';
						}


						//if the `getValues` above opened a dialog, stop now
						if ($('.tele-overlay-dialog').is(':visible')) {
							return;
						}

						if (json.str_match == '') {
							telepath.dialog({title: 'Rule Editor', msg: message});
							$('.tele-rule-string-inspection input').addClass('error');
							return false;
						}

						json.type = 'global';
					break;
						
					case 'Uri':
					
						json.subtype = $('.tele-string-inspection .tele-radio-knob',c).parent().attr('rel');
						json.negate    = $('.tele-rule-string-inspection .tele-checkbox-str-'+ json.subtype +' .checked',c).size() > 0;
						json.str_match = $('.tele-rule-string-inspection .tele-input-str-'+ json.subtype +' input',c).val();

						//if the `getValues` above opened a dialog, stop now
						if($('.tele-overlay-dialog').is(':visible')){
							return;
						}

						if(json.str_match == '') {
									telepath.dialog({ title: 'Rule Editor', msg: 'You must specify match pattern / regex' });
									$('.tele-rule-string-inspection input').addClass('error');
									return false;
								}
						
						json.method = method;
						
						json.type = 'global';
					
					break;




				
				}
				
			break;
			
			case 'pattern':
			
				json.kind = 'p';
				json.type = $('.tele-rule-anchor', c).data('tele-tele-radios').options.checked;
				json.time  = parseInt($('.tele-pattern-time input', c).val()) || 0;
				json.count = parseInt($('.tele-pattern-count input', c).val()) || 0;

				//if the `getValues` above opened a dialog, stop now
				if($('.tele-overlay-dialog').is(':visible')){
					return;
				}

				if(json.time < 1) {
					telepath.dialog({ title: 'Rule Editor', msg: 'Invalid time window' });
					$('.tele-pattern-time input', c).addClass('error');
					return;
				}
				if(json.count < 2) {
					telepath.dialog({ title: 'Rule Editor', msg: 'Invalid count parameter' });
					$('.tele-pattern-count input', c).addClass('error');
					return;
				}
				
				switch(json.type) {
					case 'IP':
					break;
					case 'SID':
					break;
					case 'User':
					break;
					case 'Other':
					
						// 1. Collect param
						$('.tele-browse-other input', c).removeClass('error');
						var param = $('.tele-browse-other input', c).data('selected');

						if(typeof(param) == 'string') {
							param = JSON.parse(param);
						}

						//if the `getValues` above opened a dialog, stop now
						if($('.tele-overlay-dialog').is(':visible')){
							return;
						}
						
						if(!param) {
							telepath.dialog({ title: 'Rule Editor', msg: 'You must select anchor parameter for type: Other' });
							$('.tele-browse-other input', c).addClass('error');
							return false;
						}


						// 2. Collect app + uri
						
						if(param.type == 'page') {
							
							// INVALID IN THIS CONTEXT
							
						} else {

							if(param.type == 'param') {
								if(param.global) {
									json.Other = {
										"paramname": param.paramname 
									}
								} else {
									json.Other = {
										"domain":    param.domain,
										"pagename":  param.pagename,
										"paramname": param.paramname 
									}
								}
							}
						}
						
					
					break;
					
				}
				
				json.subtype = $('.tele-rule-bind', c).data('tele-tele-radios').options.checked;
				
				switch(json.subtype) {
					
					case 'user':
					
					break;
					case 'action':
					
						//if($('.tele-pattern-changing .tele-multi input').size() > 0 &&
						//   $('.tele-pattern-changing .tele-multi input').data('teleSelect') &&
						//   $('.tele-pattern-changing .tele-multi input').data('teleSelect').raw) {
						var action_data = $('.tele-pattern-changing .tele-multi input').data('teleSelect');

						//if the `getValues` above opened a dialog, stop now
						if($('.tele-overlay-dialog').is(':visible')){
							return;
						}
						if ($.isEmptyObject(action_data)) {
							telepath.dialog({
								title: 'Rule Editor',
								msg: 'You must browse for repeating business action'
							});
							$('.tele-pattern-changing .tele-multi input', c).addClass('error');
							return false;
						} else {
							json.domain = action_data.raw.application;
							json.action_name = action_data.raw.action_name;
						}
					
					break;
					case 'page':
						
						// 1. Collect param
						var param = $('.tele-browse-page input', c).data('selected');

						if(typeof(param) == 'string') {
							param = JSON.parse(param);
						}
						
						else {
							telepath.dialog({ title: 'Rule Editor', msg: 'You must browse for changing page' });
							$('.tele-browse-page input', c).addClass('error');
							return false;
						}
						

						// 2. Collect app + uri
						
						if(param.type == 'page') {
							
							json.domain   = param.domain;
							json.pagename = param.pagename;
							
						} 
						
					break;
					case 'parameter':
						
						// 1. Collect param
						var param = $('.tele-browse-param input', c).data('selected');

						if(typeof(param) == 'string') {
							param = JSON.parse(param);
						}
						
						else {
							telepath.dialog({ title: 'Rule Editor', msg: 'You must browse for changing parameter' });
							$('.tele-browse-param input', c).addClass('error');
							return false;
						}
						

						// 2. Collect app + uri
						
						if(param.type == 'page') {
							
							// INVALID IN THIS CONTEXT
							
						} else {

							if(param.type == 'param') {
								if(param.global) {
									json.paramname = param.paramname;
								} else {
									json.domain = param.domain;
									json.pagename = param.pagename;
									json.paramname = param.paramname;
								}
							}
						}
						
					break;
				
				
				}
				
				
				
			break;
			
			case 'behaviour':
			
				json.kind     = 'b';
				json.type     = $('.tele-behavior-type', c).val();
				json.personal = $('.tele-personal-behavior .checked', c).size() > 0;
				
			break;
			
			case 'geographic':
			
				json.kind = 'g';
				json.type = $(".tele-geo-type .tele-radio-knob",c).parent().attr('rel');
				
				switch(json.type) {
					
					case 'velocity':
						
						var range = $('.tele-geo-ts').val();
						var count = $('.tele-geo-ts-count input').val();
						var distance= $('.rule-slider-length input').val();

						//if the `getValues` above opened a dialog, stop now
						if($('.tele-overlay-dialog').is(':visible')){
							return;
						}

						if(distance < 1) {
							telepath.dialog({ title: 'Rule Editor', msg: 'You must specify valid distance' });
							$('.rule-slider-length input').addClass('error');
							return false;
						}

						json.distance=parseInt(distance);

						if(count < 1) {
							telepath.dialog({ title: 'Rule Editor', msg: 'You must specify valid count' });
							$('.tele-geo-ts-count input').addClass('error');
							return false;
						}
						
						switch(range) {
							case 's':
								json.ts = parseInt(count);
							break;
							case 'm':
								json.ts = parseInt(count) * 60;
							break;
							case 'h':
								json.ts = parseInt(count) * 3600;
							break;
						}
					
					break;
					
					case 'inside':
					case 'outside':
						
						var selected = "";

						//if the `getValues` above opened a dialog, stop now
						if($('.tele-overlay-dialog').is(':visible')){
							return;
						}
						if($('.tele-country-list .checked', c).size() == 0) {
							$('.tele-country-list').addClass('error');
							telepath.dialog({ title: 'Rule Editor', msg: 'No countries selected' });
							return false;
						}
						$('.tele-country-list .checked', c).each(function () { 
							// fix undefined in country list, Yuli
							if ($(this).attr('dataid')) {
								selected = selected + $(this).attr('dataid') + ','; 
							}
						});
						selected = selected.substr(0, selected.length - 1);

						json.location = selected;
						
					break;
				
				}
				
			break;
			
			case 'aspect':
			
				json.kind = 'B';
				json.type = $(".tele-rule-dropdown", c).val();
				
			break;
			
		}
						
		json.enable = $('.tele-rule-toggle .checked', c).size() > 0;
		json.aggregate  = parseInt($('.tele-rule-trigger input', c).val());
		if(json.aggregate < 1) {
			$('.tele-rule-trigger input', c).addClass('error');
			telepath.dialog({ title: 'Rule Editor', msg: 'You must specify aggregation count.' });
			return false;
		}
		
		
		return json;
	
	},
	editor: function(type, container, data) {
		
		container.addClass('loaded');
		
		var that = this;
		
		if(!data || data.length == 0) {
			// Set up some defaults
			data = {
				enable: true,
				score: 95,
				aggregate: 1
			};
			
		}
		// console.log(data);
		
		// START Small Rule Bar containing TOGGLE , TRIGGER ALERT # and SCORE
		var triggerWrap  	  = $('<div>').addClass('tele-rule-trigger-wrap');
		
		this.ruleToggle       = $('<div>').teleCheckbox({ label: 'Enabled', checked: data.enable }).addClass('tele-rule-toggle');
		this.ruleTriggerAlert = $('<div>').teleInput({ label: 'Trigger alert after', suffix: 'criteria matches per session', width: 30, value: data.aggregate }).addClass('tele-rule-trigger');

		triggerWrap.append(this.ruleToggle).append(this.ruleTriggerAlert);
		container.append(triggerWrap);
		// END Small Rule Bar
		
		
		// Inner Container
		var ruleInner    = $('<div>').addClass('tele-rule-inner');		
		container.append(ruleInner);
		if (this.options.data["builtin_rule"])
		{
			$(ruleInner).hide();
			$(this.ruleTriggerAlert).hide();
		}
		
		switch(type) {
			case 'parameter':
				
				// Wrappers for 3 types
				var paramWRAP = $('<div>').addClass('tele-rule-param-wrap').hide();
				var requestWRAP    = $('<div>').addClass('tele-rule-request-wrap').hide();
				var statusCodeWRAP    = $('<div>').addClass('tele-rule-codeStatus-wrap').hide();

				// Type toggle
				var toggleType = $('<div>').teleRadios({
					radios: [
						{key: 'Parameter', label: 'Parameter'},
						{key: 'Request', label: 'Request'},
						{key: 'Response', label: 'Response'},
						{key: 'Uri', label: 'URI'}
					], callback: function (radio) {

						if (radio.key == 'Request' || radio.key == 'Response' || radio.key == 'Uri') {

							inspectionType.children('.tele-radio-wrap').hide();
							inspectionType.find('.tele-radio-radio[rel="regex"]').parent().show();
							inspectionType.find('.tele-radio-radio[rel="regex"]').click();
							// Display stringmatch when user is creating title match rule, Yuli
							inspectionType.find('.tele-radio-radio[rel="stringmatch"]').parent().show();

							if (radio.key == 'Request' || radio.key == 'Uri') {
								statusCodeWRAP.hide();
								controlsWrap.show();
								inspectionType.show();
							}

							if (radio.key == 'Response' || radio.key == 'Uri') {
								requestWRAP.hide();
							} else {
								requestWRAP.show();
							}

							if (radio.key == 'Response') {
								responseSettings.show();
								if(	responseSettings.find('.tele-radio-knob').parent().attr('rel')=='StatusCode'){
									statusCodeWRAP.show();
									controlsWrap.hide();
									inspectionType.hide();
								}else{
									statusCodeWRAP.hide();
									controlsWrap.show();
									inspectionType.show();
								}
							}
							else {
								responseSettings.hide();
							}

							paramWRAP.hide();

						} else {
							inspectionType.children('.tele-radio-wrap').show();
							requestWRAP.hide();
							controlsWrap.show();
							inspectionType.show();
							responseSettings.hide();
							statusCodeWRAP.hide();
							paramWRAP.show();
						}

					}
				}).addClass('tele-parameter-type');
				
				/* START STRING INSPECTION CONTROLS */

				
				var controlsWrap = $('<div>').addClass('tele-rule-string-inspection');
				// Heu
				// Reg
				var r_regex_control = $('<div>').addClass('rules_string_control_bool');
				var r_regex_input = $('<div style="width:200px">').teleInput({ value: '' }).addClass('tele-input-str-regex');
				var r_regex_check = $('<div>').teleCheckbox({ label: 'Not', checked: false }).addClass('tele-checkbox-str-regex');
				r_regex_control.append(r_regex_input).append(r_regex_check)
				// Con
				var r_contains_control = $('<div>').addClass('rules_string_control_bool');
				var r_contains_input = $('<div style="width:200px">').teleInput({ value: '' }).addClass('tele-input-str-stringmatch');;
				var r_contains_check = $('<div>').teleCheckbox({ label: 'Not', checked: false }).addClass('tele-checkbox-str-stringmatch');
				r_contains_control.append(r_contains_input).append(r_contains_check)

				// Fuz
				var r_fuzzy_opt  = [ 'short', 'long', 'both' ];
				var r_fuzzy_list = $('<select>').addClass('tele-rule-dropdown');

				$.each(r_fuzzy_opt, function(i, opt) { 
					var option = '<option value="' + opt + '">' + opt.charAt(0).toUpperCase() + opt.slice(1) + '</option>';
					r_fuzzy_list.append(option); 
				});
				
				//Length
				var r_len_slider_div = $('<div>');
				var r_len_input = $('<b>1</b><input id="rule-slider-length" type="text" style="width:200px" data-slider-min="1" data-slider-max="100" data-slider-step="1" data-slider-value="1" data-slider-tooltip="show"/> <b>100</b>');
				r_len_slider_div.append(r_len_input);

				//Between
				var r_bet_slider_div = $('<div>');
				var r_bet_input = $('<b>1</b><input id="rule-slider-between" data-slider-id="betSlider" type="text" style="width:200px" data-slider-min="1" data-slider-max="10" data-slider-step="1" data-slider-value="[2,5]" data-slider-tooltip="show"/> <b>10</b>');
				r_bet_slider_div.append(r_bet_input);

				//Similarity
				var r_sim_slider_div = $('<div>');
				var r_sim_input = $('<b>1</b><input id="rule-slider-similarity" data-slider-id="simSlider" type="text" style="width:200px" data-slider-min="1" data-slider-max="3" data-slider-step="1" data-slider-value="1" data-slider-tooltip="show"/> <b>3</b>');
				r_sim_slider_div.append(r_sim_input);

				// OLD SLIDERS:
				// Length
				// var r_len_slider = $('<div>').slider({
				//   min: 0,
				//   max: 100,
				//   value: 50,
				//   slide: function( event, ui ) {
				// 	console.log(ui.value);
				//   }
				// });
				// Between
				// var r_bet_slider = $('<div>').slider({
				//   range: true,
				//   min: 0,
				//   max: 100,
				//   values: [ 10, 20 ],
				//   slide: function( event, ui ) {
				// 	console.log("$" + ui.values[ 0 ] + " - $" + ui.values[ 1 ]);
				//   }
				// });
				// Similarity
				// var r_sim_slider = $('<div>').slider({
				//   min: 1,
				//   max: 20,
				//   value: 10,
				//   slide: function( event, ui ) {
				// 	console.log(ui.value);
				//   }
				// });
				
				controlsWrap.append(r_regex_control)
					.append(r_contains_control)
					.append(r_fuzzy_list)
					.append(r_len_slider_div)
					.append(r_bet_slider_div)
					.append(r_sim_slider_div);
			
				var inspectionType = $('<div>').teleRadios({
					checked: 'heuristic',
					radios: [ 
						{ key: 'heuristic', label: 'Heuristic', checked: true }, 
						{ key: 'regex', label: 'Regex' },
						{ key: 'stringmatch', label: 'String Contains' },
						{ key: 'fuzzylength', label: 'Fuzzy Length' },
						{ key: 'exactlength', label: 'Length' },
						{ key: 'rangelength', label: 'Length Between' },
						{ key: 'distance', label: 'String appear similar by' },
					], callback: function(radio) {
						
					r_regex_input.hide();
					r_regex_check.hide();
					r_contains_input.hide();
					r_contains_check.hide();
					r_fuzzy_list.hide();
					r_len_slider_div.hide();
					r_bet_slider_div.hide();
					r_sim_slider_div.hide();
						
						switch(radio.key) {
							case 'heuristic':
								r_regex_input.show();
								r_regex_check.show();
							break;
							case 'regex':
								
								r_regex_input.show();
								r_regex_check.show();
								
							break;
							
							case 'stringmatch': 
							
								r_contains_input.show();
								r_contains_check.show();
								
							break;
							
							case 'fuzzylength':
								
								r_fuzzy_list.show();
							
							break;
							
							case 'exactlength':
								
								r_len_slider_div.show()
							
							break;
							
							case 'rangelength':
								
								r_bet_slider_div.show();
							
							break;
							
							case 'distance':
								
								r_sim_slider_div.show();
								
							break;
							
						}
						
				}}).addClass('tele-string-inspection');
				
				/* END STRING INSPECTION CONTROLS */
				
				var browseOpt = { mode: 'param', type: 'param' }
				if(data.type) {
					if(data.type == 'inclusive' && data.inclusive) {
						browseOpt.global = true;
						browseOpt.paramname = data.inclusive.paramname;
					}
					if(data.type == 'specific' && data.specific) {
						browseOpt.global = false;
						browseOpt.paramname = data.specific.paramname;
						browseOpt.pagename = data.specific.pagename;
						browseOpt.domain = data.specific.domain;
					}
				}
				
				// Parameter settings
				var paramBrowse = $('<div>').teleBrowse(browseOpt);
				paramWRAP.append(paramBrowse);
				
				// Request settings
				this.requestPOST   = $('<div>').teleCheckbox({ label: 'POST' });
				this.requestGET    = $('<div>').teleCheckbox({ label: 'GET' });
				this.requestHEADER = $('<div>').teleCheckbox({ label: 'HEADER' });
				//this.requestTITLE = $('<div>').teleCheckbox({ label: 'TITLE' });
				
				requestWRAP.append(this.requestPOST).append(this.requestGET).append(this.requestHEADER);

				// Response settings
				var responseSettings = $('<div>').teleRadios({
					checked: 'Title',
					radios: [
						{key: 'Title', label: 'Title',},
						{key: 'StatusCode', label: 'Status Code'},
						{key: 'Body', label: 'Body'}
					],
					callback: function (radio) {
						if (radio.key == 'StatusCode') {
							controlsWrap.hide();
							inspectionType.hide();
							statusCodeWRAP.show();
						}
						else {
							statusCodeWRAP.hide();
							controlsWrap.show();
							inspectionType.show();
						}
					}
				}).addClass('tele-response-settings');

				// Status code settings
				statusCodeWRAP.append(getStatusCodeRangeUI(''));



				var inspectionTitle = $('<div>').addClass('tele-title-1').html('String Inspection').hide();
				var toggleReqTitle  = $('<div>').addClass('tele-title-1').html('Parameter').hide();


				// Append all UI elements to rule inner container
				ruleInner.append(toggleReqTitle)
					.append(toggleType)
					.append(paramWRAP)
					.append(requestWRAP)
					.append(responseSettings)
					.append(inspectionTitle)
					.append(inspectionType)
					.append(statusCodeWRAP)
					.append(controlsWrap);
				// DONE BUILDING UI
				
				
				
				// LOAD RULE DATA AT THIS STAGE
				
				if(data) {
				
					if(!data.method) { data.method = '' }
				
					switch(data.method) {
						case '':
							// Check param
							$('.tele-parameter-type [rel="Parameter"]', container).trigger('click');
						break;
						case 'Title':
							// Check Title
							$('.tele-parameter-type [rel="Response"]', container).trigger('click');
							$('.tele-response-settings [rel="Title"]', container).trigger('click');
						break;
						case 'StatusCode':
							// Check Status Code
							$('.tele-parameter-type [rel="Response"]', container).trigger('click');
							$('.tele-response-settings [rel="StatusCode"]', container).trigger('click');
						break;
						case 'Body':
							// Check Body
							$('.tele-parameter-type [rel="Response"]', container).trigger('click');
							$('.tele-response-settings [rel="Body"]', container).trigger('click');
						break;
						case 'Uri':
							// Check URI
							$('.tele-parameter-type [rel="Uri"]', container).trigger('click');
							break;
						default: 
							// Check request
							$('.tele-parameter-type [rel="Request"]', container).trigger('click');
							
							if(data.method.length > 0) {
								data.method = data.method.split(',');
								$.each(data.method, function(i, method) {

									switch(method) {
										case 'P':
											$('.tele-checkbox-checkbox', that.requestPOST).trigger('click');
										break;
										case 'G':
											$('.tele-checkbox-checkbox', that.requestGET).trigger('click');
										break;
										case 'H':
											$('.tele-checkbox-checkbox', that.requestHEADER).trigger('click');
										break;
									}
									
								});
							}
							
						break;
					}
					
										
					/*
					if(data.pr_attribute_displayname != '') {
						tmp.option('value', data.pr_attribute_displayname);
					}
					if(data.att_id != '') {
						tmp.option('dataID', data.att_id);
					}
					*/
					
					if(data.subtype) {
					
						switch(data.subtype) {
										
						case 'stringmatch':
						
							inspectionType.find('.tele-radio-radio[rel="stringmatch"]').click();
							$('input', r_contains_input).val(data.str_match);
							if(data.not_signal) {
								$('.tele-checkbox-checkbox', r_contains_check).click();
							}

							
						break;
						case 'regex':
							
							inspectionType.find('.tele-radio-radio[rel="regex"]').click();
							$('input', r_regex_input).val(data.str_match);
							if(data.not_signal) {
								$('.tele-checkbox-checkbox', r_regex_check).click();
							}
						
						break;
						case 'code':
							statusCodeWRAP.empty();
							statusCodeWRAP.append(getStatusCodeRangeUI(data.str_match));


						break;
						case 'fuzzylength':
							
							inspectionType.find('.tele-radio-radio[rel="fuzzylength"]').click();
							r_fuzzy_list.val(data.str_length);
						
						break;
						case 'exactlength':
							
							inspectionType.find('.tele-radio-radio[rel="exactlength"]').click();

							$('input', r_len_input).attr('value', data.str_length);
							// r_len_slider_div.data('ui-slider').option('value', data.str_length)
						
						break;
						case 'rangelength':
							
							inspectionType.find('.tele-radio-radio[rel="rangelength"]').click();

							var tmp = data.str_length.split("-");
							$('input', r_bet_input).attr('value', data.str_length);
							// r_bet_slider.data('ui-slider').option('values', [tmp[0], tmp[1]]);
						
						break;
						case 'distance':
							
							inspectionType.find('.tele-radio-radio[rel="distance"]').click();

							$('input', r_sim_input).attr('value', data.str_length);
							// r_sim_slider.data('ui-slider').option('value', data.str_similarity);
						
						break;
						default: 
							
							inspectionType.find('.tele-radio-radio[rel="heuristic"]').click();


							break;
						
						}
					} else {
						inspectionType.find('.tele-radio-radio[rel="heuristic"]').click();

					}
				
				}
				
			
			break;
			case 'pattern':
				
				var patTitle  = $('<div>').addClass('tele-title-1').html('Anchor');
				
				var changingWindow = $('<div>').addClass('tele-pattern-changing');
				
				// console.log(data);
				
				
				// OTHER BROWSE
				
				var browseOpt = { mode: 'param', label: 'Other (parameter)', type: 'param' };
				
				if(data.type == 'Other' && data.Other) {
					browseOpt = $.extend(browseOpt, data.Other);
					browseOpt.global = !data.Other.domain;
				}
				
				var otherBrowse  = $('<div>').teleBrowse(browseOpt).hide().addClass('tele-browse-other');
				
				
				// PAGE BROWSE
				
				var browseOpt = { mode: 'page' , label: 'Repeating Page', type: 'page' };
				
				if(data.subtype == 'page' && data.pagename && data.domain) {
					browseOpt.pagename = data.pagename;
					browseOpt.domain = data.domain;
				}
				
				var pageBrowse   = $('<div>').teleBrowse(browseOpt).hide().addClass('tele-browse-page');
				
				
				// PARAM BROWSE
				
				var browseOpt = { mode: 'param', label: 'Changing parameter' };
				
				if(data.subtype == 'parameter' && data.paramname) {
					browseOpt.paramname = data.paramname; 
					if(data.pagename) {
						browseOpt.pagename = data.pagename;
						browseOpt.domain = data.domain;
						browseOpt.global = false;
					} else {
						browseOpt.global = true;
					}
				}
				
				var paramBrowse  = $('<div>').teleBrowse(browseOpt).hide().addClass('tele-browse-param');
				
				// Business Action Select

				if (!data.action_name&&!data.domain){
					var action_data=[{}];
				}
				else{
					var action_data = [ { text: data.domain + ' :: ' + data.action_name, raw: { application: data.domain, action_name: data.action_name } } ];

				}
				var actionSelect = $('<div>').teleSelect({ type: 'action', values: action_data, click: function () { } }).hide();
				$('input', actionSelect).css({ width: 300 });
				$('.tele-multi-control', actionSelect).hide();
								
				changingWindow.append(otherBrowse).append(pageBrowse).append(paramBrowse).append(actionSelect);
				
				var patAnchor = $('<div>').teleRadios({ 
					radios: [ 
						{ key: 'IP', label: 'IP Address' }, 
						{ key: 'SID', label: 'Session ID' },
						{ key: 'User', label: 'User' },
						{ key: 'Other', label: 'Other' }
					], callback: function(radio) {
						
						otherBrowse.hide();
						
						switch(radio.key) {
							case 'IP':
							case 'SID':
							case 'User':
							break;
							case 'Other':
								otherBrowse.show();
							break;
						}
					
										
				}}).addClass('tele-rule-anchor');
				
				var patLinked = $('<div>').teleRadios({ 
					radios: [ 
						{ key: 'page', label: 'Page' }, 
						{ key: 'parameter', label: 'Changing Parameter' },
						{ key: 'action', label: 'Repeating Business Action' },
						{ key: 'user', label: 'Changing Username' },
					], callback: function(radio) {
						
						pageBrowse.hide();
						paramBrowse.hide();
						actionSelect.hide();
						
						switch(radio.key) {
							case 'page':
								pageBrowse.show();
							break;
							case 'parameter':
								paramBrowse.show();
							break;
							case 'action':
								actionSelect.show();
							break;
						}
										
				}}).addClass('tele-rule-bind');
				
				var patWindowWrap = $('<div>').addClass('tele-rule-pattern-window');
				var patCount = $('<div>').teleInput({ label: 'Count', value: data.count ? data.count : 3 }).addClass('tele-pattern-count');
				var patWindow = $('<div>').teleInput({ label: 'Window', value: data.time ? data.time : 60 }).addClass('tele-pattern-time');
				var patGaps   = [ 'Seconds', 'Minutes', 'Hours' ];
				var patGap    = $('<select>').addClass('tele-rule-dropdown');
				$.each(patGaps, function(i, opt) { patGap.append('<option value="' + opt + '">' + opt + '</option>'); });
				
		
				patWindowWrap.append(patCount).append(patWindow).append(patGap);
				ruleInner.append(patTitle).append(patAnchor).append(patLinked).append(changingWindow).append(patWindowWrap);
				
				// Load type
				if(!data.type) {
					data.type = 'IP';
				}
				$('.tele-rule-anchor [rel="' + data.type + '"]').click();
				
				// Load subtype
				if(!data.subtype) {
					data.subtype = 'user';
				}
				$('.tele-rule-bind [rel="' + data.subtype + '"]').click();
				
			break;
			case 'behaviour':
				
				var behavTitle  = $('<div>').addClass('tele-title-1').html('Aspects are different characteristics of web users\' behavior');
				
				var behaviorTypes  = [ 
					{ k:'average', v:'Average' },
					{ k:'query',   v:'Query'   }, 
					{ k:'landing', v:'Speed' },
					{ k:'geo',     v:'Location'     },
					{ k:'flow',    v:'Direction'    },
					{ k:'presence',v:'Structure'    }

				];
				
				var behavSelect = $('<select>').addClass('tele-rule-dropdown').addClass('tele-behavior-type');
				var behavTitle2  = $('<div>').addClass('tele-title-2').html('Select aspect type');
				
				if(!data.type) {
					data.type = 'average';
				}
				
				$.each(behaviorTypes, function(i, opt) { var selected = data.type == opt.k ? 'selected': ''; behavSelect.append('<option ' + selected + ' value="' + opt.k + '">' + opt.v + '</option>'); });
				
				this.personalBehavior = $('<div>').teleCheckbox({ label: 'Personal', checked: data.personal ? true : false }).addClass('tele-personal-behavior');
				
				
				ruleInner.append(behavTitle).append(behavTitle2).append(behavSelect).append(this.personalBehavior);

				// <select style="clear: both; float: left;background-color: #CECECE;border: 1px solid #CECECE;">
				
			break;
			case 'geographic':
				
				
				
				var geoTitle     = $('<div>').addClass('tele-title-2').html('Select geographic type');
				var geoCountries = $('<div>').teleCountry().hide();
				var geoVelocity  = $('<div>').addClass('tele-velocity-wrap');

				var velocityTimeScopes  = [
					{ k:'s', v:'Seconds' },
					{ k:'m', v:'Minutes' }, 
					{ k:'h', v:'Hours' }
				];
				var velocityTimeSelect = $('<select>').addClass('tele-geo-ts');
				$.each(velocityTimeScopes, function(i, opt) { velocityTimeSelect.append('<option value="' + opt.k + '">' + opt.v + '</option>'); });

				if(!data.ts) {
					data.ts = 60;
				}

				if (data.ts%60==0&&data.ts!=60){
					if (data.ts%3600==0){
						velocityTimeSelect.val('h');
						data.ts=data.ts/3600
					}
					else {
						velocityTimeSelect.val('m');
						data.ts=data.ts/60
					}
				}

				var g_time_input = $('<div>').teleInput({label:'KM in', width: 30, value: data.ts }).addClass('tele-geo-ts-count').css({'margin-right':'5px'});
				$('input', g_time_input);

				if(!data.distance){data.distance=1}


				var g_radius_slider_div = $('<div>');
				var g_radius_slider =  $('<div>').teleInput({width: 30, value:data.distance}).addClass('rule-slider-length');
				$('input', g_radius_slider);
				g_radius_slider_div.append(g_radius_slider);

				geoVelocity.append(g_radius_slider_div).append(g_time_input).append(velocityTimeSelect);
				
				var geoType = $('<div>');
				
				container.append(geoTitle);
				container.append(geoType);
				container.append(geoVelocity);
				container.append(geoCountries);
				
				geoType.teleRadios({ 
					radios: [ 
						{ key: 'velocity', label: 'Geographic velocity' }, 
						{ key: 'inside', label: 'Within selected countries' },
						{ key: 'outside', label: 'Outside selected countries' },
					], callback: function(radio) {
										
						switch(radio.key) {
							case 'velocity':
								geoCountries.hide();
								geoVelocity.show();
							break;
							case 'inside':
							case 'outside':
								geoCountries.show();
								geoVelocity.hide();
							break;
						}
										
				}}).addClass('tele-geo-type');
				
				if(!data.type) {
					data.type = 'velocity';
				}
				
				$('.tele-geo-type [rel="' + data.type + '"]').click();
				
				if(data.location && data.location != '') {
					var country_list = data.location.split(',');
					$.each(country_list, function(i, iso) {
						$('.tele-country-list [dataid="' + iso + '"]').click();
					});
				}
				
				
			break;
			case 'aspect':
				
				var botTitle  = $('<div>').addClass('tele-title-2').html('Bot Type');
				var botTypes  = [ 
					{ k:'Known-Bot', v:'Known-Bot' }, 
					{ k: 'Tor', v:'Tor' }
				];				
				var botSelect = $('<select>').addClass('tele-rule-dropdown');
				
				if(!data.type) {
					data.type = 'Known-Bot';
				}
				
				$.each(botTypes, function(i, opt) { var selected = data.type == opt.k ? 'selected': ''; botSelect.append('<option ' + selected + ' value="' + opt.k + '">' + opt.v + '</option>'); });

				ruleInner.append(botTitle).append(botSelect);
				
				if(data && data.aspect) {
					botSelect.val(data.aspect);
				}
				
			break;
		}
		
		if(data) {
			
			if(data.numeric_score) {
				ruleScore.data('tele-teleInput').option('value', data.numeric_score);
			}
			
			if(data.seq_index) {
				triggerRule.data('tele-teleInput').option('value', parseInt(data.seq_index)+1);
			}
			if(data.appearance) {
				triggerAlert.data('tele-teleInput').option('value', data.appearance);
			}
		}
		
	$('#rule-slider-length').slider({
		tooltip: 'always'
		});
	
	$('#rule-slider-between').slider({
		tooltip: 'always'
		});

	$('#rule-slider-similarity').slider({
		tooltip: 'always'
		});

	}
});
