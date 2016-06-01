$.widget( "tele.pagination", {
 
    options: {
		count: 0,
		name: 'Alert',
		current: 0,
		callback: function () {}
    },
    _create: function() {
	
        this.element.addClass( "tele-pagination" );
        this._update();
		
    },
     _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
	_destroy: function() {
		$(document).unbind('keydown', this.keyDown);
	},
    _update: function() {
		
		var that = this;
		telepath.pagination = this;
		
		this.element.empty();
		
		// Pluralize
		var itemName  = this.options.count > 1 ? this.options.name + 's' : this.options.name;
		var itemCount = (this.options.current + 1) + '/' + this.options.count;
		
		this.titleEl = $('<div>').addClass('tele-pagination-title').html(itemCount + '&nbsp;' + itemName);
		this.prevEl  = $('<div>').addClass('tele-pagination-prev').addClass('tele-icon').addClass('tele-icon-prev');
		this.nextEl  = $('<div>').addClass('tele-pagination-next').addClass('tele-icon').addClass('tele-icon-next');
		
		this.prevEl.hover(function () { $(this).addClass('hover') }, function() { $(this).removeClass('hover') });
		this.nextEl.hover(function () { $(this).addClass('hover') }, function() { $(this).removeClass('hover') });
		
		this.element.append(this.prevEl, this.titleEl, this.nextEl);
		
		this.prevEl.click(function () { that.scrollPrev(); });
		this.nextEl.click(function () { that.scrollNext(); });
		
		this._constrain();
		
		// Small fix for title selection
		this.titleEl.attr('unselectable', 'on').css('user-select', 'none').on('selectstart', false);
		
		// Bind / ReBind
		$(document).unbind('keydown', this.keyDown);
		$(document).bind('keydown', this.keyDown);
		
    },
	_constrain: function() {
			
		if(this.options.current == 0) {
			this.prevEl.css({ visibility: 'hidden' });
		} else {
			this.prevEl.css({ visibility: 'visible' });
		}
		if(this.options.current == this.options.count - 1) {
			this.nextEl.css({ visibility: 'hidden' });
		} else {
			this.nextEl.css({ visibility: 'visible' });
		}
		
	},
	scrollPrev: function() {
		if(this.options.current > 0) { 
			this.options.current--; 
			this._update();
			this.options.callback(this.options.current);
		}
	},
	scrollNext: function() {
		if(this.options.current < this.options.count - 1) { 
			this.options.current++; 
			this._update(); 
			this.options.callback(this.options.current);
		}
	},
	keyDown: function (e) {
		if(e.keyCode == '37') { telepath.pagination.scrollPrev(); }
		if(e.keyCode == '39') { telepath.pagination.scrollNext(); }
	}
});