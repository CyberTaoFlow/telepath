telepath.config.notifications = {
	init: function () {
		// Get full width
		this.contentLeft.hide();
		this.contentRight.css({ minWidth: '100%', width: '100%', height: '700px' });
		this.contentRight.append(telepath.loader);
		this.load();
		
	}, 
	load: function() {
		
		var that = this;
		
		$('.tele-loader', this.contentRight).remove();
		
		//this.contentRight.append();
		
		this.wrap = $('<div>').css({ padding: 20, height: 80 });
		
		this.contentRight.append(this.wrap);
		
		this.wrap.append($('<div>').addClass('tele-title-1').html('Notification Settings'));

		var poll_interval_slider = $('<div>').addClass('poll-interval-slider');
		
		this.wrap.append(poll_interval_slider);
		
		poll_interval_slider.slider({
				orientation: "horizontal",
				label:"Poll interval of ",
				suffix: " seconds.",
				range: false,
				max: 60,
				step: 5,
				value: 30
			});
			
		this.settings = [
			{ id: 'alerts', label: 'Alerts', desc: 'Notify of new alerts', icon: 'alerts' },
			{ id: 'alerts', label: 'Case Alerts', desc: 'Notify of new case alerts', icon: 'case' },
			{ id: 'suspects', label: 'Suspects', desc: 'Notify of new suspects above', icon: 'suspect', limit: true, limit_min: 0, limit_max: 100, limit_val: 90, limit_step: 1, limit_label_suffix: ' average score.'  },
			{ id: 'applications', label: 'Applications', desc: 'Telepath detects new applications', icon: 'application' },
			{ id: 'requests', label: 'Network', desc: 'Incoming request rates exceeding', icon: 'config', limit: true, limit_step: 100, limit_min: 0, limit_max: 10000, limit_val: 1000, limit_label_suffix: 'req/minute.' },
			{ id: 'pages', label: 'System', desc: 'Telepath detects new pages', icon: 'config' },
			{ id: 'updates', label: 'Updates', desc: 'System updates', icon: 'config' },
			{ id: 'upgrades', label: 'Upgrades', desc: 'Check for new version', icon: 'config' },
		];
		
		$.each(this.settings, function(i, setting) {
			
			var box   = $('<div>').addClass('tele-rounded-box').addClass('tele-notification-setting');
			var wrap = $('<div>').addClass('tele-rounded-wrap');
			var icon  = $('<div>').addClass('tele-icon tele-icon-' + setting.icon);
			var cb    = $('<div>').teleCheckbox({ label: setting.label });
			
			var desc  = $('<div>').html(setting.desc).addClass('tele-notification-desc');
			
			wrap.append(icon).append(cb);
			box.append(wrap).append(desc);

			that.contentRight.append(box);

			if(setting.limit) {
				
				// var label_slider = $('<span>').html(setting.limit_label_suffix);
				// $('.slider-track').after(setting.limit_label_suffix);
				var range = $('<input>').addClass('notifications-intervals');
				box.append(range);
				
				range.slider({
					orientation: "horizontal",
					range: false,
					max: setting.limit_max,
					min: setting.limit_min,
					step: setting.limit_step,
					value: setting.limit_val,
					label: setting.limit_label_suffix
				});
			
			} else {
				desc.css({ paddingTop: 10 });
			}
			
		});
		
		this.buttonsEl = $('<div>').addClass('tele-form-buttons');
		this.applyBtn  = $('<a href="#" class="tele-button tele-button-apply">Apply</a>');
		this.cancelBtn = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');
		
		this.buttonsEl.append(this.applyBtn).append(this.cancelBtn);
		that.contentRight.append(this.buttonsEl);
		
	}
}
