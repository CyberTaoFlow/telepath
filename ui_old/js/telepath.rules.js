var lastSelectedNode;
var group_node,rule_node;
var window_failure;
var group_success;
var rule_success;
var node_to_display_info;
var add_rule_to_group=false;
var param_record_to_override;
var loaded_params=false;

telepath.rules = {
	
	mouseEnter: function(dataview, record, item, index, e, eOpts) {
	
	},
	mouseLeave: function(dataview, record, item, index, e, eOpts) {
		
	},
	addIPsButtonClick: function(where) {
	
		switch(where) {
			
			case 'editor':
				
				telepath.ipconfig.show(Ext.getCmp('b_riIpField').value, 'Add IP to rule limit', function (ip) {
					Ext.getCmp('b_riIpField').setValue(ip);
				});
				
			break;
			
			case 'wizard':
				
				telepath.ipconfig.show(Ext.getCmp('RW2_f5_2_2').value, 'Add IP to rule limit', function (ip) {
					Ext.getCmp('RW2_f5_2_2').setValue(ip);
				});
			
			break;
		
		}
		
	},
	changeRulePatternCB: function (field, newValue, oldValue, eOpts) {
		
		// Rule Editor
		function RI_pat_page_disable() {
			Ext.getCmp('b_ri_page_radio').setValue(false);
			Ext.getCmp('b_riPageField_pat').disable();
			Ext.getCmp('b_riPageField_browse_pat').disable();
			Ext.getCmp('b_riPageField_remove_pat').disable();
			Ext.getCmp('b_riPageField_pat').reset();
			Ext.getCmp('b_riPageField_id_pat').reset();
		}
		function RI_pat_param_disable() {
			Ext.getCmp('b_ri_parameter_radio').setValue(false);
			Ext.getCmp('b_riDynamic_Pat').disable();
			Ext.getCmp('b_riDynamicBrowser_Pat').disable();
			Ext.getCmp('b_riDynamicRemove_Pat').disable();
			Ext.getCmp('b_riDynamic_Pat').reset();
			Ext.getCmp('b_riDynamic_Pat_id').reset();
		}
		function RI_pat_flow_disable() {
			Ext.getCmp('b_ri_flow_radio').setValue(false);
			Ext.getCmp('b_ri_flow_select').disable();
			Ext.getCmp('b_ri_flow_select').reset();
		}
		function RI_pat_page_enable() {
			Ext.getCmp('b_riPageField_pat').enable();
			Ext.getCmp('b_riPageField_browse_pat').enable();
			Ext.getCmp('b_riPageField_remove_pat').enable();
		}
		function RI_pat_param_enable() {
			Ext.getCmp('b_riDynamic_Pat').enable();
			Ext.getCmp('b_riDynamicBrowser_Pat').enable();
			Ext.getCmp('b_riDynamicRemove_Pat').enable();
		}
		function RI_pat_flow_enable() {
			Ext.getCmp('b_ri_flow_select').enable();
		}
		function RI_pat_user_enable() {
			Ext.getCmp('b_ri_user_radio').setValue(true);
		}
		function RI_pat_user_disable() {
			Ext.getCmp('b_ri_user_radio').setValue(false);
		}
		
		// Rule Wizard
		function RW_pat_page_disable() {
			Ext.getCmp('RWpat_page_radio').setValue(false);
			Ext.getCmp('RWpat_f4_page').disable();
			Ext.getCmp('RWpat_browse_page').disable();
			Ext.getCmp('RWpat_remove_page').disable();
			Ext.getCmp('RWpat_f4_page').reset();
			Ext.getCmp('RWpat_f4_id_page').reset();
		}
		function RW_pat_param_disable() {
			Ext.getCmp('RWpat_parameter_radio').setValue(false);
			Ext.getCmp('RWpat_f4').disable();
			Ext.getCmp('RWpat_remove_pararm').disable();
			Ext.getCmp('RWpat_browse_pararm').disable();
			Ext.getCmp('RWpat_f4').reset();
			Ext.getCmp('RWpat_f4_id').reset();
		}
		function RW_pat_flow_disable() {
			Ext.getCmp('RWpat_flow_radio').setValue(false);
			Ext.getCmp('RWpat_flow_select').disable();
			Ext.getCmp('RWpat_flow_select').reset();
		}
		function RW_pat_user_disable() {
			Ext.getCmp('RWpat_user_radio').setValue(false);
		}
		function RW_pat_user_enable() {
			Ext.getCmp('RWpat_user_radio').setValue(true);
		}
		function RW_pat_page_enable() {
			Ext.getCmp('RWpat_f4_page').enable();
			Ext.getCmp('RWpat_browse_page').enable();
			Ext.getCmp('RWpat_remove_page').enable();
			Ext.getCmp('RWpat_f4').reset();
			Ext.getCmp('RWpat_f4_id').reset();
		}
		function RW_pat_param_enable() {
			Ext.getCmp('RWpat_f4').enable();
			Ext.getCmp('RWpat_remove_pararm').enable();
			Ext.getCmp('RWpat_browse_pararm').enable();
		}
		function RW_pat_flow_enable() {
			Ext.getCmp('RWpat_flow_select').enable();
		}
		
		var type = field.id.split('_')[0];
		var id   = false;
		
		switch(field.id) {
			
				case 'RWpat_f1':
					
					if (newValue){
						Ext.getCmp('RWpat_f2').setValue(false);
						Ext.getCmp('RWpat_f3').setValue(false);
						Ext.getCmp('RWpat_f10').setValue(false);
						
						Ext.getCmp('RWpat_page_radio').enable();
						Ext.getCmp('RWpat_parameter_radio').enable();
						Ext.getCmp('RWpat_flow_radio').enable();
						Ext.getCmp('RWpat_user_radio').enable();
						
					}
					return;
				
				break;
				
				case 'RWpat_f2':
				
					if (newValue){
						Ext.getCmp('RWpat_f1').setValue(false);
						Ext.getCmp('RWpat_f3').setValue(false);
						Ext.getCmp('RWpat_f10').setValue(false);
						
						Ext.getCmp('RWpat_page_radio').enable();
						Ext.getCmp('RWpat_parameter_radio').enable();
						Ext.getCmp('RWpat_flow_radio').enable();
						Ext.getCmp('RWpat_user_radio').enable();
						
					}
					return;
					
				break;
				
				case 'RWpat_f10':
					
					
					if (newValue){
						Ext.getCmp('RWpat_f1').setValue(false);
						Ext.getCmp('RWpat_f2').setValue(false);
						Ext.getCmp('RWpat_f3').setValue(false);
						
						Ext.getCmp('RWpat_page_radio').disable();
						Ext.getCmp('RWpat_parameter_radio').enable().setValue(true);
						Ext.getCmp('RWpat_flow_radio').disable();
						Ext.getCmp('RWpat_user_radio').disable();
						
					}
					return;
				
				break;
				
				case 'RWpat_f3':
					
					if (newValue){
						Ext.getCmp('RWpat_f2').setValue(false);
						Ext.getCmp('RWpat_f1').setValue(false);
						Ext.getCmp('RWpat_f10').setValue(false);
						Ext.getCmp('RWpat_f3_1').enable();
						Ext.getCmp('RWpat_f3_2').enable();
						
						Ext.getCmp('RWpat_page_radio').disable();
						Ext.getCmp('RWpat_parameter_radio').enable().setValue(true);
						Ext.getCmp('RWpat_flow_radio').disable();
						Ext.getCmp('RWpat_user_radio').disable();
						
					}
					else{
						Ext.getCmp('RWpat_f3_1').disable();
						Ext.getCmp('RWpat_f3_2').disable();
					}
					return;
				
				break;
			
				case 'b_riIp_Pat':
					
					
					if (newValue){
					
						Ext.getCmp('b_riSid_Pat').setValue(false);
						Ext.getCmp('b_riOther_Pat').setValue(false);
						Ext.getCmp('b_riOtherField_Pat').setValue(null);
						Ext.getCmp('b_riUser_Pat').setValue(false);
						
						Ext.getCmp('b_ri_page_radio').enable();
						Ext.getCmp('b_ri_user_radio').enable();
						Ext.getCmp('b_ri_flow_radio').enable();
						Ext.getCmp('b_ri_parameter_radio').enable();
												
					}
					
					return;
				
				break;
				
				case 'b_riSid_Pat':
					
					if (newValue){
						Ext.getCmp('b_riIp_Pat').setValue(false);
						Ext.getCmp('b_riOther_Pat').setValue(false);
						Ext.getCmp('b_riOtherField_Pat').setValue(null);
						Ext.getCmp('b_riUser_Pat').setValue(false);
						
						Ext.getCmp('b_ri_page_radio').enable();
						Ext.getCmp('b_ri_user_radio').enable();
						Ext.getCmp('b_ri_flow_radio').enable();
						Ext.getCmp('b_ri_parameter_radio').enable();
						
					}
					
					return;
					
				break;
				
				case 'b_riUser_Pat':
					
					if (newValue){
						Ext.getCmp('b_riSid_Pat').setValue(false);
						Ext.getCmp('b_riOther_Pat').setValue(false);
						Ext.getCmp('b_riOtherField_Pat').setValue(null);
						Ext.getCmp('b_riIp_Pat').setValue(false);
						
						Ext.getCmp('b_ri_page_radio').setValue(false).disable();
						Ext.getCmp('b_ri_user_radio').setValue(false).disable();
						Ext.getCmp('b_ri_flow_radio').setValue(false).disable();
						Ext.getCmp('b_ri_parameter_radio').setValue(true).enable();
						
						
					}
					
					return;
				
				break;
				
				case 'b_riOther_Pat':
					
					if (newValue){
						Ext.getCmp('b_riSid_Pat').setValue(false);
						Ext.getCmp('b_riIp_Pat').setValue(false);
						Ext.getCmp('b_riOtherField_Pat').enable();
						Ext.getCmp('b_riOtherBrowser_Pat').enable();
						Ext.getCmp('b_riUser_Pat').setValue(false);
						
						Ext.getCmp('b_ri_page_radio').setValue(false).disable();
						Ext.getCmp('b_ri_user_radio').setValue(false).disable();
						Ext.getCmp('b_ri_flow_radio').setValue(false).disable();
						Ext.getCmp('b_ri_parameter_radio').setValue(true).enable();
												
					}
					else{
						Ext.getCmp('b_riOtherBrowser_Pat').disable();
						Ext.getCmp('b_riOtherField_Pat').disable();
					}
					
					return;
				
				break;
				
		}
		
		if(type == 'RWpat') {
			type = 'wizard';
			id = field.id.split('_')[1];
		} else {
			type = 'editor';
			id = field.id.split('_')[2];
		}

		if(newValue) {
			switch(id) {
				case 'page':
				
					if(type == 'wizard') {
						RW_pat_flow_disable();
						RW_pat_param_disable();
						RW_pat_user_disable();
						RW_pat_page_enable();
						if(Ext.getCmp('RWpat_f3').getValue() || Ext.getCmp('RWpat_f10').getValue()) {
							Ext.getCmp('RWpat_f1').setValue(true);
						}
						
					}
					if(type == 'editor') {
						RI_pat_flow_disable();
						RI_pat_param_disable();
						RI_pat_user_disable();
						RI_pat_page_enable();
						if(Ext.getCmp('b_riOtherField_Pat').getValue() || Ext.getCmp('b_riUser_Pat').getValue()) {
							Ext.getCmp('b_riSid_Pat').setValue(true);
						}
					}
					
					
				break;
				
				case 'parameter':
				
					if(type == 'wizard') {
						RW_pat_flow_disable();
						RW_pat_page_disable();
						RW_pat_user_disable();
						RW_pat_param_enable();
					} 
					if(type == 'editor') {
						RI_pat_flow_disable();
						RI_pat_page_disable();
						RI_pat_user_disable();
						RI_pat_param_enable();
					}
					
				break;
				
				case 'flow':
				
					if(type == 'wizard') {
						RW_pat_page_disable();
						RW_pat_param_disable();
						RW_pat_user_disable();
						RW_pat_flow_enable();
						if(Ext.getCmp('RWpat_f3').getValue() || Ext.getCmp('RWpat_f10').getValue()) {
							Ext.getCmp('RWpat_f1').setValue(true);
						}
					}
					if(type == 'editor') {
						RI_pat_page_disable();
						RI_pat_param_disable();
						RI_pat_user_disable();
						RI_pat_flow_enable();
						if(Ext.getCmp('b_riOtherField_Pat').getValue() || Ext.getCmp('b_riUser_Pat').getValue()) {
							Ext.getCmp('b_riSid_Pat').setValue(true);
						}
					}
					
				break;
				
				case 'user':
				
					if(type == 'wizard') {
						RW_pat_page_disable();
						RW_pat_param_disable();
						RW_pat_flow_disable();
						RW_pat_user_enable();
						if(Ext.getCmp('RWpat_f3').getValue() || Ext.getCmp('RWpat_f10').getValue()) {
							Ext.getCmp('RWpat_f1').setValue(true);
						}
					}
					if(type == 'editor') {
						RI_pat_page_disable();
						RI_pat_param_disable();
						RI_pat_flow_disable();
						RI_pat_user_enable();
						if(Ext.getCmp('b_riOtherField_Pat').getValue() || Ext.getCmp('b_riUser_Pat').getValue()) {
							Ext.getCmp('b_riSid_Pat').setValue(true);
						}
					}
					
				break;
				
				
			}
		}
		
	},
	init: function() {
		
		telepath.rules.fetchData();
		
		if(!telepath.access.has_perm('Rules', 'add')) {
			Ext.getCmp("b_addRule").hide();
		}
		if(!telepath.access.has_perm('Rules', 'del')) {
			Ext.getCmp("b_delete").hide();
		}
		if(!telepath.access.has_perm('Rules', 'set')) {
			Ext.getCmp("b_saveRule").hide();
			Ext.getCmp("b_saveGroup").hide();
		}
		

	},
	fetchData: function () {
		
		telepath.util.request('/rules', { mode:"get_rule_groups_categories"	}, function(data) {
			
			telepath.rules.initTree(data);
			Ext.getStore('general_rule_group_categories_store').reload();
			Ext.getStore('general_rules_store').reload();
		
		}, 'Error loading rule categories.');
	
	},
	initTree: function(jsondata) {
		
		var root = { expanded: true, children: [ ] };    
		var category, node;
		
		for(var i=0;i<jsondata.items.length;i++) {
			category = {
						text:jsondata.items[i]['category'],
						category_id:jsondata.items[i]['id'],
						expanded:false,
						opened:false,
						type:'category',
						children: [{text:""}]	
					};
			root.children.push(category);
		}
		
		Ext.getCmp('b_ruleGroups').getStore().setRootNode(root);
		Ext.getCmp('b_startPanel').show();
		Ext.getCmp('b_groupInfoPanel').hide();
		Ext.getCmp('b_ruleInfoPanel').hide();
		
		//every time we re-load the tree,if we want to select the node 
		//the expansion of the tree is necessary,
		
		if (node_to_display_info!=undefined) {
			//we need to take the category id and expand the node that corespondes to that
			var node_to_expand = Ext.getCmp('b_ruleGroups').getStore().getRootNode().findChild('category_id',node_to_display_info.category_id)
			if (node_to_expand){
				select_node_from_tree(node_to_expand)
			}
				
		}
	
	},
	openParameters: function (sendTarget) {
		
		sendTo = sendTarget;
		
		// Page Related
		
		if(sendTo == "Parameter_View_Page") {
			selection_state = "page";
		}
		if(sendTo == "page_view") {
			selection_state = "page";
		}
		if(sendTo == "Parameter_View_Attribute") {
			selection_state = "parameter";
		}
		if(sendTo == "Pattern_Dynamic_View") {
			selection_state = "parameter";
		}
		if(sendTo == "Pattern_Other_View") {
			selection_state = "parameter";
		}
		
		// Wizard Related
		
		if(sendTo == "Parameter_Attribute_RW") {
			selection_state = "parameter";
		}
		if(sendTo == "Parameter_Page_RW") {
			selection_state = "page";
		}
		if(sendTo == "Pattern_Dynamic_RW") {
			selection_state = "parameter";
		}
		if(sendTo == "Pattern_Dynamic_Page_RW") {
			selection_state = "page";
		}
		if(sendTo == "Pattern_Other_RW") {
			selection_state = "parameter";
		}
		
		telepath.parameters.show(false);
		
	},
	onWizardShow: function (component) {
	
		Ext.getCmp('main_panel').disable();
		
		var pos = component.getPosition()
		if (pos[1]<20){
			component.setPosition(pos[0],40)
			//bug fix
			Ext.getCmp('b_addRule').showMenu()
			Ext.getCmp('b_addRule').hideMenu()
		}
		
		var category_id;
		var group_name;
		var selected_groups = Ext.getCmp("b_ruleGroups").getSelectionModel().getSelection();
		
		if (selected_groups.length==1 && selected_groups[0].data.type=='rule') {
			selected_groups = [ selected_groups[0].parentNode ];
		}
				
		if (selected_groups.length==1 && selected_groups[0].data.type=='category'){
			category_id = selected_groups[0].data.category_id;
			category_record = Ext.getCmp('RW1_exists_category_name').getStore().findRecord('id',category_id);
			category_name = category_record.get('category');
			Ext.getCmp('RW1_exists_category_name').setValue(category_name);
			Ext.getCmp('RW1_exists_category_cb').setValue(true);  
		}

		else if (selected_groups.length==1 && selected_groups[0].data.type=='group'){
			//get category and mark as selected
			category_id = selected_groups[0].data.category_id;
			category_record = Ext.getCmp('RW1_exists_category_name').getStore().findRecord('id',category_id);
			category_name = category_record.get('category');
			Ext.getCmp('RW1_exists_category_name').setValue(category_name);
			Ext.getCmp('RW1_exists_category_cb').setValue(true);
			
			group_name = selected_groups[0].data.text;
			Ext.getCmp('RW1_f13').setValue(group_name);
			Ext.getCmp('RW1_f12').setValue(true);

			//move to the next window
			if (add_rule_to_group) {
				add_rule_to_group = false
				Ext.getCmp('RulesWizard_category_next').fireEvent('click')
			}
		}

	
	},
	selectRuleOrGroup: function(selModel,record,index,option) {
	
	/*
	 * Function: selectRuleOrGroup
	 * 
	 * Invoked:
	 * 	On click event of the rules tree. Loads all the information to the rules editor
	 *
	 * Parameters:
	 * 	selModel - the group id to load
	 * 	record - the record of the selected node in the rules tree
	 * 	index - the index of the node
	 * 	option - object
	 * 
	 * Returns:
	 * 	undefined
	 * 
	 */
	 
	//loaded_params = {};
	//build_rule_params(loaded_params);
	//loaded_params = {};
	//loaded_params = buildRulesGroupInfoString();
	delete(loaded_params.mode);
	var modified = false;
	var nodeType = '';
	// Check if something was modified
	if(loaded_params.ruleid && loaded_params.groupId) {
		nodeType = 'rule';
		// Rule
		new_params = {};
		build_rule_params(new_params);
		
		if(JSON.stringify(new_params) != JSON.stringify(loaded_params)) {
			modified = true;
			// Changed
		}
		
	} else if(loaded_params.groupid) {
		nodeType = 'group';
		// Group
		new_params = {};
		new_params = buildRulesGroupInfoString();
		
		if(JSON.stringify(new_params) != JSON.stringify(loaded_params)) {
			// Changed
			modified = true;
		}
		
	} else {
		// Nothing
	}
	
	var maybeSelectedNode = record;
	
	if(modified) {
		if(!isNaN(new_params.ruleid) || !isNaN(new_params.groupid)) {
			Ext.MessageBox.show({
				 title:'Save Changes?',
				 msg: 'You are closing a tab that has unsaved changes. Would you like to save your changes?',
				 buttons: Ext.Msg.YESNOCANCEL,
				 icon: Ext.Msg.QUESTION,
				 fn: function(btn){                    
					if (btn == "no") {
						new_params = {};
						loaded_params = {};
						select_node_from_tree(maybeSelectedNode);
						//selectRuleOrGroup(false, lastSelectedNode, false, false);
					}
					if (btn == "yes") {
						
						loaded_params = new_params;
						new_params = {};
						loaded_params = {};
						// YES
						lastSelectedNode = record;
						
						if(nodeType == 'group') {updateRulesGroupValues(new_params); }
						if(nodeType == 'rule') { updateRuleValues(new_params); }
						
						select_node_from_tree(lastSelectedNode);
						
					}
			   }                
			});
			return false;
		}
		
	}
	
	lastSelectedNode = record;
	
	changeAbility(false,"rule");
	changeAbility(false,"group");
	
	var selected  = record;
	Ext.getCmp("b_delete").enable();
	Ext.getCmp('b_riBrowse').disable();
	
	if (selected.data.type!='category') {
		Ext.getCmp('b_startPanel').hide();
	} else {
		Ext.getCmp('b_startPanel').show();
	}
		
	////CASE OF RULE
	if (selected.data.type=='rule') {
	
		nodeClicked = record; //save node to get the application for the parameter view 
		if(record.parentNode.data.group_id!==Ext.getCmp('b_lastSelected1').getValue()){
			getRulesListForRulesTable(record.parentNode.data.group_id);
			Ext.getCmp('b_lastSelected1').setValue(record.parentNode.data.group_id);
		}

		Ext.getCmp('b_groupInfoPanel').hide();
		Ext.getCmp('b_ruleInfoPanel').show();
		Ext.getCmp('b_riRuleType').disable(); // Don't allow ruletype change
		
		telepath.util.request('/rules', { mode:"get_rule_information", rule_id : record.data.rule_id }, function(data) {
			
			// Reload Rule Country Stores (NTT Issue 2-2)
			var countryStores = ['b_geoInsideAll', 'b_geoOutsideAll', 'rw_geoInsideAll', 'rw_geoOutsideAll', 'b_geoInsideChosen', 'b_geoOutsideChosen'];
			var countryArray = [];
			$.each(aliasToName, function(alias, name) { countryArray.push({ Alias: alias, Country: name });	});
			$.each(countryStores, function (i, v) {
				var c_store = Ext.StoreManager.getByKey(v);
				c_store.setProxy = { 'type' : 'memory' };
				c_store.loadRawData(countryArray);
			});
			
			Ext.getCmp('b_ruleId').setValue(record.data.rule_id);
			Ext.getCmp('b_riName').setValue(data.name);
			Ext.getCmp('b_riDesc').setValue(data.description);
			Ext.getCmp('b_riOwner').setValue(data.owner);
			Ext.getCmp('b_riStartIndex').setValue(parseInt(data.seq_index)+1);
			Ext.getCmp('b_riAppearance').setValue(data.appearance);
			
			if(data.enable_rule=="1")
				Ext.getCmp('b_riRuleEnabled').setValue(true);
			else
				Ext.getCmp('b_riRuleEnabled').setValue(false);
				
			var store  = Ext.getCmp('b_riAppField').getStore();
			Ext.getCmp('b_riAppField').setValue(null);
			if(data.app_id != ""){
				Ext.getCmp('b_riAppCB').setValue(true);
					store.each(function(record){
							if (record.get('id')==data.app_id){
								Ext.getCmp('b_riAppField').setValue(record.get('app'));
							}
					});   
				}
				else{
					Ext.getCmp('b_riAppCB').setValue(false); 
				}
				
				Ext.getCmp('b_riIpNot').setValue(parseInt(data.ip_neg));
				
				if(data.user_ip!=""){
					Ext.getCmp('b_riIpCB').setValue(true); 
					Ext.getCmp('b_riIpField').setValue(data.user_ip);
				}
				else{
					Ext.getCmp('b_riIpCB').setValue(false);
					Ext.getCmp('b_riIpField').setValue(null);
				}
				if(data.user!=""){
					Ext.getCmp('b_riUserCB').setValue(true); 
					Ext.getCmp('b_riUserField').setValue(data.user);
				}
				else{
					Ext.getCmp('b_riUserCB').setValue(false);
					Ext.getCmp('b_riUserField').setValue(null);
				}
				changeAbility(false,"rule");
				if (data.ruletype== "Aspect"){
					
				Ext.getCmp('b_aspectRuleCont').reset();
				Ext.getCmp('b_parameterRuleCont').reset();
				Ext.getCmp('b_patternRuleCont').reset();
				Ext.getCmp('b_geoRuleCont').reset();
				Ext.getCmp('b_botRuleCont').reset();
				Ext.getCmp('b_aspectRuleCont').show();
				Ext.getCmp('b_riRuleType').setValue(Ext.getStore('b_riRuleTypeStore').getAt(1));
				if (data.aspect.toLowerCase()=='flow')
					Ext.getCmp('b_riAspectType_A').setValue('Navigation');
				else if (data.aspect.toLowerCase()=='landing')
					Ext.getCmp('b_riAspectType_A').setValue('Speed');
				else if (data.aspect.toLowerCase()=='query')
					Ext.getCmp('b_riAspectType_A').setValue('Query');
				else if (data.aspect.toLowerCase()=='day')
					Ext.getCmp('b_riAspectType_A').setValue('Day');
				else if (data.aspect.toLowerCase()=='hour')
					Ext.getCmp('b_riAspectType_A').setValue('Hour');
				else if (data.aspect.toLowerCase()=='geo')
					Ext.getCmp('b_riAspectType_A').setValue('Location');
				else if (data.aspect.toLowerCase()=='entry')
					Ext.getCmp('b_riAspectType_A').setValue('Page Context');
				else if (data.aspect.toLowerCase()=='week_day')
					Ext.getCmp('b_riAspectType_A').setValue('Date');
				else if (data.aspect.toLowerCase()=='average')
					Ext.getCmp('b_riAspectType_A').setValue('Average');
				if(data.personal=="1"){
					Ext.getCmp('b_riPersonal_A').setValue(true);
				}
				else{
						Ext.getCmp('b_riPersonal_A').setValue(false);
				}
				}

				else if(data.ruletype=== "Pattern"){
					
				Ext.getCmp('b_aspectRuleCont').reset();
				Ext.getCmp('b_parameterRuleCont').reset();
				Ext.getCmp('b_patternRuleCont').reset();
				Ext.getCmp('b_geoRuleCont').reset();
				Ext.getCmp('b_botRuleCont').reset();
				Ext.getCmp('b_patternRuleCont').show();
				Ext.getCmp('b_riRuleType').setValue(Ext.getStore('b_riRuleTypeStore').getAt(2));

					if(data.anchor_type==="IP"){
					Ext.getCmp('b_riIp_Pat').setValue(true);
					}

				else if(data.anchor_type==="SID"){
					Ext.getCmp('b_riSid_Pat').setValue(true);
				}

				else if(data.anchor_type==="User"){
						Ext.getCmp('b_riUser_Pat').setValue(true);
				}

				else{
						Ext.getCmp('b_riOther_Pat').setValue(true);
						Ext.getCmp('b_riOtherField_Pat').setValue(data.p_anchor_displayname);
						Ext.getCmp('b_riOtherField_Pat_id').setValue(data.anchor);
				}
				if (data.page_id!=""){
					Ext.getCmp('b_ri_page_radio').setValue(true);
					Ext.getCmp('b_riPageField_id_pat').setValue(data.page_id);
					if (data.page_id==0)
						Ext.getCmp('b_riPageField_pat').setValue("/");					
					else
						Ext.getCmp('b_riPageField_pat').setValue(data.p_page_displayname);
				}
				if (data.att_id!=""){///meaning its a number
					Ext.getCmp('b_ri_parameter_radio').setValue(true);
					Ext.getCmp('b_riDynamic_Pat_id').setValue(data.att_id);				
					Ext.getCmp('b_riDynamic_Pat').setValue(data.p_dynamic_displayname);
				}
				if(data.business_id!="") {
					Ext.getCmp('b_ri_flow_radio').setValue(true);
					
					var store  = Ext.getCmp('b_ri_flow_select').getStore();
			
					store.each(function(record){
							if (record.get('id')==data.business_id){
								Ext.getCmp('b_ri_flow_select').setValue(record.get('group'));
							}
					});
					
				}
				if(data.dynamic_type != '') {
					Ext.getCmp('b_ri_user_radio').setValue(true);
				}
						
				Ext.getCmp('b_riCount_Pat').setValue(data.count);
				if(data.timetype==="min"){
						Ext.getCmp('b_riTime1_Pat').setValue(parseInt(data.time/60));
						Ext.getCmp('b_riTime2_Pat').setValue("Minutes");
				}
				else if(data.timetype==="hours"){
						Ext.getCmp('b_riTime1_Pat').setValue(parseInt(data.time/3600));
					Ext.getCmp('b_riTime2_Pat').setValue("Hours");
				}
				else {
						Ext.getCmp('b_riTime1_Pat').setValue(parseInt(data.time));
						Ext.getCmp('b_riTime2_Pat').setValue("Seconds");
				}
					
				}
				else if(data.ruletype === "ParameterRule") {
				Ext.getCmp('b_aspectRuleCont').reset();
				Ext.getCmp('b_parameterRuleCont').reset();
				Ext.getCmp('b_patternRuleCont').reset();
				Ext.getCmp('b_geoRuleCont').reset();
				Ext.getCmp('b_botRuleCont').reset();
				Ext.getCmp('b_parameterRuleCont').show();
				Ext.getCmp('b_riRuleType').setValue(Ext.getStore('b_riRuleTypeStore').getAt(0));
				
				$('#b_riAttributeField_P-inputEl').on('keydown', function () { 
					Ext.getCmp('b_riAttributeField_P_id').setValue('');
					Ext.getCmp('b_riPageField_id_par').setValue('');
					Ext.getCmp('b_riPageField_par').setValue('');
				});
				
				//check if its a POST/GET/HEADER rule
				if (data.aspect) {
					
					if(data.aspect == 'Title') {
						
						Ext.getCmp('b_par_radio_3').setValue(true);
						
					} else if(data.aspect == 'Uri') {
						
						Ext.getCmp('b_par_radio_4').setValue(true);
						
					} else {
					
						data.aspect = data.aspect.split(',');
						Ext.getCmp('b_par_radio_2').setValue(true);
						
						$.each(data.aspect, function (i, aspect) {
							
							switch(aspect) {
								case 'GET':
								case 'G':
									Ext.getCmp('b_par_get_cb').setValue(true);
								break;
								case 'POST':
								case 'P':
									Ext.getCmp('b_par_post_cb').setValue(true);
								break;
								case 'HEADER':
								case 'H':
									Ext.getCmp('b_par_header_cb').setValue(true);
								break;
								// For backward compat
								case 'BOTH':
									Ext.getCmp('b_par_get_cb').setValue(true);
									Ext.getCmp('b_par_post_cb').setValue(true);
								break;
							}
						
						});
					
					}
										
				} else {
					Ext.getCmp('b_par_radio_1').setValue(true);
				}
				
				if (data.pr_attribute_displayname!=""){
					Ext.getCmp('b_riAttributeField_P').setValue(data.pr_attribute_displayname);
					Ext.getCmp('b_riAttributeField_P_id').setValue(data.att_id);
				}
				else{
					Ext.getCmp('b_riAttributeField_P').setValue(null);
					Ext.getCmp('b_riAttributeField_P_id').setValue(null);
				}					
				if (data.page_id==0)
					Ext.getCmp('b_riPageField_par').setValue("/");
				else
					Ext.getCmp('b_riPageField_par').setValue(data.p_page_displayname);
				Ext.getCmp('b_riPageField_id_par').setValue(data.page_id);
				if(data.pr_attrType==="stringmatch"){
						Ext.getCmp('b_riStringCB_P').setValue(true);
						Ext.getCmp('b_riStringField_P').setValue(data.str_match);
						Ext.getCmp('b_riStringNotCB_P').setValue(data.not_signal);
				}
				else if(data.pr_attrType==="gex"){
					Ext.getCmp('b_riRegexCB_P').setValue(true);
					Ext.getCmp('b_riRegexField_P').setValue(data.str_match);
					Ext.getCmp('b_riRegexNotCB_P').setValue(data.not_signal);
				}
				else if(data.pr_attrType === "fuzzylength"){
					Ext.getCmp('b_riFuzzyCB_P').setValue(true);
					Ext.getCmp('b_riFuzzyField_P').setValue(data.str_length==="short"?"Is too Short":(data.str_length==="long"?"Is too Long":(data.str_length==="both"?"Either Short/Long":console.log("Error: Fuzzy Length Value"))));
				}
				else if(data.pr_attrType === "exactlength") {
					Ext.getCmp('b_riCharLengthCB_P').setValue(true); 
					Ext.getCmp('b_riCharLengthField_P').setValue(data.str_length); 
				}
				else if(data.pr_attrType === "rangelength") {
					Ext.getCmp('b_riLengthCB_P').setValue(true);
					var splitedlengthvalues = data.str_length.split("-");
					Ext.getCmp('b_riLengthField1_P').setValue(splitedlengthvalues[0]);
					Ext.getCmp('b_riLengthField2_P').setValue(splitedlengthvalues[1]);
				}
				else if(data.pr_attrType === "distance_similarity"){
					Ext.getCmp('b_riSimilarCB_P').setValue(true);
					Ext.getCmp('b_riSimilarField_P').setValue(data.str_similarity);
				}
				else
					Ext.getCmp('b_riHeuristicCB_P').setValue(true);

				}
				else if(data.ruletype=== "Geo"){
				Ext.getCmp('b_aspectRuleCont').reset();
				Ext.getCmp('b_parameterRuleCont').reset();
				Ext.getCmp('b_patternRuleCont').reset();
				Ext.getCmp('b_geoRuleCont').reset();
				Ext.getCmp('b_botRuleCont').reset();
				Ext.getCmp('b_geoRuleCont').show();
				Ext.getCmp('b_riRuleType').setValue(Ext.getStore('b_riRuleTypeStore').getAt(3));
				if (data.radius!="" && data.time!=""){
					Ext.getCmp("b_riVelocity1_geo").setValue(true);
					Ext.getCmp("b_riVelocity2_geo").setValue(data.time);
					Ext.getCmp("b_riVelocity3_geo").setValue(data.radius);
					Ext.getCmp("b_riAllowCountry1_geo").setValue(false);
					Ext.getCmp("b_riBlockCountry1_geo").setValue(false);
				}
				else if(data.not_signal==1){
					Ext.getCmp("b_riAllowCountry1_geo").setValue(true);
					Ext.getCmp("b_riBlockCont").disable();
					Ext.getCmp("b_riAllowCont").enable();
					var countriesArray = data.str_match.split(",");
					var countriesStoreChosen = Ext.getCmp("b_riAllowChosenList2_geo").getStore();
					var countriesStoreAll = Ext.getCmp("b_riAllowAllList2_geo").getStore()
					for(var i=0;i<countriesArray.length;i++){
						if (!countriesArray[i]) 
							continue;
						countriesStoreChosen.add({Country:aliasToName[countriesArray[i]],Alias:countriesArray[i]});
						countriesStoreAll.remove(countriesStoreAll.findRecord('Country',aliasToName[countriesArray[i]]));
					}

				}
				else if(data.not_signal==0){
					Ext.getCmp("b_riBlockCountry1_geo").setValue(true);
					Ext.getCmp("b_riAllowCont").disable();
					Ext.getCmp("b_riBlockCont").enable();
					var countriesArray = data.str_match.split(",");
					var countriesStore = Ext.getCmp("b_riBlockChosenList2_geo").getStore();
					var countriesStoreAll = Ext.getCmp("b_riBlockAllList2_geo").getStore();
					for(var i=0;i<countriesArray.length;i++){
						if (!countriesArray[i]) 
							continue;							
						var country = aliasToName[countriesArray[i]]
						countriesStore.add({'Country':aliasToName[countriesArray[i]],'Alias':countriesArray[i]});
						countriesStoreAll.remove(countriesStoreAll.findRecord('Country',aliasToName[countriesArray[i]]));
					}
				}

				}
			else if(data.ruletype=== "Bot"){
				Ext.getCmp('b_aspectRuleCont').reset();
				Ext.getCmp('b_parameterRuleCont').reset();
				Ext.getCmp('b_patternRuleCont').reset();
				Ext.getCmp('b_geoRuleCont').reset();
				Ext.getCmp('b_botRuleCont').reset();					
				Ext.getCmp('b_botRuleCont').show();
				Ext.getCmp('b_riRuleType').setValue("Bot-Intelligence");
				Ext.getCmp('b_riBotType_bot').setValue(data.str_match);
			} else {
				// Reset and hide all panels
				Ext.getCmp('b_riRuleType').setValue("Built In");
				Ext.getCmp('b_riRuleType').disable();
				Ext.getCmp('b_aspectRuleCont').reset();
				Ext.getCmp('b_parameterRuleCont').reset();
				Ext.getCmp('b_patternRuleCont').reset();
				Ext.getCmp('b_geoRuleCont').reset();
				Ext.getCmp('b_botRuleCont').reset();
				Ext.getCmp('b_aspectRuleCont').hide();
				Ext.getCmp('b_parameterRuleCont').hide();
				Ext.getCmp('b_patternRuleCont').hide();
				Ext.getCmp('b_geoRuleCont').hide();
				Ext.getCmp('b_botRuleCont').hide();
			}

			if(data.numeric_score!= ""){
				Ext.getCmp('b_riScoreType1').setValue("Numeric");
				Ext.getCmp('b_riScoreType2').setValue(data.numeric_score);
			}
				else{
				Ext.getCmp('b_riScoreType1').setValue("Literal");
				Ext.getCmp('b_riScoreType3').setValue(	(data.literal_score=="med"? "Medium":
									(data.literal_score=="low"? "Low":
									(data.literal_score=="high"? "High":
									(console.log("Error: invalid value for literal score type : "+data.literal_score))))));
				}


			Ext.getCmp('b_ruleInfoPanel').setTitle(record.parentNode.parentNode.data.text+" -> "+record.parentNode.data.text+" -> "+record.data.text);           
			
			loaded_params = {};
			build_rule_params(loaded_params);
						
		}, 'Error loading criteria information.');
			
	}
	//CASE OF GROUP
	else if (selected.data.type=='group') {

		record.expand();
		Ext.getCmp('b_ruleInfoPanel').hide();
		Ext.getCmp('b_groupInfoPanel').show();
		Ext.getCmp('b_lastSelected1').setValue(record.data.group_id);
		
		telepath.util.request('/rules', { mode:"get_group_information", rulesgroup_id:record.data.group_id }, function(data) {
				
			Ext.getCmp('b_groupId').setValue(record.data.group_id);
			Ext.getCmp('b_rgiName').setValue(data.name);
			Ext.getCmp('b_rgiDescription').setValue(data.desc);
			Ext.getCmp('b_rgiLogCB').setValue(data.action_log? true :false);
			Ext.getCmp('b_rgiEmailCB').setValue(data.action_email==1? true :false);
			var emails = data.action_email_field;
			var emails_arr = data.action_email_field.split(',');
			var emails_for_store = [];
			for (var i=0;i<emails_arr.length;i++){
				if (!emails_arr[i]) continue;
				emails_for_store.push({address:emails_arr[i]});
			}
			Ext.getCmp('b_rgiEmailField').getStore().loadData(emails_for_store);
			Ext.getCmp('b_rgiSyslogCB').setValue(data.action_syslog==1? true :false);
				var ans;
			if(data.businessflow_id != '-1'){
				Ext.getCmp('b_rgiFlowCB').setValue(true);
				var store=Ext.getCmp('b_rgiFlowField').getStore();
				store.each(function(record){
						if(parseInt(record.get('id'))==data.businessflow_id){
						Ext.getCmp('b_rgiFlowField').setValue(record.get('group'));
						ans = true
						return ans;
						}
				});
			if (!ans)
				Ext.getCmp('b_rgiFlowCB').setValue(false);						
			}
			else{
				Ext.getCmp('b_rgiFlowCB').setValue(false);
				Ext.getCmp('b_rgiFlowField').reset();
			}
			//load parameters to the group
			Ext.getCmp('b_ri_alert_params').getStore().loadData(data.alert_param_ids);

			Ext.getCmp('b_groupInfoPanel').setTitle(record.parentNode.data.text+" -> "+record.data.text);         
			getRulesListForRulesTable(record.data.group_id);
			
			loaded_params = {};
			loaded_params = buildRulesGroupInfoString();
			
		}, 'Error loading rule information.');
		
	}
	
	}
	
};



