$.widget( "tele.ip", {
	
	getIP: function() {
		
		var result = '';
		for(var i = 0; i < 4; i++) {
			result += $(this.segments[i]).val() + '.';
		}
		result = result.substring(0, result.length - 1);
		
		return this.ip2long(result) !== false ? result : false;
		
	},
    // Default options.
    options: {
		data: '',
    },
 
    _create: function() {
	
        this.element.addClass( "tele-ip" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		
		this.segments = [];
		
		if(this.options.data) {
			this.options.data = this.options.data.split('.');
		}
		
		for(var i = 0; i < 4; i++) {
			var segment = $('<input>').addClass('tele-ip-segment').attr('rel', i);
			this.element.append(segment);
			this.segments.push(segment);
			
			if(this.options.data[i]) {
				segment.val(this.options.data[i]);
			}
			
		}
		this.bindEvents();
		
    },
	bindEvents: function() {
			
			var that = this;
			
			for(var i = 0; i < 4; i++) {
				
				$(this.segments[i]).change(function(e) {
					
					if(that.options.skipEvents) {
						return;
					}
					
					that.options.skipEvents = true;
					
					var val = $(this).val().replace(/[^-.0-9]/g,'').split('.');

					if(val.length > 1) {	
						for(var z = 0; z < 4; z++) {
							if(typeof(val[z]) != 'undefined') {
								$(that.segments[z]).val(val[z]);
							}
						}
					
					}

					that.options.skipEvents = false;
					
				});
				
				$(this.segments[i]).keydown(function(e) { 
					
					var i = parseInt($(this).attr('rel'));
					
					var key = e.charCode || e.keyCode || 0;
					// fix keycode for numeric keyboard, Yuli
					// http://www.cambiaresearch.com/articles/15/javascript-char-codes-key-codes
					if (key >= 96 && key <= 105)
					{
						key = key - 48;
					}
					
					if(e.ctrlKey && key == 86) {
						return;
					}
					
					// Blank and input is 0 == Move Right
					if(($(this).val() == '0' || $(e.target).val() == '' && String.fromCharCode(key) == '0') && (key >= 48 && key <= 57)) {
						$(this).val('0');
						if(i < 3) {
							$(that.segments[i + 1]).focus();
						} else {
							
						}
						return false;
					}
					
					// Invalid value of > 255
					if((parseInt($(this).val() + parseInt(String.fromCharCode(key)) ) > 255)) {
						if(i < 3) {
							$(that.segments[i + 1]).focus();
						} else {
							return false;
						}
					}

					// More than 2 digits already == Move Right
					if($(this).val().length > 2 && (key >= 48 && key <= 57) ) {
						if(i < 3) {
							$(that.segments[i + 1]).focus().val(String.fromCharCode(key));
						}
						return false;
					}
					
					// Decimal on item == Move Right
					if(key == 110 || key == 190) {
						if(i < 3) {
							$(that.segments[i + 1]).focus();
						}
						return false;
					}
					
					// Backspace on blank == Move Left
					if(key == 8 && $(e.target).val() == '') {
						if(i > 0) {
							$(that.segments[i - 1]).focus();
							return false;
						}
					}
					
					// Tab on blank == 0
					if(key == 9 && $(e.target).val() == '') {
						$(this).val('0');
					}
					
					// Arrows, HOME, END, Numbers, Keypad Numbers
					if(key == 8 || key == 9 || key == 46 ||
						(key >= 35 && key <= 40) ||
						(key >= 48 && key <= 57) ||
						(key >= 96 && key <= 105)) {
						
					} else {
						// Stop Event
						return false;
					}
				
				});

		}
		
	},
	ip2long: function (IP) {
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
		

});
