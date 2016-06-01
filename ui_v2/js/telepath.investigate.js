var investigateJSON = {};
var changeAttribute = false;
var showNextAttribute = false;
var showPrevAttribute = false;
var showSearchWindow = false;
var getCount=1;
var g_last_investigate_params = {}
var g_sort_field;
var g_sort_dir;
var canceled_query=false;
var investigate_server_timeout = 30000;
var i_cell_num_selected;//updates on each item click on the investigate table
var i_selected_record;//updates on each item click on the investigate table
var i_selected_index;


telepath.investigate = {
	notFilters: ['user','useragent','sid','ip','city','country','action','method'],
	scaleChange: function(newValue) {
		
		var Date1 = new Date(new Date().getTime() + (60 * 60 * 24 * 1000));
		var Date2 = new Date();
		
		Ext.getCmp('i_fromHour').setValue(date_format('H:i'));
		Ext.getCmp('i_toHour').setValue(date_format('H:i'));
		
		switch(newValue) {
			case 'Year':
				Date2.setFullYear(Date2.getFullYear() - 1);
			break;
			case 'Month':
				Date2.setMonth(Date2.getMonth() - 1);
			break;
			case 'Week':
				Date2.setDate(Date2.getDate() - 7);
			break;
			case 'Day':
				Date2.setDate(Date2.getDate() - 1);
			break;
			case 'Hour':
				Date2.setHours(Date2.getHours() - 1);
				Ext.getCmp('i_fromHour').setValue(date_format('H:i', Date2));
			break;
			case 'Now':
				Date2.setHours(Date2.getHours() - 1);
				Ext.getCmp('i_fromHour').setValue(date_format('H:i', Date2));
			break;
			case 'Custom Range':
				
			break;
		}
		
		Ext.getCmp('i_startDate').setValue(Date2);
		Ext.getCmp('i_endDate').setValue(Date1);

	},
	refreshDisplayDetails: function () {
		openAttributesWin(i_selected_record);
	},
	browse_att: function () {
		
		sendTo= "Investigate_Attribute";
		selection_state = "parameter";

		telepath.parameters.show(false);

	},
	init: function () {
		
		// Hide buttons for which the user doesn't have access to
		if(!telepath.access.has_perm('Investigate', 'set')) {
			Ext.getCmp('i_saveProfiles').hide();
		}
		if(!telepath.access.has_perm('Investigate', 'del')) {
			Ext.getCmp('i_deleteProfiles').hide();
		}
		
		$('#i_searchPanel-innerCt').append($('<div>').teleSelect({ constrain: 4, type: 'subdomain', label: 'Application', values: [{ text: 'All', id: '-1', sub_id: '-1' }] }));

		// Init Investigate
		Ext.StoreManager.getByKey('i_profiles2Store').proxy.url = telepath.controllerPath + '/investigate/get_profile_details';
		Ext.StoreManager.getByKey('i_profilesStore').proxy.url  = telepath.controllerPath + '/investigate/get_profile_list';
		Ext.StoreManager.getByKey('i_profilesStore').load();
				
		// Bind doubleclick
		Ext.getCmp('i_suspects').addListener(
			'itemdblclick',
			function(dv, record, item, index, e) {
				openAttributesWin(record);
			}
		);
		
		initHUB();
		
		Ext.getCmp('i_range').setValue('Week');
				
		Ext.getCmp('i_filterAvgTot0').setValue('Average');
		Ext.getCmp('i_filterComparator0').setValue('At Least');
		
		// Custom Expand Tool
		var tool = $('<div>').addClass('x-tool x-box-item x-tool-default x-tool-after-title custom-expand-tool').css({ height: 15, width: 15 });
		var tool_icon = $('<img>').addClass('x-tool-img x-tool-collapse-top').attr('src', 'data:image/gif;base64,R0lGODlhAQABAID/AMDAwAAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==');
		tool.append(tool_icon);
		
		tool.hover(function () { $(this).addClass('x-tool-over'); }, function () { $(this).removeClass('x-tool-over'); }).click(function () {
			
			if($('img', this).hasClass('x-tool-collapse-top')) {
				$('img', this).removeClass('x-tool-collapse-top').addClass('x-tool-collapse-bottom');
				Ext.getCmp('i_filterPanel').collapse();
			} else {
				$('img', this).removeClass('x-tool-collapse-bottom').addClass('x-tool-collapse-top');
				Ext.getCmp('i_filterPanel').expand();
			}
			
		});
		$("#i_suspects_header_hd").append(tool);
		// END Custom Expand Tool
		telepath.investigate.initialized = true;
		
		$(window).keydown(function(e) {
			var rq = Ext.getCmp('RequestAttributes');
			if(rq && rq.isVisible()) {
				switch(e.keyCode) {
					case 37: prevAtt(); break;
					case 39: nextAtt(); break;
					case 27: Ext.WindowManager.get("RequestAttributes").destroy(); break;
				}
			}
		});
		
		// Hook keyboard events for KB nav in request details

		
	
	},
	requestAttributesResize: function(window, width, height, eOpts) {
	
		var RA_Details    = Ext.getCmp('RA_Details');
		var RA_Parameters = Ext.getCmp('RA_Parameters');
		var RA_Scores     = Ext.getCmp('RA_Scores');
		var RA_Next       = Ext.getCmp('RequestAttributes_next');
		var RA_Prev       = Ext.getCmp('RequestAttributes_prev');

		RA_Prev.setPosition(10,80);
		RA_Next.setPosition(width-20-22,80);
		RA_Scores.setPosition(width - RA_Scores.getWidth() - 50, 10);
		RA_Details.setWidth(width - RA_Scores.getWidth() - 100);
		RA_Parameters.setWidth(width - 90);
		RA_Parameters.setHeight(height - 262);
		
		// Small Fix
		$('.x-grid-view', RA_Parameters.body.el.dom).css('overflow-x', 'hidden');
	
	},
	suspectsResize: function(height) {
		
		//Ext.getCmp('i_filterPanel').setHeight(Ext.getCmp('investigatePanel').getHeight() - height);
		
	},
	suspectsContextMenu: function(dataview,record,item,index,e,eOpts) {
		
		var dataIndex = i_selected_index.trim();
		
		var allowAddFilter = ['td1', 'td2', 'td3', 'td4', 'td5', 'td15', 'td16', 'td14', 'td20'];
		
		if($.inArray(dataIndex, allowAddFilter) > -1) {
			Ext.getCmp("i_menu2").enable();     
			Ext.getCmp("i_menu1").enable();     
		} else {
			Ext.getCmp("i_menu2").disable(); 
			Ext.getCmp("i_menu1").disable();     
		}
		
		var menu = Ext.getCmp("i_menu").show();
		e.preventDefault();
		menu.setPosition(e.xy[0],e.xy[1]-20,false);
		menu.show();
		i_selected_record = record;

	},
	requestAttributesFilter: {},
	requestAttributesData: [],
	requestAttributesFilterSet: function() {
		
		var that = this;
		
		var filterTypes = ['get', 'post', 'json', 'xml', 'header'];
		$.each(filterTypes, function(i, filter) {
			that.requestAttributesFilter[filter] = Ext.getCmp('RA_Parameter_filter_' + filter).value;
		});
		
	},
	requestAttributesFilterUpdate: function () {
	
		this.requestAttributesRefresh();
		
	},
	requestAttributesRefresh: function () {
		
		Ext.StoreManager.getByKey('RequestParameters').clearData();
		
		this.requestAttributesFilterSet();
		
		var data   = this.requestAttributesData;
		var filter = this.requestAttributesFilter;
		var that   = this;
		var filteredParams = [];
		
			$.each(data['Params'], function(i, row) {
				
			var new_row = { td0 : decodeURIComponent(row.td0), td1: row.td1, td2: row.td2, type: 'Unknown', display: false, id: row.id, mask: row.mask, alias: row.alias };
			
			switch(row.td3) {
				case 'H':
					new_row.display = that.requestAttributesFilter['header'];
					new_row.type    = 'Header';
				break;
				case 'P':
					new_row.display = that.requestAttributesFilter['post'];
					new_row.type    = 'Post';
				break;
				case 'J':
					new_row.display = that.requestAttributesFilter['json'];
					new_row.type    = 'Json';
				break;
				case 'X':
					new_row.display = that.requestAttributesFilter['xml'];
					new_row.type    = 'XML';
				break;
				case 'G':
					new_row.display = that.requestAttributesFilter['get'];
					new_row.type    = 'Get';
				break;
			}
			
			if(row.td2 >= 70) {
				new_row.td1 = '<b style="color: red;">' + row.td1 + '</b>';
			}
			
			new_row.td2 = row.td2;
			
			if(row.alias != '') {
				new_row.td0 = '<span style="color:green;">' + row.alias + '</span>&nbsp(' + decodeURIComponent(row.td0) + ')';
			}
			if(row.mask == '1') {
				new_row.td0 = new_row.td0 + '&nbsp;<span style="color:red;">*</span>';
			}

			if(new_row.display) {
				filteredParams.push(new_row);
			}
			
		});
		
		if(!data.scoresProcessed) {
			
			$.each(data['Scores'], function (i, row) {
			
				if(row.value >= 70) {
					data['Scores'][i].key   = '<b style="color: red;">' + row.key + '</b>';
				} 
				data['Scores'][i].value = row.value;
			
			});
		
			data.scoresProcessed = true;
			
		}

		Ext.StoreManager.getByKey('RequestAttributes_details').loadData(data['Details']);
		Ext.StoreManager.getByKey('RequestScores').loadData(data['Scores']);
		
		Ext.StoreManager.getByKey('RequestParameters').loadData(filteredParams);
		
	},
	parameterContextMenu: function (dataview, record, item, index, event, eOpts) {
	
		// Param -- Alias Add
		var menu_param_alias_add  = {
			text: 'Add Parameter Alias',
			iconCls: 'context-menu-aliasAdd',
			handler: function () {
				telepath.handlers.param.aliasAdd(record.data.id, telepath.investigate.refreshDisplayDetails);
			}
		}
		// Param -- Alias Remove
		var menu_param_alias_remove  = {
			text: 'Remove Parameter Alias',
			iconCls: 'context-menu-aliasRemove',
			handler: function () {
				telepath.handlers.param.aliasRemove(record.data.id, telepath.investigate.refreshDisplayDetails);
			}
		}
		// Param -- Alias Edit
		var menu_param_alias_edit  = {
			text: 'Edit Parameter Alias',
			iconCls: 'context-menu-aliasEdit',
			handler: function () {
				telepath.handlers.param.aliasEdit(record.data.id, record.data.alias, telepath.investigate.refreshDisplayDetails);
			}
		}
		
		// Context Param Mask
		var menu_param_mask  = {
			text: 'Mask Parameter',
			iconCls: 'context-menu-paramMask',
			handler: function () {
				telepath.handlers.param.maskAdd(record.data.id, record.data.td0, telepath.investigate.refreshDisplayDetails);
			}
		}
		
		// Context Param Un-Mask
		var menu_param_unmask  = {
			text: 'Un-Mask Parameter',
			iconCls: 'context-menu-paramUnMask',
			handler: function () {
				telepath.handlers.param.maskRemove(record.data.id, record.data.td0, telepath.investigate.refreshDisplayDetails);
			}
		}
		
		// Add to investigate filter
		var menu_param_investigate = {
			text: 'Add to Investigate filter',
			iconCls: '',
			handler: function() {
			
				$.each(telepath.investigate.requestAttributesData['Params'], function(i, param) {
					if(param.id == record.data.id) {
						Ext.getCmp('i_filterAtt').setValue(param.td0);
						Ext.getCmp('i_filterAtt_id').setValue(record.data.id);
						Ext.getCmp('i_filterAttValue').setValue(param.td1);
						return false;
					}
				});
				
			}
		}
		
		// Copy to clipboard
		var menu_param_copy = {
			text: 'Copy value to clipboard',
			iconCls: '',
			handler: function () {
				
				$.each(telepath.investigate.requestAttributesData['Params'], function(i, param) {
					if(param.id == record.data.id) {
						Ext.getCmp('i_filterAttValue').setValue(param.td1);
						copy_to_clipboard(param.td1);
						return false;
					}
				});
				
			}
		}

		var menuItems = [];
		
		if(record.data.mask === 0) {
			menuItems.push(menu_param_mask);
		} else {
			menuItems.push(menu_param_unmask);
		}

		menuItems.push({ xtype: 'menuseparator' });
		
		if(record.data.alias == '') {
			menuItems.push(menu_param_alias_add);
		} else {
			menuItems.push(menu_param_alias_edit);
			menuItems.push(menu_param_alias_remove);
		}
		
		menuItems.push({ xtype: 'menuseparator' });
		menuItems.push(menu_param_investigate);
		menuItems.push(menu_param_copy);

		var contextMenu = new Ext.menu.Menu({ items : menuItems });
		contextMenu.showAt(event.getXY());
		event.stopEvent();
	
	},
	parameterMouseEnter: function(dataview, record, item, index, e, eOpts) {
		$.each(telepath.investigate.requestAttributesData['Params'], function(i, param) {
			if(param.id == record.data.id) {
				
				if(telepath.investigate.toolTip) {
					telepath.investigate.toolTip.destroy();
				}	
				
				
				if(param.td2 > 70) {
					
					var html = '';
					
					switch(param.structure) {
						case 'Url':
							if(param.td2 == 100) {
								html = 'This Url was never seen before';
							} else {
								html = 'This Url seems out of context';
							}
						break;
						case 'enum':
							if(param.td2 == 100) {
								html = 'This choise was never selected before';
							} else {
								html = 'This choise seems highly unlikely';
							}
						break;
						case 'text':
							if(param.td2 == 100) {
								html = 'This string appears for the first time';
							} else {
								html = 'This string is out of context';
							}
						break;
						case 'numeric':
							if(param.td2 == 100) {
								html = 'This number is out of range';
							} else {
								html = 'This number is out of range';
							}
						break;
						case 'free':
							if(param.td2 == 100) {
								html = 'This text appears for the first time';
							} else {
								html = 'This text seems out of context';
							}
						break;
					}

					if(html != '') {
						
						telepath.investigate.toolTip = Ext.create('Ext.tip.ToolTip', {
							target: $('td:nth-child(2)', item)[0],
							html: html,
							anchor: 'right',
							dismissDelay: 0,
							showDelay: 0,
							autoHide: false
						});
				
						telepath.investigate.toolTip.show();
					
					}
					
				}

			}
		});
	},
	parameterMouseLeave: function(dataview, record, item, index, e, eOpts) {
		if(telepath.investigate.toolTip) {
			telepath.investigate.toolTip.destroy();
		}
	}
}