/*
 * Function: open_parameters_window_for_alerts
 * 
 * Invoked:
 * 	Whenever the browse/edit button is clicked on the group edition page/rule wizard.The function opens the parameters window
 *
 * Parameters:
 * 	table_id - the id of the parameters table for the group, to which the parameter chosen will be send
 * 
 * Returns:
 * 	undefined
 * 
 */
function open_parameters_window_for_alerts(table_id,id){
	var store=Ext.getCmp(table_id).getStore()
	if (id)
		param_record_to_override = store.findRecord('td1',id,0,false,false,true)
	if (table_id=="rw_alert_params")
		sendTo="rule_alert_parameters_rules_wizard";
	else	
		sendTo="rule_alert_parameters_rule_editor";
		
	selection_state = "parameter";

	telepath.parameters.show(false);
	
}

/*
 * Function: getRulesListForRulesTable
 * 
 * Loads the rules list for the selected group on the tree
 *
 * Parameters:
 * 	group_ip - the group id to load
 * 
 * Returns:
 * 	undefined
 * 
 */
function getRulesListForRulesTable(group_id){
	/*Ext.getCmp('b_RuleList').getStore().load({
		params:{
			group_id:group_id
		}
	});*/	
}

/*
 * Function: b_itemExpand
 * 
 * Invoked:
 * 	On expand event for the tree.
 *
 * Parameters:
 * 	treepanel - the tree panel of the rules
 * 	options - empty object
 * 
 * Returns:
 * 	undefined
 * 
 */
