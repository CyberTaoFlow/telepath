$.widget( "tele.listitem", {
 
    // Default options.
    options: {
		itemID: 0,
		checkable: false,
		checked: false,
		favorites: false,
		favorite: false,
		favoriteCallBack: false,
		count: false,
		time: false,
		timeFormat: 'd/m/y g:i A',
		timeLabel: '',
		progbar: false,
		progbarValue: 0,
		progbarBig: false,
		popup: false,
		icon: 'alert',
		title: '',
		description: '',
		details: [],
		offset: 30,
		raw: [],
    },
	_resize: function() {
		
		var offset = this.options.offset;
		$('.tele-checkbox', this.element).each(function () {
			offset += $(this).outerWidth(true);
		});
		
		if(offset < 30) { offset += 20; }
		
		var innerWidth = $(this.element).width() - offset;
		
		$('.tele-listitem-inner', this.element).width(innerWidth  - 15 );
		
		var offset = 15;
		
		if(this.options.progbar) {
			if(this.options.progbarBig) {
				offset = offset + $('.tele-listitem-bigprogbar', this.element).outerWidth() + 55;
			} else {
				offset = offset + $('.tele-listitem-progbar', this.element).outerWidth()+25;
			}
		}
		
		if(this.options.icon) {
			offset = offset + $('.tele-listitem-icon', this.element).outerWidth();
		}
	
		var infoWidth = innerWidth - offset;
		
		$('.tele-listitem-info', this.element).css({ width: infoWidth });
		
		var that = this;
		var total = 30;
		if ($('.tele-listitem-info').width()<800){
			$( "ul li" ).filter( '[class*=tele-listitem-]').css({ 'padding-right':'5%' });
		}
		else {
			$( "ul li" ).filter( '[class*=tele-listitem-]').css({ 'padding-right':'10%' });
		}

		$('.tele-listitem-info li', this.element).each(function () {
			
			$(this).show();
			$(this).prev().css({ borderRightWidth: 1 });
		
			total = total + $(this).outerWidth();



			
			if(total > infoWidth) {



				if($(this).hasClass('tele-list-hosts')) {
					
					//
					var x = $(this).width();
					var y = total - infoWidth;
					$(this).width(x - y).css({ 'white-space' : 'nowrap', 'overflow' : 'hidden', 'text-overflow' : 'ellipsis', 'height' : '15px' });
					
					var hosts = $('b', this).text();
					var splited = hosts.split(',');
					if (splited.length > 1){
						var view = $('<a>View all ' + splited.length + ' hosts</a>');
						$('b', this).html(view);
						$('.tele-list-hosts a').tipsy({
							fallback: hosts,
							gravity: 'sw',
							trigger: 'hover'
						});
					}
					
					total = total + $(this).outerWidth();
					if(total > infoWidth) {
						$(this).hide();
						$(this).prev().css({ borderRightWidth: 0 });
					}
					
					
				} else {
					$(this).hide();
					$(this).prev().css({ borderRightWidth: 0 });
				}				
			} else {
				
				if($(this).hasClass('tele-list-hosts')) {
					$(this).css({ 'width' : 'auto' });
				}				
				
			}
		});
				
	},
    _create: function() {
	
        this.element.addClass( "tele-listitem" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
		
		if(key == 'checked') {
			if(value) {
				this.checkEl.teleCheckbox({ checked: true });
			} else {
				this.checkEl.teleCheckbox({ checked: false });
			}
			return;
		}
	
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		
		this.element.empty();
		
		if(this.options.checkable) {
			this.checkEl = $('<a>').teleCheckbox({ checked: this.options.checked, callback: function (widget) {
				that.options.checked = widget.options.checked;
			}});
			this.element.append(this.checkEl);
		}
		
		if(this.options.favorites) {
			this.favEl = $('<a>').teleCheckbox({ icon: 'favorites', checked: this.options.favorite, callback: function(widget) {
				that.options.favorite = widget.options.checked;
				if(typeof(that.options.favoriteCallBack) == 'function') {
					that.options.favoriteCallBack(that);
				}
				// console.log(that.options);
			}});
			this.element.append(this.favEl);
		}
		
		var el = $('<div>').addClass('tele-listitem-inner');
		this.element.append(el);

		if(this.options.icon) {
			var iconEl = $('<div>').addClass('tele-listitem-icon').addClass('tele-icon-' + this.options.icon);
			el.append(iconEl);
		}


		if(this.options.count) {
			var countEl = $('<div>').addClass('tele-listitem-count').html(this.options.count);
			el.append(countEl);
			if(parseInt(this.options.count) > 999) {
				countEl.css({ "fontSize" : '12px', "left" : '30px' });
			}
			
		}
		
		if(this.options.progbar) {
			var progbar 	 = $('<div>')
			
			if(this.options.progbarBig) {
				progbar.addClass('tele-listitem-bigprogbar');
			} else {
				progbar.addClass('tele-listitem-progbar');
			}
			
			this.options.progbarValue = parseFloat(this.options.progbarValue);
			
			if(this.options.progbarValue > 0 ) {
				if (this.options.progbarValue <= 1){
					this.options.progbarValue = this.options.progbarValue * 100;
				}
				else{
					this.options.progbarValue = parseInt(this.options.progbarValue);
				}
			}
			
			var progbarInner = $('<div>').addClass('tele-listitem-progbar-inner').css({ width: this.options.progbarValue + '%' });
			
			progbar.append(progbarInner);
			el.append(progbar);
		}
		
		if(this.options.title) {
			var titleEl = $('<div>').addClass('tele-listitem-title').html(this.options.title).attr('title', this.options.title);
			el.append(titleEl);
		}
		if(this.options.updating) {
			var updatingEl = $('<div>').addClass('tele-listitem-updating tele-icon-ring').html('Update in progress').attr('title', 'Update in progress');
			el.append(updatingEl);
		}
		if(this.options.description) {
			var descEl = $('<div>').addClass('tele-title-2').html(this.options.description);
			el.append(descEl);
		}
		
		if(this.options.details) {
		
			if(this.options.details.length > 0) {
				
				var list = $('<ul>').addClass('tele-listitem-info');
				
				if(!this.options.title) {
					list.addClass('no-title');
				}
				
				$.each(this.options.details, function (i, detail) {
						
					if(detail.key || detail.value) {
						
						var li = $('<li>');
						
						switch(detail.key) {
							
							case 'Active':
								
								var ActIcon = $('<span>');
								var ActText = $('<span>');
								
								if(detail.value) {
									ActIcon.addClass('tele-icon-active');
									ActText.text('Active');
								} else {
									ActIcon.addClass('tele-icon-inactive');
									ActText.text('Inactive');
								}
								
								li.append(ActIcon).append(ActText);
							
							break;
							
							case 'city':
								
								if(detail.key && detail.key == 'city' && detail.value && detail.value != 'Unknown') {
									li.append(detail.key).append(':&nbsp;').append('<b>' + escapeHtml(detail.value) + '</b>');
								} else {
									li = false;
								}
							
							
							break;
							
							case 'user':
								
								if(detail.key && detail.key == 'user' && detail.value && detail.value != '') {
									var iconEl = $('<div>').addClass('tele-listitem-user-name');
									li.append(iconEl).append('&nbsp&nbsp;').append(escapeHtml(detail.value));
								} else {
									li = false;
								}
							
							
							break;
							
							
							case 'Email':
								
								var mailTo = $('<a>').attr('href', 'mailto:' + escapeHtml(detail.value)).text(escapeHtml(detail.value));
								li.append(mailTo);
							
							break;
							
							case 'country':
								
								if(detail.value.length == 2) {
								
									li.append('<span class="flag flag-' + escapeHtml(detail.value) + '"></span>');
									li.append('<span class="tele-country">' + telepath.countries.a2n(detail.value) + '</span>');
									
								} else {
									
									li.append(detail.key);
									if(detail.value) { li.append(':&nbsp;'); }
									li.append('<b>' + escapeHtml(detail.value) + '</b>');

								}

							break;
							
							case 'alerts':
								
								if(parseInt(detail.value) > 0) {
								
									var iconEl = $('<div>').addClass('tele-listitem-icon').addClass('tele-icon-alert');
									var countEl = $('<div>').addClass('tele-listitem-count').html(detail.value);
									li.addClass('tele-listitem-alerts-count');
									li.append(iconEl).append(countEl);
								
								} else {
								
									li = false;
									
								}
								
							break;
							
							case 'actions':
								
								if(parseInt(detail.value) > 0) {
								
									var iconEl = $('<div>').addClass('tele-listitem-icon').addClass('tele-icon-actions');
									var countEl = $('<div>').addClass('tele-listitem-count').html(detail.value);
									li.addClass('tele-listitem-actions-count');
									li.append(iconEl).append(countEl);
								
								} else {
								
									li = false;
									
								}
								
							break;

							case 'cases':
								if (parseInt(detail.value)>0){

									var iconEl = $('<div>').addClass('tele-listitem-icon tele-icon-case');
									var countEl = $('<div>').addClass('tele-listitem-count').html(detail.value);
									li.addClass('tele-listitem-cases-count');
									li.append(iconEl).append(countEl);

								} else {

									li = false;

								}
								break;

							// case 'host': 
								
							// 	if(detail.value) {
									
							// 		li.addClass('tele-list-hosts');
							// 		li.append(detail.key);
							// 		li.append(':&nbsp;');
							// 		li.append('<b>' + detail.value + '</b>');
									
							// 	} else {
									
							// 		li = false;
									
							// 	}
							
							// break;
							
							default: 
							
								if(detail.key) {
									li.append(detail.key);
									if(detail.value) { li.append(':&nbsp;'); }
								}
								
								if(detail.value) {
									li.append('<b>' + escapeHtml(detail.value) + '</b>');
								}
							
							break;
							
						}
						
						if(li !== false) {
							list.append(li);
						}
																	
					}
					
					
					
				});

				el.append(list);
				
			}
		}
		
		if(this.options.time !== false) {
			var time = $('<div>').addClass('tele-listitem-time').html(date_format(this.options.timeFormat, this.options.time));
			if(this.options.timeLabel) {
				time.prepend('<label>' + this.options.timeLabel + '</label>');
			}
			el.append(time);
		}

		this._resize();
		
		
		$(window).resize(function () {
			if(that.timer) {
				clearTimeout(that.timer);
			}
			that.timer = setTimeout(function () {
				that._resize();
			}, 50);
		});
			
		el.hover(function () { $(this).addClass('hover'); }, function() { $(this).removeClass('hover'); }).click(function () {
			if(that.options.callback) {
				that.options.callback(that);
			}
		});
		
    },
	bindPopup: function () {
		
		var that = this;
		
		this.element.hover(function () {
			
			
		}, function () {
		
		
		});
		
		
	}
	
	
 
});
