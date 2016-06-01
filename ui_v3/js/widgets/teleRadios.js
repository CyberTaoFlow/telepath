$.widget( "tele.teleRadios", {
 
    // Default options.
    options: {
		radios: [],
		callback: function() {},
		checked: false
    },
    _create: function() {
        this.element.addClass( "tele-radios" );
        this._update();
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {

		var that = this;
		
		if(this.options.title) {
			$('<div>').html(this.options.title).addClass('tele-title-1').appendTo(this.element);
		}
		
		this.radios = [];
		
		$.each(this.options.radios, function (i, radio) {
		
			var radioWrap  = $('<div>').addClass('tele-radio-wrap');
			var radioLabel = $('<div>').addClass('tele-radio-label').html(radio.label);
			var radioInput = $('<div>').addClass('tele-radio-radio').attr('rel', radio.key);
						
			radioWrap.click(function () {
				
				$('.tele-radio-knob', that.element).remove();
				$('.tele-radio-radio', this).append('<div class="tele-radio-knob"></div>');
				that.options.callback(radio);
				that.options.checked = radio.key;
				
			}).hover(function () { 
				$(this).addClass('hover'); 
			}, function () { 
				$(this).removeClass('hover'); 
			});
			
			radioWrap.append(radioInput).append(radioLabel);
			that.element.append(radioWrap);
			that.radios.push(radioWrap);
			
			if(radio.checked || radio.key == that.options.checked) {
				radioInput.addClass('checked');
				radioWrap.click();
			}
			
		});
				
    }

});