function b_itemExpand(treepanel,options){

	if (treepanel.data.type=='category')
		expand_category_node(treepanel)
	else if (treepanel.data.type=='group')
		expand_group_node(treepanel)
}

/*
 * Function: expand_category_node
 * 
 * Invoked:
 * 	On expand event a category node. Loads all the groups of that category
 *
 * Parameters:
 * 	treepanel - the tree panel of the rules
 * 	options - empty object
 * 
 * Returns:
 * 	undefined
 * 
 */
function expand_category_node(treepanel) {
	
	var scrollTop = Ext.getCmp('b_ruleGroups').getView().getEl().getScrollTop();

	treepanel.removeAll();
	
	telepath.util.request('/rules', { mode:"expand_category", category_id:treepanel.data.category_id }, function(data) {

		var toAdd     = data.items;
		var nodes_num = toAdd.length;
		
		Ext.suspendLayouts();
		
		for(var i=0;i<nodes_num;i++) {			
			node = Ext.data.NodeInterface.create({});
			treepanel.createNode(node);
			node.text = toAdd[i]['td1'];
			node.group_id = toAdd[i]['td0'];
			node.category_id = toAdd[i]['td2'];
			node.type = 'group'
			node.leaf=false;
			node.expandable=true;
			node.icon="Htmls/images/group.png";
			node.children=[{text:""}];
			node.qtip = toAdd[i]['hover'];
			new_node = treepanel.appendChild(node);
			
		}
		
		Ext.getCmp('b_ruleGroups').getView().refresh();
		
		Ext.resumeLayouts(true);
		
		var group_node;
		if (node_to_display_info!=undefined && (node_to_display_info.type=='group' || node_to_display_info.type=='rule' )) {
			group_node = treepanel.findChild('group_id',node_to_display_info.group_id);
			select_node_from_tree(group_node)
			if (node_to_display_info.type=='group')//no need to expand the group node
				node_to_display_info = undefined;			
		}
		
		Ext.getCmp('b_ruleGroups').scrollByDeltaY(scrollTop);
		
	}, 'Error expanding rules category.');

}


/*
 * Function: expand_group_node
 * 
 * Invoked:
 * 	On expand event a group node. Loads all the rules of that category
 *
 * Parameters:
 * 	treepanel - the tree panel of the rules
 * 	options - empty object
 * 
 * Returns:
 * 	undefined
 * 
 */
function expand_group_node(treepanel) {

	var scrollTop = Ext.getCmp('b_ruleGroups').getView().getEl().getScrollTop();

	treepanel.removeAll();
	
	telepath.util.request('/rules', { mode:"expand_group", group_id:treepanel.data.group_id }, function(data) {
		
		var toAdd     = data.items;
		var nodes_num = toAdd.length;
		
		for(var i=0;i<nodes_num;i++) {			
			node = Ext.data.NodeInterface.create({});
			treepanel.createNode(node);
			node.text = toAdd[i]['td1'];
			node.rule_id = toAdd[i]['td0'];
			node.group_id = toAdd[i]['td3'];
			node.category_id = toAdd[i]['td4'];
			node.type = 'rule'
			node.leaf=true;
			node.expandable=false;
			node.icon="Htmls/images/rule.png";
			treepanel.appendChild(node);
		}
		
		Ext.getCmp('b_ruleGroups').getView().refresh();
		
		var rule_node;
		if (node_to_display_info!=undefined && node_to_display_info.type=='rule' ){
			rule_node = treepanel.findChild('rule_id',node_to_display_info.rule_id);
			select_node_from_tree(rule_node)
			node_to_display_info = undefined;			
		}
		
		Ext.getCmp('b_ruleGroups').scrollByDeltaY(scrollTop);
	
	}, 'Error expanding rule.');
		
}

/*
 * Function: getCountries
 * 
 * Invoked:
 * 	Before submitting a geo rule from the rules wizard. The function makes a string of an array of all the chosen countries
 *
 * Parameters:
 * 	type - the countries table name to take the countries from
 * 
 * Returns:
 * 	undefined
 * 
 */
function getCountries(type){
	if (type=='Allow'){
		var chosenArr = [];
		Ext.getCmp("RWgeo_f2_2_selected").getStore().each(function(record){
			chosenArr.push(nameToAlias[record.get('Country')]);
		});
		return chosenArr.toString();
	}
	else if(type=='Block'){
		var chosenArr = [];
		Ext.getCmp("RWgeo_f3_2_selected").getStore().each(function(record){
			chosenArr.push(nameToAlias[record.get('Country')]);
		});
		return chosenArr.toString();
	}
}

/*
 * Function: updateRulesGroupValues
 * 
 * Invoked:
 * 	Whenever the save button for the group editor was clicked.
 * 	
 *
 * Parameters:
 * 	
 * 
 * Returns:
 * 	undefined
 * 
 */
function updateRulesGroupValues(new_params) {
	
	loaded_params = buildRulesGroupInfoString();
	
	if(new_params) {
		params = new_params;
	} else {
		params = buildRulesGroupInfoString();
	}
	
	if (!params) { return; }
		
	params['mode'] = "update_group";

	telepath.util.request('/rules', params, function(data) {
		
		if (group_success==undefined) {
					group_success = Ext.create('MyApp.view.groupEdited_success');
				}
		group_success.show();
		setTimeout(function(){group_success.hide();},1500);
		changeAbility(false,"group");   
		Ext.getCmp('b_ruleGroups').enable();
		telepath.rules.fetchData();	
	
	}, function(data) {
	
		if(data && data.success === false) {
			
			if (data.exists) {
				Ext.getCmp('b_rgiName').markInvalid("Rule already exists. Please select a different name");
			}
			
			window_failure=Ext.getCmp('ruleEdited_failure');
			
			if (window_failure==undefined) {
				window_failure = Ext.create('MyApp.view.ruleEdited_failure');
			}
			window_failure.show();
			setTimeout(function(){window_failure.destroy();},1500);
		
		} else {
			
			Ext.MessageBox.alert('Update Group', 'Error updating rule.');
		
		}
	
	});
	
}



/*
 * Function: buildRulesGroupInfoString
 * 
 * Invoked:
 * 	To create the headers object for the server, with all the new group information.
 * 	
 *
 * Parameters:
 * 	
 * 
 * Returns:
 * 	The object with all the fields, or false if the validation failed
 * 
 */
