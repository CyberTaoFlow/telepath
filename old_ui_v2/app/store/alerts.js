/*
 * File: app/store/alerts.js
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

Ext.define('MyApp.store.alerts', {
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
            defaultSortDirection: 'DESC',
            autoLoad: false,
            sortOnLoad: false,
            storeId: 'alerts',
            groupDir: 'DESC',
            pageSize: 30,
            sortOnFilter: false,
            proxy: {
                type: 'ajax',
                extraParams: {
                    start: 0,
                    limit: 30,
                    // other params
                    sortorder: 'DESC',
                    sortfield: 'date',
                    val: 'None',
                    variable: 'None',
                    mode: 'get_alerts'
                },
                reader: {
                    type: 'json',
                    root: 'items'
                }
            },
            fields: [
                {
                    name: 'td0'
                },
                {
                    name: 'td1'
                },
                {
                    name: 'td2'
                },
                {
                    name: 'td3'
                },
                {
                    name: 'td4'
                },
                {
                    name: 'td5'
                },
                {
                    name: 'td6'
                },
                {
                    name: 'td7'
                },
                {
                    name: 'td8'
                },
                {
                    name: 'td9'
                },
                {
                    name: 'td10'
                },
                {
                    name: 'td11'
                },
                {
                    name: 'td12'
                },
                {
                    name: 'td13'
                }
            ]
        }, cfg)]);
    }
});