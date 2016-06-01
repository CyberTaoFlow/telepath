/* Telepath Datasource */

telepath.ds = {

	get: function(resource, params, success, error, flag) {
		
		var dataType = 'json';
		var method   = 'POST';

		$.ajax({
		
			  type: method,
			  url: telepath.controllerPath + resource,
			  data: params,
			  flag: flag,
			  success: function (data) {
				
				// Perform logout
				if(data.logout) { window.location.reload(true); return;	}
				
				// Debug window
				if(data.debug || data.queries) {}
				
				// Evaluate Script
				if(data.eval) {	eval(data.eval); }
				
				// Console log
				if(data.console) { console.log(data.console); }				
				
				// Success or Fail
				if(data.success) { if(typeof success == 'function') {
					success(data, this.flag);
				} }
				else {
					if(typeof error == 'function') { error(data); }
					if(typeof error == 'string') { console.log(error); }
				}
			  },
			  error: function (xhr, textStatus, e) {
				// Error
				if(typeof error == 'function') { error(textStatus);	} 
				if(typeof error == 'string') { console.log(error); }
			  },
			  dataType: dataType
			  
		});
	
	}

}

telepath.dsync = {

        get: function(resource, params, success, error) {

                var dataType = 'json';
                var method   = 'POST';

                $.ajax({

                          type: method,
                          url: telepath.controllerPath + resource,
                          data: params,
			async: false,
                          success: function (data) {

                                // Perform logout
                                if(data.logout) { window.location.reload(true); return; }

                                // Debug window
                                if(data.debug || data.queries) {}

                                // Evaluate Script
                                if(data.eval) { eval(data.eval); }

                                // Console log
                                if(data.console) { console.log(data.console); }

                                // Success or Fail
                                if(data.success) { if(typeof success == 'function') { success(data); } }
                                else {
                                        if(typeof error == 'function') { error(data); }
                                        if(typeof error == 'string') { console.log(error); }
                                }
                          },
                          error: function (xhr, textStatus, e) {
                                // Error
                                if(typeof error == 'function') { error(textStatus);     }
                                if(typeof error == 'string') { console.log(error); }
                          },
                          dataType: dataType

                });
        }

}

