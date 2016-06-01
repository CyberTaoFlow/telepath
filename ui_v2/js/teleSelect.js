$.fn.h = function() {
    this.hover(function () { $('a', this).addClass('x-over x-btn-over x-btn-default-small-over'); }, function() { $('a', this).removeClass('x-over x-btn-over x-btn-default-small-over'); });
	return this;
};

telepath.loader = '<div class="tele-loader"></div>';

telepath.autocomplete = {
	disabled: false,
	get: function(element, type, value) {
		
		$('.tele-loader', element.parent()).remove();
		$('.tele-autocomplete-select', 'body').remove();
		
		if(this.disabled) {	return;	}
		
		if(typeof(value) == 'undefined') {
			value = $(element).val();
			if(value == '' || value == 'All') {
				$(element).val('All');
				$(element).data('tele-select', { id: -1, root: true, sub_id: -1 });
				return;
			}
		}
				
		$(element).css({ backgroundColor: '#cecece' });
		element.parent().append(telepath.loader);

		console.log('Seeking autocomplete of type ' + type + ' value ' + value);
		
		var url = '';
		switch(type) {
			case 'application':
				url = '/applications/get_autocomplete';
			break;
			case 'subdomain':
				url = '/applications/get_subdomain_autocomplete';
			break;
		}
				
		// v2/v3 compat
		var ajax_obj = telepath.ds ? telepath.ds : telepath.util.request;

		ajax_obj(url, { text: value }, function (data) {
			$('.tele-loader', element.parent()).remove();
			$(element).css({ backgroundColor: 'white' });
			if(data.items) {
				telepath.autocomplete.render(element, data.items);
			}
		}, 'Failed autocomplete');

	},
	render: function(element, items) {
	
		var container  = element.parent();
		var resultsEl  = $('<div>').addClass('tele-autocomplete-select');
		var offset     = element.offset();
		
		$('body').click(function (e) {
			if($(e.target).parents('.tele-autocomplete-select').size() == 0) {
				$('.tele-autocomplete-select').remove();
			}
		});
		
		$('.tele-autocomplete-select', 'body').remove();
		
		resultsEl.css({ position: 'absolute', top: offset.top + 24, left: offset.left, width: element.outerWidth() - 2 }).appendTo('body');
		
		$.each(items, function(i, item) {
		
			var resultEl = $('<div>').addClass('tele-autocomplete-item')
									 .text(item.text)
									 .data('tele-select', item)
									 .hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover'); });			
		
			if(item.root) {
				resultEl.addClass('rootdomain');
			} else {
				resultEl.addClass('subdomain');
			}
		
			resultEl.click(function () {
				telepath.autocomplete.disabled = true;
				element.val(item.text);
				element.data('tele-select', item);
				$('.tele-autocomplete-select', 'body').remove();
				setTimeout(function () { telepath.autocomplete.disabled = false; }, 1500);
				$(element).parents('.tele-multi').data('tele-teleSelect').options.click();
				// Lookup another event listener
				
			}).h();
			resultsEl.append(resultEl);
		});
	}
}

