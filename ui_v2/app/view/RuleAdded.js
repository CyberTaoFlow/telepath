/*
 * File: app/view/RuleAdded.js
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

Ext.define('MyApp.view.RuleAdded', {
    extend: 'Ext.window.Window',

    requires: [
        'Ext.form.Label',
        'Ext.Img'
    ],

    draggable: false,
    height: 109,
    id: 'ruleAdded',
    width: 340,
    resizable: false,
    layout: 'absolute',
    closable: false,
    title: '',

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            items: [
                {
                    xtype: 'label',
                    x: 100,
                    y: 30,
                    height: 40,
                    style: 'font-weight:bold; text-align:center;',
                    width: 190,
                    text: 'The criterion has been successfuly added'
                },
                {
                    xtype: 'image',
                    x: 20,
                    y: 20,
                    height: 50,
                    width: 50,
                    src: 'Htmls/images/success.gif'
                }
            ],
            listeners: {
                hide: {
                    fn: me.onRuleAddedHide,
                    scope: me
                }
            }
        });

        me.callParent(arguments);
    },

    onRuleAddedHide: function(component, eOpts) {
        component.destroy();
    }

});