$.widget( "tele.notifications", {

    options: {
		interval: 10000
    },	
	toShow: [],
	indexes: false,
	showNotifications: function () {
		
		$.each(this.toShow, function(i, notifyObject) {
			$.pnotify(notifyObject);
		});
		
		this.toShow = [];
		this.count.html(0);
		
	},
    _create: function() {
		
		var that = this;
	
        this.element.addClass( "tele-notify" );
		
		// Button
	/*	this.button = $('<a>').attr('href', '#');
		this.element.append(this.button);
		
		// Counter
		this.count = $('<div>').addClass('tele-notify-count').html('0');
		this.element.append(this.count);
		
		//setInterval(function () {
		//	that._update();
		//}, this.options.interval);
		
		this.button.click(function () { that.showNotifications(); });
		this.count.click(function () { that.showNotifications(); });*/
		
        this._update();
		
		/*
		telepath.ds.get('/notifications/db_time', function (data) {
			that.time = data.items.time;
		});
		*/
		
    },
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
    _update: function() {
		
		var that = this;
		
		telepath.ds.get('/notifications/get_syslog', { }, function (data) {
			
			if(data.items && data.items.length > 0) {
					
				$.each(data.items, function(i, text) {
					
					if(text.substr(0,3) == 'CMD') {
						eval(text.substr(4));
					} else {
						console.log('SYSLOG:: ' + text);
					}
				
				});
					
			}
			
		});
		
		telepath.ds.get('/notifications/get_indexes', { time: this.time }, function (data) {
			if(!that.indexes) {
				that.indexes = data.items;
			} else {
				
				//that.toShow = [];
				
				$.each(that.indexes, function(index_name, index_value) {
					
					$.each(data.items, function(data_name, data_value) {
						
						if(index_name == data_name && data_value > index_value) {
							
							// Get DIFF
							var diff = data_value - index_value;
							
							var icon  = '';
							var title = '';
							var link  = '';
							var text  = '';
							
							// Type of notification
							switch(index_name) {
							
								case 'alerts':
									title = 'New Alerts';
									link = '<a onclick="$(\'.tele-nav-alerts a\').click()">here</a>';
									text  = diff + ' new alerts. click ' + link + ' to view';
									icon  = 'alert';
								break;
								
								case 'requests':
									title = 'Traffic';
									text  = 'Current traffic is ' + (diff * 6) + ' requests/min.';
									icon  = 'config';
									that.indexes[index_name] = data_value; // Req's / Minute dont aggregate
								break;
								
								// TODO:
								/*
								case 'applications':
									title = 'System';
									text  = 'Detected ' + diff + ' new applications.';
									icon  = 'application';
								break;
								case 'top_suspects':
									title = 'Suspects';
									link = '<a onclick="$(\'.tele-nav-suspects a\').click()">here</a>';
									text  = diff + ' new suspects. click ' + link + ' to view';
									icon  = 'suspect';
								break;
								case 'pages':
									title = 'System';
									text  = 'Detected ' + diff + ' new pages.';
									icon  = 'application';
								break;
								case 'case_alerts':
									title = 'New Case Alerts';
									text  = diff + ' new case alerts.';
									link = '<a onclick="$(\'.tele-nav-cases a\').click()">here</a>';
									text  = diff + ' new case alerts. click ' + link + ' to view';
									icon  = 'case';
								break;
								*/
							}
							
							var nData = {
								title: title,
								text: text,
								type: 'success',
								delay: 1000,
								opacity: 0.95,
								styling: 'bootstrap',
								icon: 'tele-icon tele-icon-' + icon,
							};

							// Display
							// that.toShow.push(nData);   TO RESTORE NOTIFICATIONS UPDATE UNCOMMENT THIS LINE 
							
							// Dont update new value in the indexes, essentially will aggregate data
							// that.indexes[index_name] = data_value;
						
						}
					
					});
				
				});				
				
				// Update the little bubble with the count
				if(that.count) {
					that.count.html(that.toShow.length);
				}
												
			}
		});

    }

});
