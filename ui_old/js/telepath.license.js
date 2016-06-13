
telepath.license = {

	init: function() {
		
		if(telepath.licenseValid !== 'VALID') {
		
			yepnope({
			load: [ telepath.basePath + "/sencha/auth/app/view/LicenseWindow.js?"],
				complete: function () {
					telepath.license.panel = Ext.create('MyApp.view.LicenseWindow');
					telepath.license.panel.modal = true;
					telepath.license.panel.show();
				}
			});
		
		}
		
	},
	go: function() {
		var input_el = Ext.getCmp('LicenseKey_input');
		var label_el = Ext.getCmp('LicenceKey_label');
		if(input_el.isValid()) {
			label_el.el.setHTML('Checking ...');
			$.post(telepath.controllerPath + '/telepath/check', { key: input_el.value }, function(data) {
				if(data.success) {
					switch(data.valid) {
						case 'VALID':
							label_el.el.setHTML('Success!');
							setTimeout(function () {
								window.location.reload(true);
							}, 1000);
						break;
						case 'INVALID':
							label_el.el.setHTML('The key you have entered is invalid.');
						break;
						case 'EXPIRED':
							label_el.el.setHTML('Your trial key had expired.');
						break;
					}
				} else {
					label_el.el.setHTML('Unknown error occured.');
				}
			}, 'json');
		} else {
			label_el.el.setHTML('The key you have entered is invalid.');
		}
		window.e = input_el;
	}

}

telepath.license.init();
