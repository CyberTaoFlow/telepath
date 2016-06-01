telepath.config.accounts = {
	selectedGroup: 'All',
	editGroup: function (group_id) {
		
		var that = this;
		
		telepath.ds.get('/groups/get_group', { id: group_id }, function (data) {

			data = data.items;
			that.showGroup(data);

		});
		
	},
	showGroup: function(data) {
		
		var that = this;
		
		that.contentRight.empty();
		that.data = data;
		
		that.groupLeftWrap = $('<div>').addClass('tele-group-wrap-left');
		that.groupRightWrap = $('<div>').addClass('tele-group-wrap-right');
		
		that.contentRight.append(that.groupLeftWrap);
		that.contentRight.append(that.groupRightWrap);
		
		that.groupInputs = $('<div>').addClass('group-input')
		that.groupName = $('<div>').teleInput({ label: 'Group name', value: data.group.name });
		that.groupDesc = $('<div>').teleInput({ label: 'Group description', value: data.group.description });
		
		that.groupInputs.append(that.groupName);
		that.groupInputs.append(that.groupDesc);
		
		that.groupLeftWrap.append(that.groupInputs);
		
		var btnContain = $('<div>').addClass('tele-button-container-group');
		var saveBtn   = $('<a href="#" class="tele-button tele-button-apply">Save</a>');
		var cancelBtn  = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');
		
		cancelBtn.click(function () {
			that.getUsers();
		});

		btnContain.append(saveBtn).append(cancelBtn);
		
		// Apps limit
		var apps_data = [];
		$.each(that.data.apps, function(i, app) {
			apps_data.push({ text: app });
		});
		
		that.limitApps = $('<div>').teleSelect({ type: 'subdomain', values: apps_data, click: function () { } }).hide();
		that.limitRanges = $('<div>').addClass('tele-limit-ranges-group').hide();
	
		that.toggleType = $('<div>').teleRadios({
			title: 'Configure applications access limits',
			radios: [ 
			{ key: 'none',   label: 'None', 			  checked: that.data.apps.length == 0 && that.data.ranges.length == 0 }, 
			{ key: 'ranges', label: 'IP Ranges', 		  checked: that.data.apps.length == 0 && that.data.ranges.length > 0 },
			{ key: 'hosts',  label: 'Specific hostnames', checked: that.data.apps.length > 0 && that.data.ranges.length == 0 },
			], callback: function(radio) {
			
			that.limitApps.hide();
			that.limitRanges.hide();
			
			switch(radio.key) {
				case 'none':
				break;
				case 'ranges':
					that.limitRanges.show();
				break;
				case 'hosts':
					that.limitApps.show();
				break;
			}
			
			// console.log();
			
		}}).addClass('tele-limit-type-group');
		
		that.groupLeftWrap.append(that.toggleType);
		
		that.groupLeftWrap.append(that.limitApps);
		
		function getRangeUI(data) {
			
			if(!data) { data = ''; } else { data = data.trim() }
			
			var is_range = data.split('-').length > 1;
								
			var ipWrap   = $('<div>').addClass('tele-ip-group-wrap');
			var ipStart  = $('<div>').addClass('tele-ip').ip({ data: data.split('-')[0] });
			var ipDash   = $('<div>').addClass('tele-ip-dash').html('_');
			
			var ipEnd    = $('<div>').addClass('tele-ip').ip({ data: is_range ? data.split('-')[1] : '' });
			
			if(!is_range) {
				ipDash.hide();
				ipEnd.hide();
			}
			
			var ipAdd = $('<div>').addClass('tele-ip-add')
									 .addClass('tele-icon')
									 .addClass('tele-icon-plus')
									 .hover(function () { $(this).addClass('hover'); }, 
											function () { $(this).removeClass('hover'); })
									 .click(function () { that.limitRanges.append(getRangeUI()); });
			
			var ipRemove = $('<div>').addClass('tele-ip-remove')
									 .addClass('tele-icon')
									 .addClass('tele-icon-minus')
									 .hover(function () { $(this).addClass('hover'); }, 
											function () { $(this).removeClass('hover'); })
									 .click(function () { $(this).parent().remove(); });
									 
			var ipToggle = $('<div>').toggleFlip({ 
			
				left_value: 'Single', 
				right_value: 'Range',
				flip: function () {
					ipEnd.toggle();
					ipDash.toggle();
				},
				flipped: is_range
				
			});
			
			ipWrap.append(ipAdd).append(ipRemove).append(ipToggle).append(ipStart).append(ipDash).append(ipEnd);
			return ipWrap;
			
		}
				
		that.permissions = $('<div>').addClass('tele-group-permissions').append('<div class="tele-title-1">Permissions</div>');

		// Create tabs UI
		that.perm_tabs   = $('<div>').addClass('tele-perm-tabs');
		that.tabsEl = $('<div>').addClass('tabs');
		that.tabsUl = $('<ul>');
		that.perm_tabs.append(that.tabsUl).append(that.tabsEl);
		
		// Define tab headers
		var tabs = [
			{ id: 'get', text: 'Retrieve' },
			{ id: 'set', text: 'Modify' }
		];
		
		// Populate headers and create content divs
		for(x in tabs) {
			var tab = tabs[x];
			var tabEl = $('<div>').attr('id', 'tele-perm-' + tab.id);
			var tabLi = $('<li>');
			var tabA  = $('<a>').attr('href', '#tele-perm-' + tab.id).text(tab.text);
			tabLi.append(tabA);
			that.tabsUl.append(tabLi);
			that.tabsEl.append(tabEl);
			
		}
		
		telepath.ds.get('/permissions/get_list', { }, function (data) {
			
			$.each(data.items, function (i, row) {
				
				var checked = false;
				$.each(that.data.perm, function (i, perm) {
					if(perm.id == row.id) {
						checked = true;
						return false;
					}
				});
								
				var cb = $('<div>').teleCheckbox({
				checked: checked,
				label: row.description,
				dataID: row.id,
				callback: function (el, v) {}
				});
				
				$('#tele-perm-' + row['function']).append(cb);
				
				//that.permissions.append(cb);
			
			});
			
			that.perm_tabs.tabs({ 
				activate: function( event, ui ) {	
				}
				
			});
			
		});

		that.permissions.append(that.perm_tabs);
		that.groupRightWrap.append(that.permissions);

		// Load provided by settings
		
		$.each(that.data.ranges, function (i, ip) {
			that.limitRanges.append(getRangeUI(ip));
		});

		// Another blank
		that.limitRanges.append(getRangeUI(''));
		
		that.groupLeftWrap.append(that.limitRanges);
		
		that.contentRight.append(btnContain);
		
		


		// telepath.ds.get('/permissions/get_list', { }, function (data) {
						
		// 	$.each(data.items, function (i, row) {
				
		// 		var checked = false;
		// 		$.each(that.data.perm, function (i, perm) {
		// 			if(perm.id == row.id) {
		// 				checked = true;
		// 				return false;
		// 			}
		// 		});
				
		// 		var cb = $('<div>').teleCheckbox({
		// 		checked: checked,
		// 		label: row.description,
		// 		dataID: row.id,
		// 		callback: function (el, v) {}
		// 		});
				
		// 		that.permissions.append(cb);
			
		// 	});
		// });
		
		// that.contentRight.append(that.permissions);
		
		// Save function 
		
		saveBtn.click(function () {
			
			var groupData = { group: {}, perm: [], apps: [], ranges: [] };
			
			groupData.group.id   = data.group.id;
			groupData.group.name = $('input', that.groupName).val();
			groupData.group.description = $('input', that.groupDesc).val();
			groupData.mode = $('.tele-radio-knob', that.toggleType).parent().attr('rel');
			
			$('.tele-group-permissions .tele-checkbox-checkbox.checked').each(function () { 
				groupData.perm.push($(this).attr('dataid')); 
			});
			
			switch(groupData.mode) {
			
				case 'hosts':
					
					result = '';
					$('input', that.limitApps).each(function () {
						groupData.apps.push($(this).val().trim());
					});
				
				break;
				case 'ranges':
				
					$('.tele-ip-wrap', that.limitRanges).each(function () {
				
						var is_range = $('.tele-mini-toggle', this).data('tele-toggleFlip').options.flipped;
						var ip_start = $('.tele-ip:first', this).data('tele-ip').getIP();
						var ip_end   = $('.tele-ip:last', this).data('tele-ip').getIP();
						
						if(is_range) {
							if(ip_start && ip_end && ip2long(ip_start) < ip2long(ip_end)) {
								groupData.ranges.push(ip_start + '-' + ip_end);
							}
						} else {
							if(ip_start) {
								groupData.ranges.push(ip_start);
							}
						}

					});
				
				break;
				
			}
			
			if(groupData.group.id == 'new') {
				
				telepath.ds.get('/groups/add_group', { items: groupData }, function (data) {
					that.loadData();
				});

			} else {
				
				telepath.ds.get('/groups/set_group', { items: groupData }, function (data) {
					that.loadData();
				});
			}
		});
	},
	deleteGroup: function (id) {
		var that = this;
		telepath.ds.get('/groups/del_group', { id: id }, function (data) {
			that.loadData();
		});
	},
	init: function () {
		
		var that = this;
		
		var leftTitle = $('<div>').addClass('tele-panel-subtitle-text').html('Groups');
		var rightTitle = $('<div>').addClass('tele-panel-subtitle-text').html('Users');
		
		this.cmdNewGroup    = $('<div>').btn({ icon: 'plus', text: 'New Group' });
		this.cmdImportGroup = $('<div>').btn({ icon: 'import', text: 'Import Group' });
		this.cmdNewUser     = $('<div>').btn({ icon: 'plus', text: 'New User' });
		
		this.barLeft.append(leftTitle).append(this.cmdNewGroup); // .append(this.cmdImportGroup);
		
		this.cmdDeleteUsers = $('<div>').btn({ icon: 'delete', text: 'Delete', callback: function () {
			var selected = that.list.data('tele-teleList').getSelected();
			var data = telepath.config.accounts.data;
			var to_delete = [];
			$.each(data, function(d_index, d_val) {
				$.each(selected, function(s_index, s_val) {
					var user_id = d_val['id'];
					if(s_val == user_id){
						if(d_val['login'] != 'admin'){
							to_delete.push(user_id);
						}else{
							// console.log('Cannot delete admin');
						};
					}
				});
			});
			telepath.dialog({type:'dialog', 
				msg:'This operation will delete the selected user(s). Are you sure?', 
				callback:function(){
								telepath.ds.get('/users/del_user', { id: to_delete }, function (data) {
									that.loadData();
								})}});
		}});
	
		this.barRight.append(this.cmdDeleteUsers).append(rightTitle).append(this.cmdNewUser);
		
		this.contentLeft.append(telepath.loader);
		this.contentRight.append(telepath.loader);
		
		this.loadData();
		
		this.cmdNewGroup.click(function () {
			that.showGroup({ group: { id: 'new', name: '', description: '', mode: 'none' }, perm: [], apps: [], ranges: [] });
		});
		
		this.cmdNewUser.click(function () {
			telepath.config.account.init('new', that.contentRight);
			telepath.config.account.showData({ user: { id: 'new', active: true }, perm: [], apps: [], ranges: [], groups: [] });
		});
		
	},
	loadData: function() {
		
		this.getGroups();
		this.getUsers();
	
	},
	getGroups: function() {	
	
		var that = this;
		
		telepath.ds.get('/groups/get_list', { }, function (data) {
			
			that.groups = data.items;
			
			var treeData = [];
			
			treeData.push({ text: 'All', data: {id: 'All', description: '', name: 'All' }, id: 'tele_group_all' });
			
			$.each(data.items, function(i, row) {
				treeData.push({ text: row.name, data: {id: row.id, description: row.description, name: row.name }});
			});
						
			that.groupsTree = $('<div>');
		
			that.groupsTree.jstree({ 'core' : {
				'data' : treeData
			}, 
			plugins: ["grid", "wholerow", "theme"],
			grid: {
				columns: [
					{width: 390 },
					{value: function (node) {
						
						if(node.name !== 'All') {
						
						return $('<div>').btn({ icon: 'edit', callback: function () {
							
							if(node.name == 'admin') {
								telepath.dialog({ msg: 'Admin is a built in group which cannot be modified.' });
								return;
							}
						
							telepath.config.accounts.editGroup(node.id);
						}});
						
						} else {
							return '';
						}
						
					}, width: 40 },
					{value: function (node) {
						
						if(node.name !== 'All') {
						
						return $('<div>').btn({ icon: 'delete', callback: function () {
							
							if(node.name == 'admin') {
								telepath.dialog({ msg: 'Admin is a built in group which cannot be removed.' });
								return;
							}else{
								telepath.dialog({type: 'dialog', msg: 'This operation will delete the selected group. Are you sure?', callback:function(){
									telepath.config.accounts.deleteGroup(node.id);
								}});
							};
						}});
						
						} else {
							return '';
						}
						
					}, width: 40 }
				],
				resizable:true
			},
			}).on('changed.jstree', function (e, data) {
				data.instance.element.find('.jstree-wholerow').css('background-color', '#FFFFFF');
				data.instance.element.find('.jstree-wholerow-hovered').css("background-color", "rgba(189, 189, 189, 0.85)");
				// console.log('Group Changed - ' + data.node.text );
				telepath.config.accounts.selectedGroup = data.node.data.id;
				telepath.config.accounts.getUsers();
			});
			
			that.contentLeft.empty().append(that.groupsTree);
			that.groupsTree.jstree("select_node", "#tele_group_all");
			
		}, 'Error loading group list.');
	
	},
	getUsers: function() {	
	
		var that = this;
		
		telepath.ds.get('/users/get_list', { group: this.selectedGroup }, function (data) {
		
		that.data = data.items;
		
		// Create List
		that.list = $('<div>');
		that.contentRight.empty().append(that.list);
		
		// Init List
		that.list.teleList({ 
		data: that.data,
		formatter: function(item) {
			
			var title = (item.first_name ? (item.first_name + ' ') : '');
			title    += (item.last_name  ? (item.last_name + ' ') : '');
			title    += (item.login ? ('(' + item.login + ')') : '');
			
			var result = {
				dataID: item.id,
				checkable: true,
				time: item.last_login,
				timeLabel: 'Last Login:',
				icon: 'user',
				title: title,
				count: item.count,
				details: [ 
					{ key: 'Active', value: item.active },
					{ key: 'Email', value: item.email }
				]
			};

			return result;
			
		}, callbacks: {
			click: function (el, id) {
				telepath.config.account.init(el.options.dataID, that.contentRight);
			},
			hover_in: function (el, id) {
				
			},
			hover_out: function (el, id) {

			},
		}});

		}, 'Error loading users list.');	
	
	}
}
