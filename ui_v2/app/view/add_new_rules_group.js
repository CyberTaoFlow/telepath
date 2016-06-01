/*
 * File: app/view/add_new_rules_group.js
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

Ext.define('MyApp.view.add_new_rules_group', {
    extend: 'Ext.window.Window',

    requires: [
        'Ext.form.field.ComboBox',
        'Ext.button.Button'
    ],

    height: 191,
    id: 'add_new_rules_group',
    width: 400,
    layout: 'absolute',
    title: 'New Rule',
    modal: true,

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            items: [
                {
                    xtype: 'combobox',
                    x: 20,
                    y: 30,
                    id: 'add_new_rules_group_category',
                    width: 340,
                    fieldLabel: 'Category',
                    labelSeparator: ' ',
                    allowBlank: false,
                    emptyText: 'Please select a category',
                    enableKeyEvents: true,
                    editable: false,
                    displayField: 'category',
                    queryCaching: false,
                    store: 'general_rule_group_categories_store',
                    listeners: {
                        keyup: {
                            fn: me.onRW1_f13Keyup1,
                            scope: me
                        }
                    }
                },
                {
                    xtype: 'textfield',
                    x: 20,
                    y: 70,
                    id: 'add_new_rules_group_field',
                    width: 340,
                    fieldLabel: 'Rule Name',
                    labelSeparator: ' ',
                    allowBlank: false,
                    emptyText: 'Please enter a name',
                    enableKeyEvents: true,
                    listeners: {
                        keypress: {
                            fn: me.onAdd_new_rules_group_fieldKeypress,
                            scope: me
                        }
                    }
                },
                {
                    xtype: 'button',
                    x: 180,
                    y: 120,
                    id: 'add_new_rules_group_button',
                    text: 'Create',
                    listeners: {
                        click: {
                            fn: me.onAdd_new_rules_group_buttonClick,
                            scope: me
                        }
                    }
                }
            ],
            listeners: {
                destroy: {
                    fn: me.onAdd_new_rules_groupDestroy,
                    scope: me
                },
                show: {
                    fn: me.onAdd_new_rules_groupShow,
                    scope: me
                }
            }
        });

        me.callParent(arguments);
    },

    onRW1_f13Keyup1: function(textfield, e, eOpts) {
        var combo = textfield;
        var event = e;
        var key = String.fromCharCode(event.getKey()),
            boundList = combo.getPicker(),
            store = boundList.getStore(),
            record = store.findRecord(combo.displayField, key);

        if (record) {
            boundList.highlightItem(boundList.getNode(record));
        }
    },

    onAdd_new_rules_group_fieldKeypress: function(textfield, e, eOpts) {
        if (e.keyCode==13)
        Ext.getCmp('add_new_rules_group_button').fireEvent('click')
    },

    onAdd_new_rules_group_buttonClick: function(button, e, eOpts) {
        var value = Ext.getCmp('add_new_rules_group_field').getValue()
        if (Ext.getCmp('add_new_rules_group_field').isValid() & Ext.getCmp('add_new_rules_group_category').isValid() ){
            var category = Ext.getCmp('add_new_rules_group_category').valueModels[0].raw.id
            create_new_group(value,category);
        }
    },

    onAdd_new_rules_groupDestroy: function(component, eOpts) {
        Ext.getCmp('main_panel').enable()
    },

    onAdd_new_rules_groupShow: function(component, eOpts) {
        Ext.getCmp('main_panel').disable()
        var field = Ext.getCmp('add_new_rules_group_category');
        //field.getStore().load();
        var selected = Ext.getCmp('b_ruleGroups').getSelectionModel().getSelection();
        if (selected.length>0){
            var node = selected[0]
                if (node.data.type=='category'){
                    var record = field.getStore().findRecord('category',node.data.text,false,true,true)
                    if(record){
                        Ext.getCmp('add_new_rules_group_category').setValue(record)
                    }
                }
        }
    }

});