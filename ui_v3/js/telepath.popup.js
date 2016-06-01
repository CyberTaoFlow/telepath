$.widget( "tele.popup", {
 
    options: {
		bindTo: false
    },
    _create: function() {
	
        this.element.addClass( "tele-popup" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		
		if(!this.options.bindTo) {
			return;
		}
		
		this.element.empty();
		
		var bind = this.options.bindTo;		
		var top = bind.offset().top;
		//var left = bind.offset().left + 10;
		var left = $(window).width() - this.element.width() - 100;
		
		this.element.css({ top: top, left: left });
		
    },

});