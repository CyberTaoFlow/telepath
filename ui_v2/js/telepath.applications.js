telepath.applications = {
	
	auth: {
		toggleCB: function (fieldName, newValue) {
			
			switch(fieldName) {
				
				case 'login_cookie':
					
					if (newValue) {
						Ext.getCmp('c_app_authentication_success_cookie_name').enable();
						Ext.getCmp('c_app_authentication_success_cookie_value').enable();
						Ext.getCmp('c_app_authentication_success_cookie_missing').enable();
						Ext.getCmp('c_app_authentication_success_cookie_appears').enable();
						Ext.getCmp('c_app_authentication_success_cookie_changed').enable();
					} else {
						Ext.getCmp('c_app_authentication_success_cookie_name').disable();
						Ext.getCmp('c_app_authentication_success_cookie_value').disable();
						Ext.getCmp('c_app_authentication_success_cookie_missing').disable();
						Ext.getCmp('c_app_authentication_success_cookie_appears').disable();
						Ext.getCmp('c_app_authentication_success_cookie_changed').disable();
					}
					
				break;
				
				case 'login_cookie_appears':
					
					if (newValue) {
						Ext.getCmp('c_app_authentication_success_cookie_missing').setValue(false);
						Ext.getCmp('c_app_authentication_success_cookie_changed').setValue(false);
						Ext.getCmp('c_app_authentication_success_cookie_value').enable();
					}
					
				break;
				
				case 'login_cookie_missing':
					
					if (newValue) {
						Ext.getCmp('c_app_authentication_success_cookie_appears').setValue(false);
						Ext.getCmp('c_app_authentication_success_cookie_changed').setValue(false);
						Ext.getCmp('c_app_authentication_success_cookie_value').enable();
					}
					
				break;
				
				case 'login_cookie_changed':
					
					if (newValue){
						Ext.getCmp('c_app_authentication_success_cookie_appears').setValue(false);
						Ext.getCmp('c_app_authentication_success_cookie_missing').setValue(false);
						Ext.getCmp('c_app_authentication_success_cookie_value').reset();
						Ext.getCmp('c_app_authentication_success_cookie_value').disable();
					}
					
				break;
				
				case 'login_redirect':
					
					if (newValue) {
						Ext.getCmp('c_app_authentication_success_redirect_page').enable();
						Ext.getCmp('c_app_authentication_success_redirect_page_id').enable();
						Ext.getCmp('c_app_authentication_success_redirect_page_browse').enable();
						Ext.getCmp('c_app_authentication_success_redirect_response_1').enable();
						Ext.getCmp('c_app_authentication_success_redirect_response_2').enable();
					} else {
						Ext.getCmp('c_app_authentication_success_redirect_page').disable();
						Ext.getCmp('c_app_authentication_success_redirect_page_id').disable();
						Ext.getCmp('c_app_authentication_success_redirect_page_browse').disable();
						Ext.getCmp('c_app_authentication_success_redirect_response_1').disable();
						Ext.getCmp('c_app_authentication_success_redirect_response_2').disable();
					}
				
				break;
			
				case 'login_body':
				
					if (newValue) {
						Ext.getCmp('c_app_authentication_success_body_value_value').enable();
					} else {
						Ext.getCmp('c_app_authentication_success_body_value_value').disable();
					}
				
				break;
				
				case 'logout_disabled':
					
					if(newValue) {
						Ext.getCmp('c_app_authentication_logout_param_cb').setValue(false);
						Ext.getCmp('c_app_authentication_logout_page_cb').setValue(false);
					}
					
				break;
				
				case 'logout_page':

					if (newValue) {
						Ext.getCmp('c_app_authentication_logout_param_cb').setValue(false);
						Ext.getCmp('c_app_authentication_logout_disabled_cb').setValue(false);
						Ext.getCmp('c_app_authentication_logout_page_name').enable();
						Ext.getCmp('c_app_authentication_logout_page_browse').enable();
						Ext.getCmp('c_app_authentication_logout_page_id').enable();
					} else {
						Ext.getCmp('c_app_authentication_logout_page_name').disable();
						Ext.getCmp('c_app_authentication_logout_page_browse').disable();
						Ext.getCmp('c_app_authentication_logout_page_id').disable();
					}
					
				break;
				
				case 'logout_param':

					if (newValue) {
						Ext.getCmp('c_app_authentication_logout_page_cb').setValue(false);
						Ext.getCmp('c_app_authentication_logout_disabled_cb').setValue(false);
						Ext.getCmp('c_app_authentication_logout_param_name').enable();
						Ext.getCmp('c_app_authentication_logout_param_browse').enable();
						Ext.getCmp('c_app_authentication_logout_param_id').enable();
						Ext.getCmp('c_app_authentication_logout_param_value').enable();
					} else {
						Ext.getCmp('c_app_authentication_logout_param_name').disable();
						Ext.getCmp('c_app_authentication_logout_param_id').disable();
						Ext.getCmp('c_app_authentication_logout_param_browse').disable();
						Ext.getCmp('c_app_authentication_logout_param_value').disable();
					}
					
				break;

			}
		
		}
	},
	init: function () {
		
		// Applications	
		
		Ext.StoreManager.getByKey('c_app_cookieStore').proxy.url 		   		   = telepath.controllerPath + '/applications/get_cookie_suggestion';
		Ext.StoreManager.getByKey('d_appsStore').proxy.url 		   				   = telepath.controllerPath + '/applications/get_apps_combobox_general';
		Ext.StoreManager.getByKey('general_appsStore').proxy.url 		   		   = telepath.controllerPath + '/applications/get_apps_combobox_general_without_all';
		
		Ext.StoreManager.getByKey('d_appsStore').load();
		Ext.StoreManager.getByKey('general_appsStore').load();
	
	},
	get_list: function (callback) {
		
		telepath.util.request('/applications/get_list', { }, function(data) {
			
			callback(data);
		
		}, 'Error loading application list.');

	},
	del_application: function (app_ids) {
		
		telepath.util.request('/applications/del_application', { app_id: app_ids }, function(data) {
			
			Ext.StoreManager.getByKey('d_appsStore').load();
			Ext.StoreManager.getByKey('general_appsStore').load();
			telepath.config.appsReload();
		
		}, 'Error deleting application.');
			
	}

}

