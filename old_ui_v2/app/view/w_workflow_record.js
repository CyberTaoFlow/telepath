/*
 * File: app/view/w_workflow_record.js
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

Ext.define('MyApp.view.w_workflow_record', {
    extend: 'Ext.window.Window',

    requires: [
        'Ext.form.Label',
        'Ext.form.field.Text',
        'Ext.button.Button',
        'Ext.ProgressBar',
        'Ext.form.field.Checkbox'
    ],

    height: 242,
    id: 'w_workflow_record',
    width: 503,
    layout: 'absolute',
    title: 'Business Action Recording',
    modal: true,

    initComponent: function() {
        var me = this;

        Ext.applyIf(me, {
            items: [
                {
                    xtype: 'label',
                    x: 20,
                    y: 110,
                    text: 'Note: In order to display all pages, please clean the browser\'s cache.'
                },
                {
                    xtype: 'label',
                    x: 20,
                    y: 60,
                    text: '2. Click the link'
                },
                {
                    xtype: 'textfield',
                    x: 20,
                    y: 80,
                    border: false,
                    id: 'w_record_url',
                    width: 450,
                    fieldStyle: '',
                    readOnly: true
                },
                {
                    xtype: 'label',
                    x: 20,
                    y: 160,
                    text: '3. Once the page above has finished loading, click \'Start\''
                },
                {
                    xtype: 'button',
                    x: 20,
                    y: 180,
                    id: 'w_record_start',
                    text: 'Start'
                },
                {
                    xtype: 'progressbar',
                    x: 20,
                    y: 210,
                    id: 'w_detection_progress_bar',
                    width: 450,
                    value: 0.4
                },
                {
                    xtype: 'label',
                    x: 20,
                    y: 10,
                    text: '1. Name your action'
                },
                {
                    xtype: 'checkboxfield',
                    x: 20,
                    y: 130,
                    id: 'w_ip_recording_mode',
                    boxLabel: 'Track session using unique IP'
                },
                {
                    xtype: 'textfield',
                    x: 20,
                    y: 30,
                    id: 'w_record_name',
                    width: 150,
                    allowBlank: false,
                    allowOnlyWhitespace: false
                }
            ],
            listeners: {
                show: {
                    fn: me.onW_workflow_recordShow,
                    scope: me
                },
                destroy: {
                    fn: me.onW_workflow_recordDestroy,
                    scope: me
                }
            }
        });

        me.callParent(arguments);
    },

    onW_workflow_recordShow: function(component, eOpts) {
        Ext.getCmp('main_panel').disable()
        render_record_url()
    },

    onW_workflow_recordDestroy: function(component, eOpts) {
        Ext.getCmp('main_panel').enable();
        telepath.workflow.stopTracking();
    }

});