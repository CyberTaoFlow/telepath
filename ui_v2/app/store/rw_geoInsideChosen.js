/*
 * File: app/store/rw_geoInsideChosen.js
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

Ext.define('MyApp.store.rw_geoInsideChosen', {
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
            storeId: 'rw_geoInsideChosen',
            proxy: {
                type: 'ajax',
                reader: {
                    type: 'json',
                    root: 'countries'
                }
            },
            fields: [
                {
                    name: 'Country'
                },
                {
                    name: 'Alias'
                }
            ],
            listeners: {
                add: {
                    fn: me.onJsonstoreAdd,
                    scope: me
                },
                remove: {
                    fn: me.onJsonstoreRemove,
                    scope: me
                }
            }
        }, cfg)]);
    },

    onJsonstoreAdd: function(store, records, index, eOpts) {
        Ext.getCmp('RWgeo_submit').enable();
    },

    onJsonstoreRemove: function(store, record, index, eOpts) {
        if (store.getCount()==0){
            Ext.getCmp('RWgeo_submit').disable();
        }
    }

});