function buildRulesGroupInfoString()
{
	var error_string = ""
	var valid_fields = (	Ext.getCmp('b_rgiName').isValid() &
				Ext.getCmp('b_rgiDescription').isValid() &				
				!(Ext.getCmp('b_rgiFlowCB').getValue() && !Ext.getCmp('b_rgiFlowField').isValid()) &
				!(Ext.getCmp('b_rgiEmailCB').getValue() && !Ext.getCmp('b_rgiEmailField').getStore().getCount())
			   )
	if (!valid_fields){
		if (Ext.getCmp('b_rgiEmailCB').getValue() && !Ext.getCmp('b_rgiEmailField').getStore().getCount()){
			error_string+= " and insert at least one valid email address"	
		}
		Ext.Msg.alert('Info', 'Please fill out all required fields'+error_string+'.');
		return false;		
	}
		
	var params = {};
	var node = lastSelectedNode;
	node_to_display_info = {
					type:'group',
					//rule_id : parseInt(node.data.rule_id),
					group_id : parseInt(node.data.group_id),
					category_id : parseInt(node.data.category_id)
			 	}; 
	params.groupid = parseInt(node.data.group_id);
	params.groupname = Ext.getCmp('b_rgiName').getValue();
	params.groupdesc = Ext.getCmp('b_rgiDescription').getValue();
	params.action_email = (Ext.getCmp('b_rgiEmailCB').getValue()?1:0);
	var emails = "";
	Ext.getCmp('b_rgiEmailField').getStore().each(function(record){
		emails+=record.get('address')+","
	})
	emails = emails.substring(0,emails.length-1);
	params.action_email_address = emails;
	params.action_syslog = (Ext.getCmp('b_rgiSyslogCB').getValue()?1:0);
	params.action_log = (Ext.getCmp('b_rgiLogCB').getValue()?1:0);
	params.group_flowcheckbox = (Ext.getCmp('b_rgiFlowCB').getValue()?1:0);
	if (params.group_flowcheckbox){
		params.group_flowCombo	
	}
	if (Ext.getCmp('b_rgiFlowField').getValue())
		params.group_flowCombo = Ext.getCmp('b_rgiFlowField').valueModels[0].raw.id

	var param_ids = ""
	Ext.getCmp('b_ri_alert_params').getStore().each(function(record){
		param_ids+=record.get('td1')+","
	})
	param_ids = param_ids.substring(0,param_ids.length-1);
	params.general_param_ids = param_ids

	return params; 
}



/*
 * Function: editRuleOrGroup
 * 
 * Invoked:
 * 	Whenever an edit button was clicked, from the right click menu or the tree panel.
 * 	
 *
 * Parameters:
 * 	
 * 
 * Returns:
 * 	
 * 
 */

function editRuleOrGroup(){
	
	var selections = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection();
	
	if (selections.length==0){
		Ext.Msg.alert('Info', 'Please select a category, group or rule to edit.');
		return;
	}
	node = selections[0];
	if (node.data.type=='rule'){
		Ext.getCmp('b_ruleGroups').disable();
		changeAbility(true,"rule");
	}
	else if(node.data.type=='group') {
		Ext.getCmp('b_ruleGroups').disable();
		changeAbility(true,"group");
	}
	else if(node.data.type=='category'){
		open_edit_category_window()	
	}
}


/*
 * Function: cancel_changes
 * 
 * Invoked:
 * 	Whenever a cancel button was clicked for the rule edition.
 * 	We select the lastSelectedNode and reload everything from the server
 * 	
 *
 * Parameters:
 * 	
 * 
 * Returns:
 * 	
 * 
 */
function cancel_changes(){
	select_node_from_tree(lastSelectedNode);
}

/*
 * Function: cancel_group_edit
 * 
 * Invoked:
 * 	Whenever a cancel button was clicked for the group edition.
 * 	We select the lastSelectedNode and reload everything from the server
 * 	
 *
 * Parameters:
 * 	
 * 
 * Returns:
 * 	
 * 
 */
function cancel_group_edit(){
	changeAbility(false,"group");
	Ext.getCmp('b_ruleGroups').enable();
	cancel_changes();
}

/*
 * Function: select_node_from_tree
 * 
 * Invoked:
 * 	Whenever we want to trigger the select event for a node and load its data from the server.
 * 	
 *
 * Parameters:
 * 	node - the node object to select
 * 
 * Returns:
 * 	
 * 
 */
function select_node_from_tree(node){
	if (node != undefined){
		Ext.getCmp('b_ruleGroups').getSelectionModel().deselect(node);
		Ext.getCmp('b_ruleGroups').getSelectionModel().select(node);
	}	
}

/*
 * Function: updateRuleValues
 * 
 * Invoked:
 * 	When the save button was clicked for the rule editor.
 * 	
 *
 * Parameters:
 * 	
 * 
 * Returns:
 * 	
 * 
 */
function updateRuleValues(new_params)
{		
	var params = {};
	
	if(new_params) {
		params = new_params;
	} else {
		if (!build_rule_params(params))	return;
	}

	loaded_params = params;
	
	params['mode'] = "update_rule";
	Ext.getCmp('b_ruleGroups').enable();
	
	telepath.util.request('/rules', params, function(data) {
			
		rule_success = Ext.create('MyApp.view.ruleEdited_success');
		rule_success.show();
		setTimeout(function(){rule_success.hide();},1500);
		changeAbility(false,"rule");   
		telepath.rules.fetchData();
		
	}, function(data) {
	
		if(data && data.success === false) {
			
			window_failure = Ext.create('MyApp.view.ruleEdited_failure');
			window_failure.show();
			setTimeout(function(){window_failure.hide();},1500);
			changeAbility(false,"rule");   
			telepath.rules.fetchData();
			
		} else {
			
			Ext.MessageBox.alert('Update Rule', 'Error updating rule.');
		
		}
	
	});
	
}



/*
 * Function: build_rule_params
 * 
 * Invoked:
 * 	From updateRuleValues() above 
 * 	
 *
 * Parameters:
 * 	params - the object to which all the new fields and values will be pushed
 * 
 * Returns:
 * 	False if the validation failed, true otherwise.
 * 
 */
function build_rule_params(params)
{
	var node = lastSelectedNode;
	params.groupId = parseInt(node.data.group_id);	
	params.ruleid = parseInt(node.data.rule_id);
	node_to_display_info = {
					type:'rule',
					rule_id : parseInt(node.data.rule_id),
					group_id : parseInt(node.data.group_id),
					category_id : parseInt(node.data.category_id)
			 	}; 
	/////////////////////////validate general rule information fields ////////////////////
	var requirements_fulfilled_general = (	Ext.getCmp('b_riName').isValid() &
						Ext.getCmp('b_riDesc').isValid() &
						Ext.getCmp('b_riOwner').isValid() &
						Ext.getCmp('b_riAppearance').isValid() &
						Ext.getCmp('b_riRuleType').isValid() &
				 		!(Ext.getCmp('b_riAppCB').getValue() & !Ext.getCmp('b_riAppField').isValid()) &
						!(Ext.getCmp('b_riIpCB').getValue() & !Ext.getCmp('b_riIpField').isValid()) &
						!(Ext.getCmp('b_riUserCB').getValue() & !Ext.getCmp('b_riUserField').isValid()) &
						Ext.getCmp('b_riScoreType1').isValid() &
						!(Ext.getCmp('b_riScoreType1').getValue()=='Numeric' & !Ext.getCmp('b_riScoreType2').isValid()) &
						!(Ext.getCmp('b_riScoreType1').getValue()=='Literal' & !Ext.getCmp('b_riScoreType3').isValid())
				     	     )

	/////////////////////////validate aspect (behaviour) rule fields  ////////////////////////////////
	var requirements_fulfilled_aspect = (	Ext.getCmp('b_riRuleType').getValue()=="Behavior" &&						
						Ext.getCmp('b_riAspectType_A').isValid()
					    )

	/////////////////////////validate pattern rule fields  ////////////////////////////////
	var requirements_fulfilled_pattern = (	Ext.getCmp('b_riRuleType').getValue()=="Pattern" &&						
				 		((!(Ext.getCmp('b_riOther_Pat').getValue() && !Ext.getCmp('b_riOtherField_Pat').isValid()) &
				 		!(Ext.getCmp('b_ri_page_radio').getValue() && !Ext.getCmp('b_riPageField_pat').isValid()) &
				 		!(Ext.getCmp('b_ri_parameter_radio').getValue() && !Ext.getCmp('b_riDynamic_Pat').isValid()) &
						!(Ext.getCmp('b_ri_flow_radio').getValue() && !Ext.getCmp('b_ri_flow_select').isValid())) ||
						(Ext.getCmp('b_ri_user_radio').getValue())
						) &
						Ext.getCmp('b_riCount_Pat').isValid() &
						Ext.getCmp('b_riTime1_Pat').isValid() &
						Ext.getCmp('b_riTime2_Pat').isValid()
					     )
	/////////////////////////validate pattern rule fields  ////////////////////////////////
	var requirements_fulfilled_parameter = (Ext.getCmp('b_riRuleType').getValue()=="Parameter" &&
					 		(
						 		Ext.getCmp('b_par_radio_1').getValue() &&
						 		Ext.getCmp('b_riPageField_par').isValid() &
								Ext.getCmp('b_riAttributeField_P').isValid() &
								//Ext.getCmp('b_riAttributeField_P_id').isValid() &
						 		!(Ext.getCmp('b_riRegexCB_P').getValue() && !Ext.getCmp('b_riRegexField_P').isValid()) &
						 		!(Ext.getCmp('b_riStringCB_P').getValue() && !Ext.getCmp('b_riStringField_P').isValid()) &
						 		!(Ext.getCmp('b_riFuzzyCB_P').getValue() && !Ext.getCmp('b_riFuzzyField_P').isValid()) &
						 		!(Ext.getCmp('b_riCharLengthCB_P').getValue() && !Ext.getCmp('b_riCharLengthField_P').isValid()) &
						 		!(Ext.getCmp('b_riLengthCB_P').getValue() && !Ext.getCmp('b_riLengthField1_P').isValid() & !Ext.getCmp('b_riLengthField2_P').isValid()) &
								!(Ext.getCmp('b_riSimilarCB_P').getValue() && !Ext.getCmp('b_riSimilarField_P').isValid()) /*&
								Ext.getCmp('b_riScoreType_P').isValid() &
								!(Ext.getCmp('b_riScoreType_P').getValue()=='Numeric' & !Ext.getCmp('b_riScoreType2_P').isValid()) &
								!(Ext.getCmp('b_riScoreType_P').getValue()=='Literal' & !Ext.getCmp('b_riScoreType3_P').isValid())*/
							)
							||
							(
								Ext.getCmp('b_par_radio_2').getValue() &&
								( Ext.getCmp('b_par_post_cb').getValue() || Ext.getCmp('b_par_get_cb').getValue() || Ext.getCmp('b_par_header_cb').getValue() ) &
								Ext.getCmp('b_riRegexField_P').isValid()
							)
							|| 
							(
								Ext.getCmp('b_par_radio_3').getValue() &&
								!(Ext.getCmp('b_riRegexCB_P').getValue() && !Ext.getCmp('b_riRegexField_P').isValid()) &
								!(Ext.getCmp('b_riStringCB_P').getValue() && !Ext.getCmp('b_riStringField_P').isValid())
							)
							|| 
							(
								Ext.getCmp('b_par_radio_4').getValue() &&
								!(Ext.getCmp('b_riRegexCB_P').getValue() && !Ext.getCmp('b_riRegexField_P').isValid()) &
								!(Ext.getCmp('b_riStringCB_P').getValue() && !Ext.getCmp('b_riStringField_P').isValid())
							)
					     )
	if (	Ext.getCmp('b_riRuleType').getValue()=="Parameter" && 
		Ext.getCmp('b_par_radio_1').getValue() && 
		Ext.getCmp('b_riLengthCB_P').getValue() &&
		(Ext.getCmp('b_riLengthField1_P').getValue()>=Ext.getCmp('b_riLengthField2_P').getValue())
	){
		requirements_fulfilled_general = false;
		Ext.Msg.alert('Info', 'Please make sure that the first string\'s length does not exceed the second one\'s.');
		return false;
	}
	if (	Ext.getCmp('b_riRuleType').getValue()=="Parameter" && 
		!Ext.getCmp('b_par_radio_1').getValue() && 
		!Ext.getCmp('b_par_radio_2').getValue() &&
		!Ext.getCmp('b_par_radio_3').getValue() &&
		!Ext.getCmp('b_par_radio_4').getValue()
	){
		requirements_fulfilled_general = false;
		Ext.Msg.alert('Info', 'Please select either request, page, title or URI');
		return false;
	}	
	if (	!requirements_fulfilled_parameter &&
		(Ext.getCmp('b_riRuleType').getValue()=="Parameter") &&
		Ext.getCmp('b_par_radio_2').getValue() ) {
			Ext.Msg.alert('Info', 'Please make sure you checked POST , GET or HEADER.');
			return false;
		}

	if (	!requirements_fulfilled_general 	| 
    		(Ext.getCmp('b_riRuleType').getValue()=="Parameter" && !requirements_fulfilled_parameter) ||	
    		(Ext.getCmp('b_riRuleType').getValue()=="Behavior" && !requirements_fulfilled_aspect) ||	
    		(Ext.getCmp('b_riRuleType').getValue()=="Pattern" && !requirements_fulfilled_pattern)
	   ){
	   	Ext.Msg.alert('Info', 'Please make sure you checked the red highlighted fields.');
		return false;
	}
	/////////////////////////////////////////////////////////////////////////////
	params.rulename = Ext.getCmp('b_riName').getValue();
	params.ruledesc = Ext.getCmp('b_riDesc').getValue();
	params.ruleowner = Ext.getCmp('b_riOwner').getValue();
	params.rulestartindex = Ext.getCmp('b_riStartIndex').getValue()-1;
	params.ruleAppearance = Ext.getCmp('b_riAppearance').getValue();
	params.ruleEnable = (Ext.getCmp('b_riRuleEnabled').getValue()?1:0);
	params.rule_applicationcheckbox = (Ext.getCmp('b_riAppCB').getValue()?1:0);
	var app = Ext.getCmp('b_riAppField').getStore().findRecord("app",Ext.getCmp('b_riAppField').getValue());
	if (app!=null)
		params.rule_applicationcombo = app.get("id");
	params.rule_ipcheckbox = (Ext.getCmp('b_riIpCB').getValue()?1:0);
	params.rule_iptext = Ext.getCmp('b_riIpField').getValue();
	params.rule_ipneg = (Ext.getCmp('b_riIpNot').getValue()?1:0);
	params.rule_usernamecheckbox = (Ext.getCmp('b_riUserCB').getValue()?1:0);
	params.rule_usernametext = Ext.getCmp('b_riUserField').getValue();



	if   	(Ext.getCmp('b_riRuleType').getValue()=='Behavior')
		params.rule_ruletypecombo = 'Aspect';
	else if (Ext.getCmp('b_riRuleType').getValue()=='Parameter')
		params.rule_ruletypecombo = 'ParameterRule';
	else if (Ext.getCmp('b_riRuleType').getValue()=='Pattern')
		params.rule_ruletypecombo = 'Pattern';
	else if (Ext.getCmp('b_riRuleType').getValue()=='Geographic')
		params.rule_ruletypecombo = 'Geo';	
	else if (Ext.getCmp('b_riRuleType').getValue()=='Bot-Intelligence')
		params.rule_ruletypecombo = 'Bot';

	if(Ext.getCmp('b_riRuleType').getValue() == "Behavior"){
		params.rule_aspect_aspecttype = (Ext.getCmp('b_riAspectType_A').getValue() == 'Speed'?'landing':
						(Ext.getCmp('b_riAspectType_A').getValue() == 'Page Context'?'entry':
						(Ext.getCmp('b_riAspectType_A').getValue() == 'Query'?'query':
						(Ext.getCmp('b_riAspectType_A').getValue() == 'Location'?'geo':
						(Ext.getCmp('b_riAspectType_A').getValue() == 'Date'?'week_day':
						(Ext.getCmp('b_riAspectType_A').getValue() == 'Hour'?'hour':
						(Ext.getCmp('b_riAspectType_A').getValue() == 'Average'?'average':
						'flow')))))))
		params.rule_aspect_personal = (Ext.getCmp('b_riPersonal_A').getValue()?1:0);
	}
	/////build geo rule
	else if(Ext.getCmp('b_riRuleType').getValue() == "Geographic")
	{
		params.rule_geo_type = 	(Ext.getCmp('b_riVelocity1_geo').getValue()? "Velocity" :
					(Ext.getCmp('b_riAllowCountry1_geo').getValue()? "Allow" : 
											    "Block") );
		
		params.rule_geo_radius = Ext.getCmp('b_riVelocity3_geo').getValue();
		params.rule_geo_time = Ext.getCmp('b_riVelocity2_geo').getValue();
		var allowedStore = Ext.getCmp('b_riAllowChosenList2_geo').getStore();
		var allowedChosen = "";
		allowedStore.each(function(record){
			allowedChosen+=nameToAlias[record.get("Country")]+",";
		},this);
		params.rule_geo_allow = allowedChosen.substring(0,allowedChosen.length-1);

		var blockedStore = Ext.getCmp('b_riBlockChosenList2_geo').getStore();
		var blockedChosen = "";
		blockedStore.each(function(record){
			blockedChosen+=nameToAlias[record.get("Country")]+",";
		},this);
		params.rule_geo_block = blockedChosen.substring(0,blockedChosen.length-1);
								 		
	}
	/////build pattern rule
	else if(Ext.getCmp('b_riRuleType').getValue() == "Pattern")
	{
		if(Ext.getCmp('b_riIp_Pat').getValue()) //IP
		{
			params.Pattern_Anchor='IP';
		}
		else if(Ext.getCmp('b_riSid_Pat').getValue()) //SID
		{
			params.Pattern_Anchor='SID';
		}
		else if(Ext.getCmp('b_riUser_Pat').getValue()) //User
		{
			params.Pattern_Anchor='User';
		}
		else //Other
		{
			params.Pattern_Anchor="Other";
			params.Pattern_OtherTextField = Ext.getCmp('b_riOtherField_Pat_id').getValue();
		}
		
		params.Pattern_DynamicTextField = Ext.getCmp('b_riDynamic_Pat_id').getValue();
		params.Pattern_PageTextField = Ext.getCmp('b_riPageField_id_pat').getValue();
		
		var flow = Ext.getCmp('b_ri_flow_select').getStore().findRecord("group",Ext.getCmp('b_ri_flow_select').getValue());
		if (flow!=null)
			params.Pattern_FlowSelect = flow.get("id");
		
		params.Pattern_CountTextField = Ext.getCmp('b_riCount_Pat').getValue();
		params.Pattern_TimeWindowTextField = Ext.getCmp('b_riTime1_Pat').getValue();
		params.Pattern_TimeWindowCombo = (Ext.getCmp("b_riTime2_Pat").getValue()=="Seconds"? "sec":
						 (Ext.getCmp("b_riTime2_Pat").getValue()=="Minutes"? "min":
						 	  					     "hours") )
		
		params.Pattern_DynamicType = Ext.getCmp('b_ri_user_radio').getValue() ? 'User' : '';
		
	}
	/////build parameter rule
	else if(Ext.getCmp('b_riRuleType').getValue() == "Parameter")
	{
		if (Ext.getCmp('b_par_radio_2').getValue()) {
			
			params.rule_aspect_aspecttype = '';
			
			if(Ext.getCmp('b_par_post_cb').getValue()) {
				params.rule_aspect_aspecttype += 'P,';
			}
			if(Ext.getCmp('b_par_get_cb').getValue()) {
				params.rule_aspect_aspecttype += 'G,';
			}
			if(Ext.getCmp('b_par_header_cb').getValue()) {
				params.rule_aspect_aspecttype += 'H,';
			}
			
			// Trim tailing comma
			params.rule_aspect_aspecttype = params.rule_aspect_aspecttype.slice(0,-1);
			
			params.ParameterRule_AttributeTextField = "-1"
			params.ParameterRule_PageTextField = "-1"
			
		}
		else if(Ext.getCmp('b_par_radio_1').getValue()){

			params.ParameterRule_AttributeTextField = Ext.getCmp('b_riAttributeField_P_id').getValue();
			params.ParameterRule_AttributeTextRaw   = Ext.getCmp('b_riAttributeField_P').getValue();
			params.ParameterRule_PageTextField 		= Ext.getCmp('b_riPageField_id_par').getValue();
			
		} else if (Ext.getCmp('b_par_radio_3').getValue()){
			params.rule_aspect_aspecttype = 'Title';
		} else if (Ext.getCmp('b_par_radio_4').getValue()){
			params.rule_aspect_aspecttype = 'Uri';
		}
		if(Ext.getCmp('b_riHeuristicCB_P').getValue()) //Huristics
		{
			params.tParameterRule_AttributeTypeRadio='heuristic';
		}
		else if(Ext.getCmp('b_riStringCB_P').getValue()) //StringMatch
		{
			params.tParameterRule_AttributeTypeRadio="stringmatch";
		}
		else if(Ext.getCmp('b_riRegexCB_P').getValue()) //StringMatch
		{
			params.tParameterRule_AttributeTypeRadio="gex";
		}
		else if(Ext.getCmp('b_riFuzzyCB_P').getValue()) //StringMatch
			params.tParameterRule_AttributeTypeRadio="fuzzylength"

		else if(Ext.getCmp('b_riCharLengthCB_P').getValue()) //StringMatch
			params.tParameterRule_AttributeTypeRadio='exactlength';
		else if(Ext.getCmp('b_riLengthCB_P').getValue()) //StringMatch
			params.tParameterRule_AttributeTypeRadio="rangelength"
		else if(Ext.getCmp('b_riSimilarCB_P').getValue()) //StringMatch
			params.tParameterRule_AttributeTypeRadio="distance_similarity"
		else
			params.tParameterRule_AttributeTypeRadio="unknown"
	
		if (Ext.getCmp('b_riStringCB_P').getValue() & Ext.getCmp('b_riStringNotCB_P').getValue())
			params.tParameterRule_NotSignal = "1"
		else if (Ext.getCmp('b_riRegexCB_P').getValue() & Ext.getCmp('b_riRegexNotCB_P').getValue())
			params.tParameterRule_NotSignal = "1"
		else
			params.tParameterRule_NotSignal = "0"	
		if (Ext.getCmp('b_riStringCB_P').getValue())
			params.tParameterRule_StringMatchTextField = Ext.getCmp('b_riStringField_P').getValue();
		else if (Ext.getCmp('b_riRegexCB_P').getValue())
			params.tParameterRule_StringMatchTextField = Ext.getCmp('b_riRegexField_P').getValue();
			
		params.tParameterRule_LengthCombo = (Ext.getCmp('b_riFuzzyField_P').getValue()=="Is too Long"?  "long":
						    (Ext.getCmp('b_riFuzzyField_P').getValue()=="Is too Short"? "short":
						    							 "both"));
						    
		
		params.tParameterRule_exactlength=Ext.getCmp('b_riCharLengthField_P').getValue();
		params.tParameterRule_rangelengthlow=Ext.getCmp('b_riLengthField1_P').getValue();
		params.tParameterRule_rangelengthhigh=Ext.getCmp('b_riLengthField2_P').getValue();
		params.tParameterRule_stringsimilarity=Ext.getCmp('b_riSimilarField_P').getValue();

		
	}
	else if(Ext.getCmp('b_riRuleType').getValue() == "Bot-Intelligence")
	{
		params.Bot_bot_type = Ext.getCmp('b_riBotType_bot').getValue();
	}
	else{ }
	///user score
	params.rule_scoretype = (Ext.getCmp('b_riScoreType1').getValue()=='Literal'? 'literal':'numeric');
	params.rule_scoretype_literal =(Ext.getCmp('b_riScoreType3').getValue()=="Low"? "low":
					      (Ext.getCmp('b_riScoreType3').getValue()=="High"? "high":
			  									  "med"));					  		
	params.rule_scoretype_numeric =Ext.getCmp('b_riScoreType2').getValue();



	return true; 

}