/*
 * Function: i_start_progress_bar()
 * 
 * Invoked:
 * 	Starts running the progress bar on the investigate panel
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<investigate_main>
 */
function i_start_progress_bar(){
	var p = Ext.getCmp('i_progress')
	p.wait({
		interval: 50, //bar will move fast!
		duration: investigate_server_timeout,
		increment: 3000,
		animate:true,
		text: 'Searching for Requests...',
		scope: this,
		fn: function(){
			p.updateText('The server got a timeout');
		}
	});
}


/*
 * Function: investigate_main
 * 
 * Invoked:
 * 	Whenever we want to investigate. 
 * 	
 * 
 * 
 * Parameters:
 * 	pagenumber - int, 0 for the first page, the page number we want to get the result from
 * 	params - json. the params to investigate by. If the params parameter is null or undefined, the investigation is made by the search fields.
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<init_search_params>
 * 	<i_disable_buttons>
 * 	<i_enable_buttons>
 * 	<count_investigate_results>
 * 	
 * 	
 */
function investigate_main(pagenumber,params) {
	
	SIDS = {};
	
	i_start_progress_bar();
	canceled_query = false;
	lastPageReached = pagenumber+1;
	var results = Ext.getCmp('i_results').getValue();
	
	if (pagenumber==0){
	    Ext.getCmp('i_currentPage').setValue(pagenumber);
	//    Ext.getCmp('i_currentPageLarge').setValue(pagenumber);
	}
	i_disable_buttons();
	Ext.getCmp("i_searching").show();
	
	
	if(params == undefined || params == "" || params=={} )
	{
		g_last_investigate_params={}
		init_search_params(g_last_investigate_params);
	}
	else
	{
		//clean all previous fields
		Ext.getCmp('i_reset').fireEvent('click')
		//
		set_fields_from_params(params)
		g_last_investigate_params = params;
	}
	g_last_investigate_params['page_num'] = pagenumber;
	g_last_investigate_params['page_rows_number'] = Ext.getCmp('i_results').getValue();
	g_last_investigate_params['mode'] = 'investigate';
	Ext.Ajax.timeout = investigate_server_timeout;
	
	try {
		Ext.getStore('i_suspectsStore').removeAll();
	} catch(e) {}
	
	Ext.Ajax.request({
		url: telepath.controllerPath + '/investigate/get_investigate',
		params:g_last_investigate_params,
		success:function(response){
			if (canceled_query || response.responseText==""){//case of error or cancelled
					Ext.getCmp('i_progress').reset();
				if (canceled_query)
					Ext.getCmp('i_progress').updateProgress(1,"The search has been canceled",true);	
				else
					Ext.getCmp('i_progress').updateProgress(1,"The server has timeout",true);
				setTimeout(function(){
						Ext.getCmp("i_searching").hide();                
					},1000);
				i_enable_buttons();
				return;
				
			}
			var store = Ext.getStore('i_suspectsStore');
			try { store.removeAll(); } catch(e) {}
			var data;
			try{
				data = Ext.decode(response.responseText);				
			}
			catch(err){
				Ext.getCmp('i_progress').reset()
				if (canceled_query)
					Ext.getCmp('i_progress').updateProgress(1,"The search has been canceled",true);	
				else
					Ext.getCmp('i_progress').updateProgress(1,"The server has timeout",true);
				console.log(err.message)
				i_enable_buttons();
				Ext.getCmp("i_searching").hide();
				Ext.Ajax.timeout = 60000;
				return
			}
			
			if(data.success === false) {
			
				Ext.getCmp('i_progress').reset()
				Ext.getCmp('i_progress').updateProgress(1,"Error",true);
				setTimeout(function() {
				
					Ext.getCmp("i_searching").hide();
					showSearchWindow=false;
					
				} ,1000);
				i_enable_buttons(); 	
				return;
				
			}
			
			Ext.getCmp('i_currentPage').setValue(pagenumber+1);
			//Ext.getCmp('i_currentPageLarge').setValue(pagenumber+1);
			
			var results = data['items'];
			
			// For coloring info
			var uniqueSID = [];
			
			// For Show/Hide Workflow (always show?)
			var has_flow_data = false;
			
			// Reformat Data
			$.each(results, function (i, row) {
			
				// Date formatter
				results[i]['td14'] = date_format('d/m/y H:i:s', results[i]['td14']);
				
				// Flag, country, city formatter
				results[i]['td5']  = '<div><span class="flag flag-' + results[i]['td5'].toLowerCase() + '"></span>&nbsp;&nbsp;' 
									 + aliasToName[results[i]['td5']] + (results[i]['td4'] != 'Unknown' ? ', ' + results[i]['td4'] : '') + '</div>';
				
				if(results[i]['td23'] && results[i]['td23'] != '') {
					has_flow_data = true;
				}
				
			});
			
			try {
			if(has_flow_data) {
				Ext.getCmp('investigate_flow').show();
			} else {
				Ext.getCmp('investigate_flow').hide();
			}
			} catch (e) {
				console.log(e);
			}
	
			try {
				store.loadData(results);
			} catch (e) {}
				
			Ext.getCmp('i_progress').reset()
			Ext.getCmp('i_progress').updateProgress(1,"Done!",true);
			setTimeout(function(){
			
		    	Ext.getCmp("i_searching").hide();
		    	showSearchWindow=false;
				
			},1000);
		    	
			if (getCount==0){
				i_enable_buttons();      
			}
			else 
				count_investigate_results(g_last_investigate_params,pagenumber)
	    		getCount=1;
		},	
		failure:function(xhr, ajaxOptions, thrownError){
			Ext.getCmp('i_progress').reset()
			if (canceled_query)
				Ext.getCmp('i_progress').updateProgress(1,"The search has been canceled",true);	
			else
				Ext.getCmp('i_progress').updateProgress(1,"The server has timeout",true);	
			i_enable_buttons();
			Ext.getCmp("i_searching").hide();
			getCount=1;//raise count flag for next time
			Ext.Ajax.timeout = 60000;
		}
	});
}


