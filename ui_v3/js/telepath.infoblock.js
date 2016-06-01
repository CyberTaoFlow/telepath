$.widget( "tele.infoblock", {
 
    options: {
	
		'icon': 'person',
		'title': 'Andy Keren',
		'subtitle': 'Developer @ HybridSec',
		'avatar': 'img/avatar.png',
		'score': 95,
		'social': {
			'facebook': 'https://www.facebook.com/andy.keren',
			'twitter' : 'https://twitter.com/NASA_Johnson/',
			'linkedin': 'www.linkedin.com/groups/Web-Fraud-Prevention-4066554'
		},
		'details': {
			'Last Seen': 'Now and again',
			'Phone': '555-555-555',
			'Email': 'ndkeren@gmail.com',
			'Address': 'Somewhere 555 st, Let-Viva, Israel'
		}
		
    },
    _create: function() {
        this.element.addClass( "tele-infoblock" );
        this._update();
    },
 
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
 
    _update: function() {
		
		var that = this;
		this.element.empty();
		
		// Containers
		
		var smallTitle   = $('<div>').addClass('tele-infoblock-smalltitle').html('Client Details');
		var imageWrap    = $('<div>').addClass('tele-infoblock-imagewrap'); 
		var scoreEl      = $('<div>').addClass('tele-infoblock-score');
		var imageEl      = $('<img>').addClass('tele-infoblock-image').attr('src', this.options.avatar); 
		var blockTitle   = $('<div>').addClass('tele-infoblock-title').html(this.options.title);
		var subTitle     = $('<div>').addClass('tele-infoblock-subtitle').html(this.options.subtitle);
		var socialList   = $('<ul>').addClass('tele-infoblock-social');
		var detailsTable = $('<table>').addClass('tele-infoblock-details');
		
		imageWrap.append(scoreEl).append(imageEl);
		
		// Social Links
		
		$.each(this.options.social, function(network, link) {
			
			var socialLi   = $('<li>');
			var socialLink = $('<a>').attr('href', link).addClass('tele-icon').addClass('tele-icon-' + network);
			
			socialLi.append(socialLink);
			socialList.append(socialLi);
			
		});
		
		// Details
		
		$.each(this.options.details, function(key, value) {
			
			var detailsTr   = $('<tr>');
			var detailKey   = $('<td>').html(key + ':');
			var detailValue = $('<td>').html('<b>' + value + '</b>');
			
			detailsTr.append(detailKey).append(detailValue);
			detailsTable.append(detailsTr);
			
		});
		
		// Append All
		this.element.append(smallTitle)
					.append(imageWrap)
					.append(blockTitle)
					.append(subTitle)
					.append(socialList)
					.append(detailsTable);
		
		// Done!
    }

});