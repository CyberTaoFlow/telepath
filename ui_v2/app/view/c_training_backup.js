/*
 * File: app/view/c_training_backup.js
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

Ext.define('MyApp.view.c_training_backup', {
    extend: 'Ext.window.Window',

    requires: [
        'Ext.form.Label',
        'Ext.form.field.Radio',
        'Ext.button.Button'
    ],

    height: 216,
    id: 'c_training_backup',
    width: 343,
    layout: 'absolute',
    title: '',

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            items: [
                {
                    xtype: 'label',
                    x: 20,
                    y: 30,
                    text: 'Please select one of the following:'
                },
                {
                    xtype: 'radiofield',
                    x: 40,
                    y: 60,
                    id: 'c_training_backup_backup',
                    fieldLabel: '',
                    boxLabel: 'Backup database (May take up to 30 mins)',
                    checked: true,
                    listeners: {
                        change: {
                            fn: me.onC_training_backup_backupChange,
                            scope: me
                        }
                    }
                },
                {
                    xtype: 'radiofield',
                    x: 40,
                    y: 90,
                    id: 'c_training_backup_delete',
                    fieldLabel: '',
                    boxLabel: 'Delete database',
                    listeners: {
                        change: {
                            fn: me.onC_training_backup_deleteChange,
                            scope: me
                        }
                    }
                },
                {
                    xtype: 'button',
                    x: 20,
                    y: 130,
                    text: 'Select',
                    listeners: {
                        click: {
                            fn: me.onButtonClick1,
                            scope: me
                        }
                    }
                },
                {
                    xtype: 'button',
                    x: 80,
                    y: 130,
                    text: 'Cancel',
                    listeners: {
                        click: {
                            fn: me.onButtonClick,
                            scope: me
                        }
                    }
                }
            ],
            listeners: {
                show: {
                    fn: me.onC_training_backupShow,
                    scope: me
                },
                destroy: {
                    fn: me.onC_training_backupDestroy,
                    scope: me
                }
            }
        });

        me.callParent(arguments);
    },

    onC_training_backup_backupChange: function(field, newValue, oldValue, eOpts) {
        if(newValue){
            Ext.getCmp('c_training_backup_delete').setValue(false)
        }
    },

    onC_training_backup_deleteChange: function(field, newValue, oldValue, eOpts) {
        if(newValue){
        	Ext.getCmp('c_training_backup_backup').setValue(false)
        }
    },

    onButtonClick1: function(button, e, eOpts) {
        switch_to_training();
    },

    onButtonClick: function(button, e, eOpts) {
        Ext.getCmp('c_training_backup').destroy();
        Ext.getCmp("c_Training").setValue(false);
    },

    onC_training_backupShow: function(component, eOpts) {
        Ext.getCmp('main_panel').disable();
    },

    onC_training_backupDestroy: function(component, eOpts) {
        Ext.getCmp('main_panel').enable();
    }

});