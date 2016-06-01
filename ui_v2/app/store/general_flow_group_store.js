/*
 * File: app/store/general_flow_group_store.js
 *
 * This file was generated by Sencha Architect version 3.0.4.
 * http://www.sencha.com/products/architect/
 *
 * This file requires use of the Ext JS 4.1.x library, under independent license.
 * License of Sencha Architect does not include license for Ext JS 4.1.x. For more
 * details see http://www.sencha.com/license or contact license@sencha.com.
 *
 * This file will be auto-generated each and everytime you save your project.
 *
 * Do NOT hand edit this file.
 */

Ext.define('MyApp.store.general_flow_group_store', {
    extend: 'Ext.data.Store',

    requires: [
        'Ext.data.proxy.Ajax',
        'Ext.data.reader.Json',
        'Ext.data.Field'
    ],

    constructor: function(cfg) {
        var me = this;
        cfg = cfg || {};
        me.callParent([Ext.apply({
            autoLoad: false,
            storeId: 'general_flow_group_store',
            proxy: {
                type: 'ajax',
                extraParams: {
                    mode: 'get_groups_combobox_general',
                    sort: '[{"property":"td1","direction":"ASC"}]'
                },
                url: '',
                reader: {
                    type: 'json',
                    root: 'items'
                }
            },
            fields: [
                {
                    name: 'group'
                },
                {
                    name: 'id'
                },
                {
                    name: 'app_id'
                },
                {
                    name: 'app_name'
                }
            ]
        }, cfg)]);
    }
});