////////////////////////////////////////////Applications Wizard/////////////////////////////////////////////////

var app_row_to_select_on_load;
var certificate_file_name;
var private_key_file_name;
var certificate_finished=true;
var private_key_finished=true;
var certificate_started=false;
var private_key_started=false;

/*
 * Function: add_new_app
 * 
 * Invoked:
 * 	When the user clicks 'Save' on the 'Application Wizard' window.
 * 	The function validates all requirements, uploads the ceritifcate and private key if needed,
 * 	then it takes all the data from the authentication, General and Advanced tabs and saves it 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<upload_new_app>
 */

function add_new_app(){
	var app_id = Ext.getCmp('app_wizard_app_id_to_add').getValue();
	//validate fields
	var general_valid = validate_general_window('app_wizard')
	var authentication_valid = validate_authentication_window('app_wizard')
	var ssl_valid = validate_ssl_window('app_wizard')
	var advanced_valid = validate_advanced_window('app_wizard')
	if (!general_valid){
		Ext.getCmp('app_wizard_general_button').fireEvent('click');
	}
	else if(!authentication_valid){
		Ext.getCmp('app_wizard_authentication_button').fireEvent('click');
	}
	else if(!ssl_valid){
		Ext.getCmp('app_wizard_ssl_button').fireEvent('click');
	}
	else if(!advanced_valid){
		Ext.getCmp('app_wizard_advanced_button').fireEvent('click');
	}
	if (!general_valid || !authentication_valid || !ssl_valid || !advanced_valid){
		Ext.Msg.alert('Success', 'Please make sure the fields meet the requirements');
		return;
	}
	if (!general_valid || !authentication_valid || !ssl_valid || !advanced_valid)
		return;
	
	//ssl window
	if (Ext.getCmp('app_wizard_ssl_on').getValue()){
		certificate_finished=false;
		private_key_finished=false;
		certificate_started=true;
		private_key_started=true;
		//upload certificate
		var form = Ext.getCmp('app_wizard_upload_certificate')
		if(Ext.getCmp('app_wizard_upload_certificate_form').isValid()){
			form.submit({
				url: telepath.controllerPath + '/applications/set_ssl_certificate',
				waitMsg: 'Uploading certificate...',
				params:{
					app_id : app_id,
					waitMsg: 'Uploading certificate...',
					file_name: Ext.getCmp('app_wizard_upload_certificate_form').getValue()
				},
				success: function(fp, o) {
					if (o.result.success=='true'){
						certificate_finished = true
						certificate_file_name = o.result.file
						upload_new_app('app_wizard')
						
					}
					else
						Ext.Msg.alert('Error', + o.result.error + '.');
				}
			});
		}
		//upload private key
		var form = Ext.getCmp('app_wizard_upload_private_key')
		if(Ext.getCmp('app_wizard_upload_private_key_form').isValid()){
			form.submit({
				url: telepath.controllerPath + '/applications/set_ssl_private_key',
				waitMsg: 'Uploading certificate...',
				params:{
					app_id : app_id,
					waitMsg: 'Uploading private key...',
					file_name: Ext.getCmp('app_wizard_upload_private_key_form').getValue()
				},
				success: function(fp, o) {
					if (o.result.success=='true'){
						private_key_file_name = o.result.file
						private_key_finished = true
						upload_new_app('app_wizard')
					}
					else
						Ext.Msg.alert('Error', + o.result.error + '.');
				}
			});
		}
	}
	else
		upload_new_app('app_wizard')
}


/*
 * Function: upload_new_app
 * 
 * Invoked:
 * 	Whenever we finished uploading a certificate and a private key, and we need to save all the other information on the server.
 * 	The function takes all the information from the authentication,general and advanced tabs and saves them on the server.
 * 
 * Parameters:
 * 	prefix - string,the prefix of the id of all fields that we want to upload
 * 		 if its the fields of the application wizard, invoke with "app_wizard", if its in edit mode, invoke with 'a_app'
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<save_new_app>
 */
