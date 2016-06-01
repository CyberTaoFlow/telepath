/*
 * File: app/view/add_new_rules_category.js
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

Ext.define('MyApp.view.add_new_rules_category', {
    extend: 'Ext.window.Window',

    requires: [
        'Ext.form.field.Text',
        'Ext.button.Button'
    ],

    height: 151,
    id: 'add_new_rules_category',
    width: 400,
    layout: 'absolute',
    title: 'New Category',
    modal: true,

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            items: [
                {
                    xtype: 'textfield',
                    x: 20,
                    y: 30,
                    id: 'add_new_rules_category_field',
                    width: 340,
                    fieldLabel: 'Category Name',
                    labelSeparator: ' ',
                    allowBlank: false,
                    enableKeyEvents: true,
                    listeners: {
                        keypress: {
                            fn: me.onAdd_new_rules_criterion_fieldKeypress,
                            scope: me
                        }
                    }
                },
                {
                    xtype: 'button',
                    x: 180,
                    y: 70,
                    id: 'add_new_rules_category_button',
                    text: 'Create',
                    listeners: {
                        click: {
                            fn: me.onAdd_new_rules_criterion_buttonClick,
                            scope: me
                        }
                    }
                }
            ],
            listeners: {
                destroy: {
                    fn: me.onAdd_new_rules_criterionDestroy,
                    scope: me
                },
                show: {
                    fn: me.onAdd_new_rules_criterionShow,
                    scope: me
                }
            }
        });

        me.callParent(arguments);
    },

    onAdd_new_rules_criterion_fieldKeypress: function(textfield, e, eOpts) {
        if (e.keyCode==13)
            Ext.getCmp('add_new_rules_category_button').fireEvent('click')
    },

    onAdd_new_rules_criterion_buttonClick: function(button, e, eOpts) {
        var value = Ext.getCmp('add_new_rules_category_field').getValue()
        if (Ext.getCmp('add_new_rules_category_field').isValid()){
            create_new_category(value)
            Ext.WindowManager.get('add_new_rules_category').destroy()
        }
    },

    onAdd_new_rules_criterionDestroy: function(component, eOpts) {
        Ext.getCmp('main_panel').enable()
    },

    onAdd_new_rules_criterionShow: function(component, eOpts) {
        Ext.getCmp('main_panel').disable()
    }

});