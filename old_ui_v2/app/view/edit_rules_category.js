/*
 * File: app/view/edit_rules_category.js
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

Ext.define('MyApp.view.edit_rules_category', {
    extend: 'Ext.window.Window',

    requires: [
        'Ext.form.field.Text',
        'Ext.button.Button',
        'Ext.form.field.Hidden'
    ],

    height: 151,
    id: 'edit_rules_category',
    width: 400,
    layout: 'absolute',
    title: 'Edit Category',
    modal: true,

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            items: [
                {
                    xtype: 'textfield',
                    x: 20,
                    y: 30,
                    id: 'edit_rules_category_field',
                    width: 340,
                    fieldLabel: 'Category Name',
                    labelSeparator: ' ',
                    allowBlank: false,
                    enableKeyEvents: true,
                    listeners: {
                        keypress: {
                            fn: me.onEdit_rules_criterion_fieldKeypress,
                            scope: me
                        }
                    }
                },
                {
                    xtype: 'button',
                    x: 180,
                    y: 70,
                    id: 'edit_rules_category_button',
                    text: 'Update',
                    listeners: {
                        click: {
                            fn: me.onEdit_rules_criterion_buttonClick,
                            scope: me
                        }
                    }
                },
                {
                    xtype: 'hiddenfield',
                    id: 'edit_rules_category_id',
                    fieldLabel: 'Label'
                }
            ],
            listeners: {
                destroy: {
                    fn: me.onEdit_rules_criterionDestroy,
                    scope: me
                },
                show: {
                    fn: me.onEdit_rules_criterionShow,
                    scope: me
                }
            }
        });

        me.callParent(arguments);
    },

    onEdit_rules_criterion_fieldKeypress: function(textfield, e, eOpts) {
        if (e.keyCode==13)
        Ext.getCmp('edit_rules_category_button').fireEvent('click')
    },

    onEdit_rules_criterion_buttonClick: function(button, e, eOpts) {
        var value = Ext.getCmp('edit_rules_category_field').getValue()
        var id = Ext.getCmp('edit_rules_category_id').getValue()
        if (Ext.getCmp('edit_rules_category_field').isValid()){
            edit_category(value,id)
            Ext.WindowManager.get('edit_rules_category').destroy()
        }
    },

    onEdit_rules_criterionDestroy: function(component, eOpts) {
        Ext.getCmp('main_panel').enable()
    },

    onEdit_rules_criterionShow: function(component, eOpts) {
        Ext.getCmp('main_panel').disable()
        var record = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection()[0]
        Ext.getCmp('edit_rules_category_id').setValue(record.get('category_id'))
        Ext.getCmp('edit_rules_category_field').setValue(record.get('text'))
    }

});