/*
 * Function: delete_rule_or_group
 * 
 * Invoked:
 * 	When a delete button was clicked, from the right click menu or the panel toolbar.
 * 	The tree is reloaded after deletion without expanding any node.
 *
 * Parameters:
 * 
 * 
 * Returns:
 * 	
 * 
 */
function delete_rule_or_group(){
	var node = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
	
	if(!telepath.access.has_perm('Rules', 'del')) { return; }
	
	if (node.data.type=='rule') {
		
		telepath.util.request('/rules', { mode:"delete_rule", ruleid:node.data.rule_id }, function (data) { 
			
			node_to_display_info = {
								   type:'group',
								   group_id : parseInt(node.parentNode.data.group_id),
								   category_id : parseInt(node.parentNode.data.category_id)
								   }; 				
									
			telepath.rules.fetchData();
			
		}, 'Error deleting rule.');

	}
	else if (node.data.type=='group') {
		
		telepath.util.request('/rules', { mode:"delete_group", groupid:node.data.group_id }, function (data) { 
			
			node_to_display_info = {
								   type:'category',
								   category_id : parseInt(node.parentNode.data.category_id)
								   }; 				
								   
			telepath.rules.fetchData();
			
		}, 'Error deleting group.');
		
	}
	else if (node.data.type=='category') {
		
		telepath.util.request('/rules', { mode:"delete_category", category_id:node.data.category_id }, function (data) { 			
			node_to_display_info = undefined;	
			telepath.rules.fetchData();
			
		}, 'Error deleting category.');

	}
	
	Ext.getCmp("b_delete").disable();
	Ext.getCmp("main_panel").enable();
	Ext.WindowManager.get("DeleteRule").hide();	

}


/////////////////////////////////////////COPY AND PASTE NODES/////////////////////////

////global vars for cut n paste
//the rule node clicked
var ruleClicked = undefined;
//the group node clicked
var groupClicked = undefined;
//booleans
var groupCut = false;
var groupCopied = false;
var ruleCut = false;
var ruleCopied = false;

var b_menu_rule_items 	  = [];
var b_menu_group_items 	  = [];
var b_menu_category_items = [];
	
if(telepath.access.has_perm('Rules', 'add')) {

	b_menu_rule_items.push({ xtype: 'menuitem', id: 'b_copy_rule', text: 'Copy', icon:'Htmls/images/copy.png' });
	b_menu_group_items.push({ xtype: 'menuitem', id: 'b_dup_group',	text: 'Duplicate rule', disabled : true, icon:'Htmls/images/duplicate.png'	});
	b_menu_group_items.push({ xtype: 'menuitem', id: 'b_copy_group', text: 'Copy', icon:'Htmls/images/copy.png' });
	b_menu_group_items.push({ xtype: 'menuitem', id: 'b_add_rule', text: 'Add criteria', icon:'Htmls/images/add.gif' });
	b_menu_category_items.push({ xtype: 'menuitem', id: 'b_add_group', text: 'Add new rule', icon:'Htmls/images/add.gif' });

	}

if(telepath.access.has_perm('Rules', 'set')) {

	b_menu_rule_items.push({ xtype: 'menuitem',	id: 'b_cut_rule', text: 'Cut', icon:'Htmls/images/cut.png' });
	b_menu_group_items.push({ xtype: 'menuitem', id: 'b_cut_group', text: 'Cut', icon:'Htmls/images/cut.png' });
	b_menu_group_items.push({ xtype: 'menuitem', id: 'b_paste_rule', text: 'Paste criteria', icon:'Htmls/images/paste.png' });
	b_menu_category_items.push({ xtype: 'menuitem',	id: 'b_paste_group', text: 'Paste rule', icon:'Htmls/images/paste.png' });
	b_menu_category_items.push({ xtype: 'menuitem',	id: 'b_edit_category', text: 'Edit category', icon:'images/icon-edit.png' });
	b_menu_group_items.push({ xtype: 'menuitem', id: 'b_enable_all_rules_group', text: 'Enable all criteria', icon:'Htmls/images/checkall.png' });
	b_menu_group_items.push({ xtype: 'menuitem', id: 'b_disable_all_rules_group', text: 'Disable all criteria', icon:'Htmls/images/unchecked.png' });
	b_menu_category_items.push({ xtype: 'menuitem',	id: 'b_enable_all_rules_cat', text: 'Enable all rules', icon:'Htmls/images/checkall.png' });
	b_menu_category_items.push({ xtype: 'menuitem', id: 'b_disable_all_rules_cat', text: 'Disable all rules', icon:'Htmls/images/unchecked.png' });
	
}

if(telepath.access.has_perm('Rules', 'get')) {

	b_menu_rule_items.push({ xtype: 'menuitem', id: 'b_undo2', text: 'Undo', disabled : true, icon:'Htmls/images/undo.png' });
	b_menu_group_items.push({ xtype: 'menuitem', id: 'b_undo1', text: 'Undo', disabled : true, icon:'Htmls/images/undo.png' });
	b_menu_category_items.push({ xtype: 'menuitem', id: 'b_undo3',	text: 'Undo', disabled : true, icon:'Htmls/images/undo.png' });
	
}

if(telepath.access.has_perm('Rules', 'del')) {

	b_menu_rule_items.push({ xtype: 'menuitem',	id: 'b_delete_rule', text: 'Delete', icon:'Htmls/images/delete.gif'	});
	b_menu_group_items.push({ xtype: 'menuitem', id: 'b_delete_group', text: 'Delete', icon:'Htmls/images/delete.gif' });
	b_menu_category_items.push({ xtype: 'menuitem', id: 'b_delete_category', text: 'Delete', icon:'Htmls/images/delete.gif' });
	
}

var b_menu_rule = Ext.create('Ext.menu.Menu', {
	floating: true,
	hidden: true,
	hideMode:'display',//crucial to fix black squares bug in chrome
	id: 'b_menu_rule',
	itemId: 'b_menu_rule',
	items: b_menu_rule_items
});

//group right click menu
var b_menu_group = Ext.create('Ext.menu.Menu', {
	floating: true,
	hidden: true,
	hideMode:'display',//crucial to fix black squares bug in chrome
	id: 'b_menu_group',
	itemId: 'b_menu_group',
	items: b_menu_group_items
});

//cateogry right click menu
var b_menu_category = Ext.create('Ext.menu.Menu', {
	floating: true,
	hidden: true,
	hideMode:'display',//crucial to fix black squares bug in chrome
	id: 'b_menu_category',
	itemId: 'b_menu_category',
	items: b_menu_category_items
});

/*
 * Function: handleRightClick
 * 
 * Invoked:
 * 	upon right click on the tree. Recognizes the type of node clicked and opens the relevant menu.
 * 
 * 
 * Parameters:
 * 	see itemcontextmenu event click for node for extjs
 * 
 * Returns:
 * 	undefined
 * 
 */
function handleRightClick(dataview,record,item,index,e,options){
	//check if the node clicked is a rule or a group
	var node = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
	e.preventDefault();
	
	if (ruleClicked != undefined) {
		if(Ext.getCmp('b_paste_rule')) { Ext.getCmp('b_paste_rule').enable(); }
	} else {
		if(Ext.getCmp('b_paste_rule')) { Ext.getCmp('b_paste_rule').disable(); }
	}
		
	if (groupClicked != undefined) {
		if(Ext.getCmp('b_paste_group')) { Ext.getCmp('b_paste_group').enable(); }
	} else {
		if(Ext.getCmp('b_paste_group')) { Ext.getCmp('b_paste_group').disable(); }
	}
	
	if (node.data.type=='rule'){
		b_menu_rule.showAt(e.xy[0],e.xy[1]-20);
	}
	if (node.data.type=='group'){
		b_menu_group.showAt(e.xy[0],e.xy[1]-20);
	}
	if (node.data.type=='category'){
		b_menu_category.showAt(e.xy[0],e.xy[1]-20);
	}	
	
}

/*
 * Menu buttons binding to functions
 */
if(Ext.getCmp('b_undo1')) {
Ext.getCmp('b_undo1').addListener(
	'click',
	function(){
		b_undo(ruleClicked);
	}
);
}
if(Ext.getCmp('b_undo2')) {
Ext.getCmp('b_undo2').addListener(
	'click',
	function(){
		b_undo(ruleClicked);
	}
);
}

if(Ext.getCmp('b_undo3')) {
Ext.getCmp('b_undo3').addListener(
	'click',
	function(){
		b_undo(groupClicked);
	}
);
}

if(Ext.getCmp('b_add_group')) {
Ext.getCmp('b_add_group').addListener(
	'click',
	function(){
		Ext.create('MyApp.view.add_new_rules_group').show();
	}
);
}

if(Ext.getCmp('b_enable_all_rules_cat')) {
Ext.getCmp('b_enable_all_rules_cat').addListener(
	'click',
	function(){
		toggle_all_rules_ability('enable')
	}
);
}

if(Ext.getCmp('b_enable_all_rules_group')) {
Ext.getCmp('b_enable_all_rules_group').addListener(
	'click',
	function(){
		toggle_all_rules_ability('enable')
	}
);
}

if(Ext.getCmp('b_disable_all_rules_cat')) {
Ext.getCmp('b_disable_all_rules_cat').addListener(
	'click',
	function(){
		toggle_all_rules_ability('disable')
	}
);
}

if(Ext.getCmp('b_disable_all_rules_group')) {
Ext.getCmp('b_disable_all_rules_group').addListener(
	'click',
	function(){
		toggle_all_rules_ability('disable')
	}
);
}

