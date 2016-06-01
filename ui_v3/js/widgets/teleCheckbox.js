$.widget( "tele.teleCheckbox", {
 
    // Default options.
    options: {
		'label': false,
		'checked': false,
		'icon': 'checkbox',
		'inputFirst': true,
		'dataID': false
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
		this.element.empty();
		
		this.input = $('<div>').addClass('tele-checkbox-' + this.options.icon);
		
		// Append input first
		if(this.options.inputFirst) {	
			this.element.append(this.input);
		}
		
		if(this.options.dataID) {
			this.input.attr('dataID', this.options.dataID);
		}
		
		// Append label
		if(this.options.label) {
			this.label = $('<label>').addClass('tele-input-label').html(this.options.label);
			this.element.append(this.label);
		} else {
			this.element.addClass('no-label');
		}
		
		// Append input last
		if(!this.options.inputFirst) {	
			this.element.append(this.input);
		}
		
		if(this.options.checked) {
			this.input.addClass('checked');
		}
		
		this.input.click(function () {
			
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