/*
 * Function: cancel_query
 * 
 * Invoked:
 * 	when the user clicks the cancel button. the function kills the queries on the server.
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<investigate_main>
 * 	<init_search_params>
 * 	<i_disable_buttons>
 * 	<i_enable_buttons>
 * 	<count_investigate_results>
 */
function cancel_query(){
	canceled_query = true;
	Ext.Ajax.request({
		url: telepath.controllerPath + '/investigate/get_investigate_cancel',
		success:function(response){
			Ext.getCmp('i_progress').reset();
			Ext.getCmp('i_progress').updateProgress(1,"Search Cancelled!",true);
			setTimeout(function(){
					Ext.getCmp("i_searching").hide();                
				},1000);
			i_enable_buttons();
			
			/* DEBUG CODE */
			try {
				
				var jsondata = Ext.decode(response.responseText);
				if(jsondata.queries && jsondata.queries.length > 0) {
					
					var description = '';
					$.each(jsondata.queries, function (i, query) {
						description += "Stopped Query: \n" + query + "\n";
					});
					
					var debugWindow = new Ext.Window({
					modal: true, height: 250, width: 500, draggable: true, closable: true, 
					closeAction: 'destroy', title: 'Debug', padding: '10', layout: 'fit',
					items: [{
						xtype: 'textarea',
						value: description,
					}]}).show();
					
				}
			
			} catch(e) {}
			/* END DEBUG CODE */
			
			return;			
		},
		failure:function(){
			Ext.getCmp('i_progress').reset();
			Ext.getCmp('i_progress').updateProgress(1,"Search Cancelled!",true);
			setTimeout(function(){
					Ext.getCmp("i_searching").hide();                
				},1000);
			i_enable_buttons();
			return;				
		}
	})
}


/*
 * Function: count_investigate_results
 * 
 * Invoked:
 * 	only from investigate_main. after we get the results, we want to count how many of them we have. the count is slow, so we make it separately from the select query.
 * 
 * Parameters:
 * 	pagenumber - int, 0 for the first page, the page number we want to get the result from
 * 	params - json. the params to investigate by. If the params parameter is null or undefined, the investigation is made by the search fields.
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<investigate_main>
 */
function count_investigate_results(params,pagenumber) {
	
	var countParams = { only_count: true };
	
	jQuery.extend(countParams,params);
	
	if(getCount==1){
		Ext.getCmp('i_totalPages').setValue("Updating");
		Ext.getCmp('i_totalResults').setValue("Updating");
		var results_per_page = Ext.getCmp('i_results').getValue();
		Ext.Ajax.request({
			url: telepath.controllerPath + '/investigate/get_investigate',
			params: countParams,
			success:function(response){
    				if (canceled_query || response.responseText==""){
					Ext.getCmp('i_totalPages').setValue(0);
					Ext.getCmp('i_currentPage').setValue(0);
					Ext.getCmp('i_totalReqs').setValue(0); 
					Ext.getCmp('i_totalResults').setValue(0);
					i_enable_buttons();					
					Ext.getCmp("i_searching").hide();
					return;
				}
				var store = Ext.getCmp('i_suspects').getStore();				
				var jsondata; 
				try{
					jsondata = Ext.decode(response.responseText);					
				}
				catch(err){
					//if(!canceled_query)
					//	Ext.Msg.alert('Error #666', 'Error while getting requests numbers');
					if (canceled_query) {
						Ext.getCmp('i_progress').reset();
						Ext.getCmp('i_progress').updateProgress(1,"The search has been canceled",true);	
					}
						
					Ext.getCmp("i_searching").hide();
					return;					
				}
				if (jsondata['total']==-1){//interrupted bby cancel
					return;
				}
				if (jsondata['total']==0)
					Ext.getCmp('i_currentPage').setValue(0);
				else
					Ext.getCmp('i_currentPage').setValue(pagenumber+1);
				Ext.getCmp('i_totalReqs').setValue(jsondata['total']);
				Ext.getCmp('i_totalResults').setValue(jsondata['total']);
				pages = parseInt(jsondata['total']/results_per_page)+1;
				if (jsondata['total']%results_per_page==0)
					pages--;   
				Ext.getCmp('i_totalPages').setValue(pages);
				//Ext.getCmp('i_totalPagesLarge').setValue(pages);	    			
				i_enable_buttons();
	    			getCount=1;
			},
			failure:function(response){
	    			getCount=1;
			}
		});
	}
}

/*
 * Function: set_fields_from_params
 * 
 * Invoked:
 * 	Sets all the search filters from a given json.
 * 
 * Parameters:
 * 	p - the object by which to set all search fields
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<investigate_main>
 */
function set_fields_from_params(p){
	if (p['fromdate']){
		Ext.getCmp('i_startDate').setValue(new Date(p['fromdate']*1000))
		
	}
	if (p['todate']){
		Ext.getCmp('i_endDate').setValue(new Date(p['todate']*1000))
		
	}
	if(p['app_id']){
		
		$.each(Ext.StoreManager.getByKey('d_appsStore').data.items, function(i, item) {
			if(item.data.id == p['app_id']) {
				$('#i_searchPanel .tele-multi input:last').val(item.data.app);
			}
		});
		
	}
	if(p['user']){
		Ext.getCmp("i_filterUser").setValue(p['user'])
	}
	if(p['SID']){
		Ext.getCmp("i_filterSession").setValue(p['SID'])
	}
	if(p['user_ip']){
		Ext.getCmp("i_filterIp").setValue(p['user_ip'])
	}
	if(p['user_agent']){
		Ext.getCmp("i_filterUserAgent").setValue(p['user_agent'])
	}		
	if(p['path']){
		Ext.getCmp("i_filterPage").setValue(p['path'])
		Ext.getCmp("i_filterPageCriterion").reset()
	}
	if(p['country']){
		Ext.getCmp("i_filterCountry").setValue(p['country'])
	}
	if(p['city']){
		Ext.getCmp("i_filterCity").setValue(p['city'])
	}

	if(p['flow']){
		var record = Ext.getCmp('i_filterFlow').getStore().find('id',p['flow'],0,false,false,true)
		Ext.getCmp('i_filterFlow').setValue(record)
	}
	if(p['method']){
		Ext.getCmp("i_filterMethod").setValue(p['method'])
	}
	if(p['attribute_filter'] && p['attribute_filter_val'] && p['attribute_filter_name']){
		Ext.getCmp("i_filterAttValue").setValue(p['attribute_filter_val'])
		Ext.getCmp("i_filterAtt_id").setValue(p['attribute_filter'])
		Ext.getCmp("i_filterAtt").setValue(p['attribute_filter_name'])
	}



	

}
/*
 * Function: init_search_params
 * 
 * Invoked:
 * 	Prepares the search object to send to the server as the investigate parameters
 * 
 * Parameters:
 * 	params - the object to prepare to send to the server
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<investigate_main>
 */
function init_search_params(params){
	params['sort_field'] = 'td14'
	params['sort_order'] = 'DESC'
	g_sort_field='td14';
	g_sort_dir='DESC';
	
	params['fromdate'] =  Ext.getCmp('i_startDate').getValue().getTime()/1000;
	params['fromdate'] += Ext.getCmp('i_fromHour').getValue().getHours()*60*60;
	params['fromdate'] += Ext.getCmp('i_fromHour').getValue().getMinutes()*60;
	params['fromdate'] += Ext.getCmp('i_fromHour').getValue().getSeconds();
	
	params['todate'] =  Ext.getCmp('i_endDate').getValue().getTime()/1000;
	params['todate'] += Ext.getCmp('i_toHour').getValue().getHours()*60*60;
	params['todate'] += Ext.getCmp('i_toHour').getValue().getMinutes()*60;
	params['todate'] += Ext.getCmp('i_toHour').getValue().getSeconds();
	
	params['selected_apps'] = JSON.stringify($('#i_searchPanel .tele-multi').data('tele-tele-select').result());

	//Scores Filter
	var score_query ; 
	var comparator;
	var prev;
	var j = 0;
	for (var i=0;i<=criteriaNum;i++) {
	
	    if (Ext.getCmp('i_filterAvgTot'+i)==undefined)//check if exists
	    continue;
	    if (Ext.getCmp('i_andOr'+i)!=undefined){
		prev = j-1;
		params['relation'+prev+'_'+j]=Ext.getCmp('i_andOr'+i).rawValue;
	    }
	    score_query = Ext.getCmp('i_filterAvgTot'+i).getValue();
	    params['score_field'+j]= (score_query=="Average"? 'avg_score' :
								 (score_query=="Query"? 'query_score' :
								 (score_query=="Speed"? 'request_score' :
								 (score_query=="Flow"? 'flow_score' :
								 (score_query=="Landing Speed"? 'landing_normal' :
								 (score_query=="Geographic Location"? 'geo_normal' :
								 (score_query=="Day"? 'week_day_score' :'hour_score' )))))));
					
	    comparator = Ext.getCmp('i_filterComparator'+i).getValue();
	    params['score_field_type'+j]= 	(comparator=="Greater Than"? 'more' :
	    					(comparator=="Less Than"? 'less' :
						(comparator=="Is Exactly"? 'equals':
						(comparator) )));
	    
	    params['score_field_num'+j]= Ext.getCmp('i_filterScore'+i).getValue();
	    j++;//j counts only the consecutive numbers
		
	}
	params['criteriaNum']=j;
	//
	var att = Ext.getCmp("i_filterAtt").getValue();
	var att_id = Ext.getCmp("i_filterAtt_id").getValue();
	var att_val = Ext.getCmp("i_filterAttValue").getValue();
	if (att!=undefined & att!=null & att!=""){
		params['attribute_filter']=att_id;
		if (att_val!=null & att_val!="")
			params['attribute_filter_val']=att_val;
	}
	if (Ext.getCmp("i_filterUser").getValue()) 
		params["user"] = Ext.getCmp("i_filterUser").getValue()
	
	if (Ext.getCmp("i_filterSession").getValue()) 
		params["SID"] = Ext.getCmp("i_filterSession").getValue()

	if (Ext.getCmp("i_filterIp").getValue()) 
		params["user_ip"] = Ext.getCmp("i_filterIp").getValue()

	if (Ext.getCmp("i_filterUserAgent").getValue()) 
		params["user_agent"] = Ext.getCmp("i_filterUserAgent").getValue()

	if (Ext.getCmp("i_filterPage").getValue()){
		params["path"] = Ext.getCmp("i_filterPage").getValue()
		params["path_mode"] = Ext.getCmp("i_filterPageCriterion").getValue()
	}
	
	if (Ext.getCmp("i_filterCountry").getValue()) {
		var country = Ext.getCmp("i_filterCountry").getValue();
		if(country.length > 2) {
			country = nameToAlias[country];
		}
		params["country"] = country;
	}
	
	if (Ext.getCmp("i_filterCity").getValue()) {
		params["city"] = Ext.getCmp("i_filterCity").getValue();
	}

	if (Ext.getCmp('i_filterFlow').getValue()) {
		params['flow'] = Ext.getCmp('i_filterFlow').valueModels[0].data.id;
	}

	if (Ext.getCmp("i_filterMethod").getValue()) {
		params["method"] = Ext.getCmp("i_filterMethod").getValue();
	}
	
	if (Ext.getCmp("i_session_alerts")) {
		params["session_alerts"] = Ext.getCmp("i_session_alerts").value ? '1' : '0';
	}
	
	// Now collect all NOT flags
	$.each(telepath.investigate.notFilters, function(i, filter) {
		if(Ext.getCmp('i_' + filter + '_not')) {
			params[filter + "_not"] = Ext.getCmp('i_' + filter + '_not').value ? '1' : '0';
		}
	});
	
}
/*
 * Function: i_disable_buttons
 * 
 * Invoked:
 * 	Disables search reset and cancel buttons
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<investigate_main>
 */