function upload_new_app(prefix) {

	//wait for both flags to be raised
	if ((private_key_started & !private_key_finished) || (certificate_started & !certificate_finished))
		return;
	else{
		certificate_finished = false;
		private_key_finished = false;
		if (certificate_started && private_key_started){
			Ext.Msg.alert('Success', 'Certificate File \''+certificate_file_name+'\' and Private Key file "' +private_key_file_name + '" have been uploaded.');
			certificate_started = false;
			private_key_started = false;		
		}
		else if (certificate_started){
			Ext.Msg.alert('Success', 'Certificate File \''+certificate_file_name+'\' has been uploaded.');
			certificate_started = false;
		}
		else if (private_key_started){
			Ext.Msg.alert('Success', 'Private Key \''+private_key_file_name+'\' has been uploaded.');
			private_key_started = false;
		}
	}
	var app_id = Ext.getCmp(prefix+'_app_id_to_add').getValue();
	//finished validation
	var params = {}

	params['td18'] = (Ext.getCmp(prefix+'_ssl_on').getValue()?1:0)
	//general window
	//app_id
	params['td0'] = app_id;
	//app name
	params['td2'] = Ext.getCmp(prefix+'_general_display_name').getValue();
	//app domain
	params['td1'] = Ext.getCmp(prefix+'_general_application_domain').getValue();
	//get cookie values
	var store = Ext.getCmp(prefix+'_general_add_cookie_list').getStore();
	var cookies="";
	store.each(function(record){
		cookies += record.get('cookie')+","
	})
	params['td9'] = cookies;	
	//get ips
	store = Ext.getCmp(prefix+'_general_add_ip_list').getStore();
	var ips=""
	store.each(function(record){
		ips += record.get('ip')+","
	})
	params['td20'] = ips;
	/////Authentication window
	params['td25'] = (Ext.getCmp(prefix+'_authentication_form_cb').getValue()?1:0)
	params['td26'] = Ext.getCmp(prefix+'_authentication_form_parameter_id').getValue()
	params['td27'] = Ext.getCmp(prefix+'_authentication_form_parameter').getValue()
	params['td23'] = (Ext.getCmp(prefix+'_authentication_basic_cb').getValue()?1:0)
	params['td24'] = (Ext.getCmp(prefix+'_authentication_digest_cb').getValue()?1:0)
	params['td12'] = (Ext.getCmp(prefix+'_authentication_ntlm_cb').getValue()?1:0)
	
	// Login Criteria
	params['td29'] = (Ext.getCmp(prefix+'_authentication_success_cookie_cb').getValue()?1:0)	
	params['td36'] = Ext.getCmp(prefix+'_authentication_success_cookie_name').getValue()
	params['td37'] = Ext.getCmp(prefix+'_authentication_success_cookie_value').getValue()
	params['td5'] = (Ext.getCmp(prefix+'_authentication_success_cookie_missing').getValue()?0:1)	
	params['td30'] = (Ext.getCmp(prefix+'_authentication_success_redirect_cb').getValue()?1:0)	

	params['td31'] = Ext.getCmp(prefix+'_authentication_success_redirect_page_id').getValue()
	params['td32'] = Ext.getCmp(prefix+'_authentication_success_redirect_page').getValue()
	params['td33'] = Ext.getCmp(prefix+'_authentication_success_redirect_response_1').getValue()+"-"+Ext.getCmp(prefix+'_authentication_success_redirect_response_2').getValue()
	params['td34'] = (Ext.getCmp(prefix+'_authentication_success_body_value_cb').getValue()?1:0)
	params['td35'] = Ext.getCmp(prefix+'_authentication_success_body_value_value').getValue()
	
	
	// Logout Criteria
	
	if(Ext.getCmp(prefix+'_authentication_logout_page_cb')) {
		
		// Page
		
		params['logout_page_cb']   = Ext.getCmp(prefix+'_authentication_logout_page_cb').getValue() ? 1 : 0;
		if(Ext.getCmp(prefix+'_authentication_logout_page_cb').getValue()) {
			params['td6']   = Ext.getCmp(prefix+'_authentication_logout_page_id').getValue(); // logout_page_id
		} else {
			params['td6']   = '';
		}
		
	} else {
		params['logout_page_cb'] = 0;
		params['td6']   = '';
	}
	
	
	if(Ext.getCmp(prefix+'_authentication_logout_param_cb')) {
	
		// Param
		
		params['logout_param_cb']    = Ext.getCmp(prefix+'_authentication_logout_param_cb').getValue() ? 1 : 0;
		if(Ext.getCmp(prefix+'_authentication_logout_param_cb').getValue()) {
			params['td7'] = Ext.getCmp(prefix+'_authentication_logout_param_id').getValue(); // logout_att_id
			params['td8'] = Ext.getCmp(prefix+'_authentication_logout_param_value').getValue(); // logout_att_value
		} else {
			params['td7'] = '';
			params['td8'] = '';
		}
		
	} else {
		params['logout_param_cb'] = 0;
		params['td7'] = '';
		params['td8'] = '';
	}

	/////SSL window
	params['td18'] = (Ext.getCmp(prefix+'_ssl_on').getValue()?1:0)
	params['td19'] = Ext.getCmp(prefix+'_ssl_port').getValue()
	params['td21'] = Ext.getCmp(prefix+'_ssl_password').getValue()
	//advanced window
	//get global pages
	var store = Ext.getCmp(prefix+'_advanced_global_pages_list').getStore();
	var global_pages="";
	store.each(function(record){
		global_pages += record.get('page')+","
	})
	params['td15'] = global_pages
	//get api
	params['td22'] = Ext.getCmp(prefix+'_advanced_injected_header_name').getValue();
	//get captcha //sessionc clear
	params['td10'] = Ext.getCmp(prefix+'_advanced_session_clear_name').getValue();
	params['td11'] = Ext.getCmp(prefix+'_advanced_session_clear_value').getValue();
	
	telepath.util.request('/applications/add_application', params, function(data) {
		
		// Ext.Msg.alert('Success', 'Application \'' + data.app_name + '\' has been '+(prefix=='app_wizard'?'added':'updated'));
		
		if (prefix=='app_wizard') {
		
			Ext.WindowManager.get('application_wizard').destroy()
			
		} else if (prefix=='c_app') {
		
			edit_mode_flag = false;
			set_readonly_app_general_info(true);
			set_readonly_app_authentication_info(true);
			set_readonly_app_ssl_info(true);
			set_readonly_app_advanced_info(true);
			Ext.getCmp('c_app_save_button').disable();
			Ext.getCmp('c_app_cancel_button').disable();							
			
		}
		telepath.config.appsReload();
		
	}, 'Error while trying to ' + (prefix=='app_wizard'?'create':'update') + ' the application.');
	
}


