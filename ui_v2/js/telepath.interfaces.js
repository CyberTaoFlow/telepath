telepath.interfaces = {
	setDropDown: function() {
	
		var el = Ext.getCmp('interfacesDropdown');
		var data = [];
		$.each(telepath.interfaces.data, function (i, val) {
			data.push([ val, val ]);
		});
		el.store.loadRawData(data);
		
	},
	addAgent: function() {
		
		var agentsStore = this.store;
		var count       = agentsStore.count();
		agentsStore.add({ agent_id: 'Agent #' + count, filterExpression: '', networkInterface: '' });
	
	},
	validate: function() {
		
		var agentsStore = this.store;
		var result = true;
		
		$.each(agentsStore.data.items, function(i, item) {
			
			if(!item.data.FilterExpression || item.data.FilterExpression == '') {
				Ext.MessageBox.alert('Error', 'Missing Filter Expression');
				result = false;
			}
			
			if(!item.data.NetworkInterface || item.data.NetworkInterface == '') {
				Ext.MessageBox.alert('Error', 'Missing Filter Expression');
				result = false;
			}
		
		});

		return result;
	
	},
	container: 'network-interfaces-innerCt',
	data: [],
	get: function () {
	
		var result = [];
		var agentsStore = this.store;
		$.each(agentsStore.data.items, function(i, item) {
			result.push(item.data);
		});
		
		return result;
		
	},
	init: function (data, agentsData) {
		
		this.data  = data;
		this.store = Ext.StoreManager.get('agents');
		this.store.loadRawData(agentsData);
	
	}

}