$.widget( "tele.teleSelect", {
	items: [],
	result: function () {
			
		var result = [];
		$('.tele-multi-input input', this.element).each(function () { result.push($(this).data('tele-select'));	});
		return result;
			
	},
    options: {	
		type: 'application',
		constrain: 5, 
		click: function () {
			
		},
		template: function(element, value) {
		
			element.addClass('tele-multi-input');
			
			var that  = this;
			var input = $('<input>');
			
			input.val(value.text);
			input.data('tele-select', value);
			
			var dd_arrow = $('<div class="x-trigger x-form-trigger x-form-arrow-trigger" role="button"></div>');
			
			element.append(input);
			element.append(dd_arrow);
			
			input.on('keypress', function () {
				if(window.autocompleteTimer) { clearTimeout(window.autocompleteTimer); }
				window.autocompleteTimer = setTimeout(function () { telepath.autocomplete.get(input, that.type); }, 500);
				value.text = input.val();
				input.data('tele-select', value);
			});
			input.on('change', function () {
				if(window.autocompleteTimer) { clearTimeout(window.autocompleteTimer); }
				window.autocompleteTimer = setTimeout(function () {	telepath.autocomplete.get(input, that.type); }, 500);
				value.text = input.val();
				input.data('tele-select', value);
			});
			
			input.on('click', function () {
				if($(this).val() == 'All') {
					$(this).select();
					setTimeout(function () { telepath.autocomplete.get(input, that.type, ''); }, 500);
				} else {
					setTimeout(function () { telepath.autocomplete.get(input, that.type); }, 500);
				}
			});
			
			dd_arrow.on('click', function() {
				input.trigger('click');
			});
						
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
		
		//var tpl_add   = '';
		//var tpl_close = '';
		
		var tpl_add   = '<a class="x-btn x-unselectable x-abs-layout-item x-btn-default-small x-icon x-btn-icon x-btn-default-small-icon"><span class="x-btn-wrap"><span class="x-btn-button"><span class="x-btn-inner x-btn-inner-center">&nbsp;</span><span style="background-image:url(Htmls/images/add.gif);" class="x-btn-icon-el">&nbsp;</span></span></span></a>';
		var tpl_close = '<a class="x-btn x-unselectable x-abs-layout-item x-btn-default-small x-icon x-btn-icon x-btn-default-small-icon"><span class="x-btn-wrap"><span class="x-btn-button"><span class="x-btn-inner x-btn-inner-center">&nbsp;</span><span style="background-image:url(Htmls/images/x.gif);" class="x-btn-icon-el">&nbsp;</span></span></span></a>';
		
		if(this.options.label) {
			var label = $('<div>').addClass('tele-label').html(this.options.label);
			this.element.append(label);
		}
		
		if(this.options.values) {
		
			$.each(this.options.values, function(i, val) {
				
				var container   = $('<div>').addClass('tele-multi-value');
				var controls    = $('<div>').addClass('tele-multi-control');
				var cmd_delete  = $('<div>').addClass('tele-icon tele-icon-close').html(tpl_close).h();
				var el			= $('<div>');
				
				that.options.template(el, val);
					
				cmd_delete.click(function () {
					container.remove();
					that.constrain();
				});
				
				controls.append(cmd_delete);
				container.append(el).append(controls);
				
				that.element.append(container);
				
				that.items.push(container);
				
			});
		}
		
		var container   = $('<div>').addClass('tele-multi-value');
		var controls    = $('<div>').addClass('tele-multi-control');
		this.cmd_create  = $('<div>').addClass('tele-icon tele-icon-plus').html(tpl_add).h();
		
		controls.append(this.cmd_create);
		container.append(controls);
		that.element.append(container);
		
		this.cmd_create.click(function () {
			
			var container   = $('<div>').addClass('tele-multi-value');
			var controls    = $('<div>').addClass('tele-multi-control');
			var cmd_delete  = $('<div>').addClass('tele-icon tele-icon-close').html(tpl_close).h();

			var el			= $('<div>');
			that.options.template(el, { text: 'All', id: '-1', sub_id: '-1', root: true });
			
			controls.append(cmd_delete);
			
			cmd_delete.click(function () {
				container.remove();
				that.constrain();
			});
			
			container.append(el).append(controls);
			
			that.cmd_create.parent().parent().before(container);
			
			that.constrain();
			
		}).h();
		
		this.cmd
		
    },
	constrain: function() {
		
		if($('.tele-multi-value', this.element).size() > this.options.constrain) {
			this.cmd_create.hide();
		} else {
			this.cmd_create.show();
		}
		
		if($('.tele-multi-value', this.element).size() < 2) {
			this.cmd_create.click();
		}
	
	}

});