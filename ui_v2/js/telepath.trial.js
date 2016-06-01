telepath.trial = {

	init: function() {
	
		yepnope({
		load: [ telepath.basePath + "/sencha/auth/app/view/trial_ended.js?"],
			complete: function () {
				telepath.trial.panel = Ext.create('MyApp.view.trial_ended');
				telepath.trial.panel.modal = false;
				telepath.trial.panel.show();
			}
		});
		
	},

}

telepath.trial.init();
