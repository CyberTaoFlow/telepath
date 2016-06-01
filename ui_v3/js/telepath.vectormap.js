$.widget( "tele.vMap", {
 
    // Default options.
    options: {
		'data': [],
		'title': false
    },
 
    _create: function() {
		
        this.element.addClass( "tele-map" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
	
    _update: function() {
		
		this.element.empty();
		
		if(this.options.title) {
			var graphTitle = $('<div>').addClass('tele-graph-title').html(this.options.title);
			this.element.append(graphTitle);
		}
		
		var scaleColors = ['#C4E8B2', '#60BB3C'];
		
		// TODO:: Make this changeable instead of CSS
		
		
		
		var max   = 0;
		var total = 0;
		
		// Only real countries go here..
		$.each(this.options.data, function(c_id, val) {
			if(c_id !== '00' && val > max) {
				max = val;
			}
			total += val;
		});
		
		this.scaleWrap =  $('<div>').addClass('tele-scale-wrap');
		this.scaleLeft =  $('<div>').addClass('tele-scale-left').html('0');
		this.scaleGrad =  $('<div>').addClass('tele-scale-grad');
		this.scaleRight = $('<div>').addClass('tele-scale-right').html(max.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",") + ' (' + total.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",") + ')');
		
		this.scaleWrap.append(this.scaleLeft).append(this.scaleGrad).append(this.scaleRight);
		
		this.vectorMap = $('<div>').css({ clear: 'both', height: 300, padding: '15px 0px 0px 0px' });
		
		this.element.append(this.scaleWrap);
		// this.element.resize(function () {
		//	console.log($(this).width() + 'x' + $(this).height());
		// });
		
		
		this.element.append(this.vectorMap);


		var country_rate =Array();
		$.each(this.options.data, function(index, value) {
			country_rate[index.toLowerCase()] = value;
		});

		this.vectorMap.vectorMap({
			map: 'world_en',
			backgroundColor: '#304F68',
			color: '#C4E8B2',
			hoverOpacity: 0.7,
			selectedColor: '#666666',
			enableZoom: true,
			showTooltip: true,
			values: country_rate,
			scaleColors: scaleColors,
			normalizeFunction: 'polynomial',
			onRegionClick: function(x, y) {
			
				telepath.alerts.searchString = 'country_code:' + y.toUpperCase();
				$('.tele-nav-alerts a').click();
				setTimeout(function () {
					$('.tele-panel-alerts .tele-search-input').val(telepath.alerts.searchString);
					$('.jqvmap-label').remove();
				}, 100);
				
			},
			onLabelShow: function(event, label, code) {
				
				var row   = $('<div>').addClass('tele-popover-row');
				var icon  = $('<div>').addClass('tele-icon tele-icon-alert');
				var count = $('<div>').addClass('tele-count').html(telepath.dashboard.data.items.map[code.toUpperCase()] ? telepath.dashboard.data.items.map[code.toUpperCase()] : 0);
				var title = $('<span>').addClass('tele-popover-subtitle').html(telepath.countries.a2n(code.toUpperCase()));
				
				row.append(icon).append(count).append(title);
				label.empty();
				label.append(row);
				
			},
			/*
			onRegionOver: function(x, y, z) {
				console.log(x);
				console.log(y);
				console.log(z);
			},
			onRegionOut: function(x, y, z) {
				console.log(x);
				console.log(y);
				console.log(z);
			},
			onRegionTipShow: function(x, y, z) {
				console.log(x);
				console.log(y);
				console.log(z);
			}
			*/
		});
		
	}
});
