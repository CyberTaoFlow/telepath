$.widget( "tele.teleForm", {
 
    // Default options.
    options: {
		title: false,
		items: false,
		buttons: true,
		data: {},
		callback: function () {},
		cancel: function () {}
    },
    _create: function() {
	
        this.element.addClass( "tele-form" );
		
		// Title
		if(this.options.title) {
		
			this.title = $('<div class="tele-form-title">').html(this.options.title);
			this.element.append(this.title);
			
		} else {
			
			this.element.addClass( "no-title" );
			
		}
		
		// Create Items
		this.container = $('<div class="tele-form-inner">');
		this.element.append(this.container);
		
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		// Setup
		var that = this;
		this.container.empty();
		this.items = [];

		$.each(this.options.items, function(i, item) {
			
			var item_container = $('<div>').addClass('tele-form-item');
			var item_element   = $('<div>');
			
			switch(item.type) {
				
				case 'html':
					item_container.append(item.value);
				break;
				
				case 'checkbox':
				
					item_element.teleCheckbox({ 
						label: item.label, 
						checked: that.options.data[item.dataIndex] ? that.options.data[item.dataIndex] : false 
					});
					item.data = item_element.data('tele-teleCheckbox');
					
					if(item.dataIndex) {
						item_element.attr('rel', item.dataIndex);
					}
					
				break;
				
				case 'password':
					
					item_element.telePassword({ 
						label: item.label, 
					});
					item.data = item_element.data('tele-telePassword');
					
				break;
				
				case 'text':
				default:
				
					item_element.teleInput({ 
						label: item.label, 
						value: that.options.data[item.dataIndex] ? that.options.data[item.dataIndex] : '' 
					});
					
					if(item.dataIndex) {
						item_element.attr('rel', item.dataIndex);
					}
					
					item.data = item_element.data('tele-teleInput');
					
			}
			
			if(item.type != 'html') {
				item_container.append(item_element);
			}
			
			that.container.append(item_container);
			
			//console.log(that.options.items);
			
		});
		
		if(this.options.buttons) {
		
			this.buttonsEl = $('<div>').addClass('tele-form-buttons');
			this.applyBtn  = $('<a href="#" class="tele-button tele-button-apply">Apply</a>');
			this.cancelBtn = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');
			
			this.buttonsEl.append(this.applyBtn).append(this.cancelBtn);
			this.container.append(this.buttonsEl);
			
			// BIND Validate
			this.applyBtn.click(that.validate(that));
			// BIND Cancel
			this.cancelBtn.click(that.options.cancel(that));
			
		}

    },
	validate: function(that) {
		// Trigger Apply Hook
		// console.log('HOOK');
		that.options.callback();
	}

});