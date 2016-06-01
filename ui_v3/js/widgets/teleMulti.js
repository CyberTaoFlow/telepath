$.widget( "tele.teleMulti", {
	
    options: {
		template: function(element, value) {
			element.teleInput({ value: value });
		},
		values: []
    },
    _create: function() {
        this.element.addClass( "tele-multi" );
        this._update();
    },
     _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
    _update: function() {

		// Init
		var that = this;
		
		if(this.options.title) {
			var title = $('<div>').addClass('tele-title-1').html(this.options.title);
			this.element.append(title);
		}
		
		if(this.options.values) {
		
			$.each(this.options.values, function(i, val) {
				
				var container   = $('<div>').addClass('tele-multi-value');
				var controls    = $('<div>').addClass('tele-multi-control');
				//var cmd_edit    = $('<div>').addClass('tele-icon tele-icon-edit');
				var cmd_delete  = $('<div>').addClass('tele-icon tele-icon-delete');

				var el			= $('<div>');
				that.options.template(el, val);
					
				cmd_delete.click(function () {
					container.remove();
				});
				
				controls.append(cmd_delete);
				container.append(el).append(controls);
				
				that.element.append(container);
				
			});
		}
		
		var container   = $('<div>').addClass('tele-multi-value');
		var controls    = $('<div>').addClass('tele-multi-control');
		var cmd_create  = $('<div>').addClass('tele-icon tele-icon-plus').h();
		
		controls.append(cmd_create);
		container.append(controls);
		that.element.append(container);
		
		cmd_create.click(function () {
			
			var container   = $('<div>').addClass('tele-multi-value');
			var controls    = $('<div>').addClass('tele-multi-control');
			//var cmd_edit    = $('<div>').addClass('tele-icon tele-icon-edit');
			var cmd_delete  = $('<div>').addClass('tele-icon tele-icon-delete').h();

			var el			= $('<div>');
			that.options.template(el, '');
			
			controls.append(cmd_delete);
			
			cmd_delete.click(function () {
				container.remove();
			});
			
			container.append(el).append(controls);
			
			cmd_create.parent().parent().before(container);
	
		});
		
    }

});