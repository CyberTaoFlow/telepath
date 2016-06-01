telepath.formatConditionBrief = function(container, data) {

	var result = '';
	if(!data.value)
	{
		if(container)
		{
			container.html('');
		}
		return ''; 
	}
	if(data.negate) { result = 'Not '; }
	
	// console.log(data.type);
	
	switch(data.type) {
		
		case 'rules':
			
			result = data.value.replace(',', ', ');
		
		break;
		
		case 'IP':
		
			result = data.value.replace(',', ', ');
			
		break;
		
		case 'application':
			
			result = data.value.replace(',', ', ');
			
		break;

		case 'country':
			
			$.each(data.value.split(','), function (i, v) { result += telepath.countries.map[v] + ', '; });
			result = result.substr(0, result.length - 2);
			
		break;
			
		case 'time':
		break;

		case 'parameter':

			result = data.value.replace(',', ', ');

			break;
		
	}
	
	if(container) {
		container.html(result);
	} else {
		return result;
	}
	
	
}

$.widget( "tele.conditionList", {
	
	getOpt: function(type) {
	
		// Lookup
		for(x in this.options.data) {
			if(this.options.data[x].type == type) {
				return this.options.data[x];
			}
		}
		// Default
		return { type: type, value: '', negate: false };
		
	},
	getJSON: function() {
		
		var parent     = this.element.parent();
		var conditions = $('.tele-condition', parent);
		var widgets    = [];
		var skip       = {};
		
		// Loaded widgets
		$.each(conditions, function(i, condition) {
			var data = $(condition).data('tele-condition').getCondition();
			if(data.value.length > 0) {
				widgets.push(data);
			} else {
				skip[data.type] = true;
			}
		});
		
		// Not loaded widgets - copy from input options
		if(this.options.data.length > 0) {
			for(x in this.options.data) {
			
				var found = false;
				var type = this.options.data[x].type;
				
				for(y in widgets) {
					if(widgets[y].type == type) {
						found = true;
					}
				}
				
				if(!found && !skip[type]) {
					widgets.push(this.options.data[x]);
				}
				
			}
		}
		
		return widgets;
	
	},
    options: {
		data: [],
    },
    _create: function() {
	
        this.element.addClass( "tele-condition-select" );
        this._update();
		
		return this;
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		this.element.empty();
		
		this.conditionTypes = ['application', 'rules', 'IP', /*'user',*/ 'country', /*'time',*/ 'parameter', /* 'advanced' */];
		
		// NEW
		
		this.accordion = $('<div>').addClass('tele-condition-accordion');
		
		this.element.append(this.accordion);
		
		$.each(this.conditionTypes, function(i, condition) {
			
			var headerEl    = $('<h3>').text(condition).css('text-transform', 'capitalize');
			var containerEl = $('<div>').attr('rel', condition);
			that.accordion.append(headerEl).append(containerEl);
			
			headerEl.append('<span class="brief"></span>');
			telepath.formatConditionBrief($('.brief', headerEl), that.getOpt(condition));
			$('.brief', headerEl).css('text-transform', 'none');
			
		});
		
		this.accordion.accordion({ 
		
			heightStyle: 'fill',
			collapsible: true,
			active: false,
			autoHeight: false,
			animate: false,
			activate: function( event, ui ) {
				
				// Update condition title
				if($('.tele-condition', ui.oldPanel).size() > 0) {
					var data = $('.tele-condition', ui.oldPanel).data('tele-condition').getCondition();
					telepath.formatConditionBrief($('.brief', ui.oldHeader), data);
				}

				var container = $(ui.newPanel);
				if(!container.hasClass('loaded')) {
					
					// Load correct condition
					var type = container.attr('rel');
					var conditionEl = $('<div>').condition({ type: type, data: that.getOpt(type) });
					container.append(conditionEl).addClass('loaded');
					
				}

			}
		});
		
/*
		this.element.mCustomScrollbar({
			scrollButtons:{	enable: false },
			advanced:{ updateOnContentResize: true }
		});
*/
				
    }

});
