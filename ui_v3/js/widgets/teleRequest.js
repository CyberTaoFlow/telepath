$.widget( "tele.teleRequest", {

    options: {
		data: [],
    },
    _create: function() {
        this.element.addClass('tele-request').addClass('tele-rounded-box');
        this._update();
    },
     _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
    _update: function() {
		
		var that = this;
		
		// Title
		this.pageTitle = $('<div>').addClass('tele-request-page-title').html(this.options.data.title);
		
		// Rewrite (because recording format and actions format differs)
		if(this.options.data.pagename) {
			this.options.data.uri = this.options.data.pagename;
		}
		if(this.options.data.params) {
			this.options.data.parameters = this.options.data.params;
		}
				
		// Path
		this.pagePath  = $('<div title='+this.options.data.uri+'>').addClass('tele-request-page-path').html(this.options.data.uri);
		
		// Edit
		/*this.requestEdit  = $('<div>').btn({ icon: 'edit', callback: function () {}});*/
		
		// Delete
		this.requestDelete = $('<div>').btn({ icon: 'delete', callback: function () {
			$('.popover').remove();
			that.element.remove();
		}});
		
		var has_params = false;
		// Check if request has interesting parameters
		if(this.options.data.parameters) {
			has_params = true;
		}
		
		// Append
		this.element.append(this.pageTitle).append(this.pagePath).append(this.requestDelete).append(this.requestEdit);
		
		if(has_params) {
		// Hover / Click
		this.element.hover(function () {
			$(this).addClass('hover');
			that.showParameters();
		}, function () {
			$(this).removeClass('hover');
			that.hideParameters();
		}).click(function () {
			// console.log(that.options.data);
			that.showParameters();
		});
		} else {
			this.element.append('<span style="font-size: 11px; margin-left: 27px; color: blue; clear: both; float: left;">No parameters</span>');
		}
				
		
		
	},
	showParameters: function() {
		
		var that = this;
		
		// Cleanup
		$('.popover').remove();
		this.hideParameters();
		
		// Create popup
		this.pop = $('<div>').css({ position: 'absolute', top: $(this.element).offset().top + 20, left: $(this.element).offset().left + 600 });
		$('body').append(this.pop);
		
		// Title
		var title = (this.options.data.title && this.options.data.title != '') ? this.options.data.title : this.options.data.uri;
		var content = $('<div>');

		// Print parameters
		if(this.options.data.parameters) {
			
			$.each(this.options.data.parameters, function(i, param) {
				
				// Only Get / Post (no headers)
				//if(param.type == 'G' || param.type == 'P') {
					
					var paramContainer = $('<div>').addClass('tele-request-param');
					
					// Param Delete
					var paramEdit  = $('<div>').btn({ icon: 'delete', callback: function () {
						
						// Delete STRUCT
						that.removeParameter(param.name);
						that.showParameters();

					}});
					
					// Param
					var paramName_value = escapeHtml(param.name);
					var paramName = $('<div title='+paramName_value+'>').addClass('tele-request-param-name').html(paramName_value);
					
					/*var menu1 = [ {'Option 1':function(menuItem,menu) { alert("You clicked Option 1!"); } }, $.contextMenu.separator, {'Option 2':function(menuItem,menu) { alert("You clicked Option 2!"); } } ];
					paramName.contextMenu(menu1,{theme:'osx'});*/
					
					if(param.value) {
						param.data = escapeHtml(param.value);
					}
					
					// Param Value
					var paramValue = $('<div>').addClass('tele-request-param-value').html(param.data);
					
					// Param Value Edit
					var paramValueEdit  = $('<div>').btn({ icon: 'edit', callback: function () {
						
						// replace param value container with textbox for editing...
						var edit = $('<div>').teleInput({ value: param.data != '*' ? param.data : '' });
						
						paramValue.empty();
						paramValue.append(edit);
						
						$('input', edit).css({ width: 125 }).focus().blur(function () {
							
							var value = $(this).val();
							$(this).remove();
							paramValue.empty();
							paramValue.html(value);
							that.updateParameter(param.name, value);
							
						});

					}});
					
					paramContainer.append(paramEdit).append(paramName).append(paramValue).append(paramValueEdit);
					content.append(paramContainer);
				
				//}
			
			});
		
		}
		
		$(this.pop).popover({ 
			title: title, 
			html: true,
			content: content
		}).popover('show');
		
		$('.popover').addClass('tele-params-popover');
	
	},
	hideParameters: function() {
		
		if(this.pop) { this.pop.remove(); }
		
	},
	removeParameter: function(name) {
		
		for( var x in this.options.data.parameters) {
			if( this.options.data.parameters[x].name == name) { this.options.data.parameters.splice(x,1); }
		}
	
	},
	updateParameter: function(name, value) {
		
		for( var x in this.options.data.parameters) {
			if( this.options.data.parameters[x].name == name) {
				this.options.data.parameters[x].data  = value;
				this.options.data.parameters[x].value = value;
			}
		}
		
	}

});