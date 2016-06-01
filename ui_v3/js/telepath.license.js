$(document).ready(function () {	telepath.validate.init(); });

telepath.loader = '<img class="loader" src="img/loader.gif">';

telepath.validate = {
	
	do_validate: function() {
		
		$('#license-error').html('').hide();
		
		var key = $('#license-number').val().trim();
		
		// Validate
		if(key == '') {
			$('#license-error').html('License number field cannot be empty.').show();
			return;
		}

		if(key.length != 36) {
			$('#license-error').html('License number is not valid.').show();
			return;
		}
		
		$('.tele-license').prepend(telepath.loader);
		
		// Indicator
		var validateData = {
			key: key,
		}
		
		$.post(telepath.controllerPath + '/telepath/check', validateData, function(result) {
			
			$('.loader').remove();
			
			if(result['success']) {
				switch(result.valid) {
						case 'VALID':
							$('.tele-input-license').css('border-color','green');
							$('.tele-label-license').html('Success!');
							setTimeout(function () {
								window.location.reload(true);
							}, 1000);
						break;
						case 'INVALID':
							$('.tele-input-license').css('border-color','red');
							$('.tele-label-license').html('The key you have entered is not valid.');
						break;
						case 'EXPIRED':
							$('.tele-input-license').css('border-color','red');
							$('.tele-label-license').html('Your trial key had expired.');
						break;
					}
				} else {
					$('#license-error').html(result['error']).show();
				}
			
		}, 'json');
	
	},
	resize: function () {
		$('.tele-container-license').height($(window).height());
	},
	init: function () {
		
		$('#license-number').val('');

		// Resize
	
		$(window).resize(function() {
			telepath.login.resize();
		});
		telepath.validate.resize();
		
		// Enter Key
		
		$(document).keypress(function(e) {
			if(e.which == 13) { telepath.validate.do_validate(); }
		});
		
		// Validate Send

		$('#validate-button').click(function () { telepath.validate.do_validate(); });
		
	}
	
};