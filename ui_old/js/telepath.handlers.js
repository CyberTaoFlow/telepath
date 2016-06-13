// Helper Functions

var context_confirm = function(title, text, yesCallback, skipConfirm, noCallback) {
	
	if(skipConfirm) {
		yesCallback();
		return;
	}

	Ext.Msg.confirm(title, text, function(btn){
		if (btn == 'yes') {
			yesCallback();
		} else {
			if(typeof(noCallback) == 'function') {
				noCallback();
			}
		}
	});
	
}

var context_prompt = function(title, description, yesCallback, defaultValue) {
	Ext.Msg.prompt(title, description, function(btn, text, cfg) {
	if(btn == 'ok') {
		if(Ext.isEmpty(text)) {
			var newMsg = '<span style="color:red">' + description + '</span>';
			Ext.Msg.show(Ext.apply({}, { msg: newMsg }, cfg));
		} else {
			yesCallback(text);
		}
	}
	}, false, false, defaultValue);
	
	$(".x-message-box input").css( { width : $(".x-message-box input").parents('.x-container:first').width() } );
}

telepath.handlers = {

	param: {
	
		aliasAdd: function(record_id, callback) {
		
			context_prompt('Param Alias', 'Enter new alias', function (text) {
				
				telepath.util.request('/parameters/set_parameter_alias', { 'att_id': record_id, 'att_alias': text }, function(data) {
			
					callback(data);
		
				}, 'Error setting parameter alias.');
		
			});
		},
		aliasRemove: function(record_id, callback) {
			
			context_confirm('Param Alias', 'Remove alias?', function () {
				
				telepath.util.request('/parameters/set_parameter_alias', { 'att_id': record_id, 'att_alias': '' }, function(data) {
			
					callback(data);
		
				}, 'Error removing parameter alias.');
				
			});
		
		},
		aliasEdit: function(record_id, old_value, callback) {
			
			context_prompt('Param Alias', 'Change alias', function (text) {
				
				telepath.util.request('/parameters/set_parameter_alias', { 'att_id': record_id, 'att_alias': text }, function(data) {
			
					callback(data);
		
				}, 'Error removing parameter alias.');
				
			}, old_value);
		
		},
		maskAdd: function(record_id, record_name, callback) {
			
			context_confirm('Parameter Settings', 'Mask "' + record_name + '" Param?', function () {
				
				telepath.util.request('/parameters/set_parameter_config', { 'att_id': record_id, 'att_mask': '1' }, function(data) {
			
					callback(data);
		
				}, 'Error setting parameter mask.');
							
			});
			
		},
		maskRemove: function(record_id, record_name, callback) {
			
			context_confirm('Parameter Settings', 'UnMask ' + record_name + ' Param?', function () {
					
				telepath.util.request('/parameters/set_parameter_config', { 'att_id': record_id, 'att_mask': '0' }, function(data) {
			
					callback(data);
		
				}, 'Error removing parameter mask.');
				
			});
			
		}
	},
	page: {
			
		aliasAdd: function(record_id, callback) {
			
			context_prompt('Page Alias', 'Enter new alias', function (text) {
				
				telepath.util.request('/pages/set_page_alias', { 'page_id': record_id, 'page_alias': text, }, function(data) {
			
					callback(data);
		
				}, 'Error setting page alias.');
								
			});
			
		},
		aliasRemove: function(record_id, callback) {
			
			context_confirm('Page Alias', 'Remove alias?', function () {
				
				telepath.util.request('/pages/set_page_alias', { 'page_id': record_id, 'page_alias': '', }, function(data) {
			
					callback(data);
		
				}, 'Error removing page alias.');
				
			});
		
		},
		aliasEdit: function(record_id, old_value, callback) {
			
			context_prompt('Page Alias', 'Change alias', function (text) {
				
				telepath.util.request('/pages/set_page_alias', { 'page_id': record_id, 'page_alias': text, }, function(data) {
			
					callback(data);
		
				}, 'Error changing page alias.');
		
			}, old_value);
			
		}

	},
	app: {
		
		aliasAdd: function(record_id, callback) {
			
			context_prompt('Application Name', 'Enter new name', function (text) {
				
				telepath.util.request('/applications/set_application_alias', { 'app_id': record_id, 'app_alias': text, }, function(data) {
			
					callback(data);
		
				}, 'Error setting application name.');
								
			});
			
		},
		aliasRemove: function(record_id, callback) {
			
			context_confirm('Application Name', 'Remove name?', function () {
				
				telepath.util.request('/applications/set_application_alias', { 'app_id': record_id, 'app_alias': '', }, function(data) {
			
					callback(data);
		
				}, 'Error removing application name.');
				
			});
		
		},
		aliasEdit: function(record_id, old_value, callback) {
			
			context_prompt('Application Name', 'Change name', function (text) {
				
				telepath.util.request('/applications/set_application_alias', { 'app_id': record_id, 'app_alias': text, }, function(data) {
			
					callback(data);
		
				}, 'Error changing application name.');
		
			}, old_value);
			
		}
	
	}

}