/*
 * Function: validate_general_window
 * 
 * Invoked:
 * 	Privately from save_new_app,save_app_edition.
 * 	The function checks that all field in the general tab are valid
 * 
 * Parameters:
 * 	prefix - string,the prefix of the id of all fields that we want to upload
 * 		 if its the fields of the application wizard, invoke with "app_wizard", if its in edit mode, invoke with 'a_app'
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<save_new_app>
 * 	<save_app_edition>
 */
function validate_general_window(prefix){
	return (Ext.getCmp(prefix+'_general_display_name').isValid() & 
		Ext.getCmp(prefix+'_general_application_domain').isValid() & 
		!(Ext.getCmp(prefix+'_ssl_on').getValue() && !Ext.getCmp(prefix+'_general_add_ip_list').isValid())
	)
}
/*
 * Function: validate_authentication_window
 * 
 * Invoked:
 * 	Privately from save_new_app,save_app_edition.
 * 	The function checks that all field in the authentication tab are valid
 * 
 * Parameters:
 * 	prefix - string,the prefix of the id of all fields that we want to upload
 * 		 if its the fields of the application wizard, invoke with "app_wizard", if its in edit mode, invoke with 'a_app'
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<save_new_app>
 * 	<save_app_edition>
 */
function validate_authentication_window(prefix){
	return (
		!(Ext.getCmp(prefix+'_authentication_form_cb').getValue() & 
			!(	Ext.getCmp(prefix+'_authentication_form_parameter').isValid() &
				Ext.getCmp(prefix+'_authentication_form_parameter_id').isValid() 
			)
		) &
		!(Ext.getCmp(prefix+'_authentication_success_cookie_cb').getValue() &
			!(
				Ext.getCmp(prefix+'_authentication_success_cookie_name').isValid() &
				Ext.getCmp(prefix+'_authentication_success_cookie_value').isValid()
			)
		) &
		!(Ext.getCmp(prefix+'_authentication_success_redirect_cb').getValue() &
			!(
				Ext.getCmp(prefix+'_authentication_success_redirect_page').isValid() &
				Ext.getCmp(prefix+'_authentication_success_redirect_response_1').isValid() &
				Ext.getCmp(prefix+'_authentication_success_redirect_response_2').isValid()
			)
		) &
		!(Ext.getCmp(prefix+'_authentication_success_body_value_cb').getValue() &
			!(
				Ext.getCmp(prefix+'_authentication_success_body_value_value').isValid()
			)
		)		
		
	)
}
/*
 * Function: validate_ssl_window
 * 
 * Invoked:
 * 	Privately from save_new_app,save_app_edition.
 * 	The function checks that all field in the ssl tab are valid
 * 
 * Parameters:
 * 	prefix - string,the prefix of the id of all fields that we want to upload
 * 		 if its the fields of the application wizard, invoke with "app_wizard", if its in edit mode, invoke with 'a_app'
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<save_new_app>
 * 	<save_app_edition>
 */
function validate_ssl_window(prefix){
	if (prefix=='app_wizard')
		return	!(Ext.getCmp(prefix+'_ssl_on').getValue() &
				!(
					Ext.getCmp(prefix+'_upload_certificate_form').isValid() &
					Ext.getCmp(prefix+'_upload_private_key_form').isValid() &
					Ext.getCmp(prefix+'_ssl_port').isValid() &
					Ext.getCmp(prefix+'_ssl_password').isValid()
				)
			)
	if (prefix=='c_app')
		return	(	Ext.getCmp(prefix+'_ssl_off').getValue() ||
				(	
				Ext.getCmp(prefix+'_ssl_on').getValue() &&
				(
					(Ext.getCmp('c_app_certificate_flag').getValue() | Ext.getCmp(prefix+'_upload_certificate_form').isValid()) &
					(Ext.getCmp('c_app_private_key_flag').getValue() | Ext.getCmp(prefix+'_upload_private_key_form').isValid()) &
					Ext.getCmp(prefix+'_ssl_port').isValid() &
					Ext.getCmp(prefix+'_ssl_password').isValid()
				)
			))
}
/*
 * Function: validate_advanced_window
 * 
 * Invoked:
 * 	Privately from save_new_app,save_app_edition.
 * 	The function checks that all field in the advanced tab are valid
 * 
 * Parameters:
 * 	prefix - string,the prefix of the id of all fields that we want to upload
 * 		 if its the fields of the application wizard, invoke with "app_wizard", if its in edit mode, invoke with 'a_app'
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<save_new_app>
 * 	<save_app_edition>
 */
