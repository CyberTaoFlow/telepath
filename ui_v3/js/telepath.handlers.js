// Helper Functions

var context_confirm = function(title, text, yesCallback, skipConfirm) {

	if(skipConfirm) {
		yesCallback();
		return;
	}
	
	telepath.dialog({ 
		type: 'dialog', 
		title: title, 
		msg: text, 
		callback: yesCallback 
	});
	
}

var context_prompt = function(title, description, yesCallback, defaultValue) {
	
	telepath.dialog({ 
		type: 'prompt', 
		value: defaultValue, 
		title: title, 
		msg: description, 
		callback: yesCallback 
	});
		
}

telepath.handlers = {

	param: {
	
		aliasAdd: function(record_id, callback) {
		
			context_prompt('Param Alias', 'Enter new alias', function (text) {
				
				telepath.ds.get('/parameters/set_parameter_alias', { 'att_id': record_id, 'att_alias': text }, function(data) {
			
					callback(data);
		
				}, 'Error setting parameter alias.');
		
			});
		},
		aliasRemove: function(record_id, callback) {
			
			context_confirm('Param Alias', 'Remove alias?', function () {
				
				telepath.ds.get('/parameters/set_parameter_alias', { 'att_id': record_id, 'att_alias': '' }, function(data) {
			
					callback(data);
		
				}, 'Error removing parameter alias.');
				
			});
		
		},
		aliasEdit: function(record_id, old_value, callback) {
			
			context_prompt('Param Alias', 'Change alias', function (text) {
				
				telepath.ds.get('/parameters/set_parameter_alias', { 'att_id': record_id, 'att_alias': text }, function(data) {
			
					callback(data);
		
				}, 'Error removing parameter alias.');
				
			}, old_value);
		
		},
		maskAdd: function(record_id, record_name, callback) {
			
			context_confirm('Parameter Settings', 'Mask "' + record_name + '" Param?', function () {
				
				telepath.ds.get('/parameters/set_parameter_config', { 'att_id': record_id, 'att_mask': '1' }, function(data) {
			
					callback(data);
		
				}, 'Error setting parameter mask.');
							
			});
			
		},
		maskRemove: function(record_id, record_name, callback) {
			
			context_confirm('Parameter Settings', 'UnMask ' + record_name + ' Param?', function () {
					
				telepath.ds.get('/parameters/set_parameter_config', { 'att_id': record_id, 'att_mask': '0' }, function(data) {
			
					callback(data);
		
				}, 'Error removing parameter mask.');
				
			});
			
		}
	},
	page: {
			
		aliasAdd: function(record_id, callback) {
			
			context_prompt('Page Alias', 'Enter new alias', function (text) {
				
				telepath.ds.get('/pages/set_page_alias', { 'page_id': record_id, 'page_alias': text, }, function(data) {
			
					callback(data);
		
				}, 'Error setting page alias.');
								
			});
			
		},
		aliasRemove: function(record_id, callback) {
			
			context_confirm('Page Alias', 'Remove alias?', function () {
				
				telepath.ds.get('/pages/set_page_alias', { 'page_id': record_id, 'page_alias': '', }, function(data) {
			
					callback(data);
		
				}, 'Error removing page alias.');
				
			});
		
		},
		aliasEdit: function(record_id, old_value, callback) {
			
			context_prompt('Page Alias', 'Change alias', function (text) {
				
				telepath.ds.get('/pages/set_page_alias', { 'page_id': record_id, 'page_alias': text, }, function(data) {
			
					callback(data);
		
				}, 'Error changing page alias.');
		
			}, old_value);
			
		}

	},
	app: {
		
		aliasAdd: function(record_id, callback) {
			
			context_prompt('Application Name', 'Enter new name', function (text) {
				
				telepath.ds.get('/applications/set_application_alias', { 'app_id': record_id, 'app_alias': text, }, function(data) {
			
					callback(data);
		
				}, 'Error setting application name.');
								
			});
			
		},
		aliasRemove: function(record_id, callback) {
			
			context_confirm('Application Name', 'Remove name?', function () {
				
				telepath.ds.get('/applications/set_application_alias', { 'app_id': record_id, 'app_alias': '', }, function(data) {
			
					callback(data);
		
				}, 'Error removing application name.');
				
			});
		
		},
		aliasEdit: function(record_id, old_value, callback) {
			
			context_prompt('Application Name', 'Change name', function (text) {
				
				telepath.ds.get('/applications/set_application_alias', { 'app_id': record_id, 'app_alias': text, }, function(data) {
			
					callback(data);
		
				}, 'Error changing application name.');
		
			}, old_value);
			
		}
	
	}

}