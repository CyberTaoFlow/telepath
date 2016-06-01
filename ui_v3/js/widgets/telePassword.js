$.widget( "tele.telePassword", {
 
    // Default options.
    options: {
		'label': false,
    },
    _create: function() {
        this.element.addClass( "tele-password" );
        this._update();
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		this.element.empty();
		
		if(this.options.label) {
			this.label = $('<label>').addClass('tele-input-label').html(this.options.label);
			this.element.append(this.label);
		}
		
		this.input_1 = $('<input type="password">').addClass('tele-input-input').addClass('tele-input-password');
		this.input_2 = $('<input type="password">').addClass('tele-input-input').addClass('tele-input-password');
		
		this.element.append(this.input_1).append(this.input_2);
		
    }

});