if(Ext.getCmp('b_edit_category')) {
Ext.getCmp('b_edit_category').addListener(
	'click',
	function(){
		
		/*var node = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
		console.log(node);
		
		categoryID = '22';
		old_value  = 'fart';
		
		context_prompt('Rename Rule Category', 'Rename Category', function (text) {
			
			telepath.util.request('/rules', { mode:"set_category_name", catId : categoryID, text: text }, function (data) {
				
				console.log(data);
				
			}, 'Error adding rule criteria.');
		
		}, old_value);*/
		editRuleOrGroup();

	}
);
}



/*
 * Function: toggle_all_rules_ability
 * 
 * Invoked:
 * 	When enable/disable all rules for group or category button on the right click menu was called
 * 	All rules under that group/category need to be disabled/enabled on the server
 * 
 * Parameters:
 * 	toggle_mode - "disable" or "enable" string
 * 
 * Returns:
 * 	undefined	
 * 
 */
function toggle_all_rules_ability(toggle_mode) {

	var node = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
	var params = {}
	params[node.data.type+'_id'] = (node.data.type=='category'?node.data.category_id:node.data.group_id);
	params['mode'] = toggle_mode+"_"+node.data.type+"_rules";
	
	var msg_ok  = 'All '+node.data.text+' category rules are ' + toggle_mode + 'd.';
	var msg_err = 'Could not toggle rules for '+node.data.type+' '+node.data.text+'.';
	
	if(node.data.type == 'group') {
		var msg_ok  = 'All '+node.data.text+' rule criterions are ' + toggle_mode + 'd.';
		var msg_err = 'Could not toggle criterions for rule '+node.data.text+'.';
	} 
		
	telepath.util.request('/rules', params, function (data) {
		
		node_to_display_info = {
				type:node.data.type,
				group_id : node.data.group_id,
				category_id : node.data.category_id,
				rule_id : node.data.rule_id
		}; 
		telepath.rules.fetchData();
	
	}, msg_err);
		
}

/*
 * Function: b_undo
 * 
 * Invoked:
 * 	When undo button on the right click menu was clicked
 * 	
 * 
 * Parameters:
 * 	
 * 
 * Returns:
 * 	undefined	
 * 
 */
function b_undo(nodeClicked){
	Ext.getCmp('b_undo1').disable();
	Ext.getCmp('b_undo2').disable();
	Ext.getCmp('b_undo3').disable();
	nodeClicked.parentNode.appendChild(nodeClicked);	
}


/*
 * Function: event binding to 'Copy Group' button
 * 
 * Invoked:
 * 	When 'Copy Group' button on the right click menu was clicked
 * 	
 * 
 * Parameters:
 * 	
 * 
 * Returns:
 * 	undefined	
 * 
 */
if(Ext.getCmp('b_copy_group')) {
Ext.getCmp('b_copy_group').addListener(
	'click',
	function(){
		Ext.getCmp('b_paste_group').enable();		
		pasteMode = 'copy';
		var node = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
		groupClicked = node;
	}
);
}
/*
 * Function: event binding to 'Copy Rule' button
 * 
 * Invoked:
 * 	When 'Copy Rule' button on the right click menu was clicked
 * 	
 * 
 * Parameters:
 * 	
 * 
 * Returns:
 * 	undefined	
 * 
 */
if(Ext.getCmp('b_copy_rule')) {
Ext.getCmp('b_copy_rule').addListener(
	'click',
	function(){
		Ext.getCmp('b_paste_rule').enable();
		pasteMode = 'copy';
		var node = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
		ruleClicked = node;
	}
);
}

/*
 * Function: open_rules_wizard
 * 
 * Invoked:
 * 	When we want to open the rule wizard window
 * 	
 * 
 * Parameters:
 * 	
 * 
 * Returns:
 * 	undefined	
 * 
 */
function open_rules_wizard() {

	if (Ext.WindowManager.get('RulesWizard')==undefined)
		Ext.create('MyApp.view.RulesWizard').show();
	else 
		Ext.WindowManager.get('RulesWizard').show();
		
	Ext.getStore('rw_rule_groups_store').load();
	
}

/////////// Binding to the rule right click menu ////////////
if(Ext.getCmp('b_cut_rule')) {
Ext.getCmp('b_cut_rule').addListener(
	'click',
	function(){
		pasteMode = 'cut';
		var node = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
		var prevParent = node.parentNode;
		ruleClicked = node.copy();
		node.parentNode.removeChild(node);
		ruleClicked = node;
		ruleClicked.parentNode = prevParent;
		Ext.getCmp('b_undo1').enable();
		Ext.getCmp('b_undo2').enable();
		Ext.getCmp('b_undo3').enable();
	}
);
}

if(Ext.getCmp('b_delete_rule')) {
Ext.getCmp('b_delete_rule').addListener(
	'click',
	function(){
		open_delete_rule_window();
	}
);
}

if(Ext.getCmp('b_delete_group')) {
Ext.getCmp('b_delete_group').addListener(
	'click',
	function(){
		open_delete_rule_window();
	}
);
}

if(Ext.getCmp('b_paste_group')) {
Ext.getCmp('b_paste_group').addListener(
	'click',
	function(){
		paste_group()
	}
);
}
if(Ext.getCmp('b_delete_category')) {
Ext.getCmp('b_delete_category').addListener(
	'click',
	function(){
		open_delete_rule_window();
	}
);
}

if(Ext.getCmp('b_add_rule')) {
Ext.getCmp('b_add_rule').addListener(
	'click',
	function(){
		add_rule_to_group = true
		open_rules_wizard()
	}
);
}


//this flag is for the cancel button, if a rule was copied, it has to be saved with a different name
copy_mode = false;
if(Ext.getCmp('b_paste_rule')) {
Ext.getCmp('b_paste_rule').addListener(
	'click',
	function(){
		paste_rule()
	}
);
}

/////////// End of binding to the rule right click menu ////////////

/*
 * Function: paste_rule
 * 
 * Invoked:
 * 	When the 'paste rule' button from the right click menu for a group was clicked
 * 	
 * 
 * Parameters:
 * 	
 * 
 * Returns:
 * 	undefined	
 * 
 */
function paste_rule() {

		
		Ext.getCmp('b_undo1').disable();
		Ext.getCmp('b_undo2').disable();
		Ext.getCmp('b_undo3').disable();
		//Ext.getCmp('b_paste_rule').disable();
		if (ruleClicked == undefined){
			Ext.Msg.alert('Info', 'Please select a rule');
			return;
		}
		var groupToAddTo = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
		if (pasteMode == 'copy'){	
			 
			telepath.util.request('/rules', { mode:"create_new_rule_in_group", groupId : groupToAddTo.data.group_id	}, function (data) {
				
				var new_rule_id = parseInt(data.items.rule_id);
				var new_rule = ruleClicked.copy(new_rule_id);
				new_rule.data.rule_id = new_rule_id;
				groupToAddTo.appendChild(new_rule);					
				Ext.getCmp('b_ruleGroups').getSelectionModel().deselect(ruleClicked);
				Ext.getCmp('b_ruleGroups').getSelectionModel().select(ruleClicked);
				Ext.getCmp('b_ruleGroups').getView().suspendEvents();
				Ext.getCmp('b_ruleGroups').getSelectionModel().select(new_rule);
				Ext.getCmp('b_ruleGroups').getView().resumeEvents();
				Ext.getCmp('b_riName').setValue(Ext.getCmp('b_riName').getValue()+"_Copy");
				updateRuleValues();
				
			}, 'Error adding rule criteria.');
		
		}
		else if (pasteMode == 'cut') {
			
			telepath.util.request('/rules', { mode:"move_rule_to_group", groupId : groupToAddTo.data.group_id, ruleId : ruleClicked.data.rule_id }, function (data) {
				
				node_to_display_info = {
					type:'rule',
					group_id : groupToAddTo.data.group_id,
					category_id : groupToAddTo.data.category_id,
					rule_id : ruleClicked.data.rule_id
				}; 	
				
				telepath.rules.fetchData();
				
			}, 'Error moving rule criteria.');
			
		}			
		
}

/*
 * Function: paste_group
 * 
 * Invoked:
 * 	When the 'paste group' button from the right click menu for a category was clicked
 * 	
 * 
 * Parameters:
 * 	
 * 
 * Returns:
 * 	undefined	
 * 
 */
function paste_group() {

	Ext.getCmp('b_undo1').disable();
	Ext.getCmp('b_undo2').disable();
	Ext.getCmp('b_undo3').disable();
	
	if (groupClicked == undefined){
		Ext.Msg.alert('Info', 'Please select a rule');
		return;
	}
	var mode;
	var categoryToAddTo = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
	
	if (pasteMode == 'copy') {
		mode = "copy_group_to_category";
	}
		
	if (pasteMode == 'cut') {
		mode = "move_group_to_category";
	}
	
	telepath.util.request('/rules', { mode:mode, category_id: categoryToAddTo.data.category_id, group_id: groupClicked.data.group_id }, function (data) {
	
		node_to_display_info = {
			type:'group',
			group_id : categoryToAddTo.data.group_id,
			category_id : categoryToAddTo.data.category_id
		}; 	
			
		telepath.rules.fetchData();
		
	}, 'Error in move/copy rule to category.');
	
}

function open_delete_rule_window(){	
	if (Ext.WindowManager.get('DeleteRule')==undefined){
	    Ext.create('MyApp.view.DeleteRule').show();
	}
	else 
	    Ext.WindowManager.get('DeleteRule').show();
}

/*
 * Function: create_new_category
 * 
 * Invoked:
 * 	When we clicked save on the "add new category window" 
 * 	
 * 
 * Parameters:
 * 	name - the name of the category inserted in the name field
 * 
 * Returns:
 * 	undefined	
 * 
 */
function create_new_category(name) {
	
	telepath.util.request('/rules', { mode:"create_new_category", category_name:name }, function (data) {
	

		telepath.rules.fetchData();
	
	}, function (data) {
		
		if(data.success === false) {
			Ext.Msg.alert('Error', 'Category '+name+' already exists.');
		}
	
	});
	
}


/*
 * Function: open_edit_category_window
 * 
 * Invoked:
 * 	When we select a category from the tree and click edit on the toolbar
 * 	
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */

function open_edit_category_window(){
	var selected = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection();
	if (selected.length==0 || selected[0].data.type!='category'){
		Ext.Msg.alert('Error #15', 'No category was selected');
		return;			
	}
	else	
		Ext.create('MyApp.view.edit_rules_category').show();
}


/*
 * Function: edit_category
 * 
 * Invoked:
 * 	When we clicked save on the "edit category window"
 * 	
 * 
 * Parameters:
 * 	name - the new name entered in the field
 * 	id - the id of the category we want to edit
 * 
 * Returns:
 * 	undefined	
 * 
 */
function edit_category(name,id){
	
	telepath.util.request('/rules', { mode:"update_category", category_name:name, category_id:id }, function (data) {
		
		telepath.rules.fetchData();
		
	}, 'Error updating category.');

}


/////////// Binding to the rule right click menu ////////////
if(Ext.getCmp('b_cut_group')) {
Ext.getCmp('b_cut_group').addListener(
	'click',
	function(){
		pasteMode = 'cut';
		var node = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0];
		var prevParent = node.parentNode;
		groupClicked = node.copy();
		node.parentNode.removeChild(node);
		groupClicked = node;
		groupClicked.parentNode = prevParent;
		Ext.getCmp('b_undo1').enable();
		Ext.getCmp('b_undo2').enable();
		Ext.getCmp('b_undo3').enable();
	}
);
}

/*
 * Function: create_new_group
 * 
 * Invoked:
 * 	When we clicked create on the "add new group window"
 * 	
 * 
 * Parameters:
 * 	name - the new name entered in the field
 * 	category_id - the id of the category we want to add the group to
 * 
 * Returns:
 * 	undefined	
 * 
 */
function create_new_group(name,category_id){
	
	telepath.util.request('/rules', { mode:"create_new_group", group_name:name, category_id:category_id }, function (data) {
	
		Ext.WindowManager.get('add_new_rules_group').destroy();
		var group_id = data.group_id;
		node_to_display_info = {
			type: 'group',
			group_id : parseInt(group_id),
			category_id : parseInt(category_id)
		};
		telepath.rules.fetchData();	
		
	}, function () {
		
		if(data.success === false) {
			Ext.Msg.alert('Error', 'Rule ' + name + ' already exists.');
		} else {
			Ext.Msg.alert('Error', 'Error in creating rule.');
		}
		
	});
	
}


///////////////////////////////////////RULES WIZARD ////////////////////////////////////////


////////////////////////////////////////first and second windows///////////////////////////

/*
 * Function: get_values_from_first_and_second_windows
 * 
 * Invoked:
 * 	When clicking "Submit" on the rule wizard window, we have to take the values from the first and the scond window.
 * 
 * 
 * Parameters:
 *
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<submit_geo_rule>
 * 	<submit_pat_rule>
 * 	<submit_bot_rule>
 * 	<submit_parameter_rule>
 * 	<submit_pattern_rule>
 * 	<submit_aspect_rule>
 * 
 */
function get_values_from_first_and_second_windows(){
	
	var data={};
	var group_groupmode ;
	var category_mode;
	if (Ext.getCmp('RW1_new_category_cb').getValue()) {
		category_mode = "new";
		data.category_mode = category_mode;
		data.category_name = Ext.getCmp('RW1_new_category_name').getValue()
	}
	else{
		data.category_mode = "edit";
		data.category_id = Ext.getCmp('RW1_exists_category_name').valueModels[0].raw.id
	}
	
	
	if (Ext.getCmp('RW1_f1').getValue()) {
		group_groupmode = "new";
		data.group_groupmode = group_groupmode;

		var group_nametextfield = Ext.getCmp('RW1_f2').getValue();
		data.group_nametextfield = group_nametextfield;

		var group_desctextfield = Ext.getCmp('RW1_f3').getValue();
		data.group_desctextfield = group_desctextfield;

		var group_action_log = Ext.getCmp('RW1_f6').getValue();
		if (group_action_log==true)
			data.group_action_log = 1;

		var group_action_email = Ext.getCmp('RW1_f7').getValue();
		if (group_action_email==true){
			data.group_action_email = 1;
			var emails = "";
			Ext.getCmp('b_rwEmailField').getStore().each(function(record){
				emails+=record.get('address')+","
			})
			emails = emails.substring(0,emails.length-1);
			data.group_action_email_field = emails;
		}

		var group_action_syslog = Ext.getCmp('RW1_f8').getValue();
		if (group_action_syslog==true)
			data.group_action_syslog = 1;

		// parameters to show on alerts
		var param_ids = ""
		Ext.getCmp('rw_alert_params').getStore().each(function(record){
			param_ids+=record.get('td1')+","
		})
		param_ids = param_ids.substring(0,param_ids.length-1);
		data.general_param_ids = param_ids
	}

	else {
		
    	data.group_groupmode = "edit";
		data.group_ExistsGroupsCombobox = Ext.getStore('rw_rule_groups_store').findRecord("group",Ext.getCmp('RW1_f13').getValue()).get("id");
		
	}



	////////////END OF FIRST WINDOW////////
	////////////START SECOND WINDOW////////

	var general_rulename = Ext.getCmp('RW2_f1').getValue();
	data.general_rulename = general_rulename;

	var general_description = Ext.getCmp('RW2_f2').getValue();
	data.general_description = general_description;

	var general_owner = Ext.getCmp('RW2_f3').getValue();
	data.general_owner = general_owner;

	var general_RuleTypeCombo;
 	if (Ext.getCmp('RW2_f4').getValue()=="Parameter")
		general_RuleTypeCombo = "ParameterRule";
 	if (Ext.getCmp('RW2_f4').getValue()=="Behavior")
		general_RuleTypeCombo = "Aspect";
 	if (Ext.getCmp('RW2_f4').getValue()=="Pattern")
		general_RuleTypeCombo = "Pattern";
 	if (Ext.getCmp('RW2_f4').getValue()=="Geographic")
		general_RuleTypeCombo = "Geo";
 	if (Ext.getCmp('RW2_f4').getValue()=="Bot-Intelligence")
		general_RuleTypeCombo = "Bot";
	data.general_RuleTypeCombo = general_RuleTypeCombo;

	var general_ApplicationCheckbox = Ext.getCmp('RW2_f5_1').getValue();
	if (general_ApplicationCheckbox==true){
	    data.general_ApplicationCheckbox = 1;
	    data.general_ApplicationCombobox = Ext.getCmp('RW2_f5_1_2').getStore().findRecord("app",Ext.getCmp('RW2_f5_1_2').getValue()).get("id");
	}

	var general_ipcheckbox = Ext.getCmp('RW2_f5_2').getValue();
	if (general_ipcheckbox==true){
	    data.general_ipcheckbox = 1;
	    data.general_iptextfield = Ext.getCmp('RW2_f5_2_2').getValue();
	}
	
	data.ip_neg = Ext.getCmp('RW2_f5_not').getValue();

	var general_usercheckbox = Ext.getCmp('RW2_f5_3').getValue();
	if (general_usercheckbox==true){
	    data.general_usercheckbox = 1;
	    data.general_usertextfield = Ext.getCmp('RW2_f5_3_2').getValue();
	}

	var general_StartIndexField = Ext.getCmp('RW2_f6').getValue()-1;
	data.general_StartIndexField = general_StartIndexField;

	var general_AppearanceField = Ext.getCmp('RW2_f7').getValue();
	data.general_AppearanceField = general_AppearanceField;

	////////////END OF SECOND WINDOW////////
	////////////START LAST SCORE AND PARAMS WINDOW////////


	// rule score
	
	data.general_scoretype = (Ext.getCmp('RWrule_score_type1').getValue()=="Numeric"?"numeric":"literal")
	if (data.general_scoretype=="numeric"){
		data.general_scoretype_numeric = Ext.getCmp('RWrule_score_type2').getValue()
		data.general_scoretype_literal = ""
	}
	if (data.general_scoretype=="literal"){
		data.general_scoretype_literal = (Ext.getCmp('RWrule_score_type3').getValue()=="High"?"high":
						((Ext.getCmp('RWrule_score_type3').getValue()=="Medium"?"med":
						(Ext.getCmp('RWrule_score_type3').getValue()=="Low"?"low":""))))
						
		
		
		data.general_scoretype_numeric = ""
	}
	return data;	


}



