/*
 * File: app/view/main_panel.js
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

Ext.define('MyApp.view.main_panel', {
    extend: 'Ext.tab.Panel',

    requires: [
        'MyApp.view.Dashboard',
        'MyApp.view.Alerts',
        'MyApp.view.Investigate',
        'MyApp.view.Workflow',
        'MyApp.view.Behavior',
        'MyApp.view.Configuration',
        'Ext.panel.Panel',
        'Ext.tab.Tab'
    ],

    border: 0,
    id: 'main_panel',
    width: 1024,
    manageHeight: false,
    activeTab: 0,

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            items: [
                {
                    xtype: 'dashboard'
                },
                {
                    xtype: 'alerts'
                },
                {
                    xtype: 'investigate'
                },
                {
                    xtype: 'workflow'
                },
                {
                    xtype: 'behavior'
                },
                {
                    xtype: 'configuration'
                }
            ],
            listeners: {
                render: {
                    fn: me.onMain_panelRender,
                    scope: me
                },
                tabchange: {
                    fn: me.onMain_panelTabChange,
                    scope: me
                }
            }
        });

        me.callParent(arguments);
    },

    onMain_panelRender: function(component, eOpts) {
        //new Ext.LoadMask(component,{ msg:"Loading Data...",id:"main_panel_mask"});
    },

    onMain_panelTabChange: function(tabPanel, newCard, oldCard, eOpts) {
        // In Init.js
        main_panel_tab_change(tabPanel, newCard, oldCard, eOpts);
    }

});