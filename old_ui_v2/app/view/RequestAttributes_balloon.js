/*
 * File: app/view/RequestAttributes_balloon.js
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

Ext.define('MyApp.view.RequestAttributes_balloon', {
    extend: 'Ext.container.Container',

    requires: [
        'Ext.form.Label'
    ],

    floating: true,
    height: 21,
    id: 'RequestAttributes_balloon',
    width: 600,
    layout: 'absolute',

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            style: {
                'background-color': '#99BBE8'
            },
            items: [
                {
                    xtype: 'label',
                    height: 21,
                    id: 'RequestAttributes_balloon_label',
                    style: {
                        color: 'white'
                    },
                    width: 340,
                    text: 'My Label'
                }
            ],
            listeners: {
                render: {
                    fn: me.onRequestAttributes_balloonRender,
                    scope: me
                }
            }
        });

        me.callParent(arguments);
    },

    onRequestAttributes_balloonRender: function(component, eOpts) {
        component.getEl().addListener('mouseenter',function(){
            i_hovering_request_attributes_balloon = true;
        });

        component.getEl().addListener('mouseleave',function(){
            i_hovering_request_attributes_balloon = false;
            component.hide();
        });
    }

});