function validate_advanced_window(prefix){
	return (
		Ext.getCmp(prefix+'_advanced_injected_header_name').isValid() &
		Ext.getCmp(prefix+'_advanced_session_clear_name').isValid() &
		Ext.getCmp(prefix+'_advanced_session_clear_value').isValid()
	)
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////Web Applications Panel////////////////////////////////////////

/*
 * Function: save_app_edition
 * 
 * Invoked:
 * 	When the user clicks 'Save' on the application editor window on Configuration-->Web Application
 * 	The function validates all requirements, uploads the ceritifcate and private key if needed,
 * 	then it takes all the data from the authentication, General and Advanced tabs and saves it 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<upload_new_app>
 */
function save_app_edition(){

	var general_valid = validate_general_window('c_app')
	var authentication_valid = validate_authentication_window('c_app')
	var ssl_valid = validate_ssl_window('c_app')
	var advanced_valid = validate_advanced_window('c_app')
	if (!general_valid){
		Ext.getCmp('c_app_general_button').fireEvent('click');
	}
	else if(!authentication_valid){
		Ext.getCmp('c_app_authentication_button').fireEvent('click');
	}
	else if(!ssl_valid){
		Ext.getCmp('c_app_ssl_button').fireEvent('click');
	}
	else if(!advanced_valid){
		Ext.getCmp('c_app_advanced_button').fireEvent('click');
	}
	if (!general_valid || !authentication_valid || !ssl_valid || !advanced_valid){
		Ext.Msg.alert('Success', 'Please make sure the fields meet the requirements');
		return;
	}

	var app_id = Ext.getCmp('c_app_app_id_to_add').getValue();
	
	var needs_to_upload_certificate = ( Ext.getCmp('c_app_ssl_on').getValue() & Ext.getCmp('c_app_upload_certificate_form').getValue()!="" ) 
	var needs_to_upload_private_key = ( Ext.getCmp('c_app_ssl_on').getValue() & Ext.getCmp('c_app_upload_private_key_form').getValue()!="" ) 			

	if (needs_to_upload_certificate){
		certificate_finished=false;	
		certificate_started=true;
		var form = Ext.getCmp('c_app_upload_certificate')
		if(Ext.getCmp('c_app_upload_certificate_form').isValid()){
			form.submit({
				url: telepath.controllerPath + '/applications/set_ssl_certificate',
				waitMsg: 'Uploading certificate...',
				params:{
					app_id : app_id,
					waitMsg: 'Uploading certificate...',
					file_name: Ext.getCmp('c_app_upload_certificate_form').getValue()
				},
				success: function(fp, o) {
					if (o.result.success=='true'){
						certificate_finished = true
						certificate_file_name = o.result.file
						upload_new_app('c_app')
					}
					else
						Ext.Msg.alert('Error',o.result.error + '.');
				}
			});
		}
	}
					
	if (needs_to_upload_private_key){	
		private_key_finished=false;
		private_key_started=true;	
		//upload private key
		var private_key_form = Ext.getCmp('c_app_upload_private_key')
		if(Ext.getCmp('c_app_upload_private_key_form').isValid()){
			private_key_form.submit({
				url: telepath.controllerPath + '/applications/set_ssl_private_key',
				waitMsg: 'Uploading certificate...',
				params:{
					app_id : app_id,
					waitMsg: 'Uploading private key...',
					file_name: Ext.getCmp('c_app_upload_private_key_form').getValue()
				},
				success: function(fp, o) {
					if (o.result.success=='true'){
						private_key_file_name = o.result.file
						private_key_finished = true
						upload_new_app('c_app')
					}
					else
						Ext.Msg.alert('Error', o.result.error + '.');
				}
			});
		}
	}
	if (!needs_to_upload_private_key & !needs_to_upload_certificate)
		upload_new_app('c_app')


}

/*
 * Function: get_next_app_id
 * 
 * Invoked:
 * 	Every time the application wizard opens. 
 * 	The function gets the next app_id avaliable on the server and saves the value on Ext.getCmp('app_wizard_app_id_to_add')
 * 	
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function get_next_app_id(){
	
	telepath.util.request('/applications/get_next_id', { }, function(data) {
	
		Ext.getCmp('app_wizard_app_id_to_add').setValue(data['app_id']);
		
	}, 'Error while trying to get the next application id.');

}

/*
 * Function: edit_global_pages
 * 
 * Invoked:
 * 	Every time one clicks the edit icon on a the global pages table
 * 	The function will trigger the event plugin for that row
 * 	
 * 
 * 
 * Parameters:
 * 	list_id - string, the id of the table from which its store the record will be removed
 *  	value - string,the value as it appears on the row
 * 	plugin_id - string, the plugin id of the edit_plugin of that table
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function edit_global_pages(list_id,plugin_id,value){
	var store = Ext.getCmp(list_id).getStore()
	var record = store.find('page',value,0,false,false,true)
	if (record!=-1)
		Ext.getCmp(list_id).getPlugin(plugin_id).startEdit(record,0)	
}

/*
 * Function: load_data_per_app
 * 
 * Invoked:
 * 	Every time one clicks a row on the application table.
 * 	The function loads data from the server for that application and assigns it to the relevant fields.
 * 
 * 
 * Parameters:
 * 	record - Ext.record object from which the information of the application will be taken
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function load_data_per_app(record){

	reset_app_general_info();
	reset_app_authentication_info();
	reset_app_ssl_info();
	reset_app_advanced_info();
	set_readonly_app_general_info(true);
	set_readonly_app_authentication_info(true);
	set_readonly_app_ssl_info(true);
	set_readonly_app_advanced_info(true);
	
	//load data to general info section
	Ext.getCmp('c_app_app_id_to_add').setValue(record['td0']);
	Ext.getCmp('c_app_general_application_domain').setValue(record['td1']);
	Ext.getCmp('c_app_general_display_name').setValue(record['td2']);
	
	var store = Ext.getCmp('c_app_general_add_cookie_list').getStore();
	var cookies_array = record['td9'] ? record['td9'].split(',') : [];
	for(var i=0;i<cookies_array.length;i++){
		if (cookies_array[i]!="")
			store.add({
				cookie:cookies_array[i]
			})
	}
	store = Ext.getCmp('c_app_general_add_ip_list').getStore();
	var ips_array = record.app_ips;
	var ins_array = [];
	
	for(var i=0;i<ips_array.length;i++){
		if (ips_array[i]!="")
			/*store.add({
				ip:ips_array[i]
			})*/
			ins_array.push({ ip:ips_array[i] });
	}
	
	store.loadRawData(ins_array);
	
	//authentication section
	Ext.getCmp('c_app_authentication_form_cb').setValue(record['td25']==1);
	Ext.getCmp('c_app_authentication_form_parameter_id').setValue(record['td26']);
	Ext.getCmp('c_app_authentication_form_parameter').setValue(record['td27']);
	Ext.getCmp('c_app_authentication_basic_cb').setValue(record['td23']==1);
	Ext.getCmp('c_app_authentication_digest_cb').setValue(record['td24']==1);
	Ext.getCmp('c_app_authentication_ntlm_cb').setValue((record['td12']==1));
	// (record['td28']==1?Ext.getCmp('c_app_authentication_form_authentication_on').setValue(true):Ext.getCmp('c_app_authentication_form_authentication_off').setValue(true));
	
	Ext.getCmp('c_app_authentication_success_cookie_cb').setValue((record['td29']==1));	
	Ext.getCmp('c_app_authentication_success_cookie_name').setValue(record['td36']);
	Ext.getCmp('c_app_authentication_success_cookie_value').setValue(record['td37']);
	(record['td5']==0? Ext.getCmp('c_app_authentication_success_cookie_missing').setValue(true):Ext.getCmp('c_app_authentication_success_cookie_appears').setValue(true));
	Ext.getCmp('c_app_authentication_success_cookie_missing').setValue((record['td5']==0));
	Ext.getCmp('c_app_authentication_success_redirect_cb').setValue((record['td30']==1));	
	Ext.getCmp('c_app_authentication_success_redirect_page').setValue(record['td32']);
	Ext.getCmp('c_app_authentication_success_redirect_page_id').setValue(record['td31']);
	Ext.getCmp('c_app_authentication_success_redirect_response_1').setValue(record['td33'] ? record['td33'].split('-')[0] : '');
	Ext.getCmp('c_app_authentication_success_redirect_response_2').setValue(record['td33'] ? record['td33'].split('-')[1] : '');
	Ext.getCmp('c_app_authentication_success_body_value_cb').setValue((record['td34']==1));
	Ext.getCmp('c_app_authentication_success_body_value_value').setValue(record['td35']);
	
	
	
	Ext.getCmp('c_app_authentication_logout_disabled_cb').setValue(true);

	if(parseInt(record['td6']) > 0) {
		Ext.getCmp('c_app_authentication_logout_page_cb').setValue(true);
		Ext.getCmp('c_app_authentication_logout_page_id').setValue(record['td6']);
		Ext.getCmp('c_app_authentication_logout_page_name').setValue(record['logout_page_name']);
	}
	if(parseInt(record['td7']) > 0) {
		Ext.getCmp('c_app_authentication_logout_param_cb').setValue(true);
		Ext.getCmp('c_app_authentication_logout_param_id').setValue(record['td7']);
		Ext.getCmp('c_app_authentication_logout_param_name').setValue(record['logout_att_name']);
	}

	Ext.getCmp('c_app_authentication_logout_param_value').setValue(record['td8']);
	
	
	// Automatic (IF NOT FORM/BASIC/NTLM/DIGEST)
	Ext.getCmp('c_app_authentication_automatic_cb').setValue(record['td25']==0 && record['td23']==0 && record['td12']==0 && record['td24']==0);
	
	//ssl section
	//10-certificate
	//11-private key
	Ext.getCmp('c_app_ssl_on').setValue((record['td18']==1));
	Ext.getCmp('c_app_ssl_port').setValue(record['td19']);
	Ext.getCmp('c_app_ssl_password').setValue(record['td21']);
	Ext.getCmp('c_app_certificate_flag').setValue(record['td16']);
	Ext.getCmp('c_app_private_key_flag').setValue(record['td17']);

	//advanced page
	
	var store = Ext.getCmp('c_app_advanced_global_pages_list').getStore();
	store.removeAll();
	
	var arr = record['td15'] ? record['td15'].split(',') : '';
	for(i = 0; i < arr.length ; i++) {
		if (arr[i]!="")
			store.add({ page: arr[i] });
	}
	
	Ext.getCmp('c_app_advanced_injected_header_name').setValue(record['td22']);
	Ext.getCmp('c_app_advanced_session_clear_name').setValue(record['td10']);
	Ext.getCmp('c_app_advanced_session_clear_value').setValue(record['td11']);
	
	//finally, if there are cookie to suggest, load them to the store
	
	Ext.getCmp('c_app_general_cookie_name').getStore().load({
		params:{
			app_id:record['td0']
		},
		callback:function(response){
			if (!response)
				Ext.Msg.alert('Error #164', 'Error while trying to load cookie suggestion for application.');			
		}
	});
	
	Ext.getCmp('frameworkIcon').el.setHTML('');
	// Display framework icon if such was detected
	if(record['td41'] && record['td41'] != '') {
		Ext.getCmp('frameworkIcon').el.setHTML('<img src="' + telepath.basePath + '/images/frameworks/' + record['td41'].toLowerCase() + '.png" / alt="' + record['td41'] + '">');
	}
	
}

