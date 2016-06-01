telepath.overlay = {
	
	resize: function() {
		var newHeight = $('.tele-overlay').height() - $('.tele-overlay-header').height();
		if (this.minheight > 0 && this.minheight > newHeight)
		{
			newHeight = this.minheight;
		}
		$('.tele-overlay-content').height(newHeight);
		$('.tele-overlay').css({ marginTop: -1 * ($('.tele-overlay').height() / 2) });
	},
	keydownHandler: function (e) {
		if(e.keyCode == 27) {
			telepath.overlay.destroy();
		}
	},
	destroy: function () {
		// Delete any previous containers
		$('.tele-overlay, .tele-overlay-mask').remove();
		$(document).unbind('keydown', telepath.overlay.keydownHandler);
		//$(document).trigger('overlay_destroy');
	},
	init: function(icon, title, fullscreen, minheight) {
		
	
		telepath.overlay.destroy();
			
		// Create overlay
		this.maskEl    = $('<div>').addClass('tele-overlay-mask');
		this.overlayEl = $('<div>').addClass('tele-overlay').addClass('tele-overlay-' + icon);
		this.iconEl    = $('<div>').addClass('tele-overlay-icon').addClass('tele-icon').addClass('tele-icon-' + icon);
		this.headerEl    = $('<div>').addClass('tele-overlay-header');
		this.contentEl   = $('<div>').addClass('tele-overlay-content');
		this.titleEl     = $('<div>').addClass('tele-overlay-title').html(title);
		this.closeEl     = $('<a>').attr('href', '#').addClass('tele-overlay-close').addClass('tele-icon').addClass('tele-icon-close');
		this.minheight   = (typeof(minheight) !== 'undefined') ? minheight : 0 ;
		
		$('body').append(this.maskEl);
		$('body').append(this.overlayEl);
		
		this.overlayEl.append(this.iconEl).append(this.headerEl).append(this.contentEl);
		this.headerEl.append(this.titleEl).append(this.closeEl);
		
		this.closeEl.click(function () {
			telepath.overlay.destroy();
		}).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover'); });
		
		$(document).bind('keydown', telepath.overlay.keydownHandler);
		
		if(fullscreen) {
			var height = $(window).height() - 100;
			if (this.minheight > 0 && this.minheight > height)
			{
				height = this.minheight;
			}
			var width  = $(window).width() - 100;
			$('.tele-overlay').css({ 
				height: height, 
				width: width, 
				marginLeft: -1 * parseInt(width / 2), 
				marginTop: -1 * parseInt(height / 2) 
			});
		}
		
		$('.tele-overlay').resize(function () { telepath.overlay.resize(); });
		telepath.overlay.resize();
		
		$( this.overlayEl ).draggable({ handle: this.headerEl });
		
	}

}