/*
 * Function: submit_geo_rule
 * 
 * Invoked:
 * 	When clicking "Submit" on the geo rule window on rule wizard.
 * 	Takes all the fields and invokes submit rule to server
 * 
 * 
 * Parameters:
 *
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */

//called whenever we create a geo rule on the wizard
function submit_geo_rule(){

	var data = get_values_from_first_and_second_windows();

	data.GeoRule_Type = (Ext.getCmp("RWgeo_f1_1").getValue()?"Velocity" :(Ext.getCmp("RWgeo_f2_1").getValue()?"Allow" :"Block"));

	data.GeoRule_Radius = Ext.getCmp("RWgeo_f1_3").getValue();

	data.GeoRule_Time = Ext.getCmp("RWgeo_f1_2").getValue();

	data.GeoRule_Block = getCountries('Block');

	data.GeoRule_Allow = getCountries('Allow');


	submit_rule_to_server(data);
		

}



/*
 * Function: submit_pattern_rule
 * 
 * Invoked:
 * 	When clicking "Submit" on the pattern rule window on rule wizard.
 * 	Takes all the fields and invokes submit rule to server
 * 
 * 
 * Parameters:
 *
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function submit_pattern_rule(){

	var requirements_fulfilled = (	
				 	((!(Ext.getCmp('RWpat_f3').getValue() & !Ext.getCmp('RWpat_f3_1').isValid()) &
					!(Ext.getCmp('RWpat_page_radio').getValue() & !Ext.getCmp('RWpat_f4_page').isValid()) &
					!(Ext.getCmp('RWpat_parameter_radio').getValue() & !Ext.getCmp('RWpat_f4').isValid()) &
					!(Ext.getCmp('RWpat_flow_radio').getValue() & !Ext.getCmp('RWpat_flow_select').isValid())) ||
					Ext.getCmp('RWpat_user_radio').getValue()) &
					!(!Ext.getCmp('RWpat_f5').isValid()) &
					!(!Ext.getCmp('RWpat_f6').isValid()) )
	if (!requirements_fulfilled)
		return false;

	var data = get_values_from_first_and_second_windows();

	////////////END OF SECOND WINDOW////////
	////////////START PATTERN WINDOW////////


	if(Ext.getCmp("RWpat_f1").getValue()==true){
	    data.Pattern_Anchor = "IP";
	}

	else if(Ext.getCmp("RWpat_f2").getValue()==true){
	    data.Pattern_Anchor = "SID";
	}

	else if(Ext.getCmp("RWpat_f10").getValue()==true){
	    data.Pattern_Anchor = "User";
	}

	else if(Ext.getCmp("RWpat_f3").getValue()==true){
	    data.Pattern_Anchor = "Other";
	    data.Pattern_OtherTextFieldDisplay = Ext.getCmp("RWpat_f3_1").getValue();
	    data.Pattern_OtherTextField = parseInt(Ext.getCmp("RWpat_f3_id").getValue());
	}

	var Pattern_DynamicTextFieldDisplay = Ext.getCmp("RWpat_f4").getValue();

	data.Pattern_DynamicTextFieldDisplay = Pattern_DynamicTextFieldDisplay;

	data.Pattern_DynamicTextField = Ext.getCmp("RWpat_f4_id").getValue();

	var Pattern_PageTextFieldDisplay = Ext.getCmp("RWpat_f4_page").getValue();
	
	data.Pattern_PageTextFieldDisplay = Pattern_PageTextFieldDisplay;
	
	data.Pattern_DynamicType = Ext.getCmp('RWpat_user_radio').getValue() ? 'User' : '';

	data.Pattern_PageTextField = Ext.getCmp("RWpat_f4_id_page").getValue();
	
	var flow = Ext.getCmp('RWpat_flow_select').getStore().findRecord("group",Ext.getCmp('RWpat_flow_select').getValue());
		if (flow!=null)
			data.Pattern_FlowSelect = flow.get("id");

	var Pattern_CountTextField = Ext.getCmp("RWpat_f5").getValue();

	data.Pattern_CountTextField = Pattern_CountTextField;

	var Pattern_TimeWindowTextField = Ext.getCmp("RWpat_f6").getValue();

	data.Pattern_TimeWindowTextField = Pattern_TimeWindowTextField;

	var Pattern_TimeWindowCombo = (Ext.getCmp("RWpat_f7").getValue()=="Seconds"? "sec":(Ext.getCmp("RWpat_f7").getValue()=="Minutes"? "min":"hours") );

	data.Pattern_TimeWindowCombo = Pattern_TimeWindowCombo;
	
	submit_rule_to_server(data);

	
}

/*
 * Function: submit_bot_rule
 * 
 * Invoked:
 * 	When clicking "Submit" on the bot rule window on rule wizard.
 * 	Takes all the fields and invokes submit rule to server
 * 
 * 
 * Parameters:
 *
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function submit_bot_rule(){
	var data = get_values_from_first_and_second_windows();
	data.Bot_Type = Ext.getCmp("RWbot_type").getValue();

	submit_rule_to_server(data);
}



/*
 * Function: submit_parameters_rule
 * 
 * Invoked:
 * 	When clicking "Submit" on the parameters rule window on rule wizard.
 * 	Takes all the fields and invokes submit rule to server
 * 
 * 
 * Parameters:
 *
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function submit_parameter_rule(){
	///validate fields of param window
	if (!Ext.getCmp('RWpar_radio_1').getValue() && !Ext.getCmp('RWpar_radio_2').getValue() && !Ext.getCmp('RWpar_radio_3').getValue() && !Ext.getCmp('RWpar_radio_4').getValue()){
		Ext.Msg.alert('Info', 'Please select page, request, title or URI options.');
		return false;		
	}
	var requirements_fulfilled_option = (	(	Ext.getCmp('RWpar_radio_1').getValue() &&
							Ext.getCmp('RWpar_f1').isValid() &
						 	!(Ext.getCmp('RWpar_f9_1').getValue() & !Ext.getCmp('RWpar_f9_2').isValid()) &
							!(Ext.getCmp('RWpar_f3_1').getValue() & !Ext.getCmp('RWpar_f3_2').isValid()) &
							!(Ext.getCmp('RWpar_f4_1').getValue() & !Ext.getCmp('RWpar_f4_2').isValid()) &
							!(Ext.getCmp('RWpar_f5_1').getValue() & !Ext.getCmp('RWpar_f5_2').isValid()) &
							!(Ext.getCmp('RWpar_f6_1').getValue() & !(Ext.getCmp('RWpar_f6_2').isValid() || Ext.getCmp('RWpar_f6_3').isValid())) &
							!(Ext.getCmp('RWpar_f7_1').getValue() & !Ext.getCmp('RWpar_f7_2').isValid()) 	)
						||
						(
							Ext.getCmp('RWpar_radio_2').getValue() &&
							( Ext.getCmp('RWpar_post_cb').getValue() || Ext.getCmp('RWpar_get_cb').getValue() ) &
							Ext.getCmp('RWpar_f9_2').isValid() 
						)
						||
						(
							Ext.getCmp('RWpar_radio_3').getValue() &&
							!(Ext.getCmp('RWpar_f9_1').getValue() & !Ext.getCmp('RWpar_f9_2').isValid()) &
							!(Ext.getCmp('RWpar_f3_1').getValue() & !Ext.getCmp('RWpar_f3_2').isValid())
						)
						||
						(
							Ext.getCmp('RWpar_radio_4').getValue() &&
							!(Ext.getCmp('RWpar_f9_1').getValue() & !Ext.getCmp('RWpar_f9_2').isValid()) &
							!(Ext.getCmp('RWpar_f3_1').getValue() & !Ext.getCmp('RWpar_f3_2').isValid())
						)
					
					)
	if (Ext.getCmp('RWpar_radio_2').getValue() && Ext.getCmp('RWpar_f9_2').isValid() && !requirements_fulfilled_option){
		Ext.Msg.alert('Info', 'Please make sure you checked POST,GET or both.');
		return false;
	}
		
	else if (!requirements_fulfilled_option){
		Ext.Msg.alert('Info', 'Please make sure you checked the red highlighted fields.');
		return false;
	}		
	if (Ext.getCmp('RWpar_f6_1').getValue() && (Ext.getCmp('RWpar_f6_2').getValue()>=Ext.getCmp('RWpar_f6_3').getValue())){
		Ext.Msg.alert('Info', 'Please make sure that the first string length does not exceed the second one.');
		return false;
	}
	var data = get_values_from_first_and_second_windows();


	////////////END OF SECOND WINDOW////////
	////////////START PARAMETER WINDOW////////

	var Aspect_AspectTypeCombobox;
	if (Ext.getCmp('RWpar_radio_2').getValue()) {
		
		data.Aspect_AspectTypeCombobox = '';
		
		if(Ext.getCmp('RWpar_post_cb').getValue()) {
			data.Aspect_AspectTypeCombobox += 'P,';
		}
		if(Ext.getCmp('RWpar_get_cb').getValue()) {
			data.Aspect_AspectTypeCombobox += 'G,';
		}
		if(Ext.getCmp('RWpar_header_cb').getValue()) {
			data.Aspect_AspectTypeCombobox += 'H,';
		}
		
		// Trim tailing comma
		data.Aspect_AspectTypeCombobox = data.Aspect_AspectTypeCombobox.slice(0,-1);
				
		var ParameterRule_AttributeTextFieldDisplay = " " ;
		data.ParameterRule_AttributeTextFieldDisplay = ParameterRule_AttributeTextFieldDisplay;

		var ParameterRule_AttributeTextField = "-1";
		data.ParameterRule_AttributeTextField = ParameterRule_AttributeTextField;

		var ParameterRule_PageTextFieldDisplay = " ";
		data.ParameterRule_PageTextFieldDisplay = ParameterRule_PageTextFieldDisplay;

		var ParameterRule_PageTextField = "-1";
		data.ParameterRule_PageTextField = ParameterRule_PageTextField;		
	}

	else if(Ext.getCmp('RWpar_radio_1').getValue()) {
		var ParameterRule_AttributeTextFieldDisplay = Ext.getCmp('RWpar_f1').getValue();
		data.ParameterRule_AttributeTextFieldDisplay = ParameterRule_AttributeTextFieldDisplay;

		var ParameterRule_AttributeTextField = Ext.getCmp('RWpar_f1_id').getValue();
		data.ParameterRule_AttributeTextField = ParameterRule_AttributeTextField;

		var ParameterRule_PageTextFieldDisplay = Ext.getCmp('RWpar_f1_page').getValue();
		data.ParameterRule_PageTextFieldDisplay = ParameterRule_PageTextFieldDisplay;

		var ParameterRule_PageTextField = Ext.getCmp('RWpar_f1_id_page').getValue();
		data.ParameterRule_PageTextField = ParameterRule_PageTextField;		
	} else if(Ext.getCmp('RWpar_radio_3').getValue()) {
		data.Aspect_AspectTypeCombobox = 'Title';
	}  else if(Ext.getCmp('RWpar_radio_4').getValue()) {
		data.Aspect_AspectTypeCombobox = 'Uri';
	}
	
	var ParameterRule_AttributeTypeRadio = Ext.getCmp('RWpar_f2').getValue();
	if (ParameterRule_AttributeTypeRadio==true){
	    data.ParameterRule_AttributeTypeRadio = "heuristic";   
	}


	else if (Ext.getCmp('RWpar_f3_1').getValue()==true){
	    data.ParameterRule_AttributeTypeRadio = "stringmatch";
	    data.ParameterRule_StringMatchTextField = Ext.getCmp('RWpar_f3_2').getValue();
	    data.ParameterRule_StringMatchNot = (Ext.getCmp('RWpar_f3_3').getValue()?1:0);
	}


	else if (Ext.getCmp('RWpar_f9_1').getValue()==true){
	    data.ParameterRule_AttributeTypeRadio = "gex";
	    data.ParameterRule_StringMatchTextField = Ext.getCmp('RWpar_f9_2').getValue();
	    data.ParameterRule_StringMatchNot = (Ext.getCmp('RWpar_f9_3').getValue()?1:0);
	}


	else if (Ext.getCmp('RWpar_f4_1').getValue()==true){
	    data.ParameterRule_AttributeTypeRadio = "fuzzylength";
	    data.ParameterRule_LengthCombo = (Ext.getCmp('RWpar_f4_2').getValue()=="Is too Short"? "short":(Ext.getCmp('RWpar_f4_2').getValue()=="Is too Long"? "long":"both"));
	}


	else if (Ext.getCmp('RWpar_f5_1').getValue()==true){
	    data.ParameterRule_AttributeTypeRadio = "exactlength";
	    data.ParameterRule_exactlength = Ext.getCmp('RWpar_f5_2').getValue();
	}

	else if (Ext.getCmp('RWpar_f6_1').getValue()==true){
	    data.ParameterRule_AttributeTypeRadio = "rangelength";
	    data.ParameterRule_rangelengthlow = Ext.getCmp('RWpar_f6_2').getValue();
	    data.ParameterRule_rangelengthhigh = Ext.getCmp('RWpar_f6_3').getValue();
	}


	else if (Ext.getCmp('RWpar_f7_1').getValue()==true){
	    data.ParameterRule_AttributeTypeRadio = "distance_similarity";
	    data.ParameterRule_StringSimlarity = Ext.getCmp('RWpar_f7_2').getValue();

	}

	submit_rule_to_server(data);
}

/*
 * Function: submit_aspect_rule
 * 
 * Invoked:
 * 	When clicking "Submit" on the aspect rule window on rule wizard.
 * 	Takes all the fields and invokes submit rule to server
 * 
 * 
 * Parameters:
 *
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function submit_aspect_rule(){

	var data = get_values_from_first_and_second_windows();

	////////////END OF SECOND WINDOW////////
	////////////START OF ASPECT WINDOW////////

	var Aspect_AspectTypeCombobox ;
	var val = Ext.getCmp('RWasp_f1').getValue();
	if (val=="Page Context")
		data.Aspect_AspectTypeCombobox = "entry";
	else if (val=="Query")
		data.Aspect_AspectTypeCombobox = "query";
	else if (val=="Speed")
		data.Aspect_AspectTypeCombobox = "landing";
	else if (val=="Location")
		data.Aspect_AspectTypeCombobox = "geo";
	else if (val=="Day")
		data.Aspect_AspectTypeCombobox = "day";
	else if (val=="Hour")
		data.Aspect_AspectTypeCombobox = "hour";
	else if (val=="Navigation")
		data.Aspect_AspectTypeCombobox = "flow";
	else if (val=="Date")
		data.Aspect_AspectTypeCombobox = "week_day";
	else if (val=="Average")
		data.Aspect_AspectTypeCombobox = "average";

	var Aspect_Personal = Ext.getCmp("RWasp_f4").getValue();
	if (Aspect_Personal){
	    	data.Aspect_Personal = 1;   
	}
	else 
		data.Aspect_Personal = 0;   
	submit_rule_to_server(data);

}

/*
 * Function: submit_aspect_rule
 * 
 * Invoked:
 * 	When clicking "Submit" on rule wizard.
 * 	Saves the data in the server
 * 
 * 
 * Parameters:
 *	data - the object to send to the server
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function submit_rule_to_server(data){
	
	data['mode'] = 'wizard_submit';
	
	telepath.util.request('/rules', data, function (result) {
		
		node_to_display_info = {
			type:'rule',
			rule_id : parseInt(result.rule_id),
			group_id : parseInt(result.group_id),
			category_id : parseInt(result.category_id)
		};
			
		if(Ext.WindowManager.get("RuleAdded")==undefined) {
			var win = Ext.create('MyApp.view.RuleAdded');
			win.show();
			setTimeout(function(){win.hide();},3000);
		}
		else {
			Ext.WindowManager.get("RuleAdded").show();
			setTimeout(function(){Ext.WindowManager.get("RuleAdded").hide();},3000);
		}

		Ext.getCmp('RulesWizard_asp').hide();
		Ext.getCmp('RulesWizard_geo').hide();
		Ext.getCmp('RulesWizard_param').hide();
		Ext.getCmp('RulesWizard_pat').hide();
		Ext.getCmp('RulesWizard_win1').show();
		Ext.WindowManager.get('RulesWizard').destroy();
		Ext.getCmp('main_panel').enable();
		telepath.rules.fetchData();
		
	}, function(data) {
	
		if(data && data.success === false) {
			Ext.Msg.alert('Rule Wizard', data.error_message);
		} else {
			Ext.MessageBox.alert('Rule Wizard', 'Error in rule wizard submit.');
		}
		
		//window_failure=Ext.getCmp('ruleEdited_failure');
		//if (window_failure==undefined) { window_failure = Ext.create('MyApp.view.ruleEdited_failure'); }
		//window_failure.show();
		//setTimeout(function(){window_failure.destroy();},1500);
	
	});

}

/*
 * Function: rw_category_window_next
 * 
 * Invoked:
 * 	When clicking next on the category window in rule wizard
 * 
 * 
 * Parameters:
 *	
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function rw_category_window_next(){
	var valid= (((Ext.getCmp('RW1_new_category_cb').getValue() && Ext.getCmp('RW1_new_category_name').isValid()) ||
	(Ext.getCmp('RW1_exists_category_cb').getValue() && Ext.getCmp('RW1_exists_category_name').isValid())) & 
	(Ext.getCmp('RW1_f1').getValue() && Ext.getCmp('RW1_f2').isValid()) ||
	(Ext.getCmp('RW1_f12').getValue() && Ext.getCmp('RW1_f13').isValid()))


	if (!valid)
	return;
	//if create new group
	else if (Ext.getCmp('RW1_f1').getValue())
	{
		
		telepath.util.request('/rules', { mode:"check_group_duplication", group_name:Ext.getCmp('RW1_f2').getValue() }, function (data) { 
			
			if (!data.exists) {
				Ext.getCmp('RulesWizard_category').hide();
				Ext.getCmp('RulesWizard_win1').show();
				Ext.getCmp('RulesWizard_win1').setTitle("Step 2/4 - Rule Information")
				Ext.getCmp('RulesWizard_win2').setTitle("Step 3/4 - Criterion Information")
				Ext.getCmp('RulesWizard_param').setTitle("Step 4/4 - Parameter Criterion")
				Ext.getCmp('RulesWizard_pat').setTitle("Step 4/4 - Pattern Criterion")
				Ext.getCmp('RulesWizard_asp').setTitle("Step 4/4 - Behavior Criterion")
				Ext.getCmp('RulesWizard_geo').setTitle("Step 4/4 - Geographic Criterion")
				Ext.getCmp('RulesWizard_bot').setTitle("Step 4/4 - Bot Criterion")					
			} else {
				Ext.Msg.alert('Info', 'Rule name already exists. Please select a different name.');
				return;					
			}
			
		}, 'Error checking rule name duplication.');
	
	}

	else if (Ext.getCmp('RW1_f12').getValue())

	{
		Ext.getCmp('RulesWizard_category').hide();
		Ext.getCmp('RulesWizard_win2').show();
		Ext.getCmp('RulesWizard_win1').setTitle("Step 2/3 - Rule Inforamtion")
		Ext.getCmp('RulesWizard_win2').setTitle("Step 2/3 - Criterion Information")
		Ext.getCmp('RulesWizard_param').setTitle("Step 3/3 - Parameter Criterion")
		Ext.getCmp('RulesWizard_pat').setTitle("Step 3/3 - Pattern Criterion")
		Ext.getCmp('RulesWizard_asp').setTitle("Step 3/3 - Behavior Criterion")
		Ext.getCmp('RulesWizard_geo').setTitle("Step 3/3 - Geographic Criterion")
		Ext.getCmp('RulesWizard_bot').setTitle("Step 3/3 - Bot Criterion")    
	}
}
/*
 * Function: rw_group_window_next
 * 
 * Invoked:
 * 	When clicking next on the group window in rule wizard
 * 
 * 
 * Parameters:
 *	
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function rw_group_window_next(){
	if (Ext.getCmp('RW1_f2').isValid() & Ext.getCmp('RW1_f3').isValid() & !(Ext.getCmp('RW1_f7').getValue() && !Ext.getCmp('b_rwEmailField').getStore().getCount()))
	{
		Ext.getCmp('RulesWizard_win1').hide();
		Ext.getCmp('RulesWizard_win2').show();
	}

	if(Ext.getCmp('RW1_f7').getValue() && !Ext.getCmp('b_rwEmailField').getStore().getCount()){
	    	Ext.Msg.alert('Info', 'Please insert at least one email address');
	}
	Ext.getCmp('b_rwEmailField_field').clearInvalid()	
}
/*
 * Function: rw_rule_window_next
 * 
 * Invoked:
 * 	When clicking next on the rule window in rule wizard
 * 
 * 
 * Parameters:
 *	
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function rw_rule_window_next(){
	if (Ext.getCmp('RW2_f1').isValid() &
		Ext.getCmp('RW2_f2').isValid() &
		Ext.getCmp('RW2_f3').isValid() &
		!(Ext.getCmp('RW2_f5_1').getValue() && !Ext.getCmp('RW2_f5_1_2').isValid()) &//if cb is checked, field must be selected
		!(Ext.getCmp('RW2_f5_2').getValue() && !Ext.getCmp('RW2_f5_2_2').isValid()) &//if cb is checked, field must be selected
		!(Ext.getCmp('RW2_f5_3').getValue() && !Ext.getCmp('RW2_f5_3_2').isValid()) &//if cb is checked, field must be selected
		!(Ext.getCmp('RWrule_score_type1').getValue()=='Numeric' && (Ext.getCmp('RWrule_score_type2').getValue()<0 || Ext.getCmp('RWrule_score_type2').getValue()>100))
	)
	{
		Ext.getCmp('RulesWizard_win2').hide();
			var type = Ext.getCmp('RW2_f4').getValue();
		if(type=="Parameter") {
			Ext.getCmp('RulesWizard_param').show();
			Ext.getCmp('RWpar_radio_1').setValue(true);
		}
			
		if(type=="Pattern")
			Ext.getCmp('RulesWizard_pat').show();
		if(type=="Behavior")
			Ext.getCmp('RulesWizard_asp').show();
		if(type=="Geographic")
			Ext.getCmp('RulesWizard_geo').show();
		if(type=="Bot-Intelligence")
			Ext.getCmp('RulesWizard_bot').show();
	}	

}


/*
 * Function: changeAbility
 * 
 * Invoked:
 * 	When we want to toggle all the fields in the rule/group editor from readOnly on regular mode to editable on edit mode
 * 	The function also grays out the the panel on readOnly mode or whitens on edit mode
 * 
 * 
 * Parameters:
 *	bool - true if we move to regular mode (Will set all to readOnly and gray out), false if we move to edit mode(Will set all to editable and whiten the panel)
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */

