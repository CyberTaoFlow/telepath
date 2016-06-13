telepath.ipconfig = {
	
	segment_id_header: 'ipConfig_segment_',
	show: function(defaultValue, title, callback) {
		
		this.window = Ext.create('MyApp.view.ipConfig').show();
		this.window.setTitle(title);
		this.callback = callback;
		
		this.bindEvents();
		
		if(defaultValue) {
			
			// Initialize defaultValue
			telepath.ipconfig.skipEvents = true;
				
			defaultValue = defaultValue.split('-');
			if(defaultValue.length > 1) {
				
				Ext.getCmp('ipConfig_mode_switch').setValue('IP Range');

				for(var x = 0; x < 2; x++) {
					defaultValue[x] = defaultValue[x].replace(/[^-.0-9]/g,'').split('.');
					for(var i = 0; i < 4; i++) {
						if(typeof(defaultValue[x][i]) != 'undefined') {
							Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + i).setValue(defaultValue[x][i]);
						}
					}
				}
			
			} else {
				
				Ext.getCmp('ipConfig_mode_switch').setValue('Single IP');

				defaultValue[0] = defaultValue[0].replace(/[^-.0-9]/g,'').split('.');
				for(var i = 0; i < 4; i++) {
					if(typeof(defaultValue[0][i]) != 'undefined') {
						Ext.getCmp(telepath.ipconfig.segment_id_header + '0_' + i).setValue(defaultValue[0][i]);
					}
				}

			}
			
			telepath.ipconfig.skipEvents = false;
		
		} else {
			
			Ext.getCmp('ipConfig_mode_switch').setValue('Single IP');
			
		}
		
		this.modeSwitch(Ext.getCmp('ipConfig_mode_switch').value);
		
	},
	bindEvents: function() {
		
		for(var x = 0; x < 2; x++) {
			for(var i = 0; i < 4; i++) {
				
				Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + i).addListener('change', function(e) {
					
					
					if(telepath.ipconfig.skipEvents) {
						return;
					}
					
					telepath.ipconfig.skipEvents = true;
					
					var id  = e.id.split('_');
					var x   = parseInt(id[2]);
					var i   = parseInt(id[3]);
					var val = e.value.replace(/[^-.0-9]/g,'').split('.');

					if(val.length > 1) {	
					
						for(var z = 0; z < 4; z++) {
							if(typeof(val[z]) != 'undefined') {
								Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + z).setValue(val[z]);
							}
						}
					
					}

					telepath.ipconfig.skipEvents = false;
					
					
				});
				
				Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + i).inputEl.addListener('keydown', function(e) { 
					
					var key = e.charCode || e.keyCode || 0;
					var id = $(e.target).attr('id').replace('-inputEl', '').split('_');
					var x = parseInt(id[2]);
					var i = parseInt(id[3]);
					
					if(key >= 96 && key <= 105) {
						key = key - 48;
					}
					
					if(e.ctrlKey && key == 86) {
						return;
					}
					
					// Blank and input is 0 == Move Right
					if(($(e.target).val() == '0' || $(e.target).val() == '' && String.fromCharCode(key) == '0') && ((key >= 48 && key <= 57) || (key >= 96 && key <= 105))) {
						e.stopEvent();
						Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + i).setValue('0');
						if(i < 3) {
							$(Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + parseInt(i + 1)).inputEl.dom).focus();
						} else {
							
						}
					}
					
					// Invalid value of > 255
					if((parseInt($(e.target).val() + parseInt(String.fromCharCode(key)) ) > 255) && ((key >= 48 && key <= 57) || (key >= 96 && key <= 105))) {
						if(i < 3) {
							$(Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + parseInt(i + 1)).inputEl.dom).focus();
						} else {
							e.stopEvent();
						}
					}

					// More than 2 digits already == Move Right
					if($(e.target).val().length > 2 && ((key >= 48 && key <= 57) || (key >= 96 && key <= 105))) {
						if(i < 3) {
							$(Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + parseInt(i + 1)).inputEl.dom).focus().val(String.fromCharCode(key));
						}
						e.stopEvent();
					}
					
					// Decimal on item == Move Right
					if(key == 110 || key == 190) {
						e.stopEvent();
						if(i < 3) {
							$(Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + parseInt(i + 1)).inputEl.dom).focus();
						}
					}
					
					// Backspace on blank == Move Left
					if(key == 8 && $(e.target).val() == '') {
						if(i > 0) {
							$(Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + parseInt(i - 1)).inputEl.dom).focus();
							e.stopEvent();
						}
					}
					
					// Tab on blank == 0
					if(key == 9 && $(e.target).val() == '') {
						Ext.getCmp(telepath.ipconfig.segment_id_header + x + '_' + i).setValue('0');
					}
					
					// Arrows, HOME, END, Numbers, Keypad Numbers
					if(key == 8 || key == 9 || key == 46 ||
						(key >= 35 && key <= 40) ||
						(key >= 48 && key <= 57) ||
						(key >= 96 && key <= 105)) {
						
					} else {
						// Stop Event
						e.stopEvent();
					}
				
				});
				
			}
		}
		
	},
	modeSwitch: function(newValue) {
		switch(newValue) {
		
			case 'Single IP':
				Ext.getCmp('ipConfig_label_1').setText('IP Address:');
				Ext.getCmp('ipConfig_label_2').hide();
				Ext.getCmp('ipConfig_range_end').hide();
			break;
			case 'IP Range':
				Ext.getCmp('ipConfig_label_1').setText('Range Start:');
				Ext.getCmp('ipConfig_label_2').show();
				Ext.getCmp('ipConfig_range_end').show();
			break;
			
		}
	},
	getValue: function() {
		
		var mode = Ext.getCmp('ipConfig_mode_switch').value;
		
		var ip_from = '';
		var ip_to   = '';
		var ip_long_from = false;
		var ip_long_to   = false;
		
		var result = '';
		
		for(var i = 0; i < 4; i++) {
			var segment_value = Ext.getCmp(telepath.ipconfig.segment_id_header + '0_' + i).value;
			if(!segment_value || segment_value == '' || parseInt(segment_value) < 0 || parseInt(segment_value) > 255) {
				return false;
			}
			ip_from += segment_value + '.';
		}
		
		// Drop suffix '.'
		ip_from = ip_from.substring(0, ip_from.length - 1);
		ip_long_from = ip2long(ip_from);
		
		// Check its valid
		if(!ip_long_from) {
			return false;
		}
		
		result = ip_from;
		
		switch(mode) {
			case 'Single IP':
				// Nothing here
			break;
			case 'IP Range':
			
				for(var i = 0; i < 4; i++) {
					var segment_value = Ext.getCmp(telepath.ipconfig.segment_id_header + '1_' + i).value;
					if(!segment_value || segment_value == '' || parseInt(segment_value) < 0 || parseInt(segment_value) > 255) {
						return false;
					}
					ip_to += segment_value + '.';
				}
				
				// Drop suffix '.'
				ip_to = ip_to.substring(0, ip_to.length - 1);
				ip_long_to = ip2long(ip_to);
				
				// Check its valid
				if(!ip_long_to) {
					return false;
				}
				
				// Check ranges
				if(ip_long_from > ip_long_to) {
					return false;
				}
				
				// Not a range
				if(ip_long_from == ip_long_to) {
					return result;
				}
				
				result += '-' + ip_to;
				
				
			break;
		}

		return result;
	
	},
	applyClick: function() {
		
		var result = this.getValue();
		if(result !== false) {
			this.callback(result);
			this.window.destroy();
		}
		
	},
	cancelClick: function() {
	
		this.window.destroy();
		
	},
	add_ip_to_whitelist: function(ip) {
	
		Ext.Ajax.request({
			url: telepath.controllerPath + '/config/add_to_whitelist',
			params:{
				ip:ip
			},
			success:function(response){
				try{
					var jsondata = Ext.decode(response.responseText)
				}
				catch(err){
					Ext.Msg.alert('Error #680', 'Error while adding IP to whitelist');
					return;							
				}
				if (eval(jsondata.success)){
					Ext.Msg.alert('Success', 'IP '+ip+' has been added to the whitelist successfully.');
					Ext.getCmp('c_net_ip_whitelist').getStore().load();
				}			
			},
			failure:function(){
				console.log("Error while adding IP to whitelist");
			}
		});
		
	}

}

