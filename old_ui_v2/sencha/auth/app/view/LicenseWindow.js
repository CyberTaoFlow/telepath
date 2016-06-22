/*
 * File: app/view/LicenseWindow.js
 *
 * This file was generated by Sencha Architect version 2.2.3.
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

Ext.define('MyApp.view.LicenseWindow', {
    extend: 'Ext.window.Window',

    height: 278,
    id: 'productKey',
    width: 392,
    layout: {
        type: 'absolute'
    },
    closable: false,
    titleCollapse: true,

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            items: [
                {
                    xtype: 'image',
                    x: 60,
                    y: 20,
                    height: 90,
                    width: 260,
                    src: 'images/logo.png'
                },
                {
                    xtype: 'label',
                    x: 10,
                    y: 130,
                    height: 20,
                    width: 360,
                    text: 'Please enter your Hybrid Telepath license key'
                },
                {
                    xtype: 'label',
                    x: 10,
                    y: 150,
                    text: 'Your key was sent to your email account upon site registration'
                },
                {
                    xtype: 'textfield',
                    x: 10,
                    y: 180,
                    id: 'LicenseKey_input',
                    width: 360,
                    fieldLabel: 'License Key',
                    allowBlank: false,
                    emptyText: 'XXXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX',
                    maxLength: 36,
                    maxLengthText: 'License key should be 36 characters long',
                    minLength: 36,
                    minLengthText: 'License key should be 36 characters long'
                },
                {
                    xtype: 'button',
                    x: 330,
                    y: 210,
                    width: 40,
                    text: 'Go',
                    listeners: {
                        click: {
                            fn: me.onButtonClick,
                            scope: me
                        }
                    }
                },
                {
                    xtype: 'label',
                    x: 10,
                    y: 210,
                    height: 30,
                    id: 'LicenceKey_label',
                    width: 310,
                    text: ''
                }
            ]
        });

        me.callParent(arguments);
    },

    onButtonClick: function(button, e, eOpts) {
        telepath.license.go();
    }

});