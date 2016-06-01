$.widget( "tele.flotGraph", {
 
    // Default options.
    options: {
		data: [],
		options: [],
		dashboard: false,
		title: false,
    },
 
    _create: function() {
		
        this.element.addClass( "tele-graph" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		
		this.canvasOuter = $('<div>').addClass('tele-graph-canvas-outer');
		this.canvasInner = $('<div>').addClass('tele-graph-canvas');
		
		this.element.empty();
		
		if(this.options.title) {
			var graphTitle = $('<div>').addClass('tele-graph-title').html(this.options.title);
			this.element.append(graphTitle);
		}
		
		this.canvasOuter.append(this.canvasInner);
		this.element.append(this.canvasOuter);
		
		if(this.options.dashboard) {
			
			
			// Graph Filters		
			this.filterTypes = [
				{ type: 'noncase-alerts', label: 'Alerts', count: 0, active: true, suffix: 'Tx' },
				{ type: 'case-alerts', label: 'Cases', count: 0, active: true, suffix: 'Tx' },
				{ type: 'suspicions', label: 'Suspects', count: 0, active: true, suffix: 'Tx' },
				{ type: 'other-sessions', label: 'Normal', count: 0, active: true, suffix: 'Tx' },
				//{ type: 'score', label: 'Score', count: 0, active: true, suffix: 'Tx' }
			];
					
			var totalCount = 0;
			
			$.each(that.filterTypes, function(i, filter) {
			$.each(that.options.data, function(x, data) {
				
				if(data.label == filter.label) {
					
					filter.count = 0;
					$.each(data.data, function(z, count) {
						filter.count = filter.count + parseInt(count[1]);
						totalCount = totalCount + parseInt(count[1]);
					});					
					
				}
			
			});
			});
			
			this.dashboardGraphFilters = $('<div>').addClass('tele-dashboard-graph-filters');
			$('.tele-graph-canvas-outer').append(this.dashboardGraphFilters);
			
			$.each(this.filterTypes, function(i, filter) {
				
				var filterWrap    = $('<div>').addClass('tele-dashboard-graph-filter').addClass('tele-graph-filter-' + filter.type);
				var filterToggle  = $('<a>').attr('href', '#').addClass('tele-graph-filter-toggle').html('&nbsp;');
				var filterTitle   = $('<div>').addClass('tele-graph-filter-title').text(filter.label);
				var filterPercent = $('<div>').addClass('tele-graph-filter-percent').text((totalCount > 0 ? parseFloat(filter.count / totalCount * 100).toFixed(2) : 0) + '%');
				var filterCount   = $('<div>').addClass('tele-graph-filter-count').text(filter.count + ' ' + filter.suffix);
				
				filterWrap.append(filterToggle).append(filterTitle).append(filterPercent).append(filterCount);
				
				that.dashboardGraphFilters.append(filterWrap);
				
				filterPercent.css({ marginRight: 160 - filterTitle.width() });
				filterCount.css({ marginRight: 160 - filterTitle.width() });
				
				if(filter.active) {
					filterToggle.addClass('active');
				}
				
				filterToggle.click(function () {
					
					if($(this).hasClass('active')) {
						$(this).removeClass('active');
						that.filterTypes[i].active = false;
					} else {
						$(this).addClass('active');
						that.filterTypes[i].active = true;
					}
					
					that.updateFilters();
					
				});
				
			});
			
			this.resize();
			this.dashboardGraphFilters.resize(function () {
				that.resize();
			});
			
			this.updateFilters();
			
		}
		
		this.printData = this.options.data;
		
		this.plot();
		
    },
	updateFilters: function() {
		
		var that = this;
		
		this.printData = [];
		
		$.each(that.filterTypes, function(i, filter) {
		
			if(filter.active) {
			
				$.each(that.options.data, function(x, data) {
					
					if(data.label == filter.label) {
						
						that.printData.push(data);
						
					}
				
				});
			
			}
			
		});
		
		this.plot();
	
	},
	plot: function() {
		this.plotObj = $.plot(this.canvasInner, this.printData, this.options.options);
	},
	resize: function () {
		
		var width       = this.dashboardGraphFilters.width();
		
		$('.tele-dashboard-graph-filter').css({ margin: 0 });
		
		var filterWidth = $('.tele-dashboard-graph-filter').outerWidth();
		
		//console.log(width + ' ' + filterWidth + ' ' + (width - (filterWidth * this.filterTypes.length)) + ' ! ' );
		
		var spare = Math.floor(Math.floor(width - (filterWidth * this.filterTypes.length)) / this.filterTypes.length) - 1;
		
		$('.tele-dashboard-graph-filter').css({ marginRight: spare });
		
		$('.tele-graph-filter-title').css({ fontSize: (width < 800 ? '16px' : '20px') });
	
	}

});