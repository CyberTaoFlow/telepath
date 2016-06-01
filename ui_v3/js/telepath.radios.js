$.widget( "tele.radios", {
 
    options: {
		'items': [],
		'selected': 'time',
		'title': '',
		'tip': false,
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
		
		this.element.empty();
		
		var titleEl = $('<div>').addClass('tele-radios-title').html(this.options.title);
		this.element.append(titleEl);
		
		$.each(this.options.items, function (i, item) {
			
			var itemWrap  = $('<div>').addClass('tele-radio');
			
			if(item.tip) {
				itemWrap.attr('alt', item.tip).attr('title', item.tip);
			}
			
			var itemTitle = $('<div>').addClass('tele-radio-title').html(item.title);
			var itemIcon  = $('<div>').addClass('tele-icon').addClass('tele-icon-' + item.icon);
			
			itemWrap.hover(function () { 
				$('.tele-radio-title, .tele-icon', this).addClass('hover');
			}, function() { 
				$('.tele-radio-title, .tele-icon', this).removeClass('hover');
			});
			
			if(item.id == that.options.selected) {
				itemTitle.addClass('selected');
				itemIcon.addClass('selected');
			}
			
			itemWrap.append(itemIcon).append(itemTitle);
			
			that.element.append(itemWrap);
			
			itemWrap.click(function () {
				that._setOption('selected', item.id);
				if(that.options.callback) {
					that.options.callback(that, item.id);
				}
			});
			
		});
		
    }

});