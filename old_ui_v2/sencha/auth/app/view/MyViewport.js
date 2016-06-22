/*
 * File: app/view/MyViewport.js
 *
 * This file was generated by Sencha Architect version 2.2.2.
 * http://www.sencha.com/products/architect/
 *
 * This file requires use of the Ext JS 4.2.x library, under independent license.
 * License of Sencha Architect does not include license for Ext JS 4.2.x. For more
 * details see http://www.sencha.com/license or contact license@sencha.com.
 *
 * This file will be auto-generated each and everytime you save your project.
 *
 * Do NOT hand edit this file.
 */

Ext.define('MyApp.view.MyViewport', {
    extend: 'Ext.container.Viewport',

    padding: '',
    layout: {
        type: 'absolute'
    },

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            listeners: {
                render: {
                    fn: me.onViewportRender,
                    scope: me
                }
            },
            items: [
                {
                    xtype: 'container',
                    x: 30,
                    y: 40,
                    height: 184,
                    html: '<img src=\'/img/logo.jpg\'>',
                    width: 174
                }
            ]
        });

        me.callParent(arguments);
    },

    onViewportRender: function(component, eOpts) {
        login = new MyApp.view.Login();
        login.resizable = false;
        login.show();
    }

});