/////////////////app info ////////////////////////

/*
 * Function: reset_app_general_info
 * 
 * Invoked:
 * 	The function resets all the fields in the general info section for the app
 * 
 * 
 * Parameters:
 * 
 * Returns:
 * 	undefined	
 * 
 */
function reset_app_general_info() {

	Ext.getCmp('c_app_general_display_name').reset();
	Ext.getCmp('c_app_general_application_domain').reset();
	Ext.getCmp('c_app_general_cookie_name').reset();
	Ext.getCmp('c_app_general_add_cookie_list').getStore().removeAll();
	Ext.getCmp('c_app_general_add_ip_field').reset();
	Ext.getCmp('c_app_general_add_ip_list').getStore().removeAll();
	
}
/*
 * Function: set_readonly_app_general_info
 * 
 * Invoked:
 * 	The function sets all the fields in the general info section for the app to be readonly or not readonly
 * 
 * 
 * Parameters:
 * 	bool - true to set readonly, false to set editable
 * 
 * Returns:
 * 	undefined	
 * 
 */
function set_readonly_app_general_info(bool) {

	Ext.getCmp('c_app_general').getEl().dom.style.cssText+=(!bool?'opacity:1.0;':'opacity:0.6;');
	
	if(bool) {
		$(Ext.getCmp('c_app_general').body.dom).addClass('x-mask').removeClass('x-panel-body-default');
	} else {
		$(Ext.getCmp('c_app_general').body.dom).removeClass('x-mask').addClass('x-panel-body-default');
	}
	
	Ext.getCmp('c_app_app_id_to_add').setReadOnly(bool);
	Ext.getCmp('c_app_general_display_name').setReadOnly(bool);
	Ext.getCmp('c_app_general_application_domain').setReadOnly(bool);
	Ext.getCmp('c_app_general_cookie_name').setReadOnly(bool);
	Ext.getCmp('c_app_general_add_ip_field').setReadOnly(bool);
	
	if (bool) {
	
		Ext.getCmp('c_app_general_add_ip_button').disable();
		Ext.getCmp('c_app_general_add_cookie_button').disable();
		Ext.getCmp('c_app_general_add_cookie_list').disable();
		Ext.getCmp('c_app_general_add_ip_list').disable();
		
	} else {
	
		Ext.getCmp('c_app_general_add_ip_button').enable();
		Ext.getCmp('c_app_general_add_cookie_button').enable();
		Ext.getCmp('c_app_general_add_cookie_list').enable();
		Ext.getCmp('c_app_general_add_ip_list').enable();
		
	}
}
/*
 * Function: reset_app_authentication_info
 * 
 * Invoked:
 * 	The function resets all the fields in the authentication info section for the app
 * 
 * 
 * Parameters:
 * 
 * Returns:
 * 	undefined	
 * 
 */
