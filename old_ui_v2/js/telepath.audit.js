Ext.require([
    'Ext.grid.*',
    'Ext.data.*',
    'Ext.util.*',
    'Ext.grid.plugin.BufferedRenderer'
]);


telepath.audit = {
	
	show: function () {
		
		Ext.define('MyApp.view.AuditWindow', {
		
				extend: 'Ext.window.Window',
				modal: false,
				height: 445,
				width: 1000,
				layout: {
					type: 'fit'
				},
				title: 'Activity Log',
				initComponent: function () {
					var me = this;
					me.callParent(arguments);
				}
			
			});
		
		this.window = Ext.create('MyApp.view.AuditWindow');
		
		
		this.store = Ext.create('Ext.data.Store', {
			id: 'auditStore',
			fields: ['timestamp', 'userdata', 'class', 'function', 'request'],
			pageSize: 100,
			model: 'ForumThread',
			remoteSort: true,
			// allow the grid to interact with the paging scroller by buffering
			buffered: true,
			leadingBufferZone: 100,
			proxy: {
				// load using script tags for cross domain, if the data in on the same domain as
				// this page, an HttpProxy would be better
				type: 'ajax',
				url: telepath.controllerPath + '/telepath/audit',
				reader: {
					type: 'json',
					root: 'items',
					totalProperty: 'total'
				},
				// sends single sort as multi parameter
				simpleSortMode: true
			},
			// sends single sort as multi parameter
            simpleSortMode: true,
            // sends single group as multi parameter
            simpleGroupMode: true,

            // This particular service cannot sort on more than one field, so grouping === sorting.
            groupParam: 'sort',
            groupDirectionParam: 'dir',
			sorters: [{
				property: 'timestamp',
				direction: 'DESC'
			}],
			autoLoad: true,
			listeners: {
			}
		});
		
		this.grid = Ext.create('Ext.grid.Panel', {
			store: telepath.audit.store,
			//loadMask: true,
			disableSelection: true,
			verticalScrollerType: 'paginggridscroller',
			invalidateScrollerOnRefresh: false,
			viewConfig: {
				trackOver: false
			},
			columns:[
			{
				xtype: 'rownumberer',
				width: 50,
				sortable: false
			},
			{
				text: "Time",
				dataIndex: 'timestamp',
				width: 130,
				renderer: function(value) { return date_format('d/m/y H:i:s', value); },
				sortable: true
			},
			{
				text: "Username",
				dataIndex: 'userdata',
				width: 100,
				renderer: function(value) { return value; },
				sortable: false
			},
			{
				text: "Class",
				dataIndex: 'class',
				width: 100,
				sortable: true
			},
			{
				text: "Function",
				dataIndex: 'function',
				width: 70,
				sortable: true
			},
			{
				text: "Request",
				dataIndex: 'request',
				renderer: function(value) { 
					
					var html = '';
					$.each(value, function(key, val) {
						html += '<b>' + key + '</b> = ' + val + '&nbsp;';
					});
					return html;
				
				},
				flex: 1,
				sortable: false
			}
			]
		});
		
		this.window.show();
		this.window.add(this.grid);
		
		//this.store.guaranteeRange(0, 199);
		/*telepath.util.request('/telepath/audit', {}, function(data) {
			telepath.audit.store.loadData(data.items);
		}, 'Error loading audit data');*/
		
	}

}