var sendTo;
var selection_state;

// TODO:: Reference directly
function update_parameter_alias(att_id,att_alias,store_to_refresh) {
	telepath.parameters.updateAlias(att_id,att_alias,store_to_refresh);
}

telepath.parameters = {
	
	mapping: {
		pages: {
			"Parameter_View_Page":     ['b_riAttributeField_P','b_riAttributeField_P_id','b_riAttributeField','b_riAttributeField_P_id','RW2_f5_1','RW2_f5_1_2'],
			"page_view":		       ['b_riPageField_pat','b_riPageField_id_pat','b_riDynamic_Pat','b_riDynamic_Pat_id',null,null],
			"c_app_redirect_page":     ['c_app_authentication_success_redirect_page','c_app_authentication_success_redirect_page_id',null,null,null,null],
			"Parameter_Page_RW":       ['RWpar_f1','RWpar_f1_id','RWpar_f1','RWpar_f1_id',"RW2_f5_1",'RW2_f5_1_2'],
			"Pattern_Dynamic_Page_RW": ['RWpat_f4_page','RWpat_f4_id_page','RWpat_f4','RWpat_f4_id',"RW2_f5_1",'RW2_f5_1_2'],
		},
		params: {
			"Investigate_Attribute": 			  ['i_filterAtt','i_filterAtt_id','i_filterPage','i_filterPage_id',null,'i_filterApps'],
			"rule_alert_parameters_rule_editor":  ['b_ri_alert_name','b_ri_alert_id',null,null,"b_riAppCB","b_riAppField"],
			"rule_alert_parameters_rules_wizard": ['rw_alert_name','rw_alert_id',null,null,"RW2_f5_1","RW2_f5_1_2"],
			"Parameter_View_Attribute": 		  ['b_riAttributeField_P','b_riAttributeField_P_id','b_riPageField_par','b_riPageField_id_par',"b_riAppCB","b_riAppField"],
			"Pattern_Dynamic_View": 			  ['b_riDynamic_Pat','b_riDynamic_Pat_id',null,null,"b_riAppCB","b_riAppField"],
			"Pattern_Other_View":   			  ['b_riOtherField_Pat','b_riOtherField_Pat_id'],
			"Application Authentication Form": 	  ['c_app_authentication_form_parameter','c_app_authentication_form_parameter_id',null,null,null,null],
			"Parameter_Attribute_RW": 			  ['RWpar_f1','RWpar_f1_id','RWpar_f1_page','RWpar_f1_id_page',"RW2_f5_1","RW2_f5_1_2"],
			"Pattern_Dynamic_RW":  				  ['RWpat_f4','RWpat_f4_id',null,null,"RW2_f5_1","RW2_f5_1_2"],
			"Pattern_Other_RW":    				  ['RWpat_f3_1','RWpat_f3_id']
		}
	},
	updateAlias: function(att_id,att_alias,store_to_refresh) {
		
		telepath.util.request('/parameters/set_parameter_alias', { 'att_id': att_id, 'att_alias': att_alias }, function(data) {
			
			if(store_to_refresh) { store_to_refresh.reload(); }
		
		}, 'Error setting parameter alias.');
		
	},
	show: function(app_limit) {
		
		if(!app_limit) {
			app_limit = telepath.parameters.findAppLimit();
		}
			
		telepath.parameters.apptreeWindow = Ext.create('MyApp.view.AppTreeWindow');
		telepath.parameters.apptreeWindow.setWidth(500);
		telepath.parameters.apptreeWindow.modal = true;
		telepath.parameters.apptree = telepath.parameters.apptreeWindow.items.get(0);
		
		try {
		
		switch(selection_state) {
			case 'page':
				page_id_ext = telepath.parameters.mapping.pages[sendTo][1];
				page_id_obj = Ext.getCmp(page_id_ext);
				page_id_val = page_id_obj.getValue();
				
				if(parseInt(page_id_val) > 0) {
				
					telepath.util.request('/applications/get_path', { 'type': 'page', 'id': page_id_val }, function(data) {
						telepath.parameters.apptreeCTL = new AppTreePanel('Parameters', telepath.parameters.apptree, app_limit, data.items);
						telepath.parameters.apptreeWindow.show();
					}, function () {});
					
				} else {
					telepath.parameters.apptreeCTL = new AppTreePanel('Parameters', telepath.parameters.apptree, app_limit, data.items);
					telepath.parameters.apptreeWindow.show();
				}
				
			break;
			case 'parameter':
				att_id_ext  = telepath.parameters.mapping.params[sendTo][1];
				att_id_obj  = Ext.getCmp(att_id_ext);
				att_id_val  = att_id_obj.getValue();
				
				// If its a global header, skip detection of path, resume normal flow
				for(x in globalHeaders.items) {
					if(att_id_val == globalHeaders.items[x].value) {
						att_id_val = 0;
					}
				}
				
				if(parseInt(att_id_val) > 0) {
				
					telepath.util.request('/applications/get_path', { 'type': 'param', 'id': att_id_val }, function(data) {
						telepath.parameters.apptreeCTL = new AppTreePanel('Parameters', telepath.parameters.apptree, app_limit, data.items);
						telepath.parameters.apptreeWindow.show();
					}, function () {});
				
				} else {
					telepath.parameters.apptreeCTL = new AppTreePanel('Parameters', telepath.parameters.apptree, app_limit, data.items);
					telepath.parameters.apptreeWindow.show();
				}
				
			break;
		}
		
		} catch(e) {
			telepath.parameters.apptreeCTL = new AppTreePanel('Parameters', telepath.parameters.apptree, app_limit);
			telepath.parameters.apptreeWindow.show();
		}

	},
	findAppLimit: function() {
	
		var app_cb_id , app_field_id;
		
		if (sendTo=="Parameter_Page_RW" || 
			sendTo=="Pattern_Dynamic_RW" || 
			sendTo=="Parameter_Attribute_RW" || 
			sendTo=="Pattern_Other_RW" ||
			sendTo=="rule_alert_parameters_rules_wizard"
		) {
			app_cb_id = "RW2_f5_1"
			app_field_id = "RW2_f5_1_2"
		}
		else if(sendTo=="Parameter_View_Page" || 
			sendTo=="Parameter_View_Attribute" || 
			sendTo=="Pattern_Other_View" || 
			sendTo=="Pattern_Dynamic_View" ||
			sendTo=="rule_alert_parameters_rule_editor" ||
			sendTo=="Add_Parameter_to_Rule_Group"
		){
			app_cb_id = "b_riAppCB"
			app_field_id = "b_riAppField"		
		}
		
		if(	sendTo=="Parameter_Page_RW" || 
			sendTo=="Pattern_Dynamic_RW" || 
			sendTo=="Parameter_Attribute_RW" || 
			sendTo=="Pattern_Other_RW" ||
			sendTo=="rule_alert_parameters_rules_wizard" ||
			sendTo=="Parameter_View_Page" || 
			sendTo=="Parameter_View_Attribute" || 
			sendTo=="Pattern_Other_View" || 
			sendTo=="Pattern_Dynamic_View" ||
			sendTo=="rule_alert_parameters_rule_editor" ||
			sendTo=="Add_Parameter_to_Rule_Group"
		) {
			if(Ext.getCmp(app_field_id).getValue()!=null && Ext.getCmp(app_cb_id).getValue()) {
				var app_id = Ext.getCmp(app_field_id).getStore().findRecord("app",Ext.getCmp(app_field_id).getValue()).get("id");
				return app_id;
			}
		}

		if(sendTo=="Investigate_Attribute") {
			
			var app_id = $('#i_searchPanel .tele-multi input:first').attr('dataID');
			
			if(app_id == '-1') {
				return false; 
			} else {
				return app_id; 
			}
			
		}
		
		if(sendTo=="Application Authentication Form" || sendTo=="c_app_redirect_page" || sendTo=="c_app_logout_page" || sendTo=="c_app_logout_param") {
			var app_id = Ext.getCmp('c_app_app_id_to_add').getValue();
			return app_id;
		}
	
	},
	populate_param_fields: function (att_field, 
									 att_id_field, 
								     page_field, 
									 page_id_field, 
								     app_checkbox, 
									 app_field, 
									 data) {
		
		// Parameter Fields
		Ext.getCmp(att_field).setValue(data.att);
		Ext.getCmp(att_id_field).setValue(data.att_id);

		// Page Fields
		if (page_field != null) {
			Ext.getCmp(page_field).setValue(data.page);
		}
		if (page_id_field != null) {
			Ext.getCmp(page_id_field).setValue(data.page_id);  
		}
		
		// Application Fields
		if(app_field == 'i_filterApps') {
		
			$('#i_searchPanel .tele-multi').remove();
			$('#i_searchPanel-innerCt').append($('<div>').teleSelect({ constrain: 4, type: 'subdomain', label: 'Application', values: [{ text: data.app_name, id: data.app_id, sub_id: '-1', root: true }] }));
		
		} else {
		
		// Application Fields
		if (data.app_id != 0 && app_field != null) {                   
		
			Ext.getCmp(app_field).getStore().each(function(record) {
				if (record.get("id")==data.app_id) {
				
				if (app_checkbox!=null) {
					Ext.getCmp(app_checkbox).setValue(true);
				}
					app_name = record.get("app");
					Ext.getCmp(app_field).setValue(app_name);	        		
				}
			});    
			
		}
		
		}
		
	},
	populate_page_fields: function (page_text_field,
									page_id_field,
									par_text_field,
									par_id_field,
									app_checkbox,
									app_field,
									data){
		
		// Populate Page Fields
    	Ext.getCmp(page_text_field).setValue(data.page);
		Ext.getCmp(page_id_field).setValue(data.page_id);
		
		// Clear Parameters Fields
		if (par_text_field)
				Ext.getCmp(par_text_field).reset();
		if (par_id_field)
				Ext.getCmp(par_id_field).reset();

		// Application Fields
		
		if(app_field == 'i_filterApps') {
		
			
		
		} else {
		
		if (data.app_id != 0 && app_field != null) {                   
		
			Ext.getCmp(app_field).getStore().each(function(record) {
				if (record.get("id")==data.app_id) {
				
					if (app_checkbox!=null) {
						Ext.getCmp(app_checkbox).setValue(true);
					}

					Ext.getCmp(app_field).setValue(record.get("app"));	        		
				}
			});    
			
		}
		
		}
		
	},
	hide: function() {
		telepath.parameters.apptreeWindow.destroy();
	},
	handlerSelectPage: function(record) {
		
		// COLLECT DATA
		var collected_data = {
			page:    record.raw.text,
			page_id: record.raw.id,
			app_id:  get_parent_app_id(record),
			app_name: get_parent_app_name(record),
		};

		switch(sendTo) {
			
			case "c_app_logout_page":
				telepath.parameters.populate_page_fields('c_app_authentication_logout_page_name','c_app_authentication_logout_page_id','c_app_authentication_logout_param_name','c_app_authentication_logout_param_id',null,null, collected_data);
			break;
			
			case "Parameter_View_Page":
				telepath.parameters.populate_page_fields('b_riAttributeField_P','b_riAttributeField_P_id','b_riAttributeField','b_riAttributeField_P_id','RW2_f5_1','RW2_f5_1_2', collected_data);
			break;
			
			case "page_view":
				telepath.parameters.populate_page_fields('b_riPageField_pat','b_riPageField_id_pat','b_riDynamic_Pat','b_riDynamic_Pat_id',null,null,collected_data);
			break;
			
			case "c_app_redirect_page":
				telepath.parameters.populate_page_fields('c_app_authentication_success_redirect_page','c_app_authentication_success_redirect_page_id',null,null,null,null,collected_data);
			break;
			
			case "Parameter_Page_RW":
				telepath.parameters.populate_page_fields('RWpar_f1','RWpar_f1_id','RWpar_f1','RWpar_f1_id',"RW2_f5_1",'RW2_f5_1_2',collected_data);
			break;
			
			case "Pattern_Dynamic_Page_RW":
				telepath.parameters.populate_page_fields('RWpat_f4_page','RWpat_f4_id_page','RWpat_f4','RWpat_f4_id',"RW2_f5_1",'RW2_f5_1_2',collected_data);
			break;
			
		}
		
		telepath.parameters.hide();
	
	},
	handlerSelectParam: function(record) {
		
		var collected_data = {};
		
		if(record.data.type == 'global-param') {
			
			collected_data = {
				att:     record.data.text,
				att_id:  record.data.id.split('_')[2],
				app_id:  0
			};
			
		} else {
			
			collected_data = {
				att:     record.raw.att_name,
				att_id:  record.raw.att_id,
				page:    record.parentNode.raw.text,
				page_id: record.parentNode.raw.id,
				app_id:  get_parent_app_id(record),
				app_name: get_parent_app_name(record),
			};
			
		}

		switch(sendTo) {
			
			case "c_app_logout_param":
				telepath.parameters.populate_param_fields('c_app_authentication_logout_param_name','c_app_authentication_logout_param_id','c_app_authentication_logout_page_name','c_app_authentication_logout_page_id',null,null, collected_data);
			break;
			
			case "Investigate_Attribute":
				telepath.parameters.populate_param_fields('i_filterAtt','i_filterAtt_id','i_filterPage','i_filterPage_id',null,'i_filterApps', collected_data);
			break;
			
			case "rule_alert_parameters_rule_editor":
				telepath.parameters.populate_param_fields('b_ri_alert_name','b_ri_alert_id',null,null,"b_riAppCB","b_riAppField", collected_data);
			break;
			
			case "rule_alert_parameters_rules_wizard":
				telepath.parameters.populate_param_fields('rw_alert_name','rw_alert_id',null,null,"RW2_f5_1","RW2_f5_1_2", collected_data);
			break;
				
			case "Parameter_View_Attribute":
				telepath.parameters.populate_param_fields('b_riAttributeField_P','b_riAttributeField_P_id','b_riPageField_par','b_riPageField_id_par',"b_riAppCB","b_riAppField", collected_data);
			break;
			
			case "Pattern_Dynamic_View":
				telepath.parameters.populate_param_fields('b_riDynamic_Pat','b_riDynamic_Pat_id',null,null,"b_riAppCB","b_riAppField", collected_data);
			break;
			
			case "Pattern_Other_View":
				Ext.getCmp('b_riOtherField_Pat').setValue(collected_data.att);
				Ext.getCmp('b_riOtherField_Pat_id').setValue(collected_data.att_id);
			break;
			
			case "Application Authentication Form":
				telepath.parameters.populate_param_fields('c_app_authentication_form_parameter','c_app_authentication_form_parameter_id',null,null,null,null,collected_data);
			break;
			
			case "Parameter_Attribute_RW":
				telepath.parameters.populate_param_fields('RWpar_f1','RWpar_f1_id','RWpar_f1_page','RWpar_f1_id_page',"RW2_f5_1","RW2_f5_1_2",collected_data);
			break;
			
			case "Pattern_Dynamic_RW": 
				telepath.parameters.populate_param_fields('RWpat_f4','RWpat_f4_id',null,null,"RW2_f5_1","RW2_f5_1_2",collected_data);
			break;
			
			case "Pattern_Other_RW":
				Ext.getCmp('RWpat_f3_1').setValue(collected_data.att);
				Ext.getCmp('RWpat_f3_id').setValue(collected_data.att_id);   
			break;


		}
		
		telepath.parameters.hide();
	
	}

}