function reset_app_authentication_info() {

	// Detection Modes
	Ext.getCmp('c_app_authentication_automatic_cb').reset();
	Ext.getCmp('c_app_authentication_form_cb').reset();
	Ext.getCmp('c_app_authentication_form_parameter').reset();
	Ext.getCmp('c_app_authentication_basic_cb').reset();
	Ext.getCmp('c_app_authentication_digest_cb').reset();
	Ext.getCmp('c_app_authentication_ntlm_cb').reset();
	
	// Ext.getCmp('c_app_authentication_form_authentication_on').reset();
	
	// Login Criteria
	Ext.getCmp('c_app_authentication_success_cookie_cb').reset();
	Ext.getCmp('c_app_authentication_success_cookie_name').reset();
	Ext.getCmp('c_app_authentication_success_cookie_value').reset();
	Ext.getCmp('c_app_authentication_success_cookie_missing').reset();
	Ext.getCmp('c_app_authentication_success_redirect_cb').reset();
	Ext.getCmp('c_app_authentication_success_redirect_page').reset();
	Ext.getCmp('c_app_authentication_success_redirect_response_1').reset();
	Ext.getCmp('c_app_authentication_success_redirect_response_2').reset();
	Ext.getCmp('c_app_authentication_success_body_value_cb').reset();
	Ext.getCmp('c_app_authentication_success_body_value_value').reset();
	
	
	// Logout Criteria
	Ext.getCmp('c_app_authentication_logout_page_cb').reset();
	Ext.getCmp('c_app_authentication_logout_page_name').reset();
	Ext.getCmp('c_app_authentication_logout_page_id').reset();
	Ext.getCmp('c_app_authentication_logout_param_cb').reset();
	Ext.getCmp('c_app_authentication_logout_disabled_cb').reset();
	Ext.getCmp('c_app_authentication_logout_param_name').reset();
	Ext.getCmp('c_app_authentication_logout_param_id').reset();
	Ext.getCmp('c_app_authentication_logout_param_value').reset();
		
}
/*
 * Function: set_readonly_app_authentication_info
 * 
 * Invoked:
 * 	The function sets all the fields in the authentication info section for the app to be readonly or not readonly
 * 
 * 
 * Parameters:
 * 	bool - true to set readonly, false to set editable
 * 
 * Returns:
 * 	undefined	
 * 
 */
function set_readonly_app_authentication_info(bool) {

	Ext.getCmp('c_app_authentication').getEl().dom.style.cssText+=(!bool?'opacity:1.0;':'opacity:0.6;');
	
	if(bool) {
		$(Ext.getCmp('c_app_authentication').body.dom).addClass('x-mask').removeClass('x-panel-body-default');
	} else {
		$(Ext.getCmp('c_app_authentication').body.dom).removeClass('x-mask').addClass('x-panel-body-default');
	}
	
	Ext.getCmp('c_app_authentication_automatic_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_form_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_form_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_form_parameter').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_basic_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_digest_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_ntlm_cb').setReadOnly(bool);
	
	// Ext.getCmp('c_app_authentication_form_authentication_on').setReadOnly(bool);
	// Ext.getCmp('c_app_authentication_form_authentication_off').setReadOnly(bool);
	
	// Login Criteria	
	Ext.getCmp('c_app_authentication_success_cookie_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_cookie_name').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_cookie_value').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_cookie_missing').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_cookie_appears').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_cookie_changed').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_redirect_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_redirect_page').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_redirect_response_1').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_redirect_response_2').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_body_value_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_success_body_value_value').setReadOnly(bool);	
	
	
	// Logout Criteria	
	Ext.getCmp('c_app_authentication_logout_disabled_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_logout_page_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_logout_page_name').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_logout_page_id').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_logout_param_cb').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_logout_param_name').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_logout_param_id').setReadOnly(bool);
	Ext.getCmp('c_app_authentication_logout_param_value').setReadOnly(bool);
	
}
	