function i_disable_buttons(){
	//Ext.getCmp('i_search').disable();	
	//Ext.getCmp('i_reset').disable(); 
	Ext.getCmp('i_cancel').enable();
}

/*
 * Function: i_enable_buttons
 * 
 * Invoked:
 * 	Enables search reset and cancel buttons
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<investigate_main>
 */
function i_enable_buttons(){
	Ext.getCmp('i_search').enable();	
	Ext.getCmp('i_reset').enable();
	Ext.getCmp('i_cancel').disable();	
}




/*
 * Function: add_countries_to_search_panel
 * 
 * Invoked:
 * 	creates the country search field and adds it to the panel
 *  
 * 
 * Parameters:
 * 	abstractcomponent - extjs panel object. the panel to add the combobox to
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function add_countries_to_search_panel(abstractcomponent){
	var a = Ext.create('Ext.form.field.ComboBox',{
		xtype: 'combobox',
		x: 270,
		y: 80,
		id: 'i_filterCountry',
		style: 'margin-left:50px;',
		width: 250,
		fieldLabel: 'Country',
		labelSeparator: ' ',
		labelWidth: 120,
		enableKeyEvents: true,
		matchFieldWidth: false,
		displayField:'Country',
		store: 'Countries',
		queryMode: 'local',
		typeAhead:true,
		listConfig: {
			getInnerTpl: function() {
				
				var tpl = '<div><span class="flag flag-{Alias}"></span>&nbsp;&nbsp;{Country}</div>';
				return tpl;
				
			}
		}
	}
	);
	
	var b = new Ext.form.Checkbox({
		xtype: 'checkboxfield',
		x: 420,
		y: 80,
		id: 'i_country_not',
		fieldLabel: '',
		boxLabel: ''
	});
	
	abstractcomponent.add(a);
	abstractcomponent.add(b);
	
	a.addListener('change',function(field,newValue){
		var src = Ext.getCmp("i_current").getSource();
		var att = Ext.getCmp('i_filterCountry').getValue();
		src["Country"] = newValue;
		Ext.getCmp("i_current").setSource(src);
	})
}

function setPanelHeight(height){
	// Deprecated
}

/*
 * Function: updateChangefromHUB
 * 
 * Invoked:
 * 	When the user changes some value on the HUB, it changes the value on the corresponding field on the search filter
 * 
 * Parameters:
 * 	all params are passed from the default select event
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<upload_new_app>
 */
function updateChangefromHUB(src,recordId,value,oldValue,options){
	if (recordId == "Start Date"){
		try{
			Ext.getCmp("i_startDate").setValue(new Date(value));
		}
		catch(err){
			alert("Invalid Date Format");
			Ext.getCmp("i_startDate").setValue(new Date(oldValue));	
			src[recordId] = oldValue;			
		}
	}
	else if (recordId == "End Date"){
		try{
			Ext.getCmp("i_endDate").setValue(new Date(value));
		}
		catch(err){
			alert("Invalid Date Format");
			Ext.getCmp("i_endDate").setValue(new Date(oldValue));
			src[recordId] = oldValue;				
		}
	}
	else if (recordId == "Parameter"){
		src[recordId] = oldValue;
		if (changeAttribute){
			Ext.getCmp("i_current").setSource(src);
			changeAttribute	 = false;
		}
	}
	else if (recordId == "City"){
		Ext.getCmp("i_filterCity").setValue(value);
	}
	else if (recordId == "Country"){
		Ext.getCmp("i_filterCountry").setValue(value);
	}
	else if (recordId == "IP"){
		Ext.getCmp("i_filterIp").setValue(value);
	}
	else if (recordId == "Page"){
		Ext.getCmp("i_filterPage").setValue(value);
	}
	else if (recordId == "SID"){
		Ext.getCmp("i_filterSession").setValue(value);
	}	
	else if (recordId == "User"){
		Ext.getCmp("i_filterUser").setValue(value);
	}
	else if (recordId == "User-Agent"){
		Ext.getCmp("i_filterUserAgent").setValue(value);
	}
	else if (recordId == "Score Category"){
		src[recordId] = oldValue;
		Ext.getCmp("i_current").setSource(src);
	}
	else if (recordId == "Method"){
		var method_valid = false;
		Ext.getCmp("i_filterMethod").getStore().each(function(record){
			if (record.get("method")==value){
				method_valid = true
				return
			}				
		})
		if (method_valid){
			Ext.getCmp("i_filterMethod").setValue(value);	
		}
		else{
			src[recordId] = oldValue;
			Ext.getCmp("i_current").setSource(src);			
		}

		
	}


}


/*
 * Function: initHUB
 * 
 * Invoked:
 * 	For the first time the invetigate runs. sets the initial fields on the HUB with their default values
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function initHUB(){
	
	var selected_apps = [];
	$('#i_searchPanel .tele-multi input').each(function () {
		selected_apps.push($(this).attr('dataID'));
	});
	// If all apps is selected, ignore other options
	var all_apps = false;
	$.each(selected_apps, function(i, app) {
		if(app == '-1') { all_apps = true; }
	});
	if(all_apps) { selected_apps = ['-1']; }
	

	investigateJSON["Start Date"] = new Date(new Date().getTime() - (60 * 60 * 24 * 1000));
	investigateJSON["End Date"]   = new Date(new Date().getTime() + (60 * 60 * 24 * 1000));
	investigateJSON["Start Hour"] = Ext.getCmp('i_fromHour').getValue().getHours()+":"+(Ext.getCmp('i_fromHour').getValue().getMinutes()<10?'0':'') + Ext.getCmp('i_fromHour').getValue().getMinutes();
	investigateJSON["End Hour"] = (Ext.getCmp('i_toHour').getValue().getHours()) +":"+(Ext.getCmp('i_toHour').getValue().getMinutes()<10?'0':'') + Ext.getCmp('i_toHour').getValue().getMinutes();
	investigateJSON["Application"] = selected_apps.join(',');
	investigateJSON["Score Criteria"] = Ext.getCmp("i_filterAvgTot0").getValue()+" "+Ext.getCmp("i_filterComparator0").getValue()+" "+Ext.getCmp("i_filterScore0").getValue()+"%" ;
	investigateJSON["User"] = "";
	investigateJSON["User-Agent"] = "";
	investigateJSON["Page"] = "";
	investigateJSON["SID"] = "";
	investigateJSON["City"] = "";
	investigateJSON["Country"] = "";
	investigateJSON["Parameter"] = "";
	investigateJSON["IP"] = "";
	investigateJSON["Method"] = "";
	investigateJSON["Score Category"] = "";
	Ext.getCmp("i_current").setSource(investigateJSON);
}
var investMenu = Ext.create('Ext.menu.Menu',{
	xtype: 'menu',
	floating: true,
	hidden: true,
	id: 'i_menu',
	hideMode:'display',//crucial to fix black squares bug in chrome
	width: 175,
	dock: 'left',
	items: [
	{
	    xtype: 'menuitem',
	    id: 'i_menu1',
	    text: 'Investigate value'
	},
	{
	    xtype: 'menuitem',
	    id: 'i_menu2',
	    text: 'Add value to filter'
	},
	{
	    xtype: 'menuitem',
	    id: 'i_menu4',
	    text: 'Display request'
	},
	{
	    xtype: 'menuitem',
	    id: 'i_menu5',
	    text: 'Show session alerts'
	},
	{
	    xtype: 'menuitem',
	    id: 'i_menu7',
	    text: 'Add IP to whitelist'
	},
	{
	    xtype: 'menuitem',
	    id: 'i_menu8',
	    text: 'Copy to clipboard'
	}
	]
});




Ext.getCmp('i_menu1').addListener(
		'click',
		function(){
			addValueToFilter()
			investigate_main(0, null);
		}
);

Ext.getCmp('i_menu2').addListener(
		'click',
		function(that,  record,  index,  eOpts){
			addValueToFilter();			
		}
);


Ext.getCmp('i_menu4').addListener(
		'click',
		function(that,  record,  index,  eOpts){
			openAttributesWin(i_selected_record);
		}
);


Ext.getCmp('i_menu5').addListener(
		'click',
		function(that,  record,  index,  eOpts){
			showAlertsBySID(i_selected_record.get('td3'));
		}
);


Ext.getCmp('i_menu7').addListener(
		'click',
		function(that,  record,  index,  eOpts) {
			var ip = Ext.getCmp('i_suspects').getSelectionModel().getSelection()[0].data.td2
			
			telepath.ipconfig.show(ip, 'Add IP to Whitelist', function (ip) {
				telepath.ipconfig.add_ip_to_whitelist(ip);
			});

		}
);

Ext.getCmp('i_menu8').addListener(
		'click',
		function(){
			var text = i_selected_record.get(i_selected_index);

			if(i_selected_index == 'td5') {
				text = strip(text).trim();
			}
			copy_to_clipboard(text);
		}
);

var gRid;
var gRequestSession;
var gPage;
var gDomain;

/*
 * Function: addValueToFilter
 * 
 * Invoked:
 * 	When the use right clicks the request table and selects add value to filter. The function adds the value to the right field
 * 
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
function addValueToFilter() {
	
	var dataIndex = i_selected_index;
	
	switch(dataIndex) {
		
		case 'td1': // USER
		
			Ext.getCmp("i_filterUser").setValue(i_selected_record.get(dataIndex));
			
		break;
		case 'td2': // IP
		
			Ext.getCmp("i_filterIp").setValue(i_selected_record.get(dataIndex));
			
		break;
		case 'td3': // SESSION
		
			Ext.getCmp("i_filterSession").setValue(i_selected_record.get(dataIndex));
			
		break;
		case 'td5': // COUNTRY + CITY
		
			var html = i_selected_record.get(dataIndex);
			var data = html.substr(html.indexOf('</span>') + 7).replace(/&nbsp\;/g,'').replace('</div>', '').split(',');
			Ext.getCmp("i_filterCountry").setValue(data[0].trim());
			Ext.getCmp("i_filterCity").setValue(data[1] ? data[1].trim() : ''); // UNDEF
			
		break;
		case 'td14': // DATE RANGE (+-1 Hour)
		
			var date =  date_fromtext(i_selected_record.get(dataIndex));
		
			date.setHours(date.getHours() - 1);
			
			Ext.getCmp('i_startDate').setValue(date);
			Ext.getCmp('i_fromHour').setValue(date.getHours() + ":" + date.getMinutes());
			
			date.setHours(date.getHours() + 2);
			
			Ext.getCmp('i_toHour').setValue(date.getHours() + ":" + date.getMinutes());
			Ext.getCmp('i_endDate').setValue(date);
		
		break;
		case 'td15': // APPLICATION
		
			var app_name = i_selected_record.get(dataIndex);
			var app_id   = i_selected_record.raw.td24.split(',');
			$('#i_searchPanel .tele-multi').remove();
			$('#i_searchPanel-innerCt').append($('<div>').teleSelect({ constrain: 4, type: 'subdomain', label: 'Application', values: [{ text: app_name, id: app_id[0], sub_id: app_id[1] ? app_id[1] : '-1' }] }));
			
		break;
		case 'td16': // PATH
			
			var path = i_selected_record.get(dataIndex)
			var page;
			if (path=="/"){
				page = path
				Ext.getCmp("i_filterPage").setValue(page);
			}
			else if (path){
				path = path.split("/")
				if (path.length)
					page =  path[path.length-1]
				Ext.getCmp("i_filterPage").setValue(page);
			}
			
		break;
		case 'td20': // METHOD
		
			Ext.getCmp("i_filterMethod").setValue(i_selected_record.get(dataIndex));
			
		break;
	}

}

////////////////////////////////////////////////////////Attribute Window/////////////////////////////////////////

/*
 * Function: highlight_row
 * 
 * Invoked:
 * 	When the user is on the request parameters window and pages, the function will be invoked and highlight the row on the investigate table
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<openAttributesWin>
 */
