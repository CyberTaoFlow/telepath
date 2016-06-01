$.widget( "tele.appSelect", {
 
    // Default options.
    options: {
		title: 'Select',
		options: [],
		selected: 0,
		updatesTitle: false,
		icon: 'application',
		callback: function () {},
		value: false
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
		
		this.dropdownIcon  = $('<a>').attr('href', '#').addClass('tele-icon').addClass('tele-icon-' + this.options.icon);
		
		if(!this.options.value) {
			this.options.value = telepath.appFilter;
		}
		
		var appName;
		var all_apps = false;
		
		if(telepath.app_filter.length > 0) {
			
			for(x in telepath.app_filter) {
				if(telepath.appFilter[x] == 'All') {
					appName = 'All Applications';
					all_apps = true;
				}
			}
		
		} else {
			
			appName = 'All Applications';
			all_apps = true;
			
		}
		
		if(!all_apps) {
			if(telepath.app_filter.length > 1) {
				appName = telepath.app_filter.length + ' Applications';
			} else {
				appName = telepath.app_filter[0];
			}
		}
				
		this.dropdownValue = $('<a>').attr('href', '#').addClass('tele-dropdown-value').html(appName);
		
		this.dropdownArrow = $('<a>').attr('href', '#').addClass('tele-dropdown-arrow').html('&nbsp;');

		// remove all other nodes, Yuli	
		this.element.empty();	
		this.element.append(this.dropdownIcon).append(this.dropdownValue).append(this.dropdownArrow);
		
		this.dropdownIcon.hover(function () { that.hoverIn(); }, function () { that.hoverOut(); }).click(function () { that.click() });
		this.dropdownValue.hover(function () { that.hoverIn(); }, function () { that.hoverOut(); }).click(function () { that.click() });
		this.dropdownArrow.hover(function () { that.hoverIn(); }, function () { that.hoverOut(); }).click(function () { that.click() });
		
		
    },
	hoverIn: function() {
		this.dropdownIcon.addClass('hover');
		this.dropdownValue.addClass('hover');
		this.dropdownArrow.addClass('hover');
	},
	hoverOut: function() {
		this.dropdownIcon.removeClass('hover');
		this.dropdownValue.removeClass('hover');
		this.dropdownArrow.removeClass('hover');
	},
	click: function() {
		
		var that = this;
		
		if($(".tele-appselect-popup").css('display') == 'block') {
			$(".tele-appselect-popup").fadeOut();
			$(".tele-appselect-popup").remove();
			return;
		}
		$(".tele-appselect-popup").remove();
	
		this.popup = $('<div>').addClass('tele-popup').addClass('tele-appselect-popup').hide();
		$('body').append(this.popup);
		
		var top  = $(this.element).offset().top + $(this.element).height() + 10;
		var left = ($(this.element).offset().left + ($(this.element).width() / 2)) - 120;
		this.popup.css({ top: top, left: left, width: 230, paddingBottom: 10 }).fadeIn();
		
		var data = [];
		if(telepath.app_filter && telepath.app_filter.length > 0) {
			$.each(telepath.app_filter, function (i,x) {
				data.push({ text: x });
			});
		} else {
			data.push({ text: '' });
		}
		
		var filterApps = $('<div>').teleSelect({ type: 'subdomain', values: data, click: function () { 
		
		} });
		
		this.popup.append(filterApps);
		
		
		$(filterApps).mCustomScrollbar({
			scrollButtons:{	enable: false },
			scrollInertia: 150
		});
		
		var saveBtn   = $('<a href="#" class="tele-button tele-button-apply">Save</a>');
		var btnContain = $('<div>').addClass('tele-button-container');
		
		btnContain.append(saveBtn);
		this.popup.append(btnContain);
		
		btnContain.css({ position: 'static', margin: '0' });
		
		saveBtn.click(function () {
			
			var result = [];
			
			$('input', that.popup).each(function () {
				if($(this).val() != '') {
					result.push($(this).val());				
				}
			});
			
			that.popup.remove();
			// console.log(result);
			
			telepath.ds.get('/telepath/set_app_filter', { apps: result }, function (data) {
			
				// TODO:: detect current screen and refresh
				if(that.options.callback) {
					telepath.app_filter = result;
					that._update();
					that.options.callback();
				}
				
			});
			
			
		});
		
		
		
			
	}

});
