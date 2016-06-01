$.widget( "tele.btn", {
 
    options: {
		'icon': false,
		'text': false,
    },
    _create: function() {
	
        this.element.addClass( "tele-button" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		
		if(this.options.icon !== false) {
			this.iconEl = $('<span>').addClass('tele-icon tele-icon-' + this.options.icon);
			this.element.append(this.iconEl);
			this.iconEl.hover(function () { that.hoverIn(); }, function () { that.hoverOut(); }).click(function () { that.click() });
		}
		
		if(this.options.text !== false) {
			this.textEl = $('<span>').addClass('tele-button-text').html(this.options.text);
			this.textEl.hover(function () { that.hoverIn(); }, function () { that.hoverOut(); }).click(function () { that.click() });
			this.element.append(this.textEl);
		}
		

    },
	hoverIn: function() {
		if(this.textEl) {
			this.textEl.addClass('hover');
		}
		if(this.iconEl) {
			this.iconEl.addClass('hover');
		}
	},
	hoverOut: function() {
		if(this.textEl) {
			this.textEl.removeClass('hover');
		}
		if(this.iconEl) {
			this.iconEl.removeClass('hover');
		}
	},
	click: function() {
		if(this.options.callback) {
			this.options.callback(this, this.element);
		}
	}

});