function highlight_row(record){
	var view = Ext.getCmp('i_suspects').getView()
	Ext.getCmp('i_suspects').getStore().each(function(record_itr){
		var a = view.getNode(record_itr)
		//if its our record
		if (record_itr.id == record.id){
			view.addRowCls( record_itr, 'x-grid-row-selected x-grid-row-focused' )
			//$(a).toggleClass('x-grid-row x-grid-row-selected x-grid-row-focused')			
		}
		else{
			view.removeRowCls( record_itr, 'x-grid-row-selected x-grid-row-focused' )
		}
			
	})
}
/*
 * Function: openAttributesWin
 * 
 * Invoked:
 * 	Whenever we want to show the parameters and info of a row on the request table on the request details window.
 * 
 * 
 * Parameters:
 * 	record - record object of extjs, the record that we need to show the attributes for on the investigate table
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<highlight_row>
 * 	<getAttributesById>
 */
function openAttributesWin(record){
	highlight_row(record);
	var prefix = '';
	i_selected_record = record;
	var date_num = new Date(record.get('td14'));
	gRequestDate = (date_num.getTime() / 1000);
	gRequestSession = record.get('td3');
	gPage = record.get('td16');
	if (gPage==null)
		gPage='';
	gDomain = record.get('td15');
	gRid = record.get('td18');
	var window = Ext.WindowManager.get('RequestAttributes');
	if (!window) {
		window = Ext.create('MyApp.view.RequestAttributes');
	}
	if (window.hidden)
		window.show()
	if (gDomain=="None" | !gDomain)
		Ext.getCmp('RequestAttributes_url').setValue('<div align="center" style="color:red;">&nbsp;&nbsp;Application Deleted - No URL Available</div>')
	else
		Ext.getCmp('RequestAttributes_url').setValue("<a href='http://"+gDomain+gPage+"' target='_blank'>"+'http://'+gDomain+gPage+"</a>");
	Ext.getCmp("RequestAttributes_prev").disable();
	Ext.getCmp("RequestAttributes_next").disable();
	getAttributesById(gRid,gRequestDate);
	
	
	// Bind doubleclick
	Ext.getCmp('RA_Parameters').addListener(
		'itemdblclick',
		function(dv, record, item, index, e) {
			Ext.getCmp('a_sessionDetails').expand();
			select_row(record);
		}
	);
	
}
/*
 * Function: getAttributesById
 * 
 * Invoked:
 * 	When we want to get the parameters of a request from the request table and show it on the request parameters window
 * 
 * 
 * Parameters:
 * 	rid - int, the rid of the request from the request table
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 *	<openAttributesWin>
 */
function getAttributesById(rid, rdate){

	Ext.Ajax.request({
		url: telepath.controllerPath + '/investigate/get_params_for_request',
		params:{
			'RID':rid,
			'date':rdate
		},
		success:function(response){
			try{
				var data = Ext.decode(response.responseText)
			}
			catch(err){
				Ext.Msg.alert('Error #602', 'Error while getting request parameters');
				Ext.getCmp("RequestAttributes_prev").enable();
				Ext.getCmp("RequestAttributes_next").enable();
				return;							
			}			
			
			telepath.investigate.requestAttributesData = data;
			telepath.investigate.requestAttributesRefresh();
			
			
			
			Ext.getCmp("RequestAttributes_prev").enable();
			Ext.getCmp("RequestAttributes_next").enable();
		}	


	});
	

}

//Ext.getCmp('RequestAttributes_next').addListener(
//	'click',

/*
 * Function: nextAtt
 * 
 * Invoked:
 * 	When the user clicks right arrow on the keyboard, or the right arrow button on the request attributes window
 * 	The function goes on the next row on the investigate table and gets its data to the request attributes window
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<openAttributesWin>
 * 
 */
function nextAtt(){
	var store = Ext.getStore('i_suspectsStore');
	var index = store.indexOf(i_selected_record);
	var total = store.getCount();
	var record = store.getAt((index+1)%total);
	openAttributesWin(record);
	highlight_row(record);
}
//);
/*
 * Function: prevAtt
 * 
 * Invoked:
 * 	When the user clicks left arrow on the keyboard, or the left arrow button on the request attributes window
 * 	The function goes on the previous row on the investigate table and gets its data to the request attributes window
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<openAttributesWin>
 * 
 */
function prevAtt(){
	var store = Ext.getStore('i_suspectsStore');
	var index = store.indexOf(i_selected_record);
	var total = store.getCount();

	if ((index-1)%total<0){//bug fix in javascript for modulo 
		while ((index-1)%total<0)
			index+=total;
	}	
	var record = store.getAt((index-1)%total);
	openAttributesWin(record);
	highlight_row(record);
	
}


/*
 * Function: copyToClipboard
 * 
 * Invoked:
 * 	When the user clicks copy to clipboard on any menu on the UI
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
var input = document.createElement('input');
function copyToClipboard( text )
{
	//var input = document.getElementById( 'url' );
	input.value = text;
	input.focus();
	input.select();
	document.execCommand( 'Copy' );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////Search Criteria and/or///////////////////////////////////
/*
 * Function: loadCriteria
 * 
 * Invoked:
 * 	Used to take a string "average score larger than 20 " and feed it into the search fields
 * 
 * Parameters:
 * 	value - string , the string to load, such as "average score larger than 20 "
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<addToFilter>
 */
function loadCriteria(value){
	//first remove all existing 
	for (var i=1;i<criteriaNum;i++){
		if (Ext.getCmp('i_criterion'+(i))!=undefined)
			Ext.getCmp('i_criterion'+(i)).destroy();
		updatePanelSize("sub");
		criteriaCurrent--;		
	}
	var words = value.split(" ");
	var clauses = [];
	var clause="";
	for (i=0;i<words.length;i++){
		if (words[i]==""){
			clauses.push(clause);	
			clause="";					
			continue;
		}
		else if (words[i]=='Or' | words[i]=='And'){
			clause += words[i];
//			clause = clause.substring(0,clause.length-1);
			clauses.push(clause);	
			clause="";
			continue;
		}
		else{
			clause+=words[i]+" ";
		}
	}

	//adding first criterion-always applys
	for (i=0;i<clauses.length;i++){
		clauses[i]=clauses[i].split(" ");
	}
	var firstCriterion = clauses[0];//.split(" ");
	if (firstCriterion.length>5){//case of 2 words for Geo Location\Landing speed
		Ext.getCmp('i_filterAvgTot0').setValue(firstCriterion[0]+" "+firstCriterion[1]);
		Ext.getCmp('i_filterComparator0').setValue(firstCriterion[2]+" "+firstCriterion[3]);
		Ext.getCmp('i_filterScore0').setValue(firstCriterion[4]);				
	}
	else{//other cases
		Ext.getCmp('i_filterAvgTot0').setValue(firstCriterion[0]);
		Ext.getCmp('i_filterComparator0').setValue(firstCriterion[1]+" "+firstCriterion[2]);
		Ext.getCmp('i_filterScore0').setValue(firstCriterion[3]);		
	}	
	//finished
	//adding other criteria if exist
	for (i=1;i<clauses.length;i++){
		addSearchCriterion();//add to gui
		//Start setting its fields according to other
		Ext.getCmp('i_andOr'+(i)).setValue(clauses[i-1][(clauses[i-1].length>5 ?5:4)]);
		if (clauses[i].length>5){//case of 2 words for Geo Location\Landing speed
			Ext.getCmp('i_filterAvgTot'+i).setValue(clauses[i][0]+" "+clauses[i][1]);
			Ext.getCmp('i_filterComparator'+i).setValue(clauses[i][2]+" "+clauses[i][3]);
			Ext.getCmp('i_filterScore'+i).setValue(clauses[i][4]);				
		}
		else{//other cases
			Ext.getCmp('i_filterAvgTot'+i).setValue(clauses[i][0]);
			Ext.getCmp('i_filterComparator'+i).setValue(clauses[i][1]+" "+clauses[i][2]);
			Ext.getCmp('i_filterScore'+i).setValue(clauses[i][3]);		
		}				
	}

}

