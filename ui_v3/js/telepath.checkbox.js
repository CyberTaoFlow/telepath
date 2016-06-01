$.widget( "tele.cb", {
 
    options: {
		'icon': 'checkbox',
		'checked': false,
    },
    _create: function() {
	
        this.element.addClass( "tele-checkbox" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		
		if(this.options.icon != 'checkbox') {
			this.element.addClass('tele-checkbox-' + this.options.icon);
		}
		
		if(this.options.checked) {
			
		}
		
		this.element.click(function () {
			
			if(that.options.checked) {
				$(this).removeClass('checked');
			} else {
				$(this).addClass('checked');
			}
			that.options.checked = !that.options.checked;
			
			if(that.options.callback) {
				that.options.callback(that);
			}
			
		});
		
		
    }

});