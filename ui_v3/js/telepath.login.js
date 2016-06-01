$(document).ready(function () {	telepath.login.init(); });

telepath.loader = '<img class="loader" src="img/loader.gif">';

telepath.login = {
	
	do_reset: function() {
		
		$('#reset-error').html('').hide();
		
		// Validate
		if($('#login-email').val() == '') {
			$('#reset-error').html('Please enter your email').show();
			return;
		}
		
		$('.tele-reset').prepend(telepath.loader);
		
		$.post(telepath.controllerPath + '/auth/forgotten_password', { identity: $('#login-email').val() }, function(result) {

			$('.loader').remove();
		
			if(result['success']) {
				$('#reset-error').html('<span style="color: green;">Reset email was sent.</span>').show();
			} else {
				$('#reset-error').html(result['error']).show();
			}
					
		}, 'json');
	
	},
	do_login: function() {
		
		$('#login-error').html('').hide();
			
		// Validate
		if($('#login-username').val() == '') {
			$('#login-error').html('Please enter username').show();
			return;
		}
		if($('#login-password').val() == '') {
			$('#login-error').html('Please enter password').show();
			return;
		}
		
		$('.tele-login').prepend(telepath.loader);
		
		// Indicator
		var loginData = {
			username: $('#login-username').val(), 
			password: $('#login-password').val(),
			remember: $('#login-remember').hasClass('checked')
		}
		
		$.post(telepath.controllerPath + '/auth/login', loginData, function(result) {
			
			$('.loader').remove();
			
			if(result['success']) {
				window.location = telepath.basePath;
			} else {
				$('#login-error').html(result['error']).show();
			}
			
		}, 'json');
	
	},
	resize: function () {
		$('.tele-container').height($(window).height());
	},
	init: function () {
	
		// Resize
	
		$(window).resize(function() {
			telepath.login.resize();
		});
		telepath.login.resize();
		
		// UI
		
		this.rembemberCB = $('#login-remember').cb();
		
		// Enter Key
		
		$(document).keypress(function(e) {
			if(e.which == 13) { telepath.login.do_login(); }
		});
		
		// Login Send
		$('#login-button').click(function () { telepath.login.do_login(); });
		
		// Reset Send
		$('#reset-button').click(function () { telepath.login.do_reset(); });
		
		// Reset Toggle
		$('#login-reset').click(function () {
			$('.tele-login').hide();
			$('.tele-reset').show();
		});
		
		// Cancel Reset
		$('#login-reset-cancel').click(function () {
			$('.tele-login').show();
			$('.tele-reset').hide();
		});
		
	}
	
};