/*
 * Function: selectProfile
 * 
 * Invoked:
 * 	When the user selects a save profile on the profile table. the function loads the saved info on the right most tbale
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function selectProfile(selModel,record,index,options){
		var store = Ext.getCmp('i_profiles2').getStore();
		
		store.load({
			params:{
				id:record.get('id')
			},
			callback: function(records, operation, success) {
				// the operation object
				// contains all of the details of the load operation
				try{
					hidden_profile_fields = Ext.decode(operation.response.responseText)['hidden']
				}
				catch(err){
					hidden_profile_fields = {}
					Ext.Msg.alert('Error #620', 'Error while getting saved profile information');
					return;						
				}
			}
		})
}




/*
 * Function: SaveProfile
 * 
 * Invoked:
 * 	When the user clicks save profile on the save profile window
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function SaveProfile()
{

	var selected_apps = [];
	$('#i_searchPanel .tele-multi input').each(function () {
		selected_apps.push($(this).attr('dataID'));
	});
	// If all apps is selected, ignore other options
	var all_apps = false;
	$.each(selected_apps, function(i, app) {
		if(app == '-1') { all_apps = true; }
	});
	if(all_apps) { selected_apps = ['-1']; }

	var params = {};
	//params.name = Ext.getCmp("profileName").getValue();
	params['Start Date'] = Ext.getCmp("i_startDate").getValue().getTime();
	params['End Date'] = Ext.getCmp("i_endDate").getValue().getTime();
	params['Application'] = selected_apps.join(',');
	params['User'] = Ext.getCmp('i_filterUser').getValue();
	params['SID'] = Ext.getCmp('i_filterSession').getValue();
	params['IP'] = Ext.getCmp('i_filterIp').getValue();
	params['User Agent'] = Ext.getCmp('i_filterUserAgent').getValue();
	params['Method'] = Ext.getCmp('i_filterMethod').getValue();
	params['Country'] = Ext.getCmp('i_filterCountry').getValue();
	params['City']= Ext.getCmp('i_filterCity').getValue();
	params['Start Hour'] = Ext.getCmp('i_fromHour').getValue().getTime();
	params['End Hour'] = Ext.getCmp('i_toHour').getValue().getTime();
	params['Workflow Group']= ((Ext.getCmp('i_filterFlow').valueModels && Ext.getCmp('i_filterFlow').valueModels.length)?Ext.getCmp('i_filterFlow').valueModels[0].data.id:Ext.getCmp('i_filterFlow').getValue());
	params['Page'] = Ext.getCmp('i_filterPage').getValue();
	params['Page Criteria'] = Ext.getCmp('i_filterPageCriterion').getValue();
	params['Parameter'] = Ext.getCmp('i_filterAtt_id').getValue()
	params['Parameter Value'] = Ext.getCmp('i_filterAttValue').getValue()
	var total = criteriaNum;
	var ans = Ext.getCmp('i_filterAvgTot0').getValue()+" "+ Ext.getCmp('i_filterComparator0').getValue()+" "+Ext.getCmp('i_filterScore0').getValue()+" ";
	for (var i=1;i<total;i++){
		if (Ext.getCmp('i_filterAvgTot'+i)!=null | Ext.getCmp('i_filterAvgTot'+i)!=undefined){
			ans+= Ext.getCmp('i_andOr'+i).getValue()
			ans+=" "+Ext.getCmp('i_filterAvgTot'+i).getValue()
			ans+=" "+Ext.getCmp('i_filterComparator'+i).getValue()
			ans+=" "+ Ext.getCmp('i_filterScore'+i).getValue()+" ";
		}
	}
	params['Score'] = ans;			
	params['Profile Name'] = Ext.getCmp('profileName').getValue();

	Ext.getCmp('profileName').setValue("");
	
	Ext.Ajax.request({
	
		url: telepath.controllerPath + "/investigate/set_profile",
		params:params,
		success: function(response){
			loadProfileList();
			Ext.getCmp('i_searchProfiles').expand()
		},
		failure:function(xhr, ajaxOptions, thrownError){

		}
		
	});	
}
/*
 * Function: loadSearchFilter
 * 
 * Invoked:
 * 	When the user clicks the Load Profile button on the saved profile table
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<addToFilter>
 */
var hidden_profile_fields;
function loadSearchFilter(){
	var store = Ext.getCmp('i_profiles2').getStore()
	resetFields();
	store.each(function(record){
		addToFilter(record);	
	});
}
/*
 * Function: addToFilter
 * 
 * Invoked:
 * 	Gets a record from the saved profile fields table and loads its info to the relevant field on the search filter
 * 
 * Parameters:
 * 	val - the value to add
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<loadCriteria>
 */
function addToFilter(val){
	var val = val.data;
	var field = val.desc.substring(0,val.desc.indexOf(':'));
	if (field =="Application") {
		// TODO:: Remove current tele-multi
		// TODO:: Load application name/ids to tele-multi	
	}
	else if (field=="Start Date"){
		Ext.getCmp('i_startDate').setValue(new Date(parseInt(val.desc.substring(val.desc.indexOf(':')+2))));
	}
	else if (field=="Start Hour"){
		Ext.getCmp('i_fromHour').setValue(new Date(parseInt(val.desc.substring(val.desc.indexOf(':')+2))))
	}		
	else if (field=="End Date"){
		Ext.getCmp('i_endDate').setValue(new Date(parseInt(val.desc.substring(val.desc.indexOf(':')+2))));
	}
	else if (field=="End Hour"){
		Ext.getCmp('i_toHour').setValue(new Date(parseInt(val.desc.substring(val.desc.indexOf(':')+2))))
	}
	else if (field=="SID"){
		Ext.getCmp('i_filterSession').setValue(val.desc.substring(val.desc.indexOf(':')+2));
	}	
	else if (field=="User Agent"){
		Ext.getCmp('i_filterUserAgent').setValue(val.desc.substring(val.desc.indexOf(':')+2));
	}
	else if (field=="User"){
		Ext.getCmp('i_filterUser').setValue(val.desc.substring(val.desc.indexOf(':')+2));
	}		
	else if (field=="IP"){
		Ext.getCmp('i_filterIp').setValue(val.desc.substring(val.desc.indexOf(':')+2));
	}	
	else if (field=="Country"){
		Ext.getCmp('i_filterCountry').setValue(val.desc.substring(val.desc.indexOf(':')+2));
	}
	else if (field=="City"){
		Ext.getCmp('i_filterCity').setValue(val.desc.substring(val.desc.indexOf(':')+2));
	}
	else if (field=="Page"){
		Ext.getCmp('i_filterPage').setValue(val.desc.substring(val.desc.indexOf(':')+2));
		//Ext.getCmp('i_filterPage_id').setValue(hidden_profile_fields['Page ID']);
	}
	else if (field=="Page Criterion"){
		Ext.getCmp('i_filterPageCriterion').setValue(val.desc.substring(val.desc.indexOf(':')+2));
	}
	else if (field=="Workflow Group"){
		var wf_id = hidden_profile_fields['Workflow ID']
		var wf_record = Ext.getCmp('i_filterFlow').getStore().findRecord('id',wf_id)
		if (wf_record)
			Ext.getCmp('i_filterFlow').setValue(val.desc.substring(val.desc.indexOf(':')+2));
	}
	else if (field=="Method"){
		Ext.getCmp('i_filterMethod').setValue(val.desc.substring(val.desc.indexOf(':')+2));
	}
	else if (field=="Parameter"){
		Ext.getCmp('i_filterAtt').setValue(val.desc.substring(val.desc.indexOf(':')+2));
		Ext.getCmp('i_filterAtt_id').setValue(hidden_profile_fields['Parameter ID']);
	}
	else if (field=="Parameter Value"){
		Ext.getCmp('i_filterAttValue').setValue(val.desc.substring(val.desc.indexOf(':')+2));		
	}
	else if(field=='Score'){
  		loadCriteria(val.desc.substring(val.desc.indexOf(':')+2));
	}


	
}








