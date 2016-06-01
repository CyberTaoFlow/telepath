$.widget( "tele.teleBrowse", {

    options: {
		callback: function(node) {},
		label: false,
		mode: 'page',
		value: '',
		id: false,
		dataID: false,
		domain: false,
		pagename: false,
		paramname: false,
		global: false,
		filename:false
    },
    _create: function() {
        this.element.addClass( "tele-browse" );
        this._update();
    },
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
    _update: function() {

		var that = this;
		
		this.element.empty();
		
		if(!this.options.label) {
			this.options.label = this.options.mode == 'param' ? 'Select Parameter' : 'Select Page';
		}
		
		this.input  = $('<div>').teleInput({ label: this.options.label, value: this.options.value }).click(function () {
			that.openBrowser();
		});
		this.button = $('<div>').btn({ text: 'Browse', callback: function () {
			that.openBrowser();
		} });
		
		this.element.append(this.input).append(this.button);
		
		output = { type: this.options.mode };
		
		if(this.options.domain) {
			output.domain   = this.options.domain;
		}
		if(this.options.pagename) {
			output.pagename   = this.options.pagename;
		}
		if(this.options.paramname) {
			output.paramname   = this.options.paramname;
		}
		if(this.options.global) {
			output.global   = this.options.global;
		}

		if(output.paramname || output.pagename ){
		$('input', this.input).val(output.paramname != '' ? output.paramname : output.pagename).data('selected', JSON.stringify(output));
		}
	
    },
	openBrowser: function() {
		
		var that = this;
		
		// Create overlay (TODO:: some repeating code)
		
		$('.tele-overlay-mask-dialog').remove();
		$('.tele-overlay-dialog').remove();
		
		// Base Elemenets
		this.maskEl = $('<div>').addClass('tele-overlay-mask').addClass('tele-overlay-mask-dialog');
		this.overlayEl = $('<div>').addClass('tele-overlay').addClass('tele-overlay-dialog');
		
		this.overlayEl.css({ height: 470, width: 500 });
		
		// Elements
		this.headerEl    = $('<div>').addClass('tele-overlay-header');
		this.contentEl   = $('<div>').addClass('tele-overlay-content');
		this.titleEl     = $('<div>').addClass('tele-overlay-title').html(this.options.label);
		this.closeEl     = $('<a>').attr('href', '#').addClass('tele-overlay-close').addClass('tele-icon').addClass('tele-icon-close');
		this.textEl      = $('<div>').addClass('tele-dialog-text').html('Some Text');
		
		// Add to body
		$('body').append(this.maskEl);
		$('body').append(this.overlayEl);
		
		// Structurize
		this.overlayEl.append(this.headerEl).append(this.contentEl);
		this.headerEl.append(this.titleEl).append(this.closeEl);
			
		// Bind to close button
		this.closeEl.click(function () {
			that.maskEl.remove();
			that.overlayEl.remove();
		}).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover'); });
		
		// Make draggable
		$( this.overlayEl ).draggable({ handle: this.headerEl });
		
		// Start the browser
		this.browserEL = $('<div>');
		this.contentEl.append(this.browserEL);
		this.browserEL.teleBrowser({ callback: function(e, data) {
			
			if(data && data.node) {
			
			if(that.options.mode == data.node.data.type) {
				
				var output = { type: data.node.data.type };
				
				switch(data.node.data.type) {
				
					case 'page':
						
						output.domain   = data.node.data.host;
						output.pagename = data.node.data.path;
					
					break;
					case 'param':
						
						output.paramname = data.node.data.name;
						
						if(data.node.data.global) {
							
							output.global = true;
						
						} else {
						
							var parent = data.instance.get_node(data.node.parent);
							if (parent.data)
							{
								output.domain    = parent.data.host;
								output.pagename  = parent.data.path;
							}
							
						}
						
					break;
					
				}
				
				// console.log(data);
				window.d = data;
			
				that.maskEl.remove();
				that.overlayEl.remove();
				var selected_value = (that.options.mode == 'page') ? data.node.data.path : data.node.text;
				$('input', that.input).val(selected_value).data('selected', JSON.stringify(output));
				that.options.dataID = data.node.data.id;
				that.options.callback(data.node.data.id);
				that.options.filename=selected_value;
				that.options.text   = data.node.text;
			}
			
			}
			
		}, mode: that.options.mode });
		
	}

});
