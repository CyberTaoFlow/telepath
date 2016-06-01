$.widget( "tele.teleRange", {
 
    // Default options.
    options: {
		label: false,
		options: {
			range: true,
			min: 0,
			max: 100
		}
    },
    _create: function() {
        this.element.addClass( "tele-range" );
        this._update();
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
	update_range: function(event, ui) {
		if(ui.values) {
			$(event.target).parent().find('.tele-range-from').html(ui.values[ 0 ]);
			$(event.target).parent().find('.tele-range-to').html(ui.values[ 1 ]);
		} else {
			if(ui.value) {
				$(event.target).parent().find('.tele-range-to').html(ui.value);
			}
		}
		
	},
    _update: function() {
		
		var that = this;
		this.element.empty();
		
		// Append label
		if(this.options.label) {
			this.label = $('<label>').addClass('tele-input-label').html(this.options.label);
			this.element.append(this.label);
		} else {
			this.element.addClass('no-label');
		}
		
		this.range_from = $('<div>').addClass('tele-range-from').html(this.options.options.min);
		this.range_to   = $('<div>').addClass('tele-range-to').html(this.options.options.max);
		
		if(!this.options.options.range) {
			this.range_from.hide();
			this.range_to.html(this.options.options.value);
		}
		
		// Bind to change / update
		this.options.options.slide  = that.update_range;
		this.options.options.change = that.update_range;

		this.range = $('<div>').slider(this.options.options);
		
		this.element.append(this.range_from);
		this.element.append(this.range);
		this.element.append(this.range_to);
		
		if(this.options.suffix) {
			this.suffix = $('<label>').addClass('tele-input-suffix').html(this.options.suffix);
			this.element.append(this.suffix);
		}
		
    }

});