/*
 * Function: delete_search_profile
 * 
 * Invoked:
 * 	when the user clicks the "trash" icon next to a saved profile record or the delete button
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function delete_search_profile(){
	var value = Ext.getCmp("i_profiles").getSelectionModel().getSelection();
	for (var i=0;i<value.length;i++){
	    	if (value!=undefined | value.length!=0){    
			id = value[i].get('id');
			Ext.Ajax.request({
				url: telepath.controllerPath + "/investigate/del_profile",
		    		params:{
		        		id:id
		    		},
		    		success:function(response){
					try{
						var data = Ext.decode(response.responseText)
					}
					catch(err){
						Ext.Msg.alert('Error #601', 'Error while deleting profile');
						return;							
					}		    			
		    		Ext.getCmp('i_profiles').getStore().loadData(data['items']);
					Ext.getCmp('i_profiles2').getStore().removeAll();
					Ext.getCmp('i_deleteProfiles').disable();
					Ext.getCmp('i_loadProfiles').disable();
		    		}
			});

		}

	}
}








var selected_row;
var selected_col;
var attMenu = Ext.create('Ext.menu.Menu', {
//	width:100,
	hideMode:'display',//crucial to fix black squares bug in chrome		      
	floating: true,
	hidden: true,
	id: 'att_menu',
	itemId: 'att_menu',
	items: [
		{
			xtype: 'menuitem',
			id: 'att_menu1',
			text: 'Investigate value'
		},
		{
			xtype: 'menuitem',
			id: 'att_menu2',
			text: 'Add value to filter'
		},
		{
			xtype: 'menuitem',
			id: 'att_menu3',
			text: 'Add alias'
		},
		{
		    xtype: 'menuitem',
		    id: 'att_menu4',
		    text: 'Copy to clipboard'
		}

	]
});


Ext.getCmp('att_menu1').addListener(
	'click',
	function(){
		var params = {}
		Ext.getCmp("i_filterAtt").setValue(selected_row.get('td0'));
		Ext.getCmp("i_filterAtt_id").setValue(selected_row.get('id'));
		Ext.getCmp("i_filterAttValue").setValue(selected_row.get('td1'));
		params["attribute_filter"]=selected_row.get('id');
		params["attribute_filter_val"]=selected_row.get('td1');
		investigate_main(0,params);
		Ext.WindowManager.get('RequestAttributes').hide();
	}
);




Ext.getCmp('att_menu2').addListener(
	'click',
	function(){
		Ext.getCmp("i_filterAtt").setValue(selected_row.get('td0'));
		Ext.getCmp("i_filterAtt_id").setValue(selected_row.get('id'));
		Ext.getCmp("i_filterAttValue").setValue(selected_row.get('td1'));
		Ext.WindowManager.get('RequestAttributes').hide();
	}
);

Ext.getCmp('att_menu4').addListener(
	'click',
	function(){
		var text = selected_row.get('td1');
		copy_to_clipboard(text);
	}
);

/*
 * Function: on_hover_parameters_table
 * 
 * Invoked:
 * 	When we hover a row on the request attributes table. If the value is too large, we show a balloon with the full value
 * 
 * 
 * Parameters:
 * 	all params as they arrive from mouse over event on the table
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
var i_hovering_request_attributes_balloon = false;
var requestAttributes_balloon;
function on_hover_parameters_table(dataview,record,item,index,e,col_id){
	var row_value = record.data.td1
	if (!row_value || row_value.length<100)
		return;
	//requestAttributes_balloon = Ext.WindowManager.get('RequestAttributes_balloon')
	var row_height = $(dataview.getNode(0)).height()
	var column_width = Ext.getCmp(col_id).getWidth()
	var col_pos = Ext.getCmp(col_id).getPosition()
	var x_offset = col_pos[0]
	var y_offset = row_height*(index+1)+col_pos[1]+1;
	requestAttributes_balloon.setPosition(x_offset,y_offset)
	requestAttributes_balloon.setWidth(column_width)
	requestAttributes_balloon.setHeight(row_height*5)
	requestAttributes_balloon.show()
	Ext.getCmp('RequestAttributes_balloon_label').setText(row_value)
}

//var i_parameter_row_selected;
var i_parameter_table_selected;

Ext.getCmp('att_menu3').addListener('click',function(){
	attMenu.hide();
	if(i_parameter_table_selected=='RA_Headers')
		Ext.getCmp('RA_Headers').getPlugin('i_par_headers_row_editor').startEdit(selected_row,0)
	else if(i_parameter_table_selected=='RA_Gets')
		Ext.getCmp('RA_Gets').getPlugin('i_par_get_row_editor').startEdit(selected_row,0)
	else if(i_parameter_table_selected=='RA_Posts')
		Ext.getCmp('RA_Posts').getPlugin('i_par_post_row_editor').startEdit(selected_row,0)
})

/*
 * Function: resetFields
 * 
 * Invoked:
 * 	When the user clicks the reset button on the search filter. the function resets all search fields
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function resetFields(){

	Ext.getCmp('i_filterUserAgent').reset();
	Ext.getCmp('i_filterSession').reset();
	Ext.getCmp('i_filterPage').reset();
	Ext.getCmp('i_filterIp').reset();
	Ext.getCmp('i_filterUser').reset();
	Ext.getCmp('i_filterAvgTot0').setValue('Average');
	Ext.getCmp('i_filterComparator0').setValue('At Least');
	Ext.getCmp('i_filterScore0').reset();
	Ext.getCmp('i_endDate').setValue(new Date(new Date().getTime() + (60 * 60 * 24  * 1000)));
	Ext.getCmp('i_startDate').setValue(new Date((new Date().getTime() - (60 * 60 * 24 * 7 * 1000))));
	Ext.getCmp('i_range').setValue('Week');
	Ext.getCmp('i_fromHour').reset();
	Ext.getCmp('i_toHour').reset();
	Ext.getCmp('i_filterCountry').reset();
	Ext.getCmp('i_filterCity').reset();
	Ext.getCmp('i_filterFlow').reset();
	Ext.getCmp('i_filterMethod').reset();
	Ext.getCmp('i_filterAtt_remove').fireEvent('click');
	for(var i=1;i<criteriaNum;i++){
		if (Ext.getCmp('i_criterion'+i)!=undefined)
			Ext.getCmp('i_criterion'+(i)).destroy();
		updatePanelSize("sub");
		criteriaCurrent--;
	}
	criteriaNum = 1;
	criteriaCurrent=1;
	
	// Reset some more flags .. 
	Ext.getCmp("i_session_alerts").reset();
	$.each(telepath.investigate.notFilters, function(i, filter) {
		Ext.getCmp('i_' + filter + '_not').reset();
	});
	
	Ext.getCmp('i_filterAtt_remove').fireEvent('click');

}

/*
 * Function: open_request_attributes_menu
 * 
 * Invoked:
 * 	Right click handler for the request attributes window
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function open_request_attributes_menu(tablepanel,record,item,index,e,options){
	e.preventDefault();
	attMenu.setPosition(e.xy[0],e.xy[1]-20,false);
	attMenu.show();
	selected_col = index;
	selected_row = record;	
}


/*
 * Function: showAlertsBySID
 * 
 * Invoked:
 * 	When the user selects to show session alerts on the right click menu for the investigate table
 * 	
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<get_alerts>
 * 	
 */
function showAlertsBySID(sid) {

	Ext.getCmp('alertsPanel').show();
	get_alerts("SID",sid);
	
}
////////////////////////////////////////////////////////Investigate Paging/////////////////////////////////////////
var lastPageReached;


/*
 * Function: i_getFirst
 * 
 * Invoked:
 * 	When the user clicks the paging to first page button
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function i_getFirst(){
	lastPageReached = 1;
	var curr = parseInt(Ext.getCmp('i_currentPage').getValue());
	var totals = parseInt(Ext.getCmp('i_totalPages').getValue());
	var results = Ext.getCmp("i_results").getValue()
	if(totals >0 & curr!=1){
		getCount=0;
		investigate_main(0,g_last_investigate_params);
		//Ext.getCmp('i_currentPage').setValue(1);
	}
}
/*
 * Function: i_getPrev
 * 
 * Invoked:
 * 	When the user clicks the paging to the previous page button
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function i_getPrev(){
	var curr = parseInt(Ext.getCmp('i_currentPage').getValue());
	var totals = parseInt(Ext.getCmp('i_totalPages').getValue());
	var results = Ext.getCmp("i_results").getValue()
	if(totals>1 & curr>1){
		getCount=0;
		investigate_main(parseInt(curr)-2,g_last_investigate_params);
		//Ext.getCmp('i_currentPage').setValue(curr-1);
		lastPageReached = curr-1;
	}
}

/*
 * Function: i_getNext
 * 
 * Invoked:
 * 	When the user clicks the paging to the next page button
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function i_getNext(){
	var curr = parseInt(Ext.getCmp('i_currentPage').getValue());
	var totals = parseInt(Ext.getCmp('i_totalPages').getValue());
	var results = Ext.getCmp("i_results").getValue()
	if(totals>curr){
		getCount=0;
		investigate_main(parseInt(curr),g_last_investigate_params);
		//Ext.getCmp('i_currentPage').setValue(curr+1);
		lastPageReached = curr+1;
	}
}
/*
 * Function: i_getLast
 * 
 * Invoked:
 * 	When the user clicks the paging to the last page button
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function i_getLast(){
	var curr = parseInt(Ext.getCmp('i_currentPage').getValue());
	var totals = parseInt(Ext.getCmp('i_totalPages').getValue());
	lastPageReached = totals;
	var results = Ext.getCmp("i_results").getValue();
	var remaining = totals%4;
	if(totals!= curr){
		getCount=0;
		//in any other case we need to call it from the server
		investigate_main(parseInt(totals-1),g_last_investigate_params);
		//Ext.getCmp('i_currentPage').setValue(totals);
	}
}

/*
 * Function: i_getLast
 * 
 * Invoked:
 * 	When the user enters a page in the current page field
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<investigate_main>
 * 
 */
function getPageAt(page){
	var results = Ext.getCmp("i_results").getValue();
	if (lastPageReached==page)
		return;
	getCount=0;
	investigate_main(Ext.getCmp('i_currentPage').getValue()-1,g_last_investigate_params);		


}
/*
 * Function: sort_investigate_results
 * 
 * Invoked:
 * 	When the user clicks sort on one of the columns in the table
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	<investigate_menu>
 */
function sort_investigate_results(column,direction){
	if (g_sort_field==column.dataIndex && g_sort_dir==direction)
		return;
	getCount = 0;
	g_sort_field=column.dataIndex;
	g_sort_dir=direction;
	g_last_investigate_params['sort_field'] = column.dataIndex
	g_last_investigate_params['sort_order'] = direction
	//if (column.dataIndex='td14')
	//	Ext.getCmp('i_suspects').getStore().sort(column.dataIndex,direction)
	if ((Ext.getCmp('i_currentPage').getValue()-1)>=0){
		investigate_main(Ext.getCmp('i_currentPage').getValue()-1,g_last_investigate_params);	
	}
}

var criteriaNum = 1;//counts how many have been created overall (to not repeat )
var criteriaCurrent = 1;//counts how many now exist, decreased on deletion
/*
 * Function: addSearchCriterion
 * 
 * Invoked:
 * 	When the user clicks the + button next to the search score criterion.The function creates another score criterion field
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	
 */
