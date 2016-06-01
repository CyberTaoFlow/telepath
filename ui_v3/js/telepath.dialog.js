telepath.dialog = function(options) {
	
	var that = this;
	
	if(!options.title) {
		options.title = 'Telepath';
	}
	
	if(!options.msg) {
		return false;
	}

	//$('.tele-overlay-mask-dialog').remove();
	//$('.tele-overlay-dialog').remove();
	
	this.maskEl = $('<div>').addClass('tele-overlay-mask').addClass('tele-overlay-mask-dialog');
	this.overlayEl = $('<div>').addClass('tele-overlay').addClass('tele-overlay-dialog');
	
	this.headerEl    = $('<div>').addClass('tele-overlay-header');
	this.contentEl   = $('<div>').addClass('tele-overlay-content');
	this.titleEl     = $('<div>').addClass('tele-overlay-title').html(options.title);
	this.closeEl     = $('<a>').attr('href', '#').addClass('tele-overlay-close').addClass('tele-icon').addClass('tele-icon-close');
	this.textEl      = $('<div>').addClass('tele-dialog-text').html(options.msg);
	
	$('body').append(this.maskEl);
	$('body').append(this.overlayEl);
	
	this.overlayEl.append(this.headerEl).append(this.contentEl);
	this.headerEl.append(this.titleEl).append(this.closeEl);
		
	this.closeEl.click(function () {
		that.maskEl.remove();
		that.overlayEl.remove();
	}).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover'); });
	
	$( this.overlayEl ).draggable({ handle: this.headerEl });

	this.btnContain = $('<div>').addClass('tele-button-container');
	this.contentEl.append(this.textEl).append(this.btnContain);
	
	switch(options.type) {
		
		case 'prompt':
			
			if(!options.value) { options.value = ''; }
			
			var inputEl   = $('<div>').teleInput({ value: options.value }).css({ margin: 15 });
			
			if(options.error) { $('input', inputEl).css({ borderColor: 'red' }); }
			
			var saveBtn   = $('<a href="#" class="tele-button tele-button-apply">Ok</a>');
			var cancelBtn  = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');
				
			this.textEl.after(inputEl);
			this.btnContain.append(saveBtn).append(cancelBtn);
					
			saveBtn.click(function () {
				if($('input', inputEl).val() == '') {

					$('input', inputEl).css({ borderColor: 'red' });
					
				} else {
				
					that.maskEl.remove();
					that.overlayEl.remove();
					options.callback($('input', inputEl).val());
					
				}
			});
			
			cancelBtn.click(function () {
				that.maskEl.remove();
				that.overlayEl.remove();				
			});
			
		break;
		
		case 'dialog':
			
			var saveBtn   = $('<a href="#" class="tele-button tele-button-apply">Ok</a>');
			var cancelBtn  = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');
				
			this.btnContain.append(saveBtn).append(cancelBtn);
					
			saveBtn.click(function () {
				that.maskEl.remove();
				that.overlayEl.remove();
				options.callback();
			});
			
			cancelBtn.click(function () {
				that.maskEl.remove();
				that.overlayEl.remove();				
			});
		
		break;
		
		case 'success':
		case 'debug':
		
		default:
		case 'alert':
			
			var closeBtn   = $('<a href="#" class="tele-button tele-button-apply">Close</a>');
			this.btnContain.append(closeBtn);
			closeBtn.click(function () {
				that.maskEl.remove();
				that.overlayEl.remove();					
			});
			
		break;
		
	}
	
	// $(this.btnContain).css({ marginLeft: (-1 * (this.btnContain.width() / 2)) + 10 });
	
}