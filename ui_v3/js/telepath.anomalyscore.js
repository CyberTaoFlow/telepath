$.widget( "tele.anomalyScore", {
 
    // Default options.
    options: {
		request: [],
	},
 
    _create: function() {
	
        this.element.addClass( "tele-anomaly-score" );
        this._update();
		
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		
		var scoreTypes = [ 'speed', 'direction', 'query', 'location' ];
		
		this.options.speed 	   = this.options.request.score_landing;
		this.options.direction = this.options.request.score_flow;
		this.options.query 	   = this.options.request.score_query;
		this.options.location  = this.options.request.score_geo;
		
		//delete this.options.request;
		
		//console.log(this.options);
		
		$.each(scoreTypes, function(i, scoreType) {
			
			var value = parseInt(that.options[scoreType] * 100);
			
			var typeWrap  = $('<div>').addClass('tele-anomaly-wrap');
			var typeIcon  = $('<div>').addClass('tele-anomaly-icon').addClass('tele-icon-' + scoreType);
			var typeText  = $('<div>').addClass('tele-anomaly-text').html(scoreType.charAt(0).toUpperCase() + scoreType.slice(1));
			var typeValue = $('<div>').addClass('tele-anomaly-value').html(value + '%');//.hide();
			
			typeWrap.append(typeIcon).append(typeText).append(typeValue);
			
			if(value > 25) {
				typeIcon.addClass('active');
				typeText.addClass('active');
				typeValue.addClass('active');
			}
			
			that.element.append(typeWrap);
		
		});
		
    }

});
