$.widget( "tele.toggleFlip", {
 
    // Default options.
    options: {
		'left_value': 'On',
		'right_value': 'Off',
		'flipped': false,
		'flip': false,
		'disabled': false
    },
 
    _create: function() {
	
        this.element.addClass( "tele-mini-toggle" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		
		
		
		var toggle_left   = $('<div>').addClass('tele-mini-toggle-left').html(this.options.left_value);
		var toggle_right  = $('<div>').addClass('tele-mini-toggle-right').html(this.options.right_value);
		var toggle_middle = $('<div>').addClass('tele-mini-toggle-middle');
		var toggle_knob   = $('<a>').attr('href', '#').addClass('tele-mini-toggle-knob').html('&nbsp;');
		
		toggle_middle.append(toggle_knob);
		
		this.element.empty().append(toggle_left).append(toggle_middle).append(toggle_right);
		
		if(this.options.flipped) {
			toggle_knob.addClass('active');
			toggle_knob.css({ marginLeft: 20 });
		}
		if(this.options.disabled) {
				
			this.element.css({ opacity: 0.3 });	
				
		} else {
		
		toggle_knob.click(function (e) {
			
			e.preventDefault();
			
			if(that.options.flipped) {
				$(this).removeClass('active');
				$(this).animate({ marginLeft: 0 }, 300);
			} else {
				$(this).addClass('active');
				$(this).animate({ marginLeft: 20 }, 300);
			}
			
			that.options.flipped = !that.options.flipped;
			if(typeof(that.options.flip) == 'function') {
				that.options.flip(that.options.flipped);
			}
			
		});
		
		}
		
    }

});