function ip2long (IP) {
  // From: http://phpjs.org/functions
  // +   original by: Waldo Malqui Silva
  // +   improved by: Victor
  // +    revised by: fearphage (http://http/my.opera.com/fearphage/)
  // +    revised by: Theriault
  // *     example 1: ip2long('192.0.34.166');
  // *     returns 1: 3221234342
  // *     example 2: ip2long('0.0xABCDEF');
  // *     returns 2: 11259375
  // *     example 3: ip2long('255.255.255.256');
  // *     returns 3: false
  var i = 0;
  // PHP allows decimal, octal, and hexadecimal IP components.
  // PHP allows between 1 (e.g. 127) to 4 (e.g 127.0.0.1) components.
  IP = IP.match(/^([1-9]\d*|0[0-7]*|0x[\da-f]+)(?:\.([1-9]\d*|0[0-7]*|0x[\da-f]+))?(?:\.([1-9]\d*|0[0-7]*|0x[\da-f]+))?(?:\.([1-9]\d*|0[0-7]*|0x[\da-f]+))?$/i); // Verify IP format.
  if (!IP) {
    return false; // Invalid format.
  }
  // Reuse IP variable for component counter.
  IP[0] = 0;
  for (i = 1; i < 5; i += 1) {
    IP[0] += !! ((IP[i] || '').length);
    IP[i] = parseInt(IP[i]) || 0;
  }
  // Continue to use IP for overflow values.
  // PHP does not allow any component to overflow.
  IP.push(256, 256, 256, 256);
  // Recalculate overflow of last component supplied to make up for missing components.
  IP[4 + IP[0]] *= Math.pow(256, 4 - IP[0]);
  if (IP[1] >= IP[5] || IP[2] >= IP[6] || IP[3] >= IP[7] || IP[4] >= IP[8]) {
    return false;
  }
  return IP[1] * (IP[0] === 1 || 16777216) + IP[2] * (IP[0] <= 2 || 65536) + IP[3] * (IP[0] <= 3 || 256) + IP[4] * 1;
}