/*
 * Function: reset_app_ssl_info
 * 
 * Invoked:
 * 	The function resets all the fields in the ssl info section for the app
 * 
 * 
 * Parameters:
 * 
 * Returns:
 * 	undefined	
 * 
 */
function reset_app_ssl_info(){
	Ext.getCmp('c_app_ssl_on').reset();
	Ext.getCmp('c_app_ssl_off').reset();
	Ext.getCmp('c_app_upload_certificate_form').reset();
	Ext.getCmp('c_app_upload_private_key_form').reset();
	Ext.getCmp('c_app_ssl_port').reset();
	Ext.getCmp('c_app_ssl_password').reset();
}
/*
 * Function: set_readonly_app_ssl_info
 * 
 * Invoked:
 * 	The function sets all the fields in the ssl info section for the app to be readonly or not readonly
 * 
 * 
 * Parameters:
 * 	bool - true to set readonly, false to set editable
 * 
 * Returns:
 * 	undefined	
 * 
 */
function set_readonly_app_ssl_info(bool){
	Ext.getCmp('c_app_ssl').getEl().dom.style.cssText+=(!bool?'opacity:1.0;':'opacity:0.6;');
	
	if(bool) {
		$(Ext.getCmp('c_app_ssl').body.dom).addClass('x-mask').removeClass('x-panel-body-default');
	} else {
		$(Ext.getCmp('c_app_ssl').body.dom).removeClass('x-mask').addClass('x-panel-body-default');
	}
	
	Ext.getCmp('c_app_ssl_on').setReadOnly(bool);
	Ext.getCmp('c_app_ssl_off').setReadOnly(bool);
	Ext.getCmp('c_app_upload_private_key_form').setReadOnly(bool);
	if (bool && Ext.getCmp('c_app_ssl_off').getValue()){
		Ext.getCmp('c_app_upload_private_key').disable();
		Ext.getCmp('c_app_upload_certificate').disable();
	}
	else if(!bool && Ext.getCmp('c_app_ssl_on').getValue()){
		Ext.getCmp('c_app_upload_private_key').enable();
		Ext.getCmp('c_app_upload_certificate').enable();
		Ext.getCmp('c_app_upload_private_key_form').clearInvalid();
		Ext.getCmp('c_app_upload_certificate_form').clearInvalid();
	}
	Ext.getCmp('c_app_upload_certificate_form').setReadOnly(bool);
	//Ext.getCmp('c_app_ssl_ip').setReadOnly(bool);
	Ext.getCmp('c_app_ssl_port').setReadOnly(bool);
	Ext.getCmp('c_app_ssl_password').setReadOnly(bool);

}
/*
 * Function: reset_app_advanced_info
 * 
 * Invoked:
 * 	The function resets all the fields in the advanced info section for the app
 * 
 * 
 * Parameters:
 * 
 * Returns:
 * 	undefined	
 * 
 */
function reset_app_advanced_info(){
	Ext.getCmp('c_app_advanced_global_pages_description').reset();
	Ext.getCmp('c_app_advanced_global_pages_regex').reset();
	Ext.getCmp('c_app_advanced_global_pages_list').getStore().removeAll();
	Ext.getCmp('c_app_advanced_injected_header_name').reset();
	Ext.getCmp('c_app_advanced_session_clear_name').reset();
	Ext.getCmp('c_app_advanced_session_clear_value').reset();	
}
/*
 * Function: set_readonly_app_advanced_info
 * 
 * Invoked:
 * 	The function sets all the fields in the advanced info section for the app to be readonly or not readonly
 * 
 * 
 * Parameters:
 * 	bool - true to set readonly, false to set editable
 * 
 * Returns:
 * 	undefined	
 * 
 */
function set_readonly_app_advanced_info(bool) {

	Ext.getCmp('c_app_advanced').getEl().dom.style.cssText+=(!bool?'opacity:1.0;':'opacity:0.6;');
	
	if(bool) {
		$(Ext.getCmp('c_app_advanced').body.dom).addClass('x-mask').removeClass('x-panel-body-default');
	} else {
		$(Ext.getCmp('c_app_advanced').body.dom).removeClass('x-mask').addClass('x-panel-body-default');
	}
	
	Ext.getCmp('c_app_advanced_global_pages_description').setReadOnly(bool);
	Ext.getCmp('c_app_advanced_global_pages_regex').setReadOnly(bool);
	Ext.getCmp('c_app_advanced_injected_header_name').setReadOnly(bool);
	Ext.getCmp('c_app_advanced_session_clear_name').setReadOnly(bool);
	Ext.getCmp('c_app_advanced_session_clear_value').setReadOnly(bool);
	if (bool){
		Ext.getCmp('c_app_advanced_global_pages_regex_add').disable()
		Ext.getCmp('c_app_advanced_global_pages_list').disable()
	} else {
		Ext.getCmp('c_app_advanced_global_pages_regex_add').enable()
		Ext.getCmp('c_app_advanced_global_pages_list').enable()
	}

}