function changeAbility(bool,type){

	bool = true;

	if (type=="group"){
		
		/*if(bool) {
			Ext.getCmp('b_groupInfoPanel').enable();
		} else {
			Ext.getCmp('b_groupInfoPanel').disable();
		}*/
		
		Ext.getCmp('b_rgiName').setReadOnly(!bool);
		Ext.getCmp('b_rgiDescription').setReadOnly(!bool);
		Ext.getCmp('b_rgiLogCB').setReadOnly(!bool);
		Ext.getCmp('b_rgiEmailCB').setReadOnly(!bool);
		Ext.getCmp('b_rgiSyslogCB').setReadOnly(!bool);
		Ext.getCmp('b_rgiFlowCB').setReadOnly(!bool);//never editable
		Ext.getCmp('b_rgiFlowField').setReadOnly(!bool);//never editable
		Ext.getCmp('b_rgiEmailField_field').setReadOnly(!bool);
		Ext.getCmp('b_rgiEmailField_field').clearInvalid();
		(bool?Ext.getCmp('b_ri_alert_add').enable():Ext.getCmp('b_ri_alert_add').disable());
		(bool?Ext.getCmp('b_ri_alert_params').enable():Ext.getCmp('b_ri_alert_params').disable());
	}
	else if (type=="rule"){
				
		/*if(bool) {
			Ext.getCmp('b_ruleInfoPanel').enable();
		} else {
			Ext.getCmp('b_ruleInfoPanel').disable();
		}*/
		
		Ext.getCmp('b_riName').setReadOnly(!bool);
		Ext.getCmp('b_riDesc').setReadOnly(!bool);
		Ext.getCmp('b_riOwner').setReadOnly(!bool);
		Ext.getCmp('b_riStartIndex').setReadOnly(!bool);
		Ext.getCmp('b_riAppearance').setReadOnly(!bool);
		Ext.getCmp('b_riRuleType').setReadOnly(!bool);
		Ext.getCmp('b_riAppCB').setReadOnly(!bool);
		Ext.getCmp('b_riAppField').setReadOnly(!bool);
		Ext.getCmp('b_riIpCB').setReadOnly(!bool);
		Ext.getCmp('b_riIpField').setReadOnly(!bool);
		Ext.getCmp('b_riUserCB').setReadOnly(!bool);
		Ext.getCmp('b_riUserField').setReadOnly(!bool);
		Ext.getCmp('b_riRuleEnabled').setReadOnly(!bool);
		Ext.getCmp('b_riScoreType1').setReadOnly(!bool);
		Ext.getCmp('b_riScoreType2').setReadOnly(!bool);
		Ext.getCmp('b_riScoreType3').setReadOnly(!bool);





		//Parameter Rules


		Ext.getCmp('b_riAttributeField_P').setReadOnly(!bool);
		Ext.getCmp('b_riHeuristicCB_P').setReadOnly(!bool);
		Ext.getCmp('b_riStringCB_P').setReadOnly(!bool);
		Ext.getCmp('b_riStringField_P').setReadOnly(!bool);
		Ext.getCmp('b_riStringNotCB_P').setReadOnly(!bool);
		Ext.getCmp('b_riRegexCB_P').setReadOnly(!bool);
		Ext.getCmp('b_riRegexField_P').setReadOnly(!bool);
		Ext.getCmp('b_riRegexNotCB_P').setReadOnly(!bool);  
		Ext.getCmp('b_riFuzzyCB_P').setReadOnly(!bool);
		Ext.getCmp('b_riFuzzyField_P').setReadOnly(!bool);
		Ext.getCmp('b_riCharLengthCB_P').setReadOnly(!bool);
		Ext.getCmp('b_riCharLengthField_P').setReadOnly(!bool);
		Ext.getCmp('b_riLengthField1_P').setReadOnly(!bool);
		Ext.getCmp('b_riLengthField2_P').setReadOnly(!bool);
		Ext.getCmp('b_riLengthCB_P').setReadOnly(!bool);
		Ext.getCmp('b_riSimilarCB_P').setReadOnly(!bool);
		Ext.getCmp('b_riSimilarField_P').setReadOnly(!bool);
		Ext.getCmp('b_riPageField_par').setReadOnly(!bool);
		if( bool && Ext.getCmp('b_par_radio_1').getValue()){
			Ext.getCmp('b_riBrowse').enable();
			Ext.getCmp('b_riPageField_browse_par').enable();

		}
		else{
			Ext.getCmp('b_riBrowse').disable();
			Ext.getCmp('b_riPageField_browse_par').disable();
		}
		if( bool && Ext.getCmp('b_par_radio_2').getValue()){
			Ext.getCmp('b_par_post_cb').enable();
			Ext.getCmp('b_par_get_cb').enable();
			Ext.getCmp('b_par_header_cb').enable();
		}
		else{
			Ext.getCmp('b_par_post_cb').disable();
			Ext.getCmp('b_par_get_cb').disable();			
			Ext.getCmp('b_par_header_cb').disable();			
		}
		//////Aspect Rules
		Ext.getCmp('b_riPersonal_A').setReadOnly(!bool);
		Ext.getCmp('b_riAspectType_A').setReadOnly(!bool);


		//PATTERN RULE
		Ext.getCmp('b_ri_page_radio').setReadOnly(!bool);
		Ext.getCmp('b_ri_parameter_radio').setReadOnly(!bool);
		Ext.getCmp('b_riIp_Pat').setReadOnly(!bool);
		Ext.getCmp('b_riSid_Pat').setReadOnly(!bool);
		Ext.getCmp('b_riOther_Pat').setReadOnly(!bool);
		Ext.getCmp('b_riUser_Pat').setReadOnly(!bool);
		Ext.getCmp('b_riCount_Pat').setReadOnly(!bool);
		Ext.getCmp('b_riTime1_Pat').setReadOnly(!bool);
		Ext.getCmp('b_riTime2_Pat').setReadOnly(!bool);
		Ext.getCmp('b_riPageField_pat').setReadOnly(!bool);    

		if( bool)
			Ext.getCmp('b_riDynamicBrowser_Pat').enable();

		else
			Ext.getCmp('b_riDynamicBrowser_Pat').disable();

		if (bool  && Ext.getCmp('b_riOther_Pat').getValue())
			Ext.getCmp('b_riOtherBrowser_Pat').enable();
		if (!bool)
			Ext.getCmp('b_riOtherBrowser_Pat').disable();

		//Geo rules
		Ext.getCmp("b_riVelocity1_geo").setReadOnly(!bool);
		Ext.getCmp("b_riVelocity2_geo").setReadOnly(!bool);
		Ext.getCmp("b_riVelocity3_geo").setReadOnly(!bool);

		Ext.getCmp("b_riAllowCountry1_geo").setReadOnly(!bool);

		if( bool){
			if (Ext.getCmp("b_riAllowCountry1_geo").getValue())
				Ext.getCmp('b_riAllowCont').enable();
			if (Ext.getCmp("b_riBlockCountry1_geo").getValue())
				Ext.getCmp('b_riBlockCont').enable();
		}
		else{
			Ext.getCmp('b_riBlockCont').disable();
			Ext.getCmp('b_riAllowCont').disable();
		}
		Ext.getCmp("b_riBlockCountry1_geo").setReadOnly(!bool);

	}
	
}

/*
 * Function: select_parameter_if_needed
 * 
 * Invoked:
 * 	On the rules editor window, whenever we check the "Enable" button.
 * 	If the rule is a pattern rule, or a parameter rule , and does not have a parameter we open the parameters rule and let him choose one.
 * 
 * 
 * Parameters:
 *	newValue - true/1 if the checkbox is checked
 *
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */
function select_parameter_if_needed(newValue){

	// I don't like this code, switching it off (Andy)
	return;
	var open_window = false;
	if (newValue){
	    if (Ext.getCmp('b_riRuleType').getValue()=='Pattern' && Ext.getCmp('b_riDynamic_Pat').getValue()){

			telepath.rules.openParameters("Pattern_Other_View");
		
	    }
	    else if (Ext.getCmp('b_riRuleType').getValue()=='Parameter' && Ext.getCmp('b_riAttributeField_P_id').getValue()){
		
			telepath.rules.openParameters("Parameter_View_Attribute");
			    
		}

	}	
}