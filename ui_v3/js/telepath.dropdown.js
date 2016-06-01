$.widget( "tele.teleDropdown", {
 
    // Default options.
    options: {
		'title': 'Select',
		'options': [],
		'selected': 0,
		'updatesTitle': false,
		'icon': false,
		'callback': function () {},
		'height': 150,
		'width': 100
    },
 
    _create: function() {
		
        this.element.addClass( "tele-dropdown" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		
		if(this.icon) {
			this.dropdownIcon  = $('<a>').attr('href', '#').addClass('tele-icon').addClass('tele-icon-' + this.options.icon);
			this.dropdownIcon.hover(function () { that.hoverIn(); }, function () { that.hoverOut(); }).click(function () { that.click() });
			this.element.append(this.dropdownIcon);
		}
		
		this.dropdownValue = $('<a>').attr('href', '#').addClass('tele-dropdown-value').html(this.options.value);
		this.dropdownArrow = $('<a>').attr('href', '#').addClass('tele-dropdown-arrow').html('&nbsp;');
		
		this.element.append(this.dropdownValue).append(this.dropdownArrow);

		this.dropdownValue.hover(function () { that.hoverIn(); }, function () { that.hoverOut(); }).click(function () { that.click() });
		this.dropdownArrow.hover(function () { that.hoverIn(); }, function () { that.hoverOut(); }).click(function () { that.click() });
		
		
    },
	hoverIn: function() {
		if(this.dropdownIcon) {
			this.dropdownIcon.addClass('hover');
		}
		this.dropdownValue.addClass('hover');
		this.dropdownArrow.addClass('hover');
	},
	hoverOut: function() {
		if(this.dropdownIcon) {
			this.dropdownIcon.removeClass('hover');
		}
		this.dropdownValue.removeClass('hover');
		this.dropdownArrow.removeClass('hover');
	},
	click: function() {
		
		var that = this;
		
		if($(".tele-dd-popup").css('display') == 'block') {
			$(".tele-dd-popup").fadeOut();
			$(".tele-dd-popup").remove();
			return;
		}
		$(".tele-dd-popup").remove();
		
		
	
		this.popup = $('<div>').addClass('tele-popup').addClass('tele-dd-popup').hide();
		$(this.element).append(this.popup);
		
		var top  = $(this.element).offset().top + $(this.element).height() + 10;
		var left = ($(this.element).offset().left + ($(this.element).width() / 2)) - 120;
		this.popup.css({ height: this.options.height, width: this.options.width }).fadeIn();
		this.list = $('<div>').addClass('tele-dd-list').css({ height: this.options.height - 30, overflow: 'auto' });
		for(x in this.options.options) {
			var conf = { key: this.options.options[x].key, label: this.options.options[x].label, inputFirst: true, checked: false };
			var element = $('<div>').addClass('tele-dd-item').html(conf.label);
			element.hover(function () { this.addClass('hover'); }, function() { this.removeClass('hover'); }).click(function () {
				this.options.callback(conf);
			});
			this.list.append(element);
		}
		this.popup.append(this.list);
		/*
		$(this.list).mCustomScrollbar({
			scrollButtons:{	enable: false },
			scrollInertia: 150
		});
		*/
		
		
	
		if(this.options.callback) {
			this.options.callback();
		}
	}

});