function addSearchCriterion(){
	Ext.define('MyApp.view.i_criterion'+criteriaNum, {
	    extend: 'Ext.container.Container',
	    height: 55,
	    id: 'i_criterion'+criteriaNum,
	    width: 628,
	    layout: {
		type: 'absolute'
	    },

	    initComponent: function() {
		var me = this;

		Ext.applyIf(me, {
		    items: [
		        {
		            xtype: 'combobox',
		            id: 'i_andOr'+criteriaNum,
		            width: 60,
		            value: 'And',
		            fieldLabel: '',
		            editable: false,
		            displayField: 'type',
		            queryMode: 'local',
		            store: 'i_andOrStore',
		            valueField: '',
		            x: 470,
		            y: 0,
		            listeners: {
		                change: {
		                    fn: me.onI_andOrChange,
		                    scope: me
		                }
		            }
		        },
		        {
		            xtype: 'combobox',
		            id: 'i_filterAvgTot'+criteriaNum,
		            width: 113,
		            value: 'Average',
		            fieldLabel: '',
		            editable: false,
		            displayField: 'field',
		            queryMode: 'local',
		            store: 'i_filterscoreTypeStore',
		            x: 10,
		            y: 20,
		            listeners: {
		                change: {
		                    fn: me.onI_filterAvgTotChange,
		                    scope: me
		                },
		                afterrender: {
		                    fn: me.onI_filterAvgTotAfterRender,
		                    scope: me
		                }
		            }
		        },
		        {
		            xtype: 'combobox',
		            id: 'i_filterComparator'+criteriaNum,
		            width: 160,
		            value: 'At Least',
		            editable: false,
		            displayField: 'field',
		            queryMode: 'local',
		            store: 'i_filterComparatorStore',
		            x: 130,
		            y: 20,
		            listeners: {
		                change: {
		                    fn: me.onI_filterComparatorChange,
		                    scope: me
		                },
		                afterrender: {
		                    fn: me.onI_filterComparatorAfterRender,
		                    scope: me
		                }
		            }
		        },
		        {
		            xtype: 'numberfield',
		            id: 'i_filterScore'+criteriaNum,
		            width: 80,
		            value: 0,
		            fieldLabel: '',
		            maxValue: 100,
		            minValue: 0,
		            step: 5,
		            x: 300,
		            y: 20,
		            listeners: {
		                change: {
		                    fn: me.onI_filterScoreChange,
		                    scope: me
		                },
        	                afterrender: {
        	                    fn: me.onI_filterScoreAfterRender,
        	                    scope: me
	                        }
		            }
		        },
		        {
		            xtype: 'image',
		            height: 20,
		            hidden: true,
		            width: 20,
		            src: 'Htmls/images/close.gif',
		            x: 460,
		            y: 20,
		            listeners: {
		                render: {
		                    fn: me.onImageRender,
		                    scope: me
		                }
		            }
		        },
		        {
		            xtype: 'button',
		            id: 'i_closeCri'+criteriaNum,
			    icon:'Htmls/images/x.gif',
//		            text: 'x',
		            x: 550,
		            y: 20,
		            listeners: {
		                click: {
		                    fn: me.onButtonClick,
		                    scope: me
		                }
		            }
		        },
		        {
		            xtype: 'label',
		            text: 'Percent',
		            x: 390,
		            y: 20
		        }
		    ],
		    listeners: {
		        afterrender: {
		            fn: me.onI_criterionAfterRender,
		            scope: me
		        },
	                destroy: {
	                    fn: me.onI_criterionDestroy,
	                    scope: me
	                }
		    }
		});

		me.callParent(arguments);
	    },


	    onI_andOrChange: function(field, newValue, oldValue, options) {
		updateCriteriaOnHUB();
	    },

	    onI_andOrAfterRender: function(abstractcomponent, options) {
		updateCriteriaOnHUB();
	    },

	    onI_filterAvgTotChange: function(field, newValue, oldValue, options) {
		updateCriteriaOnHUB();
	    },

	    onI_filterAvgTotAfterRender: function(abstractcomponent, options) {
		updateCriteriaOnHUB();
	    },

	    onI_filterComparatorChange: function(field, newValue, oldValue, options) {
		updateCriteriaOnHUB();
	    },

	    onI_filterComparatorAfterRender: function(abstractcomponent, options) {
		updateCriteriaOnHUB();
	    },

	    onI_filterScoreChange: function(field, newValue, oldValue, options) {
		updateCriteriaOnHUB();
	    },

	    onI_filterScoreAfterRender: function(abstractcomponent, options) {
		updateCriteriaOnHUB();
	    },

	    onImageRender: function(abstractcomponent, options) {
		abstractcomponent.getEl().addListener(
		'click',
		function(){
		    removeSearchCriterion(abstractcomponent);
		}

		);
	    },

	    onButtonClick: function(button, e, options) {
		removeSearchCriterion(button);
	    },
	    onI_criterionAfterRender: function(abstractcomponent, options) {
	        updateCriteriaOnHUB();
	    },
	    onI_criterionDestroy: function(abstractcomponent, options) {
        	updateCriteriaOnHUB();
	    }

	});
	var newCriterion = Ext.create('MyApp.view.i_criterion'+criteriaNum);//create
	newCriterion.setPosition(0,155+55*(criteriaCurrent-1));//set position	
	updatePanelSize("add");
	Ext.getCmp('i_searchPanel').add(newCriterion);//add to the panel	
	criteriaNum++;
	criteriaCurrent++;

}

/*
 * Function: updatePanelSize
 * 
 * Invoked:
 * 	When the user clicks the + button next to the search score criterion we need to adjust the panel's height
 * 
 * Parameters:
 * 	dir - "add" | "sub" , to increase or decrease the panel's height
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	
 */
function updatePanelSize(dir,id){
	if (dir=="add") {
	
		Ext.getCmp('i_advanced').setPosition(10,Ext.getCmp('i_advanced').getPosition()[1]);//move adnavced down
		Ext.getCmp('i_buttons').setPosition( 10,Ext.getCmp('i_buttons').getPosition()[1]);//move buttons down
		Ext.getCmp('i_filterPanel').setHeight(Ext.getCmp('i_filterPanel').getHeight()+65);//increase i_insidePanel
		Ext.getCmp('i_suspects').setHeight(Ext.getCmp('i_suspects').getHeight()-65);//decrease i_insidePanel
		Ext.getCmp('investigatePanel').setHeight(Ext.getCmp('investigatePanel').getHeight()+65);//increase i_insidePanel
		
	}
	if (dir=="sub"){

		Ext.getCmp('i_advanced').setPosition(10,Ext.getCmp('i_advanced').getPosition()[1] - 130);//move adnavced down//48+63
		Ext.getCmp('i_buttons').setPosition(10,Ext.getCmp('i_buttons').getPosition()[1] - 130);//move buttons down
		Ext.getCmp('i_filterPanel').setHeight(Ext.getCmp('i_filterPanel').getHeight()-65);//increase i_insidePanel
		Ext.getCmp('i_suspects').setHeight(Ext.getCmp('i_suspects').getHeight()+65);//decrease i_insidePanel
		Ext.getCmp('investigatePanel').setHeight(Ext.getCmp('investigatePanel').getHeight()-65);//increase i_insidePanel
		
		//go over all objects of id between id+1 and criteriaCurrent and move them up 
		
		var criterion;
		var currX,currY;
		for (var i = id;i <= criteriaNum;i++){
			criterion = Ext.getCmp('i_criterion'+(i));
			if (criterion!=undefined){
				currX = criterion.getPosition()[0];
				currY = criterion.getPosition()[1];
				criterion.setPosition(0,currY-125);
			}
		}
	}
}
/*
 * Function: removeSearchCriterion
 * 
 * Invoked:
 * 	When the user clicks the - button next to the search score criterion.The function destroys the relevant score criterion field
 * 
 * Parameters:
 * 	button - the button object clicked so we'd know which field to destroy
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 	
 */
function removeSearchCriterion(button){
	var id = button.getId().substring('i_closeCri'.length);
	Ext.getCmp('i_criterion'+(id)).destroy();
	updatePanelSize("sub",id);
	criteriaCurrent--;
}

function validateSearchCriteria(){
	for (var i=0;i<=criteriaNum;i++){
		if (Ext.getCmp('i_criterion'+i)==undefined)//check if exists
			continue;
		for (var j=0;j<=criteriaNum;j++){
			if (Ext.getCmp('i_criterion'+j)==undefined)//check if exists
				continue;				
			if (Ext.getCmp('i_filterAvgTot'+j).getValue()==Ext.getCmp('i_filterAvgTot'+i).getValue() && Ext.getCmp('i_andOr'+j).getValue()=='And'){
				//Ext.create('MyApp.view.i_error').show();
				//Ext.getCmp('i_errorField').setText("");
				//setTimeout(function(){Ext.WindowManager.get('i_error').destroy();},3000);
			}
		}	
	}	

}

var useCache = true;
var resultsArray ;




function updateCriteriaOnHUB(){
	var src = Ext.getCmp("i_current").getSource();

	var newVal ="";
	//taking the first criterion and adding to string
	newVal+= Ext.getCmp('i_filterAvgTot0').getValue();
	newVal+= " "+Ext.getCmp('i_filterComparator0').getValue();
	newVal+= " "+Ext.getCmp('i_filterScore0').getValue()+"%";
	//taking the rest if they exist
	for (var i=1;i<criteriaNum+1;i++){//criteria is inc'ed only after this function runs
	    if (Ext.getCmp('i_criterion'+(i))!=undefined){
		try{
			newVal+= " "+Ext.getCmp('i_andOr'+(i)).getValue();		
			newVal+= " "+Ext.getCmp('i_filterAvgTot'+i).getValue();
			newVal+= " "+Ext.getCmp('i_filterComparator'+i).getValue();
			newVal+= " "+Ext.getCmp('i_filterScore'+i).getValue()+"%";}
		catch(err){
			//Ext.Msg.alert('Error #615', 'Error while loading score criteria.');
		}
	    }
	}
	src["Score Criteria"] = newVal;
	Ext.getCmp("i_current").setSource(src);
	
}

/*
 * Function: loadProfileList
 * 
 * Invoked:
 * 	Loads the saved profile list 
 * 
 * 
 * 
 * Parameters:
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 * See Also:
 * 
 * 
 */
function loadProfileList(){
	var store = Ext.getCmp('i_profiles').getStore();
	store.load()
}
