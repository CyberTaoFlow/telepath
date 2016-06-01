/*
 * File: app/view/copy_to_clipboard.js
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

Ext.define('MyApp.view.copy_to_clipboard', {
    extend: 'Ext.window.Window',

    requires: [
        'Ext.form.field.TextArea'
    ],

    height: 171,
    id: 'copy_to_clipboard',
    width: 491,
    layout: 'absolute',
    closeAction: 'hide',
    title: 'Copy to Clipboard',

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            items: [
                {
                    xtype: 'textareafield',
                    x: 10,
                    y: 10,
                    height: 120,
                    id: 'copy_to_clipboard_field',
                    width: 460,
                    fieldLabel: '',
                    selectOnFocus: true,
                    listeners: {
                        change: {
                            fn: me.onCopy_to_clipboard_fieldChange,
                            scope: me
                        }
                    }
                }
            ],
            listeners: {
                show: {
                    fn: me.onCopy_to_clipboardShow,
                    scope: me
                },
                hide: {
                    fn: me.onCopy_to_clipboardHide,
                    scope: me
                }
            }
        });

        me.callParent(arguments);
    },

    onCopy_to_clipboard_fieldChange: function(field, newValue, oldValue, eOpts) {
        Ext.getCmp('copy_to_clipboard_field').focus(true);
    },

    onCopy_to_clipboardShow: function(component, eOpts) {
        Ext.getCmp('copy_to_clipboard_field').focus(true);
        Ext.getCmp('main_panel').disable();
    },

    onCopy_to_clipboardHide: function(component, eOpts) {
        var ra_window = Ext.getCmp('RequestAttributes');
        if (ra_window && ra_window.isVisible()){
            Ext.getCmp('main_panel').disable();
        }
        else{
            Ext.getCmp('main_panel').enable();
        }
    }

});