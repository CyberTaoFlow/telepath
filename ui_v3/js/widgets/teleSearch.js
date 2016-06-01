$.widget( "tele.teleSearch", {
 
    options: {
		callback: function () {}
    },
    _create: function() {
	
        this.element.addClass( "tele-search" );
        this._update();
		
    },
	rewrite: function(text) {
		
		$.each(telepath.countries.map, function (k, val) {
			if(val.toLowerCase() == text.toLowerCase()) {
				text = 'country_code:' + k.toLowerCase();
			}
		});
		
		return text;
	
	},
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		this.element.empty();
		
		var inputEl  = $('<input type="text">').addClass('tele-search-input');
		var buttonEl = $('<a>').attr('href', '#').addClass('tele-search-button').attr('id', 'search-button');
		this.element.append(inputEl).append(buttonEl);
		
		buttonEl.click(function (e) {
			var term = that.rewrite(inputEl.val());
			that.options.callback(e, term);
		});
		
		$(inputEl).keydown(function (e){
			if(e.keyCode == 13) {
				var term = that.rewrite(inputEl.val());
				that.options.callback(e, term);
			}
		});
		
    },

});